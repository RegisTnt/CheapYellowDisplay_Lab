#include "TouchController.h"

namespace CYDTouch
{

TouchController::TouchController(
    SPIClass& spi,
    uint8_t chipSelectPin,
    uint8_t interruptPin
)
    : spi_(spi),
      touchscreen_(chipSelectPin, interruptPin),
      screenWidth_(320),
      screenHeight_(240),
      rawMinX_(200),
      rawMaxX_(3800),
      rawMinY_(200),
      rawMaxY_(3800)
{
}

bool TouchController::begin()
{
    const bool initialized = touchscreen_.begin(spi_);
    touchscreen_.setRotation(1);
    return initialized;
}

bool TouchController::read(TouchPoint& point)
{
    point = {0, 0, 0, 0, 0, false};

    if (!touchscreen_.touched())
    {
        return false;
    }

    const TS_Point rawPoint = touchscreen_.getPoint();
    point.rawX = rawPoint.x;
    point.rawY = rawPoint.y;
    point.pressure = rawPoint.z;
    point.x = mapAndClamp(rawPoint.x, rawMinX_, rawMaxX_, screenWidth_);
    point.y = mapAndClamp(rawPoint.y, rawMinY_, rawMaxY_, screenHeight_);
    point.touched = true;
    return true;
}

void TouchController::setScreenSize(int16_t width, int16_t height)
{
    screenWidth_ = width > 0 ? width : 1;
    screenHeight_ = height > 0 ? height : 1;
}

void TouchController::setCalibration(
    int16_t rawMinX,
    int16_t rawMaxX,
    int16_t rawMinY,
    int16_t rawMaxY
)
{
    rawMinX_ = rawMinX;
    rawMaxX_ = rawMaxX;
    rawMinY_ = rawMinY;
    rawMaxY_ = rawMaxY;
}

int16_t TouchController::mapAndClamp(
    int16_t rawValue,
    int16_t rawStart,
    int16_t rawEnd,
    int16_t screenSize
)
{
    if (rawStart == rawEnd || screenSize <= 1)
    {
        return 0;
    }

    const int16_t lowerBound = min(rawStart, rawEnd);
    const int16_t upperBound = max(rawStart, rawEnd);
    const int16_t boundedRaw = constrain(rawValue, lowerBound, upperBound);
    const long mapped = map(boundedRaw, rawStart, rawEnd, 0, screenSize - 1);
    return static_cast<int16_t>(constrain(mapped, 0L, static_cast<long>(screenSize - 1)));
}

}  // namespace CYDTouch
