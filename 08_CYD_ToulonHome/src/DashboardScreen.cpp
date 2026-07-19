#include "DashboardScreen.h"

#include <cmath>
#include <cstring>

namespace
{
constexpr uint16_t COLOR_DAY_BG = 0x0518;
constexpr uint16_t COLOR_DAY_HEADER = 0x0434;
constexpr uint16_t COLOR_DAY_CARD = 0xBEBB;
constexpr uint16_t COLOR_DAY_CARD_ALT = 0x0250;
constexpr uint16_t COLOR_NIGHT_BG = 0x008B;
constexpr uint16_t COLOR_NIGHT_HEADER = 0x0067;
constexpr uint16_t COLOR_NIGHT_CARD = 0x114C;
constexpr uint16_t COLOR_TWILIGHT_BG = 0xE2A4;
constexpr uint16_t COLOR_TWILIGHT_HEADER = 0x9A46;
constexpr uint16_t COLOR_SAND = 0xFF9A;
constexpr uint16_t COLOR_SOLAR = 0xFEC0;
constexpr uint16_t COLOR_TURQUOISE = 0x05B7;
constexpr uint16_t COLOR_GREEN = 0x05E6;
constexpr uint16_t COLOR_RED = 0xF986;
constexpr uint16_t COLOR_DARK = 0x0129;
constexpr uint16_t COLOR_MUTED = 0xBDF7;

constexpr int16_t HEADER_HEIGHT = 54;
constexpr int16_t FORECAST_HEIGHT = 55;
constexpr int16_t PORTAL_HEIGHT = 40;
constexpr int16_t CONTROLS_HEIGHT = 53;
constexpr int16_t FOOTER_HEIGHT = 38;

void formatClock(time_t timestamp, const char* format, char* output, size_t size)
{
    if (timestamp < 100000)
    {
        strncpy(output, "--:--", size - 1);
        output[size - 1] = '\0';
        return;
    }
    struct tm local = {};
    localtime_r(&timestamp, &local);
    strftime(output, size, format, &local);
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
    spriteReady_ = sprite_.createSprite(320, 64) != nullptr;
    if (spriteReady_)
    {
        sprite_.setTextWrap(false);
    }
    return spriteReady_;
}

void DashboardScreen::drawHeader(const WeatherSnapshot& weather, time_t now, const ThemePalette& theme)
{
    prepareSprite(HEADER_HEIGHT, theme.header);
    for (int16_t y = 0; y < HEADER_HEIGHT; ++y)
    {
        const uint8_t shade = static_cast<uint8_t>(255 - y * 70 / HEADER_HEIGHT);
        sprite_.drawFastHLine(0, y, 320, scaleColor(theme.header, shade));
    }

    sprite_.setTextDatum(TL_DATUM);
    sprite_.setTextColor(theme.text, theme.header);
    sprite_.fillCircle(11, 12, 4, theme.accent);
    sprite_.fillTriangle(8, 14, 14, 14, 11, 20, theme.accent);
    sprite_.drawString("TOULON", 20, 4, 2);

    if (weather.valid)
    {
        drawWeatherIcon(weather.weatherCode, 96, 23, 28, theme.header);
        char temperature[16];
        snprintf(temperature, sizeof(temperature), "%.1f C", weather.temperature);
        sprite_.setTextDatum(MC_DATUM);
        sprite_.setTextColor(theme.text, theme.header);
        sprite_.drawString(temperature, 163, 22, 4);
        sprite_.setTextDatum(TL_DATUM);
        sprite_.drawString(weatherLabel(weather.weatherCode), 8, 33, 2);
        char apparent[24];
        snprintf(apparent, sizeof(apparent), "Ressenti %.0f C", weather.apparentTemperature);
        sprite_.setTextDatum(TR_DATUM);
        sprite_.setTextColor(theme.muted, theme.header);
        sprite_.drawString(apparent, 238, 37, 1);
    }
    else
    {
        sprite_.setTextDatum(MC_DATUM);
        sprite_.setTextColor(theme.muted, theme.header);
        sprite_.drawString("METEO --", 160, 25, 4);
    }

    char clock[8];
    char date[12];
    formatClock(now, "%H:%M", clock, sizeof(clock));
    formatClock(now, "%d/%m", date, sizeof(date));
    sprite_.setTextDatum(TR_DATUM);
    sprite_.setTextColor(theme.text, theme.header);
    sprite_.drawString(clock, 313, 3, 4);
    sprite_.setTextColor(theme.muted, theme.header);
    sprite_.drawString(date, 313, 35, 2);
    pushSprite(0);
}

void DashboardScreen::drawForecast(const WeatherSnapshot& weather, const ThemePalette& theme)
{
    prepareSprite(FORECAST_HEIGHT, theme.background);
    const int16_t cardWidth = 102;
    const int16_t cardX[3] = {3, 109, 215};
    for (int16_t x : cardX) drawCard(x, 2, cardWidth, 51, theme.card, theme.accent);

    sprite_.setTextDatum(TC_DATUM);
    sprite_.setTextColor(theme.accent, theme.card);
    sprite_.drawString("CETTE NUIT", 54, 5, 1);
    sprite_.drawString("DEMAIN", 160, 5, 1);
    sprite_.drawString("SOLEIL", 266, 5, 1);

    if (weather.valid)
    {
        char buffer[24];
        drawWeatherIcon(weather.nightWeatherCode, 19, 28, 19, theme.card);
        snprintf(buffer, sizeof(buffer), "%.0f C", weather.nightTemperature);
        sprite_.setTextDatum(TL_DATUM);
        sprite_.setTextColor(theme.text, theme.card);
        sprite_.drawString(buffer, 35, 19, 2);
        snprintf(buffer, sizeof(buffer), "Pluie %d%%", weather.nightRain);
        sprite_.setTextColor(theme.muted, theme.card);
        sprite_.drawString(buffer, 35, 38, 1);

        drawWeatherIcon(weather.tomorrowWeatherCode, 124, 28, 19, theme.card);
        snprintf(buffer, sizeof(buffer), "%.0f/%.0f C", weather.tomorrowMax, weather.tomorrowMin);
        sprite_.setTextColor(theme.text, theme.card);
        sprite_.drawString(buffer, 140, 19, 2);
        snprintf(buffer, sizeof(buffer), "Pluie %d%%", weather.tomorrowRain);
        sprite_.setTextColor(theme.muted, theme.card);
        sprite_.drawString(buffer, 140, 38, 1);

        sprite_.setTextColor(COLOR_SOLAR, theme.card);
        sprite_.drawCircle(230, 26, 8, COLOR_SOLAR);
        sprite_.drawFastHLine(220, 36, 20, COLOR_SOLAR);
        sprite_.setTextColor(theme.text, theme.card);
        snprintf(buffer, sizeof(buffer), "L %s", weather.sunrise);
        sprite_.drawString(buffer, 243, 17, 1);
        snprintf(buffer, sizeof(buffer), "C %s", weather.sunset);
        sprite_.drawString(buffer, 243, 35, 1);
    }
    else
    {
        sprite_.setTextDatum(MC_DATUM);
        sprite_.setTextColor(theme.muted, theme.card);
        sprite_.drawString("--", 54, 30, 2);
        sprite_.drawString("--", 160, 30, 2);
        sprite_.drawString("--", 266, 30, 2);
    }
    pushSprite(HEADER_HEIGHT);
}

void DashboardScreen::drawPortal(const PortalSnapshot& portal, uint8_t glow, const ThemePalette& theme)
{
    prepareSprite(PORTAL_HEIGHT, theme.background);
    drawCard(4, 1, 312, 38, theme.cardAlt, theme.accent);

    if (!portal.apiOnline || portal.position == PortalPosition::Unknown)
    {
        sprite_.fillCircle(29, 20, 11, TFT_DARKGREY);
        sprite_.setTextDatum(ML_DATUM);
        sprite_.setTextColor(theme.muted, theme.cardAlt);
        sprite_.drawString("API INDISPONIBLE", 51, 19, 4);
        return pushSprite(HEADER_HEIGHT + FORECAST_HEIGHT);
    }

    const bool closed = portal.position == PortalPosition::Closed;
    const uint16_t baseColor = closed ? theme.portalClosed : theme.portalOpen;
    const uint16_t color = closed ? baseColor : scaleColor(baseColor, glow);
    sprite_.fillCircle(29, 20, 13, scaleColor(color, 100));
    sprite_.fillCircle(29, 20, 9, color);
    sprite_.setTextDatum(ML_DATUM);
    sprite_.setTextColor(color, theme.cardAlt);
    sprite_.drawString(closed ? "PORTAIL FERME" : "PORTAIL OUVERT", 51, 16, 4);
    sprite_.setTextDatum(MR_DATUM);
    sprite_.setTextColor(theme.muted, theme.cardAlt);
    sprite_.drawString(portal.busy ? "ENVOI..." : "CAPTEUR", 307, 21, 1);
    pushSprite(HEADER_HEIGHT + FORECAST_HEIGHT);
}

void DashboardScreen::drawControls(
    const PortalSnapshot& portal,
    const AnimationManager& animation,
    uint32_t nowMs,
    const ThemePalette& theme
)
{
    prepareSprite(CONTROLS_HEIGHT, theme.background);
    const bool enabled = portal.apiOnline && !portal.busy && !portal.cooldown;
    const uint16_t pedestrianColor = enabled ? COLOR_SOLAR : TFT_DARKGREY;
    const uint16_t vehicleColor = enabled ? COLOR_TURQUOISE : TFT_DARKGREY;
    const int16_t pedestrianOffset = static_cast<int16_t>(round(animation.pressDepth(AnimationManager::Button::Pedestrian) * 3.0F));
    const int16_t vehicleOffset = static_cast<int16_t>(round(animation.pressDepth(AnimationManager::Button::Vehicle) * 3.0F));

    sprite_.fillRoundRect(4, 5, 153, 46, 11, scaleColor(pedestrianColor, 110));
    sprite_.fillRoundRect(163, 5, 153, 46, 11, scaleColor(vehicleColor, 110));
    sprite_.fillRoundRect(4, 2 + pedestrianOffset, 153, 46, 11, pedestrianColor);
    sprite_.fillRoundRect(163, 2 + vehicleOffset, 153, 46, 11, vehicleColor);
    sprite_.drawRoundRect(4, 2 + pedestrianOffset, 153, 46, 11, theme.text);
    sprite_.drawRoundRect(163, 2 + vehicleOffset, 153, 46, 11, theme.text);

    const uint16_t buttonText = enabled ? COLOR_DARK : COLOR_MUTED;
    sprite_.setTextColor(buttonText);
    sprite_.setTextDatum(MC_DATUM);
    sprite_.fillCircle(35, 19 + pedestrianOffset, 5, buttonText);
    sprite_.drawLine(35, 24 + pedestrianOffset, 35, 34 + pedestrianOffset, buttonText);
    sprite_.drawLine(35, 27 + pedestrianOffset, 26, 32 + pedestrianOffset, buttonText);
    sprite_.drawLine(35, 27 + pedestrianOffset, 44, 32 + pedestrianOffset, buttonText);
    sprite_.drawString("PIETON", 100, 25 + pedestrianOffset, 4);

    sprite_.drawRoundRect(181, 19 + vehicleOffset, 30, 16, 5, buttonText);
    sprite_.drawLine(186, 19 + vehicleOffset, 191, 13 + vehicleOffset, buttonText);
    sprite_.drawLine(191, 13 + vehicleOffset, 203, 13 + vehicleOffset, buttonText);
    sprite_.drawLine(203, 13 + vehicleOffset, 208, 19 + vehicleOffset, buttonText);
    sprite_.fillCircle(188, 36 + vehicleOffset, 3, buttonText);
    sprite_.fillCircle(204, 36 + vehicleOffset, 3, buttonText);
    sprite_.drawString("VOITURE", 264, 25 + vehicleOffset, 4);

    if (portal.busy || animation.feedbackActive(nowMs))
    {
        const bool accepted = animation.feedbackAccepted();
        sprite_.fillRoundRect(106, 38, 108, 13, 5, theme.cardAlt);
        sprite_.setTextDatum(MC_DATUM);
        sprite_.setTextColor(accepted ? theme.portalClosed : theme.portalOpen, theme.cardAlt);
        sprite_.drawString(portal.busy ? "ENVOI EN COURS" : (accepted ? "REQUETE ENVOYEE" : "ECHEC ENVOI"), 160, 44, 1);
    }
    pushSprite(HEADER_HEIGHT + FORECAST_HEIGHT + PORTAL_HEIGHT);
}

void DashboardScreen::drawFooter(
    const MoonSnapshot& moon,
    const WifiSnapshot& wifi,
    const PortalSnapshot& portal,
    const WeatherSnapshot& weather,
    time_t now,
    const ThemePalette& theme
)
{
    prepareSprite(FOOTER_HEIGHT, theme.cardAlt);
    sprite_.drawFastHLine(0, 0, 320, theme.accent);
    drawMoonIcon(moon, 17, 19, 11, theme.cardAlt);
    sprite_.setTextDatum(TL_DATUM);
    sprite_.setTextColor(theme.text, theme.cardAlt);
    sprite_.drawString("LUNE", 33, 5, 1);
    char moonText[38];
    snprintf(moonText, sizeof(moonText), "%s %d%%", moon.name, moon.illumination);
    sprite_.drawString(moonText, 33, 18, 1);

    drawStatusDot(175, 10, wifi.connected, theme.cardAlt);
    sprite_.setTextColor(theme.muted, theme.cardAlt);
    sprite_.drawString("WiFi", 184, 4, 1);
    drawStatusDot(228, 10, portal.apiOnline, theme.cardAlt);
    sprite_.drawString("API", 237, 4, 1);

    char updated[8];
    const time_t timestamp = weather.updatedAt > 0 ? weather.updatedAt : now;
    formatClock(timestamp, "%H:%M", updated, sizeof(updated));
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "MAJ %s", updated);
    sprite_.setTextDatum(TR_DATUM);
    sprite_.drawString(buffer, 315, 20, 1);
    if (weather.stale)
    {
        sprite_.setTextColor(TFT_ORANGE, theme.cardAlt);
        sprite_.drawString("ANCIENNE", 222, 20, 1);
    }
    pushSprite(HEADER_HEIGHT + FORECAST_HEIGHT + PORTAL_HEIGHT + CONTROLS_HEIGHT);
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
        || (minute >= sunset - 30 && minute <= sunset + 30))
    {
        return DayPhase::Twilight;
    }
    return minute > sunrise + 30 && minute < sunset - 30 ? DayPhase::Day : DayPhase::Night;
}

