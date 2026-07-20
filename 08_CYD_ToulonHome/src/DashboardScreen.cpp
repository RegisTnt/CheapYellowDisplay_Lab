#include "DashboardScreen.h"

#include <cmath>
#include <cstring>

namespace
{
constexpr uint16_t COLOR_BG = 0x0000;
constexpr uint16_t COLOR_PANEL = 0x0841;
constexpr uint16_t COLOR_PANEL_ALT = 0x1082;
constexpr uint16_t COLOR_WHITE = 0xFFFF;
constexpr uint16_t COLOR_MUTED = 0xAD55;
constexpr uint16_t COLOR_BLUE = 0x04BF;
constexpr uint16_t COLOR_YELLOW = 0xFFE0;
constexpr uint16_t COLOR_GREEN = 0x07E0;
constexpr uint16_t COLOR_RED = 0xF800;
constexpr uint16_t COLOR_DISABLED = 0x4208;

constexpr int16_t HEADER_HEIGHT = 42;
constexpr int16_t PORTAL_HEIGHT = 92;
constexpr int16_t CONTROLS_HEIGHT = 73;
constexpr int16_t FOOTER_HEIGHT = 33;
constexpr int16_t PORTAL_Y = HEADER_HEIGHT;
constexpr int16_t CONTROLS_Y = PORTAL_Y + PORTAL_HEIGHT;
constexpr int16_t FOOTER_Y = CONTROLS_Y + CONTROLS_HEIGHT;

void formatClock(time_t timestamp, char* output, size_t size)
{
    if (timestamp < 100000)
    {
        strncpy(output, "--:--", size - 1);
        output[size - 1] = '\0';
        return;
    }
    struct tm local = {};
    localtime_r(&timestamp, &local);
    strftime(output, size, "%H:%M", &local);
}

int16_t clockMinutes(const char value[6])
{
    if (value == nullptr || strlen(value) < 5 || value[2] != ':') return -1;
    return (value[0] - '0') * 600 + (value[1] - '0') * 60
        + (value[3] - '0') * 10 + value[4] - '0';
}
}

DashboardScreen::DashboardScreen(TFT_eSPI& display)
    : display_(display), sprite_(&display)
{
}

bool DashboardScreen::begin()
{
    sprite_.setColorDepth(16);
    spriteReady_ = sprite_.createSprite(320, 96) != nullptr;
    if (spriteReady_) sprite_.setTextWrap(false);
    return spriteReady_;
}

void DashboardScreen::drawHeader(const WeatherSnapshot& weather, time_t now, const ThemePalette& theme)
{
    (void)theme;
    prepareSprite(HEADER_HEIGHT, COLOR_PANEL);
    sprite_.drawFastHLine(0, HEADER_HEIGHT - 2, 320, COLOR_BLUE);
    sprite_.drawFastHLine(0, HEADER_HEIGHT - 1, 320, COLOR_BLUE);

    if (weather.valid)
    {
        drawWeatherIcon(weather.weatherCode, 22, 20, 28, COLOR_PANEL);
        char temperature[12];
        snprintf(temperature, sizeof(temperature), "%.0f C", weather.temperature);
        sprite_.setTextColor(COLOR_WHITE, COLOR_PANEL);
        sprite_.setTextDatum(ML_DATUM);
        sprite_.drawString(temperature, 45, 20, 4);
    }
    else
    {
        sprite_.setTextColor(COLOR_MUTED, COLOR_PANEL);
        sprite_.setTextDatum(ML_DATUM);
        sprite_.drawString("-- C", 12, 20, 4);
    }

    char clock[8];
    formatClock(now, clock, sizeof(clock));
    sprite_.setTextColor(COLOR_WHITE, COLOR_PANEL);
    sprite_.setTextDatum(MR_DATUM);
    sprite_.drawString(clock, 310, 20, 4);
    pushSprite(0);
}

