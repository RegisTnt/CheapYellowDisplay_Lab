#include <Arduino.h>
#include <ArduinoOTA.h>
#include <TFT_eSPI.h>

#if __has_include("ToulonHomeConfig.h")
#include "ToulonHomeConfig.h"
#else
#include "config.example.h"
#endif

#include "AnimationManager.h"
#include "ClockManager.h"
#include "DashboardScreen.h"
#include "MoonCalculator.h"
#include "PortalClient.h"
#include "TouchManager.h"
#include "WeatherManager.h"
#include "WifiManager.h"

#ifndef DEMO_SCENARIO
#define DEMO_SCENARIO 0
#endif

namespace
{
#ifdef DEMO_MODE
constexpr bool IS_DEMO = true;
#else
constexpr bool IS_DEMO = false;
#endif

TFT_eSPI display;
DashboardScreen dashboard(display);
WifiManager wifiManager;
ClockManager clockManager;
WeatherManager weatherManager;
PortalClient portalClient;
MoonCalculator moonCalculator;
TouchManager touchManager;
AnimationManager animationManager;

AnimationManager::Button capturedButton = AnimationManager::Button::None;
uint32_t lastWeatherRevision = UINT32_MAX;
uint32_t lastPortalRevision = UINT32_MAX;
uint32_t lastWifiRevision = UINT32_MAX;
time_t lastHeaderSecond = -1;
int16_t lastMoonDay = -1;
DayPhase lastPhase = static_cast<DayPhase>(255);
CommandFeedback lastFeedback = CommandFeedback::None;
MoonSnapshot moonSnapshot;
bool otaStarted = false;
bool appReady = false;
bool themeTransitionActive = false;
uint32_t themeTransitionStartMs = 0;
uint32_t lastThemeFrameMs = 0;
ThemePalette currentPalette = {};
ThemePalette transitionFrom = {};
ThemePalette transitionTo = {};

enum class ScreenPage : uint8_t { Home, Weather };
ScreenPage currentPage = ScreenPage::Home;
uint32_t lastInteractionMs = 0;
bool fullRedrawRequired = false;
bool screenSleeping = false;
constexpr uint32_t WEATHER_TIMEOUT_MS = 20000;
constexpr uint32_t SCREEN_SLEEP_TIMEOUT_MS = 60000;

void setBacklight(bool enabled)
{
    digitalWrite(TFT_BL, enabled ? TFT_BACKLIGHT_ON : !TFT_BACKLIGHT_ON);
}

void wakeScreen(uint32_t nowMs, bool userInitiated)
{
    if (!screenSleeping) return;
    screenSleeping = false;
    setBacklight(true);
    if (userInitiated) lastInteractionMs = nowMs;
    fullRedrawRequired = true;
    Serial.println(userInitiated ? "Ecran reveille par toucher." : "Ecran reveille par etat portail.");
}

void sleepScreen()
{
    if (screenSleeping) return;
    capturedButton = AnimationManager::Button::None;
    animationManager.release();
    screenSleeping = true;
    setBacklight(false);
    Serial.println("Ecran en veille: portail ferme confirme.");
}

void configureOta()
{
    if (IS_DEMO || otaStarted) return;
#ifdef OTA_PASSWORD
    ArduinoOTA.setHostname("cyd-toulonhome");
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
    otaStarted = true;
#endif
}

void refreshMoon(time_t now)
{
    if (now < 100000) return;
    struct tm local = {};
    localtime_r(&now, &local);
    if (local.tm_yday == lastMoonDay) return;
    lastMoonDay = local.tm_yday;
    moonSnapshot = moonCalculator.calculate(now);
}

void drawEverything(
    const WeatherSnapshot& weather,
    const PortalSnapshot& portal,
    const WifiSnapshot& wifi,
    time_t now,
    const ThemePalette& theme
)
{
    dashboard.drawHeader(weather, now, theme);
    dashboard.drawPortal(portal, animationManager.portalGlow(), theme);
    dashboard.drawControls(portal, animationManager, millis(), theme);
    dashboard.drawFooter(wifi, portal, weather, now, theme);
}

void drawCurrentPage(
    const WeatherSnapshot& weather,
    const PortalSnapshot& portal,
    const WifiSnapshot& wifi,
    time_t now,
    const ThemePalette& theme
)
{
    if (currentPage == ScreenPage::Weather)
        dashboard.drawWeatherDetails(weather, moonSnapshot, now, theme);
    else
        drawEverything(weather, portal, wifi, now, theme);
}

bool buttonStillContains(AnimationManager::Button button, int16_t x, int16_t y)
{
    if (button == AnimationManager::Button::Pedestrian)
        return DashboardScreen::inPedestrianButton(x, y);
    if (button == AnimationManager::Button::Vehicle)
        return DashboardScreen::inVehicleButton(x, y);
    return false;
}

void handleTouch(const TouchEvent& event, uint32_t nowMs)
{
    if (event.type == TouchEventType::None) return;

    if (event.type == TouchEventType::Down)
    {
        if (screenSleeping)
        {
            wakeScreen(nowMs, true);
            capturedButton = AnimationManager::Button::None;
            animationManager.release();
            return; // Le toucher de réveil est toujours consommé.
        }
        lastInteractionMs = nowMs;
        if (currentPage == ScreenPage::Weather) return;
        if (DashboardScreen::inWeatherBand(event.x, event.y))
        {
            currentPage = ScreenPage::Weather;
            capturedButton = AnimationManager::Button::None;
            animationManager.release();
            fullRedrawRequired = true;
            return;
        }
    }

    if (currentPage == ScreenPage::Weather) return;

    if (event.type == TouchEventType::Down)
    {
        if (!portalClient.canSend(nowMs)) return;
        if (DashboardScreen::inPedestrianButton(event.x, event.y))
            capturedButton = AnimationManager::Button::Pedestrian;
        else if (DashboardScreen::inVehicleButton(event.x, event.y))
            capturedButton = AnimationManager::Button::Vehicle;
        animationManager.press(capturedButton);
        return;
    }

    if (event.type == TouchEventType::Move && capturedButton != AnimationManager::Button::None)
    {
        animationManager.press(buttonStillContains(capturedButton, event.x, event.y)
            ? capturedButton
            : AnimationManager::Button::None);
        return;
    }

    if (event.type != TouchEventType::Up) return;
    const AnimationManager::Button released = capturedButton;
    const bool inside = buttonStillContains(released, event.x, event.y);
    capturedButton = AnimationManager::Button::None;
    animationManager.release();
    if (!inside) return;

    const bool started = released == AnimationManager::Button::Pedestrian
        ? portalClient.requestPedestrian(nowMs)
        : portalClient.requestVehicle(nowMs);
    if (!started) animationManager.commandResult(false, nowMs);
}
}

