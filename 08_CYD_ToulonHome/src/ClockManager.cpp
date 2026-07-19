#include "ClockManager.h"

#include <cstring>

namespace
{
constexpr char TIME_ZONE[] = "CET-1CEST,M3.5.0,M10.5.0/3";
}

void ClockManager::begin(bool demoMode, uint8_t demoScenario)
{
    demoMode_ = demoMode;
    demoScenario_ = demoScenario;
    demoStartMs_ = millis();

    setenv("TZ", TIME_ZONE, 1);
    tzset();

    if (demoMode_)
    {
        struct tm value = {};
        value.tm_year = 2026 - 1900;
        value.tm_mon = 6;
        value.tm_mday = 19;
        value.tm_hour = demoScenario_ == 2 ? 22 : 14;
        value.tm_min = 23;
        value.tm_isdst = -1;
        demoBase_ = mktime(&value);
    }
}

bool ClockManager::update(bool wifiConnected)
{
    if (!demoMode_ && wifiConnected && !ntpStarted_)
    {
        configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");
        ntpStarted_ = true;
    }

    const time_t value = now();
    if (value == lastSecond_)
    {
        return false;
    }
    lastSecond_ = value;
    return true;
}

time_t ClockManager::now() const
{
    if (demoMode_)
    {
        return demoBase_ + (millis() - demoStartMs_) / 1000;
    }
    const time_t value = time(nullptr);
    return value > 1600000000 ? value : 0;
}

bool ClockManager::ready() const
{
    return now() != 0;
}

void ClockManager::formatTime(char output[6]) const
{
    const time_t value = now();
    if (value == 0)
    {
        strcpy(output, "--:--");
        return;
    }
    struct tm local = {};
    localtime_r(&value, &local);
    strftime(output, 6, "%H:%M", &local);
}

void ClockManager::formatDate(char* output, size_t size) const
{
    const time_t value = now();
    if (value == 0)
    {
        strncpy(output, "Date indisponible", size - 1);
        output[size - 1] = '\0';
        return;
    }
    struct tm local = {};
    localtime_r(&value, &local);
    strftime(output, size, "%d/%m/%Y", &local);
}