ThemePalette DashboardScreen::paletteFor(DayPhase phase) const
{
    if (phase == DayPhase::Night)
    {
        return {COLOR_NIGHT_BG, COLOR_NIGHT_HEADER, COLOR_NIGHT_CARD, COLOR_NIGHT_CARD,
                TFT_WHITE, COLOR_MUTED, COLOR_TURQUOISE, COLOR_GREEN, COLOR_RED};
    }
    if (phase == DayPhase::Twilight)
    {
        return {COLOR_TWILIGHT_BG, COLOR_TWILIGHT_HEADER, COLOR_SAND, COLOR_DARK,
                TFT_WHITE, COLOR_MUTED, COLOR_SOLAR, COLOR_GREEN, COLOR_RED};
    }
    return {COLOR_DAY_BG, COLOR_DAY_HEADER, COLOR_DAY_CARD, COLOR_DAY_CARD_ALT,
            TFT_WHITE, COLOR_DARK, COLOR_SOLAR, COLOR_GREEN, COLOR_RED};
}

ThemePalette DashboardScreen::blendPalette(
    const ThemePalette& from,
    const ThemePalette& to,
    uint8_t progress
) const
{
    return {
        blendColor(from.background, to.background, progress),
        blendColor(from.header, to.header, progress),
        blendColor(from.card, to.card, progress),
        blendColor(from.cardAlt, to.cardAlt, progress),
        blendColor(from.text, to.text, progress),
        blendColor(from.muted, to.muted, progress),
        blendColor(from.accent, to.accent, progress),
        blendColor(from.portalClosed, to.portalClosed, progress),
        blendColor(from.portalOpen, to.portalOpen, progress)
    };
}

