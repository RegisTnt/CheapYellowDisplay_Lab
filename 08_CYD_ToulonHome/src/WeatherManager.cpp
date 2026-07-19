#include "WeatherManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <cstring>

namespace
{
constexpr uint32_t REFRESH_INTERVAL_MS = 15UL * 60UL * 1000UL;
constexpr uint32_t RETRY_INTERVAL_MS = 60UL * 1000UL;
constexpr char ENDPOINT[] =
    "https://api.open-meteo.com/v1/forecast"
    "?latitude=43.1242&longitude=5.9280"
    "&current=temperature_2m,apparent_temperature,weather_code"
    "&hourly=temperature_2m,weather_code,precipitation_probability"
    "&daily=weather_code,temperature_2m_max,temperature_2m_min,"
    "precipitation_probability_max,sunrise,sunset"
    "&timezone=Europe%2FParis&forecast_days=2";

void copyClock(const char* value, char output[6])
{
    if (value == nullptr || strlen(value) < 16)
    {
        strcpy(output, "--:--");
        return;
    }
    memcpy(output, value + 11, 5);
    output[5] = '\0';
}

int16_t dominantCode(const int16_t counts[100])
{
    int16_t selected = 0;
    int16_t maximum = 0;
    for (int16_t code = 0; code < 100; ++code)
    {
        if (counts[code] > maximum)
        {
            selected = code;
            maximum = counts[code];
        }
    }
    return selected;
}
}

WeatherManager::WeatherManager()
{
    mutex_ = xSemaphoreCreateMutex();
}

WeatherManager::~WeatherManager()
{
    if (mutex_ != nullptr) vSemaphoreDelete(mutex_);
}

void WeatherManager::begin(bool demoMode, uint8_t demoScenario)
{
    demoMode_ = demoMode;
    if (demoMode_)
    {
        loadDemo(demoScenario);
    }
}

void WeatherManager::update(uint32_t nowMs, bool wifiConnected)
{
    if (demoMode_ || taskRunning_ || !wifiConnected
        || static_cast<int32_t>(nowMs - nextFetchMs_) < 0)
    {
        return;
    }

    taskRunning_ = true;
    WeatherSnapshot value = snapshot();
    value.fetching = true;
    ++value.revision;
    publish(value);

    if (xTaskCreatePinnedToCore(taskEntry, "weather-http", 12288, this, 1, nullptr, 0) != pdPASS)
    {
        taskRunning_ = false;
        value.fetching = false;
        value.stale = value.valid;
        ++value.revision;
        publish(value);
        nextFetchMs_ = nowMs + RETRY_INTERVAL_MS;
    }
}

WeatherSnapshot WeatherManager::snapshot() const
{
    WeatherSnapshot value;
    if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        value = snapshot_;
        xSemaphoreGive(mutex_);
    }
    return value;
}

void WeatherManager::taskEntry(void* context)
{
    static_cast<WeatherManager*>(context)->fetchInBackground();
    vTaskDelete(nullptr);
}

void WeatherManager::fetchInBackground()
{
    WeatherSnapshot parsed;
    const bool success = fetch(parsed);
    if (success)
    {
        WeatherSnapshot current = snapshot();
        parsed.valid = true;
        parsed.fetching = false;
        parsed.stale = false;
        parsed.updatedAt = time(nullptr);
        parsed.revision = current.revision + 1;
        publish(parsed);
        nextFetchMs_ = millis() + REFRESH_INTERVAL_MS;
    }
    else
    {
        WeatherSnapshot current = snapshot();
        current.fetching = false;
        current.stale = current.valid;
        ++current.revision;
        publish(current);
        nextFetchMs_ = millis() + RETRY_INTERVAL_MS;
    }
    taskRunning_ = false;
}

