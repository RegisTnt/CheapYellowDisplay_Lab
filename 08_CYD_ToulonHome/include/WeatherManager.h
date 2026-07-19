#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "AppData.h"

class WeatherManager
{
public:
    WeatherManager();
    ~WeatherManager();

    void begin(bool demoMode, uint8_t demoScenario);
    void update(uint32_t nowMs, bool wifiConnected);
    WeatherSnapshot snapshot() const;

private:
    static void taskEntry(void* context);
    void fetchInBackground();
    bool fetch(WeatherSnapshot& output);
    void loadDemo(uint8_t scenario);
    void publish(const WeatherSnapshot& value);

    mutable SemaphoreHandle_t mutex_ = nullptr;
    WeatherSnapshot snapshot_ = {};
    bool demoMode_ = false;
    volatile bool taskRunning_ = false;
    uint32_t nextFetchMs_ = 0;
};
