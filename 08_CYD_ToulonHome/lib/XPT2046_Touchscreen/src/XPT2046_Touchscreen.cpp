#include "XPT2046_Touchscreen.h"

namespace
{
constexpr int16_t PRESSURE_THRESHOLD = 400;
constexpr uint32_t READ_INTERVAL_MS = 3;
const SPISettings TOUCH_SPI_SETTINGS(2000000, MSBFIRST, SPI_MODE0);
}

bool XPT2046_Touchscreen::begin(SPIClass& spi)
{
    spi_ = &spi;
    pinMode(csPin_, OUTPUT);
    digitalWrite(csPin_, HIGH);
    if (irqPin_ != 255) pinMode(irqPin_, INPUT);
    return true;
}

bool XPT2046_Touchscreen::touched()
{
    if (irqPin_ != 255 && digitalRead(irqPin_) == HIGH)
    {
        rawZ_ = 0;
        return false;
    }
    update();
    return rawZ_ >= PRESSURE_THRESHOLD;
}

TS_Point XPT2046_Touchscreen::getPoint()
{
    update();
    return TS_Point(rawX_, rawY_, rawZ_);
}

int16_t XPT2046_Touchscreen::bestTwoAverage(int16_t first, int16_t second, int16_t third)
{
    const int16_t ab = abs(first - second);
    const int16_t ac = abs(first - third);
    const int16_t bc = abs(second - third);
    if (ab <= ac && ab <= bc) return (first + second) / 2;
    if (ac <= ab && ac <= bc) return (first + third) / 2;
    return (second + third) / 2;
}

void XPT2046_Touchscreen::update()
{
    if (spi_ == nullptr || millis() - lastReadMs_ < READ_INTERVAL_MS) return;
    lastReadMs_ = millis();

    int16_t samples[6] = {};
    spi_->beginTransaction(TOUCH_SPI_SETTINGS);
    digitalWrite(csPin_, LOW);
    spi_->transfer(0xB1);
    int16_t pressure = (spi_->transfer16(0xC1) >> 3) + 4095;
    pressure -= spi_->transfer16(0x91) >> 3;
    if (pressure >= PRESSURE_THRESHOLD)
    {
        spi_->transfer16(0x91);
        samples[0] = spi_->transfer16(0xD1) >> 3;
        samples[1] = spi_->transfer16(0x91) >> 3;
        samples[2] = spi_->transfer16(0xD1) >> 3;
        samples[3] = spi_->transfer16(0x91) >> 3;
    }
    samples[4] = spi_->transfer16(0xD0) >> 3;
    samples[5] = spi_->transfer16(0) >> 3;
    digitalWrite(csPin_, HIGH);
    spi_->endTransaction();

    rawZ_ = pressure > 0 ? pressure : 0;
    if (rawZ_ < PRESSURE_THRESHOLD)
    {
        rawZ_ = 0;
        return;
    }

    const int16_t x = bestTwoAverage(samples[0], samples[2], samples[4]);
    const int16_t y = bestTwoAverage(samples[1], samples[3], samples[5]);
    switch (rotation_)
    {
        case 0: rawX_ = 4095 - y; rawY_ = x; break;
        case 1: rawX_ = x; rawY_ = y; break;
        case 2: rawX_ = y; rawY_ = 4095 - x; break;
        default: rawX_ = 4095 - x; rawY_ = 4095 - y; break;
    }
}
