#pragma once

#include <Arduino.h>
#include <SPI.h>

class TS_Point
{
public:
    TS_Point() : x(0), y(0), z(0) {}
    TS_Point(int16_t pointX, int16_t pointY, int16_t pointZ)
        : x(pointX), y(pointY), z(pointZ) {}
    int16_t x;
    int16_t y;
    int16_t z;
};

class XPT2046_Touchscreen
{
public:
    constexpr XPT2046_Touchscreen(uint8_t chipSelectPin, uint8_t interruptPin = 255)
        : csPin_(chipSelectPin), irqPin_(interruptPin) {}
    bool begin(SPIClass& spi = SPI);
    bool touched();
    TS_Point getPoint();
    void setRotation(uint8_t rotation) { rotation_ = rotation % 4; }

private:
    void update();
    static int16_t bestTwoAverage(int16_t first, int16_t second, int16_t third);
    uint8_t csPin_;
    uint8_t irqPin_;
    uint8_t rotation_ = 1;
    SPIClass* spi_ = nullptr;
    int16_t rawX_ = 0;
    int16_t rawY_ = 0;
    int16_t rawZ_ = 0;
    uint32_t lastReadMs_ = 0;
};
