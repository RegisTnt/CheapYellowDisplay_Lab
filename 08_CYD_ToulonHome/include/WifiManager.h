#pragma once

#include "AppData.h"

class WifiManager
{
public:
    void begin(const char* ssid, const char* password, bool demoMode, uint8_t demoScenario);
    void update(uint32_t nowMs);
    WifiSnapshot snapshot() const;

private:
    void publish(bool connected, bool connecting, int32_t rssi);

    const char* ssid_ = nullptr;
    const char* password_ = nullptr;
    bool demoMode_ = false;
    uint8_t demoScenario_ = 0;
    uint32_t nextAttemptMs_ = 0;
    WifiSnapshot snapshot_ = {};
};
