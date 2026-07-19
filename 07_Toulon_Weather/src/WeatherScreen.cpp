#include "WeatherScreen.h"

#include <cmath>
#include <cstring>

namespace
{

constexpr uint16_t COLOR_AZURE = 0x049F;
constexpr uint16_t COLOR_TURQUOISE = 0x2EB7;
constexpr uint16_t COLOR_SOLAR = 0xFEAA;
constexpr uint16_t COLOR_SAND = 0xFF38;
constexpr uint16_t COLOR_CARD = 0x11EA;
constexpr uint16_t COLOR_NIGHT = 0x18B1;
constexpr uint16_t COLOR_TEXT_DARK = 0x0186;

void formatLocalTime(time_t timestamp, const char* format, char* output, size_t size)
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

}  // namespace

WeatherScreen::WeatherScreen(TFT_eSPI& display)
    : display_(display)
{
}

void WeatherScreen::begin()
{
    display_.fillScreen(COLOR_AZURE);
    display_.setTextWrap(false);
}

void WeatherScreen::drawStatus(const char* message, uint16_t color)
{
    display_.fillScreen(COLOR_AZURE);
    display_.setTextDatum(MC_DATUM);
    display_.setTextColor(TFT_WHITE, COLOR_AZURE);
    display_.drawString("TOULON METEO", 160, 72, 4);
    display_.setTextColor(color, COLOR_AZURE);
    display_.drawString(message, 160, 130, 2);
    display_.setTextColor(COLOR_SAND, COLOR_AZURE);
    display_.drawString("ESP32-2432S028R", 160, 166, 2);
}

void WeatherScreen::drawDashboard(
    const WeatherData& data,
    const MoonPhaseInfo& moon,
    time_t localTime,
    bool offline,
    bool stale
)
{
    if (!data.valid)
    {
        drawStatus("METEO INDISPONIBLE", TFT_ORANGE);
        return;
    }

    display_.fillScreen(COLOR_AZURE);

    char buffer[40];
    char clockText[8];
    formatLocalTime(localTime, "%H:%M", clockText, sizeof(clockText));

    display_.setTextDatum(TL_DATUM);
    display_.setTextColor(TFT_WHITE, COLOR_AZURE);
    display_.drawString("TOULON", 8, 5, 2);
    snprintf(buffer, sizeof(buffer), "%.0f C", data.currentTemperature);
    display_.setTextDatum(TC_DATUM);
    display_.drawString(buffer, 160, 2, 4);
    display_.setTextDatum(TR_DATUM);
    display_.drawString(clockText, 312, 7, 2);

    display_.setTextDatum(TL_DATUM);
    snprintf(buffer, sizeof(buffer), "%s  Ressenti %.0f C", weatherLabel(data.currentWeatherCode), data.apparentTemperature);
    display_.drawString(buffer, 8, 31, 2);
    snprintf(buffer, sizeof(buffer), "Hum %d%%  Vent %.0f km/h", data.currentHumidity, data.currentWindSpeed);
    display_.setTextColor(COLOR_SAND, COLOR_AZURE);
    display_.drawString(buffer, 8, 48, 1);

    if (offline || stale)
    {
        display_.setTextDatum(TR_DATUM);
        display_.setTextColor(TFT_ORANGE, COLOR_AZURE);
        display_.drawString(offline ? "HORS LIGNE" : "DONNEES ANCIENNES", 312, 48, 1);
    }

    drawCard(4, 64, 154, 74, COLOR_NIGHT);
    display_.setTextDatum(TL_DATUM);
    display_.setTextColor(COLOR_TURQUOISE, COLOR_NIGHT);
    display_.drawString("CETTE NUIT", 12, 70, 2);
    drawWeatherIcon(data.nightWeatherCode, 130, 80, 18);
    snprintf(buffer, sizeof(buffer), "%.0f C", data.nightTemperature);
    display_.setTextColor(TFT_WHITE, COLOR_NIGHT);
    display_.drawString(buffer, 12, 91, 4);
    display_.drawString(weatherLabel(data.nightWeatherCode), 12, 118, 1);
    snprintf(buffer, sizeof(buffer), "Pluie %d%%", data.nightPrecipitationProbability);
    display_.setTextDatum(TR_DATUM);
    display_.drawString(buffer, 150, 120, 1);

    drawCard(162, 64, 154, 74, COLOR_CARD);
    display_.setTextDatum(TL_DATUM);
    display_.setTextColor(COLOR_SOLAR, COLOR_CARD);
    display_.drawString("DEMAIN", 170, 70, 2);
    drawWeatherIcon(data.tomorrowWeatherCode, 288, 80, 18);
    snprintf(buffer, sizeof(buffer), "%.0f / %.0f C", data.tomorrowMaxTemperature, data.tomorrowMinTemperature);
    display_.setTextColor(TFT_WHITE, COLOR_CARD);
    display_.drawString(buffer, 170, 91, 4);
    display_.drawString(weatherLabel(data.tomorrowWeatherCode), 170, 118, 1);
    snprintf(buffer, sizeof(buffer), "Pluie %d%%", data.tomorrowPrecipitationProbability);
    display_.setTextDatum(TR_DATUM);
    display_.drawString(buffer, 308, 120, 1);

    drawCard(4, 142, 154, 94, COLOR_CARD);
    display_.setTextDatum(TL_DATUM);
    display_.setTextColor(COLOR_SAND, COLOR_CARD);
    display_.drawString("LUNE", 12, 148, 2);
    drawMoonIcon(moon, 31, 188, 18);
    display_.setTextColor(TFT_WHITE, COLOR_CARD);
    display_.drawString(shortMoonLabel(moon.name), 56, 172, 1);
    snprintf(buffer, sizeof(buffer), "%d %%", moon.illuminationPercent);
    display_.drawString(buffer, 56, 190, 4);

    drawCard(162, 142, 154, 94, COLOR_CARD);
    display_.setTextDatum(TL_DATUM);
    display_.setTextColor(COLOR_SOLAR, COLOR_CARD);
    display_.drawString("SOLEIL", 170, 148, 2);
    display_.setTextColor(TFT_WHITE, COLOR_CARD);
    snprintf(buffer, sizeof(buffer), "Lever   %s", data.sunrise);
    display_.drawString(buffer, 170, 172, 2);
    snprintf(buffer, sizeof(buffer), "Coucher %s", data.sunset);
    display_.drawString(buffer, 170, 194, 2);

    char updateText[20];
    formatLocalTime(data.updatedAt, "%d/%m %H:%M", updateText, sizeof(updateText));
    snprintf(buffer, sizeof(buffer), "Maj %s", updateText);
    display_.setTextColor(TFT_LIGHTGREY, COLOR_CARD);
    display_.drawString(buffer, 170, 219, 1);
}