void DashboardScreen::drawPortal(const PortalSnapshot& portal, uint8_t glow, const ThemePalette& theme)
{
    (void)theme;
    prepareSprite(PORTAL_HEIGHT, COLOR_BG);

    if (!portal.apiOnline || portal.position == PortalPosition::Unknown)
    {
        sprite_.fillCircle(48, 46, 31, COLOR_DISABLED);
        sprite_.drawCircle(48, 46, 34, COLOR_MUTED);
        sprite_.setTextDatum(ML_DATUM);
        sprite_.setTextColor(COLOR_WHITE, COLOR_BG);
        sprite_.drawString("PORTAIL", 94, 31, 4);
        sprite_.setTextColor(COLOR_MUTED, COLOR_BG);
        sprite_.drawString("INCONNU", 94, 61, 4);
        return pushSprite(PORTAL_Y);
    }

    const bool closed = portal.position == PortalPosition::Closed;
    const uint16_t baseColor = closed ? COLOR_GREEN : COLOR_RED;
    const uint16_t lampColor = closed ? baseColor : scaleColor(baseColor, glow);
    sprite_.fillCircle(48, 46, 36, scaleColor(lampColor, 55));
    sprite_.fillCircle(48, 46, 29, lampColor);
    sprite_.drawCircle(48, 46, 34, COLOR_WHITE);
    sprite_.drawCircle(48, 46, 35, COLOR_WHITE);

    sprite_.setTextDatum(ML_DATUM);
    sprite_.setTextColor(COLOR_WHITE, COLOR_BG);
    sprite_.drawString("PORTAIL", 94, 28, 4);
    sprite_.setTextColor(baseColor, COLOR_BG); // Le texte ne pulse jamais.
    sprite_.drawString(closed ? "FERME" : "OUVERT", 94, 59, 4);
    if (closed)
    {
        const int16_t accentX = 94 + sprite_.textWidth("FERM", 4) + sprite_.textWidth("E", 4) / 2;
        sprite_.drawWideLine(accentX - 3, 43, accentX + 2, 39, 2, baseColor);
    }
    pushSprite(PORTAL_Y);
}

void DashboardScreen::drawControls(
    const PortalSnapshot& portal,
    const AnimationManager& animation,
    uint32_t nowMs,
    const ThemePalette& theme
)
{
    (void)theme;
    prepareSprite(CONTROLS_HEIGHT, COLOR_BG);
    const bool enabled = portal.apiOnline && !portal.busy && !portal.cooldown;
    const uint16_t pedestrianColor = enabled ? COLOR_YELLOW : COLOR_DISABLED;
    const uint16_t vehicleColor = enabled ? COLOR_BLUE : COLOR_DISABLED;
    const uint16_t iconColor = enabled ? COLOR_BG : COLOR_MUTED;
    const int16_t pedestrianOffset = static_cast<int16_t>(round(animation.pressDepth(AnimationManager::Button::Pedestrian) * 3.0F));
    const int16_t vehicleOffset = static_cast<int16_t>(round(animation.pressDepth(AnimationManager::Button::Vehicle) * 3.0F));

    sprite_.fillRoundRect(3, 4 + pedestrianOffset, 155, 64, 8, pedestrianColor);
    sprite_.fillRoundRect(162, 4 + vehicleOffset, 155, 64, 8, vehicleColor);
    sprite_.drawRoundRect(3, 4 + pedestrianOffset, 155, 64, 8, COLOR_WHITE);
    sprite_.drawRoundRect(162, 4 + vehicleOffset, 155, 64, 8, COLOR_WHITE);
    sprite_.drawRoundRect(4, 5 + pedestrianOffset, 153, 62, 7, COLOR_WHITE);
    sprite_.drawRoundRect(163, 5 + vehicleOffset, 153, 62, 7, COLOR_WHITE);

    drawPersonIcon(28, 35 + pedestrianOffset, iconColor);
    drawCarIcon(190, 36 + vehicleOffset, iconColor);
    sprite_.setTextColor(iconColor);
    sprite_.setTextDatum(MC_DATUM);
    sprite_.drawString("PIETON", 103, 35 + pedestrianOffset, 4);
    sprite_.drawString("VOITURE", 267, 35 + vehicleOffset, 4);
    const int16_t pedestrianStart = 103 - sprite_.textWidth("PIETON", 4) / 2;
    const int16_t accentX = pedestrianStart + sprite_.textWidth("PI", 4) + sprite_.textWidth("E", 4) / 2;
    sprite_.drawWideLine(accentX - 3, 19 + pedestrianOffset, accentX + 2, 15 + pedestrianOffset, 2, iconColor);

    if (portal.busy || animation.feedbackActive(nowMs))
    {
        const bool accepted = animation.feedbackAccepted();
        sprite_.fillRoundRect(98, 54, 124, 16, 4, COLOR_PANEL_ALT);
        sprite_.setTextDatum(MC_DATUM);
        sprite_.setTextColor(portal.busy || accepted ? COLOR_GREEN : COLOR_RED, COLOR_PANEL_ALT);
        sprite_.drawString(portal.busy ? "ENVOI..." : (accepted ? "COMMANDE OK" : "ECHEC"), 160, 62, 1);
    }
    pushSprite(CONTROLS_Y);
}

