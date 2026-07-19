#pragma once

#include <Arduino.h>

#include "WeatherData.h"

class WeatherClient
{
public:
    bool fetch(WeatherData& data);
    const char* lastError() const;

    static void loadDemoData(WeatherData& data, uint8_t scenario);
    static const char* endpoint();

private:
    void setError(const char* message);
    char lastError_[48] = "";
};
