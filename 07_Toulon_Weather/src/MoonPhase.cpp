#include "MoonPhase.h"

#include <cmath>

namespace
{

constexpr double SYNODIC_MONTH_DAYS = 29.53059;
constexpr double REFERENCE_NEW_MOON_JULIAN_DAY = 2451550.1;
constexpr double UNIX_EPOCH_JULIAN_DAY = 2440587.5;
constexpr double SECONDS_PER_DAY = 86400.0;

const char* phaseName(float fraction)
{
    if (fraction < 0.0625F || fraction >= 0.9375F) return "Nouvelle Lune";
    if (fraction < 0.1875F) return "Premier croissant";
    if (fraction < 0.3125F) return "Premier quartier";
    if (fraction < 0.4375F) return "Lune gibbeuse croissante";
    if (fraction < 0.5625F) return "Pleine Lune";
    if (fraction < 0.6875F) return "Lune gibbeuse decroissante";
    if (fraction < 0.8125F) return "Dernier quartier";
    return "Dernier croissant";
}

}  // namespace

MoonPhaseInfo calculateMoonPhase(time_t timestamp)
{
    const double julianDay = UNIX_EPOCH_JULIAN_DAY
        + static_cast<double>(timestamp) / SECONDS_PER_DAY;
    double age = fmod(julianDay - REFERENCE_NEW_MOON_JULIAN_DAY, SYNODIC_MONTH_DAYS);
    if (age < 0.0)
    {
        age += SYNODIC_MONTH_DAYS;
    }

    const float fraction = static_cast<float>(age / SYNODIC_MONTH_DAYS);
    const double illumination = 0.5 * (1.0 - cos(2.0 * PI * fraction));

    MoonPhaseInfo result;
    result.ageDays = static_cast<float>(age);
    result.cycleFraction = fraction;
    result.illuminationPercent = static_cast<int16_t>(round(illumination * 100.0));
    result.name = phaseName(fraction);
    return result;
}