void DashboardScreen::drawFooter(
    const WifiSnapshot& wifi,
    const PortalSnapshot& portal,
    const WeatherSnapshot& weather,
    time_t now,
    const ThemePalette& theme
)
{
    (void)theme;
    prepareSprite(FOOTER_HEIGHT, COLOR_PANEL);
    sprite_.drawFastHLine(0, 0, 320, COLOR_BLUE);
    sprite_.drawFastHLine(0, 1, 320, COLOR_BLUE);
    sprite_.setTextDatum(ML_DATUM);
    sprite_.setTextColor(COLOR_WHITE, COLOR_PANEL);
    drawStatusDot(13, 17, wifi.connected);
    sprite_.drawString("WiFi", 23, 17, 2);
    drawStatusDot(91, 17, portal.apiOnline);
    sprite_.drawString("API", 101, 17, 2);

    char updated[8];
    formatClock(weather.updatedAt > 0 ? weather.updatedAt : now, updated, sizeof(updated));
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "MAJ %s", updated);
    sprite_.setTextDatum(MR_DATUM);
    sprite_.setTextColor(weather.stale ? COLOR_YELLOW : COLOR_WHITE, COLOR_PANEL);
    sprite_.drawString(buffer, 310, 17, 2);
    pushSprite(FOOTER_Y);
}

void DashboardScreen::drawWeatherDetails(
    const WeatherSnapshot& weather,
    const MoonSnapshot& moon,
    time_t now,
    const ThemePalette& theme
)
{
    (void)theme;
    char buffer[32];
    char clock[8];
    formatClock(now, clock, sizeof(clock));

    prepareSprite(42, COLOR_PANEL);
    sprite_.drawFastHLine(0, 40, 320, COLOR_BLUE);
    sprite_.drawFastHLine(0, 41, 320, COLOR_BLUE);
    sprite_.setTextDatum(ML_DATUM);
    sprite_.setTextColor(COLOR_WHITE, COLOR_PANEL);
    sprite_.drawString("METEO", 10, 20, 4);
    sprite_.setTextDatum(MR_DATUM);
    sprite_.drawString(clock, 310, 20, 4);
    pushSprite(0);

    prepareSprite(58, COLOR_BG);
    if (weather.valid)
    {
        drawWeatherIcon(weather.weatherCode, 34, 29, 42, COLOR_BG);
        snprintf(buffer, sizeof(buffer), "%.0f C", weather.temperature);
        sprite_.setTextDatum(ML_DATUM);
        sprite_.setTextColor(COLOR_WHITE, COLOR_BG);
        sprite_.drawString(buffer, 67, 21, 4);
        sprite_.setTextColor(COLOR_MUTED, COLOR_BG);
        sprite_.drawString(weatherLabel(weather.weatherCode), 67, 43, 2);
    }
    else
    {
        sprite_.setTextDatum(MC_DATUM);
        sprite_.setTextColor(COLOR_MUTED, COLOR_BG);
        sprite_.drawString("DONNEES INDISPONIBLES", 160, 29, 2);
    }
    pushSprite(42);

    prepareSprite(72, COLOR_PANEL_ALT);
    sprite_.drawFastVLine(160, 6, 60, COLOR_MUTED);
    sprite_.setTextDatum(TC_DATUM);
    sprite_.setTextColor(COLOR_BLUE, COLOR_PANEL_ALT);
    sprite_.drawString("CETTE NUIT", 80, 7, 2);
    sprite_.drawString("DEMAIN", 240, 7, 2);
    if (weather.valid)
    {
        drawWeatherIcon(weather.nightWeatherCode, 35, 45, 27, COLOR_PANEL_ALT);
        snprintf(buffer, sizeof(buffer), "%.0f C", weather.nightTemperature);
        sprite_.setTextDatum(ML_DATUM);
        sprite_.setTextColor(COLOR_WHITE, COLOR_PANEL_ALT);
        sprite_.drawString(buffer, 58, 43, 4);
        drawWeatherIcon(weather.tomorrowWeatherCode, 188, 45, 27, COLOR_PANEL_ALT);
        snprintf(buffer, sizeof(buffer), "%.0f/%.0f C", weather.tomorrowMax, weather.tomorrowMin);
        sprite_.drawString(buffer, 211, 43, 2);
        snprintf(buffer, sizeof(buffer), "PLUIE %d%%", weather.tomorrowRain);
        sprite_.setTextColor(COLOR_MUTED, COLOR_PANEL_ALT);
        sprite_.drawString(buffer, 211, 59, 1);
    }
    pushSprite(100);

    prepareSprite(68, COLOR_BG);
    sprite_.drawFastHLine(0, 0, 320, COLOR_BLUE);
    sprite_.setTextDatum(TC_DATUM);
    sprite_.setTextColor(COLOR_MUTED, COLOR_BG);
    sprite_.drawString("SOLEIL", 80, 8, 2);
    sprite_.drawString("LUNE", 240, 8, 2);
    sprite_.setTextColor(COLOR_YELLOW, COLOR_BG);
    sprite_.fillCircle(26, 43, 10, COLOR_YELLOW);
    sprite_.setTextDatum(ML_DATUM);
    sprite_.setTextColor(COLOR_WHITE, COLOR_BG);
    snprintf(buffer, sizeof(buffer), "%s  %s", weather.sunrise, weather.sunset);
    sprite_.drawString(buffer, 45, 43, 2);
    drawMoonIcon(moon, 186, 43, 14, COLOR_BG);
    snprintf(buffer, sizeof(buffer), "%s", moon.name);
    sprite_.drawString(buffer, 207, 36, 1);
    snprintf(buffer, sizeof(buffer), "%d%%", moon.illumination);
    sprite_.drawString(buffer, 207, 52, 2);
    pushSprite(172);
}

