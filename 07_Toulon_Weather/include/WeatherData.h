#pragma once

#include <Arduino.h>
#include <time.h>

struct WeatherData
{
    bool valid;
    float currentTemperature;
    float apparentTemperature;
    int16_t currentWeatherCode;
    float currentWindSpeed;
    int16_t currentHumidity;

    float nightTemperature;
    float nightMinTemperature;
    int16_t nightWeatherCode;
    int16_t nightPrecipitationProbability;

    float tomorrowMinTemperature;
    float tomorrowMaxTemperature;
    int16_t tomorrowWeatherCode;
    int16_t tomorrowPrecipitationProbability;

    char sunrise[6];
    char sunset[6];
    time_t updatedAt;
};
