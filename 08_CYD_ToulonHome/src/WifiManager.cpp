#include "WifiManager.h"

#include <WiFi.h>

namespace
{
constexpr uint32_t WIFI_RETRY_MS = 30000;
}

void WifiManager::begin(const char* ssid, const char* password, bool demoMode, uint8_t demoScenario)
{
    ssid_ = ssid;
    password_ = password;
    demoMode_ = demoMode;
    demoScenario_ = demoScenario;

    if (demoMode_)
    {
        const bool connected = demoScenario_ != 4;
        publish(connected, false, connected ? -48 : 0);
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid_, password_);
    nextAttemptMs_ = millis() + WIFI_RETRY_MS;
    publish(false, true, 0);
}

void WifiManager::update(uint32_t nowMs)
{
    if (demoMode_)
    {
        return;
    }

    const bool connected = WiFi.status() == WL_CONNECTED;
    if (connected)
    {
        publish(true, false, WiFi.RSSI());
        return;
    }

    publish(false, true, 0);
    if (static_cast<int32_t>(nowMs - nextAttemptMs_) >= 0)
    {
        WiFi.disconnect();
        WiFi.begin(ssid_, password_);
        nextAttemptMs_ = nowMs + WIFI_RETRY_MS;
    }
}

WifiSnapshot WifiManager::snapshot() const
{
    return snapshot_;
}

void WifiManager::publish(bool connected, bool connecting, int32_t rssi)
{
    if (snapshot_.connected == connected
        && snapshot_.connecting == connecting
        && snapshot_.rssi == rssi)
    {
        return;
    }
    snapshot_.connected = connected;
    snapshot_.connecting = connecting;
    snapshot_.rssi = rssi;
    ++snapshot_.revision;
}