DayPhase DashboardScreen::dayPhase(const WeatherSnapshot& weather, time_t now) const
{
    if (!weather.valid || now < 100000) return DayPhase::Day;
    const int16_t sunrise = clockMinutes(weather.sunrise);
    const int16_t sunset = clockMinutes(weather.sunset);
    if (sunrise < 0 || sunset < 0) return DayPhase::Day;
    struct tm local = {};
    localtime_r(&now, &local);
    const int16_t minute = local.tm_hour * 60 + local.tm_min;
    if ((minute >= sunrise - 30 && minute <= sunrise + 30)
        || (minute >= sunset - 30 && minute <= sunset + 30)) return DayPhase::Twilight;
    return minute > sunrise + 30 && minute < sunset - 30 ? DayPhase::Day : DayPhase::Night;
}

ThemePalette DashboardScreen::paletteFor(DayPhase phase) const
{
    (void)phase;
    return {COLOR_BG, COLOR_PANEL, COLOR_PANEL_ALT, COLOR_PANEL_ALT,
            COLOR_WHITE, COLOR_MUTED, COLOR_BLUE, COLOR_GREEN, COLOR_RED};
}

ThemePalette DashboardScreen::blendPalette(const ThemePalette& from, const ThemePalette& to, uint8_t progress) const
{
    return {
        blendColor(from.background, to.background, progress), blendColor(from.header, to.header, progress),
        blendColor(from.card, to.card, progress), blendColor(from.cardAlt, to.cardAlt, progress),
        blendColor(from.text, to.text, progress), blendColor(from.muted, to.muted, progress),
        blendColor(from.accent, to.accent, progress), blendColor(from.portalClosed, to.portalClosed, progress),
        blendColor(from.portalOpen, to.portalOpen, progress)
    };
}

bool DashboardScreen::inWeatherBand(int16_t x, int16_t y)
{
    return x >= 0 && x < 320 && y >= 0 && y < HEADER_HEIGHT;
}

bool DashboardScreen::inPedestrianButton(int16_t x, int16_t y)
{
    return x >= 3 && x <= 158 && y >= CONTROLS_Y + 4 && y <= CONTROLS_Y + 70;
}

bool DashboardScreen::inVehicleButton(int16_t x, int16_t y)
{
    return x >= 162 && x <= 317 && y >= CONTROLS_Y + 4 && y <= CONTROLS_Y + 70;
}

void DashboardScreen::prepareSprite(int16_t height, uint16_t background)
{
    currentHeight_ = height;
    sprite_.fillSprite(background);
    sprite_.setTextWrap(false);
}

void DashboardScreen::pushSprite(int16_t y)
{
    display_.pushImage(0, y, 320, currentHeight_, static_cast<uint16_t*>(sprite_.getPointer()));
}

