#include "WeatherClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <cstring>

namespace
{

constexpr char OPEN_METEO_ENDPOINT[] =
    "https://api.open-meteo.com/v1/forecast"
    "?latitude=43.1242&longitude=5.9280"
    "&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m"
    "&hourly=temperature_2m,weather_code,precipitation_probability"
    "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,sunrise,sunset"
    "&timezone=Europe%2FParis&forecast_days=3";

void copyClock(const char* isoTime, char output[6])
{
    if (isoTime == nullptr || strlen(isoTime) < 16)
    {
        strcpy(output, "--:--");
        return;
    }
    memcpy(output, isoTime + 11, 5);
    output[5] = '\0';
}

int16_t dominantCode(const int16_t counts[100])
{
    int16_t selectedCode = 0;
    int16_t selectedCount = 0;
    for (int16_t code = 0; code < 100; ++code)
    {
        if (counts[code] > selectedCount)
        {
            selectedCode = code;
            selectedCount = counts[code];
        }
    }
    return selectedCode;
}

}  // namespace

bool WeatherClient::fetch(WeatherData& data)
{
    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(4000);
    http.setTimeout(5000);
    http.useHTTP10(true);

    if (!http.begin(secureClient, OPEN_METEO_ENDPOINT))
    {
        setError("Initialisation HTTP impossible");
        return false;
    }

    http.addHeader("User-Agent", "CheapYellowDisplay-Lab/1.0");
    const int statusCode = http.GET();
    if (statusCode != HTTP_CODE_OK)
    {
        char message[48];
        snprintf(message, sizeof(message), "Open-Meteo HTTP %d", statusCode);
        setError(message);
        http.end();
        return false;
    }

    JsonDocument filter;
    filter["current"]["temperature_2m"] = true;
    filter["current"]["relative_humidity_2m"] = true;
    filter["current"]["apparent_temperature"] = true;
    filter["current"]["weather_code"] = true;
    filter["current"]["wind_speed_10m"] = true;
    filter["hourly"]["time"] = true;
    filter["hourly"]["temperature_2m"] = true;
    filter["hourly"]["weather_code"] = true;
    filter["hourly"]["precipitation_probability"] = true;
    filter["daily"]["time"] = true;
    filter["daily"]["weather_code"] = true;
    filter["daily"]["temperature_2m_max"] = true;
    filter["daily"]["temperature_2m_min"] = true;
    filter["daily"]["precipitation_probability_max"] = true;
    filter["daily"]["sunrise"] = true;
    filter["daily"]["sunset"] = true;

    JsonDocument document;
    const DeserializationError jsonError = deserializeJson(
        document,
        http.getStream(),
        DeserializationOption::Filter(filter)
    );
    http.end();

    if (jsonError)
    {
        setError(jsonError.c_str());
        return false;
    }

    JsonObjectConst current = document["current"];
    JsonObjectConst hourly = document["hourly"];
    JsonObjectConst daily = document["daily"];
    JsonArrayConst dailyTimes = daily["time"];
    if (current.isNull() || hourly.isNull() || dailyTimes.size() < 2)
    {
        setError("Reponse meteo incomplete");
        return false;
    }

    WeatherData parsed = {};
    parsed.currentTemperature = current["temperature_2m"] | 0.0F;
    parsed.apparentTemperature = current["apparent_temperature"] | 0.0F;
    parsed.currentWeatherCode = current["weather_code"] | 0;
    parsed.currentWindSpeed = current["wind_speed_10m"] | 0.0F;
    parsed.currentHumidity = current["relative_humidity_2m"] | 0;

    JsonArrayConst hourlyTimes = hourly["time"];
    JsonArrayConst hourlyTemperatures = hourly["temperature_2m"];
    JsonArrayConst hourlyCodes = hourly["weather_code"];
    JsonArrayConst hourlyRain = hourly["precipitation_probability"];

    char nightStart[17];
    char nightEnd[17];
    snprintf(nightStart, sizeof(nightStart), "%sT22:00", dailyTimes[0].as<const char*>());
    snprintf(nightEnd, sizeof(nightEnd), "%sT06:00", dailyTimes[1].as<const char*>());

    float temperatureSum = 0.0F;
    float minimumTemperature = 100.0F;
    int16_t maximumRain = 0;
    int16_t codeCounts[100] = {};
    size_t nightCount = 0;

    for (size_t index = 0; index < hourlyTimes.size(); ++index)
    {
        const char* hour = hourlyTimes[index];
        if (hour == nullptr || strcmp(hour, nightStart) < 0 || strcmp(hour, nightEnd) > 0)
        {
            continue;
        }

        const float temperature = hourlyTemperatures[index] | 0.0F;
        const int16_t code = hourlyCodes[index] | 0;
        const int16_t rain = hourlyRain[index] | 0;
        temperatureSum += temperature;
        minimumTemperature = min(minimumTemperature, temperature);
        maximumRain = max(maximumRain, rain);
        if (code >= 0 && code < 100)
        {
            ++codeCounts[code];
        }
        ++nightCount;
    }

    JsonArrayConst dailyMin = daily["temperature_2m_min"];
    JsonArrayConst dailyMax = daily["temperature_2m_max"];
    JsonArrayConst dailyCodes = daily["weather_code"];
    JsonArrayConst dailyRain = daily["precipitation_probability_max"];

    parsed.tomorrowMinTemperature = dailyMin[1] | 0.0F;
    parsed.tomorrowMaxTemperature = dailyMax[1] | 0.0F;
    parsed.tomorrowWeatherCode = dailyCodes[1] | 0;
    parsed.tomorrowPrecipitationProbability = dailyRain[1] | 0;

    if (nightCount > 0)
    {
        parsed.nightTemperature = temperatureSum / nightCount;
        parsed.nightMinTemperature = minimumTemperature;
        parsed.nightWeatherCode = dominantCode(codeCounts);
        parsed.nightPrecipitationProbability = maximumRain;
    }
    else
    {
        parsed.nightTemperature = parsed.tomorrowMinTemperature;
        parsed.nightMinTemperature = parsed.tomorrowMinTemperature;
        parsed.nightWeatherCode = parsed.tomorrowWeatherCode;
        parsed.nightPrecipitationProbability = parsed.tomorrowPrecipitationProbability;
    }

    JsonArrayConst sunrises = daily["sunrise"];
    JsonArrayConst sunsets = daily["sunset"];
    copyClock(sunrises[0], parsed.sunrise);
    copyClock(sunsets[0], parsed.sunset);
    parsed.updatedAt = time(nullptr);
    parsed.valid = true;

    data = parsed;
    setError("");
    return true;
}