bool WeatherManager::fetch(WeatherSnapshot& output)
{
    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(4000);
    http.setTimeout(5000);
    http.useHTTP10(true);
    if (!http.begin(secureClient, ENDPOINT)) return false;

    http.addHeader("User-Agent", "CYD-ToulonHome/1.0");
    const int status = http.GET();
    if (status != HTTP_CODE_OK)
    {
        http.end();
        return false;
    }

    JsonDocument filter;
    filter["current"]["temperature_2m"] = true;
    filter["current"]["apparent_temperature"] = true;
    filter["current"]["weather_code"] = true;
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
    const DeserializationError error = deserializeJson(
        document,
        http.getStream(),
        DeserializationOption::Filter(filter)
    );
    http.end();
    if (error) return false;

    JsonObjectConst current = document["current"];
    JsonObjectConst hourly = document["hourly"];
    JsonObjectConst daily = document["daily"];
    JsonArrayConst dailyTimes = daily["time"];
    if (current.isNull() || hourly.isNull() || dailyTimes.size() < 2) return false;

    output = {};
    output.temperature = current["temperature_2m"] | 0.0F;
    output.apparentTemperature = current["apparent_temperature"] | 0.0F;
    output.weatherCode = current["weather_code"] | 0;

    JsonArrayConst dailyMin = daily["temperature_2m_min"];
    JsonArrayConst dailyMax = daily["temperature_2m_max"];
    JsonArrayConst dailyCodes = daily["weather_code"];
    JsonArrayConst dailyRain = daily["precipitation_probability_max"];
    output.tomorrowMin = dailyMin[1] | 0.0F;
    output.tomorrowMax = dailyMax[1] | 0.0F;
    output.tomorrowWeatherCode = dailyCodes[1] | 0;
    output.tomorrowRain = dailyRain[1] | 0;

    JsonArrayConst hourlyTimes = hourly["time"];
    JsonArrayConst hourlyTemperatures = hourly["temperature_2m"];
    JsonArrayConst hourlyCodes = hourly["weather_code"];
    JsonArrayConst hourlyRain = hourly["precipitation_probability"];
    char nightStart[17];
    char nightEnd[17];
    snprintf(nightStart, sizeof(nightStart), "%sT22:00", dailyTimes[0].as<const char*>());
    snprintf(nightEnd, sizeof(nightEnd), "%sT06:00", dailyTimes[1].as<const char*>());

    float temperatureSum = 0.0F;
    int16_t maximumRain = 0;
    int16_t codeCounts[100] = {};
    size_t nightCount = 0;
    for (size_t index = 0; index < hourlyTimes.size(); ++index)
    {
        const char* timestamp = hourlyTimes[index];
        if (timestamp == nullptr || strcmp(timestamp, nightStart) < 0 || strcmp(timestamp, nightEnd) > 0)
        {
            continue;
        }
        temperatureSum += hourlyTemperatures[index] | 0.0F;
        maximumRain = max(maximumRain, static_cast<int16_t>(hourlyRain[index] | 0));
        const int16_t code = hourlyCodes[index] | 0;
        if (code >= 0 && code < 100) ++codeCounts[code];
        ++nightCount;
    }

    output.nightTemperature = nightCount > 0
        ? temperatureSum / static_cast<float>(nightCount)
        : output.tomorrowMin;
    output.nightWeatherCode = nightCount > 0 ? dominantCode(codeCounts) : output.tomorrowWeatherCode;
    output.nightRain = nightCount > 0 ? maximumRain : output.tomorrowRain;

    JsonArrayConst sunrises = daily["sunrise"];
    JsonArrayConst sunsets = daily["sunset"];
    copyClock(sunrises[0], output.sunrise);
    copyClock(sunsets[0], output.sunset);
    return true;
}

void WeatherManager::loadDemo(uint8_t scenario)
{
    WeatherSnapshot value;
    value.valid = true;
    value.temperature = 28.4F;
    value.apparentTemperature = 31.2F;
    value.weatherCode = 0;
    value.nightTemperature = 22.0F;
    value.nightWeatherCode = 0;
    value.nightRain = 10;
    value.tomorrowMin = 22.0F;
    value.tomorrowMax = 31.0F;
    value.tomorrowWeatherCode = 0;
    value.tomorrowRain = 5;
    strcpy(value.sunrise, "06:09");
    strcpy(value.sunset, "21:08");
    value.updatedAt = time(nullptr);
    value.revision = 1;

    if (scenario == 1)
    {
        value.temperature = 19.0F;
        value.apparentTemperature = 18.0F;
        value.weatherCode = 63;
        value.nightWeatherCode = 61;
        value.nightRain = 85;
        value.tomorrowWeatherCode = 80;
        value.tomorrowRain = 70;
    }
    else if (scenario == 2)
    {
        value.temperature = 18.0F;
        value.apparentTemperature = 17.0F;
        value.weatherCode = 1;
        value.nightTemperature = 16.0F;
    }
    else if (scenario == 4)
    {
        value.stale = true;
    }
    publish(value);
}

void WeatherManager::publish(const WeatherSnapshot& value)
{
    if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        snapshot_ = value;
        xSemaphoreGive(mutex_);
    }
}
