#include "TouchManager.h"

namespace
{
constexpr uint8_t TOUCH_MOSI_PIN = 32;
constexpr uint8_t TOUCH_MISO_PIN = 39;
constexpr uint8_t TOUCH_SCLK_PIN = 25;
constexpr uint8_t TOUCH_CS_PIN = 33;
constexpr uint8_t TOUCH_IRQ_PIN = 36;
constexpr int16_t RAW_MIN_X = 200;
constexpr int16_t RAW_MAX_X = 3800;
constexpr int16_t RAW_MIN_Y = 200;
constexpr int16_t RAW_MAX_Y = 3800;
constexpr uint32_t READ_INTERVAL_MS = 30;
}

TouchManager::TouchManager()
    : spi_(HSPI), controller_(TOUCH_CS_PIN, TOUCH_IRQ_PIN)
{
}

bool TouchManager::begin(int16_t width, int16_t height)
{
    width_ = width;
    height_ = height;
    spi_.begin(TOUCH_SCLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS_PIN);
    ready_ = controller_.begin(spi_);
    controller_.setRotation(1);
    return ready_;
}

TouchEvent TouchManager::update(uint32_t nowMs)
{
    TouchEvent event;
    if (!ready_ || nowMs - lastReadMs_ < READ_INTERVAL_MS)
    {
        return event;
    }
    lastReadMs_ = nowMs;

    const bool active = controller_.touched();
    if (active)
    {
        const TS_Point point = controller_.getPoint();
        lastX_ = mapAndClamp(point.x, RAW_MIN_X, RAW_MAX_X, width_);
        lastY_ = mapAndClamp(point.y, RAW_MIN_Y, RAW_MAX_Y, height_);
        event.type = touched_ ? TouchEventType::Move : TouchEventType::Down;
        event.x = lastX_;
        event.y = lastY_;
    }
    else if (touched_)
    {
        event.type = TouchEventType::Up;
        event.x = lastX_;
        event.y = lastY_;
    }
    touched_ = active;
    return event;
}

int16_t TouchManager::mapAndClamp(int16_t value, int16_t start, int16_t end, int16_t size)
{
    const int16_t bounded = constrain(value, min(start, end), max(start, end));
    return static_cast<int16_t>(constrain(
        map(bounded, start, end, 0, size - 1),
        0L,
        static_cast<long>(size - 1)
    ));
}
