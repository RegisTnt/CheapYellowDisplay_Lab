#pragma once

#include <Arduino.h>
#include <time.h>

enum class PortalPosition : uint8_t
{
    Unknown,
    Closed,
    Open
};

enum class CommandFeedback : uint8_t
{
    None,
    Sent,
    Failed
};

struct WeatherSnapshot
{
    bool valid = false;
    bool stale = false;
    bool fetching = false;
    float temperature = 0.0F;
    float apparentTemperature = 0.0F;
    int16_t weatherCode = 0;
    float nightTemperature = 0.0F;
    int16_t nightWeatherCode = 0;
    int16_t nightRain = 0;
    float tomorrowMin = 0.0F;
    float tomorrowMax = 0.0F;
    int16_t tomorrowWeatherCode = 0;
    int16_t tomorrowRain = 0;
    char sunrise[6] = "--:--";
    char sunset[6] = "--:--";
    time_t updatedAt = 0;
    uint32_t revision = 0;
};

struct PortalSnapshot
{
    PortalPosition position = PortalPosition::Unknown;
    bool apiOnline = false;
    bool busy = false;
    bool cooldown = false;
    CommandFeedback feedback = CommandFeedback::None;
    time_t updatedAt = 0;
    uint32_t openSinceMs = 0;
    uint32_t revision = 0;
};

struct WifiSnapshot
{
    bool connected = false;
    bool connecting = false;
    int32_t rssi = 0;
    uint32_t revision = 0;
};

struct MoonSnapshot
{
    const char* name = "Nouvelle lune";
    uint8_t illumination = 0;
    float cycleFraction = 0.0F;
};

enum class DayPhase : uint8_t
{
    Day,
    Twilight,
    Night
};

struct ThemePalette
{
    uint16_t background;
    uint16_t header;
    uint16_t card;
    uint16_t cardAlt;
    uint16_t text;
    uint16_t muted;
    uint16_t accent;
    uint16_t portalClosed;
    uint16_t portalOpen;
};