void setup()
{
    Serial.begin(115200);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    display.init();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);

    if (!dashboard.begin())
    {
        display.setTextDatum(MC_DATUM);
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.drawString("ERREUR MEMOIRE SPRITE", 160, 120, 2);
        Serial.println("Allocation du sprite impossible.");
        return;
    }

    const bool touchReady = touchManager.begin(display.width(), display.height());
    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD, IS_DEMO, DEMO_SCENARIO);
    clockManager.begin(IS_DEMO, DEMO_SCENARIO);
    weatherManager.begin(IS_DEMO, DEMO_SCENARIO);
    portalClient.begin(PORTAL_BASE_URL, IS_DEMO, DEMO_SCENARIO);
    const time_t now = clockManager.now();
    refreshMoon(now);
    const WeatherSnapshot weather = weatherManager.snapshot();
    const PortalSnapshot portal = portalClient.snapshot();
    const WifiSnapshot wifi = wifiManager.snapshot();
    const DayPhase phase = dashboard.dayPhase(weather, now);
    currentPalette = dashboard.paletteFor(phase);
    drawCurrentPage(weather, portal, wifi, now, currentPalette);

    lastWeatherRevision = weather.revision;
    lastPortalRevision = portal.revision;
    lastWifiRevision = wifi.revision;
    lastHeaderSecond = now;
    lastPhase = phase;
    appReady = true;
    Serial.printf("CYD Toulon Home pret. Mode demo=%s, tactile=%s\n",
        IS_DEMO ? "oui" : "non", touchReady ? "ok" : "erreur");
}

