#pragma once

#include <TFT_eSPI.h>

#include "MoonPhase.h"
#include "WeatherData.h"

class WeatherScreen
{
public:
    explicit WeatherScreen(TFT_eSPI& display);

    void begin();
    void drawStatus(const char* message, uint16_t color = TFT_WHITE);
    void drawDashboard(
        const WeatherData& data,
        const MoonPhaseInfo& moon,
        time_t localTime,
        bool offline,
        bool stale
    );

private:
    void drawCard(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color);
    void drawWeatherIcon(int16_t code, int16_t x, int16_t y, int16_t size);
    void drawMoonIcon(const MoonPhaseInfo& moon, int16_t x, int16_t y, int16_t radius);
    const char* weatherLabel(int16_t code) const;
    const char* shortMoonLabel(const char* name) const;

    TFT_eSPI& display_;
};
