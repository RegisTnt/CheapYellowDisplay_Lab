#pragma once

#include <Arduino.h>
#include <time.h>

struct MoonPhaseInfo
{
    float ageDays;
    float cycleFraction;
    int16_t illuminationPercent;
    const char* name;
};

/** Calcule une phase lunaire approximative à partir d'un instant UTC. */
MoonPhaseInfo calculateMoonPhase(time_t timestamp);