void DashboardScreen::drawWeatherIcon(int16_t code, int16_t x, int16_t y, int16_t size, uint16_t background)
{
    const int16_t radius = max<int16_t>(4, size / 4);
    if (code == 0)
    {
        sprite_.fillCircle(x, y, radius, COLOR_YELLOW);
        for (int16_t angle = 0; angle < 8; ++angle)
        {
            const float a = angle * 0.785398F;
            sprite_.drawLine(x + cosf(a) * (radius + 3), y + sinf(a) * (radius + 3),
                x + cosf(a) * (radius + 7), y + sinf(a) * (radius + 7), COLOR_YELLOW);
        }
        return;
    }
    if (code == 45 || code == 48)
    {
        for (int16_t offset = -7; offset <= 7; offset += 7)
            sprite_.drawWideLine(x - size / 2, y + offset, x + size / 2, y + offset, 3, COLOR_MUTED);
        return;
    }
    sprite_.fillCircle(x - 5, y, radius, COLOR_WHITE);
    sprite_.fillCircle(x + 4, y - 4, radius + 1, COLOR_WHITE);
    sprite_.fillRoundRect(x - 12, y, 25, 8, 3, COLOR_WHITE);
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82))
    {
        sprite_.drawWideLine(x - 7, y + 10, x - 10, y + 17, 3, COLOR_BLUE);
        sprite_.drawWideLine(x + 3, y + 10, x, y + 17, 3, COLOR_BLUE);
    }
    else if (code >= 95)
    {
        sprite_.fillTriangle(x + 2, y + 7, x - 5, y + 18, x, y + 17, COLOR_YELLOW);
    }
    (void)background;
}

void DashboardScreen::drawMoonIcon(const MoonSnapshot& moon, int16_t x, int16_t y, int16_t radius, uint16_t background)
{
    sprite_.fillCircle(x, y, radius, COLOR_WHITE);
    const int16_t shift = static_cast<int16_t>(round((moon.cycleFraction - 0.5F) * radius * 1.6F));
    sprite_.fillCircle(x + shift, y, radius, background);
    sprite_.drawCircle(x, y, radius, COLOR_MUTED);
}

void DashboardScreen::drawStatusDot(int16_t x, int16_t y, bool ok)
{
    sprite_.fillCircle(x, y, 6, ok ? COLOR_GREEN : COLOR_RED);
}

void DashboardScreen::drawPersonIcon(int16_t x, int16_t y, uint16_t color)
{
    sprite_.fillCircle(x, y - 17, 7, color);
    sprite_.drawWideLine(x, y - 9, x, y + 7, 6, color);
    sprite_.drawWideLine(x, y - 4, x - 11, y + 4, 5, color);
    sprite_.drawWideLine(x, y - 4, x + 11, y + 4, 5, color);
    sprite_.drawWideLine(x, y + 5, x - 9, y + 19, 6, color);
    sprite_.drawWideLine(x, y + 5, x + 9, y + 19, 6, color);
}

void DashboardScreen::drawCarIcon(int16_t x, int16_t y, uint16_t color)
{
    sprite_.fillRoundRect(x - 17, y - 5, 34, 18, 5, color);
    sprite_.fillTriangle(x - 12, y - 5, x - 7, y - 15, x + 8, y - 5, color);
    sprite_.fillCircle(x - 10, y + 13, 5, color);
    sprite_.fillCircle(x + 10, y + 13, 5, color);
}

const char* DashboardScreen::weatherLabel(int16_t code) const
{
    if (code == 0) return "CIEL DEGAGE";
    if (code <= 2) return "PEU NUAGEUX";
    if (code == 3) return "COUVERT";
    if (code == 45 || code == 48) return "BROUILLARD";
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return "PLUIE";
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "NEIGE";
    if (code >= 95) return "ORAGE";
    return "VARIABLE";
}

uint16_t DashboardScreen::scaleColor(uint16_t color, uint8_t intensity) const
{
    const uint16_t red = ((color >> 11) & 0x1F) * intensity / 255;
    const uint16_t green = ((color >> 5) & 0x3F) * intensity / 255;
    const uint16_t blue = (color & 0x1F) * intensity / 255;
    return (red << 11) | (green << 5) | blue;
}

uint16_t DashboardScreen::blendColor(uint16_t from, uint16_t to, uint8_t progress) const
{
    const int16_t fromRed = (from >> 11) & 0x1F, fromGreen = (from >> 5) & 0x3F, fromBlue = from & 0x1F;
    const int16_t toRed = (to >> 11) & 0x1F, toGreen = (to >> 5) & 0x3F, toBlue = to & 0x1F;
    return ((fromRed + (toRed - fromRed) * progress / 255) << 11)
        | ((fromGreen + (toGreen - fromGreen) * progress / 255) << 5)
        | (fromBlue + (toBlue - fromBlue) * progress / 255);
}