void loop()
{
    if (!appReady)
    {
        yield();
        return;
    }
    const uint32_t nowMs = millis();
    wifiManager.update(nowMs);
    const WifiSnapshot wifi = wifiManager.snapshot();
    const bool secondChanged = clockManager.update(wifi.connected);
    const time_t now = clockManager.now();
    weatherManager.update(nowMs, wifi.connected);
    portalClient.update(nowMs, wifi.connected, now);
    handleTouch(touchManager.update(nowMs), nowMs);

    if (!IS_DEMO && wifi.connected)
    {
        configureOta();
        ArduinoOTA.handle();
    }

    WeatherSnapshot weather = weatherManager.snapshot();
    PortalSnapshot portal = portalClient.snapshot();
    refreshMoon(now);

    const bool portalClosedConfirmed = portal.apiOnline
        && portal.position == PortalPosition::Closed;
    if (screenSleeping)
    {
        if (!portalClosedConfirmed)
            wakeScreen(nowMs, false);
        else
        {
            yield();
            return;
        }
    }

    if (currentPage == ScreenPage::Home
        && portalClosedConfirmed
        && nowMs - lastInteractionMs >= SCREEN_SLEEP_TIMEOUT_MS)
    {
        sleepScreen();
        yield();
        return;
    }

    if (currentPage == ScreenPage::Weather
        && nowMs - lastInteractionMs >= WEATHER_TIMEOUT_MS)
    {
        currentPage = ScreenPage::Home;
        fullRedrawRequired = true;
    }

    const bool portalAlert = portal.position == PortalPosition::Open
        && portal.openSinceMs != 0
        && nowMs - portal.openSinceMs >= 30000;
    const bool animationChanged = animationManager.update(nowMs, portalAlert);

    if (portal.feedback != lastFeedback && portal.feedback != CommandFeedback::None)
    {
        animationManager.commandResult(portal.feedback == CommandFeedback::Sent, nowMs);
    }
    lastFeedback = portal.feedback;

    const DayPhase phase = dashboard.dayPhase(weather, now);
    if (phase != lastPhase)
    {
        transitionFrom = currentPalette;
        transitionTo = dashboard.paletteFor(phase);
        themeTransitionStartMs = nowMs;
        lastThemeFrameMs = 0;
        themeTransitionActive = true;
        lastPhase = phase;
    }

    if (themeTransitionActive)
    {
        const uint32_t elapsed = nowMs - themeTransitionStartMs;
        const uint8_t progress = elapsed >= 1200 ? 255 : static_cast<uint8_t>(elapsed * 255 / 1200);
        currentPalette = dashboard.blendPalette(transitionFrom, transitionTo, progress);
        if (lastThemeFrameMs == 0 || nowMs - lastThemeFrameMs >= 100 || progress == 255)
        {
            drawCurrentPage(weather, portal, wifi, now, currentPalette);
            lastThemeFrameMs = nowMs;
        }
        lastWeatherRevision = weather.revision;
        lastPortalRevision = portal.revision;
        lastWifiRevision = wifi.revision;
        lastHeaderSecond = now;
        if (progress == 255) themeTransitionActive = false;
        return;
    }

    const ThemePalette& theme = currentPalette;

    if (fullRedrawRequired)
    {
        drawCurrentPage(weather, portal, wifi, now, theme);
        lastWeatherRevision = weather.revision;
        lastPortalRevision = portal.revision;
        lastWifiRevision = wifi.revision;
        lastHeaderSecond = now;
        fullRedrawRequired = false;
        return;
    }

    if (currentPage == ScreenPage::Weather)
    {
        if (secondChanged || now != lastHeaderSecond || weather.revision != lastWeatherRevision)
        {
            dashboard.drawWeatherDetails(weather, moonSnapshot, now, theme);
            lastHeaderSecond = now;
            lastWeatherRevision = weather.revision;
        }
        lastPortalRevision = portal.revision;
        lastWifiRevision = wifi.revision;
        yield();
        return;
    }

    const bool weatherChanged = weather.revision != lastWeatherRevision;
    const bool portalChanged = portal.revision != lastPortalRevision;
    const bool wifiChanged = wifi.revision != lastWifiRevision;

    if (secondChanged || now != lastHeaderSecond)
    {
        dashboard.drawHeader(weather, now, theme);
        lastHeaderSecond = now;
    }
    if (weatherChanged)
    {
        dashboard.drawHeader(weather, now, theme);
        lastWeatherRevision = weather.revision;
    }
    if (portalChanged || animationChanged)
    {
        dashboard.drawPortal(portal, animationManager.portalGlow(), theme);
        dashboard.drawControls(portal, animationManager, nowMs, theme);
        lastPortalRevision = portal.revision;
    }
    if (wifiChanged || portalChanged || weatherChanged || secondChanged)
    {
        dashboard.drawFooter(wifi, portal, weather, now, theme);
        lastWifiRevision = wifi.revision;
    }
    yield();
}
