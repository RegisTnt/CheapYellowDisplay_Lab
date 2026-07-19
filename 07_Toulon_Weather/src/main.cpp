#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

#if __has_include("config.h")
#include "config.h"
#else
#include "config.example.h"
#endif

#include "MoonPhase.h"
#include "WeatherClient.h"
#include "WeatherData.h"
#include "WeatherScreen.h"

namespace
{

constexpr char TIME_ZONE[] = "CET-1CEST,M3.5.0,M10.5.0/3";
constexpr uint32_t WEATHER_REFRESH_MS = 15UL * 60UL * 1000UL;
constexpr uint32_t WEATHER_RETRY_MS = 60UL * 1000UL;
constexpr uint32_t WIFI_RETRY_MS = 30UL * 1000UL;

TFT_eSPI display;
WeatherScreen weatherScreen(display);
WeatherClient weatherClient;
WeatherData weatherData = {};

uint32_t nextWeatherUpdate = 0;
uint32_t nextWifiAttempt = 0;
bool wasConnected = false;
bool lastRenderOffline = false;
bool lastRenderStale = false;
bool clockWasReady = false;

time_t currentTimestamp()
{
    const time_t now = time(nullptr);
    return now > 1600000000 ? now : 0;
}

time_t dashboardTimestamp()
{
    const time_t now = currentTimestamp();
    if (now != 0)
    {
        return now;
    }

#ifdef WEATHER_DEMO_MODE
    struct tm demoDate = {};
    demoDate.tm_year = 2026 - 1900;
    demoDate.tm_mon = 6;
    demoDate.tm_mday = 19;
    demoDate.tm_hour = 21;
    demoDate.tm_min = 45;
    demoDate.tm_isdst = -1;
    return mktime(&demoDate);
#else
    return 0;
#endif
}

void renderWeather(bool offline, bool stale)
{
    const time_t now = dashboardTimestamp();
    const MoonPhaseInfo moon = calculateMoonPhase(now);
    weatherScreen.drawDashboard(weatherData, moon, now, offline, stale);
    lastRenderOffline = offline;
    lastRenderStale = stale;
}

#ifndef WEATHER_DEMO_MODE
void startWifiConnection()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    nextWifiAttempt = millis() + WIFI_RETRY_MS;
    if (!weatherData.valid)
    {
        weatherScreen.drawStatus("Connexion Wi-Fi...", TFT_YELLOW);
    }
}

void refreshWeather()
{
    Serial.println("Open-Meteo: mise a jour...");
    if (weatherClient.fetch(weatherData))
    {
        Serial.println("Open-Meteo: donnees recues.");
        nextWeatherUpdate = millis() + WEATHER_REFRESH_MS;
        renderWeather(false, false);
    }
    else
    {
        Serial.print("Open-Meteo: ");
        Serial.println(weatherClient.lastError());
        nextWeatherUpdate = millis() + WEATHER_RETRY_MS;
        if (weatherData.valid)
        {
            renderWeather(false, true);
        }
        else
        {
            weatherScreen.drawStatus("Erreur meteo", TFT_ORANGE);
        }
    }
}
#endif

}  // namespace

void setup()
{
    Serial.begin(115200);
    delay(100);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    display.init();
    display.setRotation(1);
    weatherScreen.begin();

    setenv("TZ", TIME_ZONE, 1);
    tzset();

#ifdef WEATHER_DEMO_MODE
    WeatherClient::loadDemoData(weatherData, WEATHER_DEMO_SCENARIO);
    weatherData.updatedAt = dashboardTimestamp();
    renderWeather(true, false);
    Serial.printf("Mode demo meteo, scenario %d.\n", WEATHER_DEMO_SCENARIO);
#else
    weatherScreen.drawStatus("Demarrage...", TFT_WHITE);
    startWifiConnection();
#endif
}

void loop()
{
#ifdef WEATHER_DEMO_MODE
    delay(50);
#else
    const uint32_t now = millis();
    const bool connected = WiFi.status() == WL_CONNECTED;
    const bool clockReady = currentTimestamp() != 0;

    if (connected && !wasConnected)
    {
        Serial.print("Wi-Fi connecte: ");
        Serial.println(WiFi.localIP());
        configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");
        nextWeatherUpdate = 0;
    }
    else if (!connected && wasConnected)
    {
        Serial.println("Wi-Fi hors ligne.");
        if (weatherData.valid)
        {
            renderWeather(true, true);
        }
        else
        {
            weatherScreen.drawStatus("Hors ligne", TFT_ORANGE);
        }
    }

    if (!connected && static_cast<int32_t>(now - nextWifiAttempt) >= 0)
    {
        startWifiConnection();
    }

    if (connected && static_cast<int32_t>(now - nextWeatherUpdate) >= 0)
    {
        refreshWeather();
    }

    if (connected && lastRenderOffline && weatherData.valid)
    {
        renderWeather(false, true);
    }

    if (clockReady && !clockWasReady && weatherData.valid)
    {
        if (weatherData.updatedAt == 0)
        {
            weatherData.updatedAt = currentTimestamp();
        }
        renderWeather(lastRenderOffline, lastRenderStale);
    }

    wasConnected = connected;
    clockWasReady = clockReady;
    delay(20);
#endif
}
