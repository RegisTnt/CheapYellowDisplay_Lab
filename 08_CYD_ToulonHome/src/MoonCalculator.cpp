#include "MoonCalculator.h"

#include <cmath>

MoonSnapshot MoonCalculator::calculate(time_t timestamp) const
{
    MoonSnapshot result;
    if (timestamp < 100000)
    {
        return result;
    }

    constexpr double SECONDS_PER_DAY = 86400.0;
    constexpr double UNIX_EPOCH_JULIAN = 2440587.5;
    constexpr double REFERENCE_NEW_MOON = 2451550.1;
    constexpr double SYNODIC_MONTH = 29.53059;
    constexpr double PI_VALUE = 3.14159265358979323846;

    const double julianDay = static_cast<double>(timestamp) / SECONDS_PER_DAY + UNIX_EPOCH_JULIAN;
    double fraction = fmod((julianDay - REFERENCE_NEW_MOON) / SYNODIC_MONTH, 1.0);
    if (fraction < 0.0)
    {
        fraction += 1.0;
    }

    result.cycleFraction = static_cast<float>(fraction);
    result.illumination = static_cast<uint8_t>(round((1.0 - cos(2.0 * PI_VALUE * fraction)) * 50.0));

    if (fraction < 0.03 || fraction >= 0.97) result.name = "Nouvelle lune";
    else if (fraction < 0.22) result.name = "Premier croissant";
    else if (fraction < 0.28) result.name = "Premier quartier";
    else if (fraction < 0.47) result.name = "Gibbeuse croissante";
    else if (fraction < 0.53) result.name = "Pleine lune";
    else if (fraction < 0.72) result.name = "Gibbeuse decroissante";
    else if (fraction < 0.78) result.name = "Dernier quartier";
    else result.name = "Dernier croissant";
    return result;
}
