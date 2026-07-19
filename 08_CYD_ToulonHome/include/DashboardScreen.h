#pragma once

#include <TFT_eSPI.h>

#include "AnimationManager.h"
#include "AppData.h"

class DashboardScreen
{
public:
    explicit DashboardScreen(TFT_eSPI& display);
    bool begin();

    void drawHeader(const WeatherSnapshot& weather, time_t now, const ThemePalette& theme);
    void drawForecast(const WeatherSnapshot& weather, const ThemePalette& theme);
    void drawPortal(const PortalSnapshot& portal, uint8_t glow, const ThemePalette& theme);
    void drawControls(
        const PortalSnapshot& portal,
        const AnimationManager& animation,
        uint32_t nowMs,
        const ThemePalette& theme
    );
    void drawFooter(
        const MoonSnapshot& moon,
        const WifiSnapshot& wifi,
        const PortalSnapshot& portal,
        const WeatherSnapshot& weather,
        time_t now,
        const ThemePalette& theme
    );

    DayPhase dayPhase(const WeatherSnapshot& weather, time_t now) const;
    ThemePalette paletteFor(DayPhase phase) const;
    ThemePalette blendPalette(const ThemePalette& from, const ThemePalette& to, uint8_t progress) const;

    static bool inPedestrianButton(int16_t x, int16_t y);
    static bool inVehicleButton(int16_t x, int16_t y);

private:
    void prepareSprite(int16_t height, uint16_t background);
    void pushSprite(int16_t y);
    void drawCard(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t fill, uint16_t border);
    void drawWeatherIcon(int16_t code, int16_t x, int16_t y, int16_t size, uint16_t background);
    void drawMoonIcon(const MoonSnapshot& moon, int16_t x, int16_t y, int16_t radius, uint16_t background);
    void drawStatusDot(int16_t x, int16_t y, bool ok, uint16_t background);
    const char* weatherLabel(int16_t code) const;
    uint16_t scaleColor(uint16_t color, uint8_t intensity) const;
    uint16_t blendColor(uint16_t from, uint16_t to, uint8_t progress) const;

    TFT_eSPI& display_;
    TFT_eSprite sprite_;
    bool spriteReady_ = false;
    int16_t currentHeight_ = 0;
};
