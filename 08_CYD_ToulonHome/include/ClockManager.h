#pragma once

#include <Arduino.h>
#include <time.h>

class ClockManager
{
public:
    void begin(bool demoMode, uint8_t demoScenario);
    bool update(bool wifiConnected);
    time_t now() const;
    bool ready() const;
    void formatTime(char output[6]) const;
    void formatDate(char* output, size_t size) const;

private:
    bool demoMode_ = false;
    uint8_t demoScenario_ = 0;
    bool ntpStarted_ = false;
    time_t demoBase_ = 0;
    uint32_t demoStartMs_ = 0;
    time_t lastSecond_ = 0;
};