const char* WeatherClient::lastError() const
{
    return lastError_;
}

void WeatherClient::loadDemoData(WeatherData& data, uint8_t scenario)
{
    data = {};
    strcpy(data.sunrise, "06:12");
    strcpy(data.sunset, "21:04");
    data.valid = scenario != 2;
    data.currentHumidity = 54;
    data.currentWindSpeed = 16.0F;
    data.nightTemperature = 22.0F;
    data.nightMinTemperature = 20.0F;
    data.nightWeatherCode = 1;
    data.nightPrecipitationProbability = 10;
    data.tomorrowMinTemperature = 22.0F;
    data.tomorrowMaxTemperature = 31.0F;
    data.tomorrowWeatherCode = 0;
    data.tomorrowPrecipitationProbability = 5;

    switch (scenario)
    {
        case 1:
            data.currentTemperature = 19.0F;
            data.apparentTemperature = 18.0F;
            data.currentWeatherCode = 63;
            data.nightWeatherCode = 61;
            data.nightPrecipitationProbability = 80;
            data.tomorrowWeatherCode = 80;
            data.tomorrowPrecipitationProbability = 65;
            break;
        case 3:
            data.currentTemperature = -6.0F;
            data.apparentTemperature = -11.0F;
            data.currentWeatherCode = 71;
            data.nightTemperature = -9.0F;
            data.tomorrowMinTemperature = -10.0F;
            data.tomorrowMaxTemperature = -2.0F;
            data.tomorrowWeatherCode = 73;
            break;
        case 4:
            data.currentTemperature = 43.0F;
            data.apparentTemperature = 47.0F;
            data.currentWeatherCode = 0;
            data.nightTemperature = 31.0F;
            data.tomorrowMinTemperature = 29.0F;
            data.tomorrowMaxTemperature = 44.0F;
            break;
        case 0:
        default:
            data.currentTemperature = 28.0F;
            data.apparentTemperature = 31.0F;
            data.currentWeatherCode = 0;
            break;
    }
}

const char* WeatherClient::endpoint()
{
    return OPEN_METEO_ENDPOINT;
}

void WeatherClient::setError(const char* message)
{
    strncpy(lastError_, message, sizeof(lastError_) - 1);
    lastError_[sizeof(lastError_) - 1] = '\0';
}