bool DashboardScreen::inPedestrianButton(int16_t x, int16_t y)
{
    return x >= 4 && x <= 157 && y >= 151 && y <= 201;
}

bool DashboardScreen::inVehicleButton(int16_t x, int16_t y)
{
    return x >= 163 && x <= 316 && y >= 151 && y <= 201;
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

void DashboardScreen::drawCard(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t fill, uint16_t border)
{
    sprite_.fillRoundRect(x + 1, y + 2, width, height, 9, scaleColor(fill, 115));
    sprite_.fillRoundRect(x, y, width, height, 9, fill);
    sprite_.drawRoundRect(x, y, width, height, 9, border);
}

void DashboardScreen::drawWeatherIcon(int16_t code, int16_t x, int16_t y, int16_t size, uint16_t background)
{
    const int16_t radius = size / 4 > 3 ? size / 4 : 3;
    if (code == 0)
    {
        sprite_.fillCircle(x, y, radius, COLOR_SOLAR);
        sprite_.drawFastHLine(x - size / 2, y, size, COLOR_SOLAR);
        sprite_.drawFastVLine(x, y - size / 2, size, COLOR_SOLAR);
        return;
    }
    if (code == 45 || code == 48)
    {
        for (int16_t offset = -5; offset <= 5; offset += 5)
            sprite_.drawFastHLine(x - size / 2, y + offset, size, COLOR_MUTED);
        return;
    }

    sprite_.fillCircle(x - 4, y, radius, COLOR_MUTED);
    sprite_.fillCircle(x + 3, y - 3, radius + 1, COLOR_MUTED);
    sprite_.fillRect(x - 8, y, 17, 5, COLOR_MUTED);
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82))
    {
        sprite_.drawLine(x - 5, y + 7, x - 7, y + 12, COLOR_TURQUOISE);
        sprite_.drawLine(x + 2, y + 7, x, y + 12, COLOR_TURQUOISE);
    }
    else if ((code >= 71 && code <= 77) || code == 85 || code == 86)
    {
        sprite_.fillCircle(x - 4, y + 10, 2, TFT_WHITE);
        sprite_.fillCircle(x + 4, y + 10, 2, TFT_WHITE);
    }
    else if (code >= 95)
    {
        sprite_.drawLine(x + 1, y + 5, x - 3, y + 13, COLOR_SOLAR);
    }
    (void)background;
}

