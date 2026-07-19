#pragma once

#include <SPI.h>
#include <XPT2046_Touchscreen.h>

enum class TouchEventType : uint8_t
{
    None,
    Down,
    Move,
    Up
};

struct TouchEvent
{
    TouchEventType type = TouchEventType::None;
    int16_t x = 0;
    int16_t y = 0;
};

class TouchManager
{
public:
    TouchManager();
    bool begin(int16_t width, int16_t height);
    TouchEvent update(uint32_t nowMs);

private:
    static int16_t mapAndClamp(int16_t value, int16_t start, int16_t end, int16_t size);

    SPIClass spi_;
    XPT2046_Touchscreen controller_;
    int16_t width_ = 320;
    int16_t height_ = 240;
    bool ready_ = false;
    bool touched_ = false;
    int16_t lastX_ = 0;
    int16_t lastY_ = 0;
    uint32_t lastReadMs_ = 0;
};
