#pragma once

#include "AppData.h"

class MoonCalculator
{
public:
    MoonSnapshot calculate(time_t timestamp) const;
};