void DashboardScreen::drawMoonIcon(const MoonSnapshot& moon, int16_t x, int16_t y, int16_t radius, uint16_t background)
{
    sprite_.fillCircle(x, y, radius, COLOR_SAND);
    if (moon.cycleFraction < 0.5F)
    {
        const int16_t offset = static_cast<int16_t>(round(radius * 2.0F * moon.cycleFraction / 0.5F));
        sprite_.fillCircle(x - offset, y, radius, background);
    }
    else
    {
        const int16_t offset = static_cast<int16_t>(round(radius * 2.0F * (1.0F - moon.cycleFraction) / 0.5F));
        sprite_.fillCircle(x + offset, y, radius, background);
    }
    sprite_.drawCircle(x, y, radius, TFT_WHITE);
}

void DashboardScreen::drawStatusDot(int16_t x, int16_t y, bool ok, uint16_t background)
{
    sprite_.fillCircle(x, y, 5, ok ? COLOR_GREEN : COLOR_RED);
    sprite_.drawCircle(x, y, 6, background);
}

const char* DashboardScreen::weatherLabel(int16_t code) const
{
    if (code == 0) return "Ciel degage";
    if (code <= 2) return "Peu nuageux";
    if (code == 3) return "Couvert";
    if (code == 45 || code == 48) return "Brouillard";
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return "Pluie";
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "Neige";
    if (code >= 95) return "Orage";
    return "Variable";
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
    const int16_t fromRed = (from >> 11) & 0x1F;
    const int16_t fromGreen = (from >> 5) & 0x3F;
    const int16_t fromBlue = from & 0x1F;
    const int16_t toRed = (to >> 11) & 0x1F;
    const int16_t toGreen = (to >> 5) & 0x3F;
    const int16_t toBlue = to & 0x1F;
    const uint16_t red = fromRed + (toRed - fromRed) * progress / 255;
    const uint16_t green = fromGreen + (toGreen - fromGreen) * progress / 255;
    const uint16_t blue = fromBlue + (toBlue - fromBlue) * progress / 255;
    return (red << 11) | (green << 5) | blue;
}