void WeatherScreen::drawCard(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
{
    display_.fillRoundRect(x, y, width, height, 8, color);
    display_.drawRoundRect(x, y, width, height, 8, COLOR_TURQUOISE);
}

void WeatherScreen::drawWeatherIcon(int16_t code, int16_t x, int16_t y, int16_t size)
{
    const int16_t radius = size / 3;
    if (code == 0)
    {
        display_.fillCircle(x, y, radius, COLOR_SOLAR);
        display_.drawFastHLine(x - size / 2, y, size, COLOR_SOLAR);
        display_.drawFastVLine(x, y - size / 2, size, COLOR_SOLAR);
        return;
    }

    if (code == 45 || code == 48)
    {
        for (int16_t offset = -5; offset <= 5; offset += 5)
        {
            display_.drawFastHLine(x - size / 2, y + offset, size, TFT_LIGHTGREY);
        }
        return;
    }

    display_.fillCircle(x - 4, y, radius, TFT_LIGHTGREY);
    display_.fillCircle(x + 3, y - 3, radius + 1, TFT_LIGHTGREY);
    display_.fillRect(x - 8, y, 17, 6, TFT_LIGHTGREY);

    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82))
    {
        display_.drawLine(x - 5, y + 8, x - 7, y + 12, COLOR_TURQUOISE);
        display_.drawLine(x + 2, y + 8, x, y + 12, COLOR_TURQUOISE);
    }
    else if ((code >= 71 && code <= 77) || code == 85 || code == 86)
    {
        display_.fillCircle(x - 4, y + 10, 2, TFT_WHITE);
        display_.fillCircle(x + 4, y + 11, 2, TFT_WHITE);
    }
    else if (code >= 95)
    {
        display_.drawLine(x + 1, y + 6, x - 3, y + 13, COLOR_SOLAR);
        display_.drawLine(x - 3, y + 13, x + 4, y + 10, COLOR_SOLAR);
    }
}

void WeatherScreen::drawMoonIcon(const MoonPhaseInfo& moon, int16_t x, int16_t y, int16_t radius)
{
    display_.fillCircle(x, y, radius, COLOR_SOLAR);
    const float phase = moon.cycleFraction;
    if (phase < 0.02F || phase > 0.98F)
    {
        display_.fillCircle(x, y, radius - 1, COLOR_CARD);
    }
    else if (phase < 0.5F)
    {
        const int16_t offset = static_cast<int16_t>(round(2.0F * radius * phase / 0.5F));
        display_.fillCircle(x - offset, y, radius, COLOR_CARD);
    }
    else if (phase > 0.5F)
    {
        const int16_t offset = static_cast<int16_t>(round(2.0F * radius * (1.0F - phase) / 0.5F));
        display_.fillCircle(x + offset, y, radius, COLOR_CARD);
    }
    display_.drawCircle(x, y, radius, TFT_WHITE);
}

const char* WeatherScreen::weatherLabel(int16_t code) const
{
    if (code == 0) return "Ciel clair";
    if (code <= 2) return "Peu nuageux";
    if (code == 3) return "Couvert";
    if (code == 45 || code == 48) return "Brouillard";
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return "Pluie";
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "Neige";
    if (code >= 95) return "Orage";
    return "Variable";
}

const char* WeatherScreen::shortMoonLabel(const char* name) const
{
    if (strcmp(name, "Lune gibbeuse croissante") == 0) return "Gibbeuse crois.";
    if (strcmp(name, "Lune gibbeuse decroissante") == 0) return "Gibbeuse dec.";
    if (strcmp(name, "Premier croissant") == 0) return "1er croissant";
    if (strcmp(name, "Premier quartier") == 0) return "1er quartier";
    if (strcmp(name, "Dernier quartier") == 0) return "Dernier quart.";
    if (strcmp(name, "Dernier croissant") == 0) return "Dernier croiss.";
    return name;
}
