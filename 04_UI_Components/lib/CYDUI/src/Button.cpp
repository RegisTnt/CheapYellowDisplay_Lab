#include "Button.h"

namespace CYDUI
{

Button::Button(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    const char* text
)
    : x_(x),
      y_(y),
      width_(width),
      height_(height),
      text_(text)
{
}

void Button::draw(TFT_eSPI& display) const
{
    const int16_t radius = height_ / 6;
    const int16_t centerX = x_ + width_ / 2;
    const int16_t centerY = y_ + height_ / 2;
    const uint8_t previousDatum = display.getTextDatum();
    const uint16_t previousTextColor = static_cast<uint16_t>(display.textcolor);
    const uint16_t previousBackgroundColor = static_cast<uint16_t>(display.textbgcolor);

    display.fillRoundRect(x_, y_, width_, height_, radius, TFT_NAVY);
    display.drawRoundRect(x_, y_, width_, height_, radius, TFT_CYAN);

    display.setTextDatum(MC_DATUM);
    display.setTextColor(TFT_WHITE, TFT_NAVY);
    display.drawString(text_ != nullptr ? text_ : "", centerX, centerY, 2);

    display.setTextDatum(previousDatum);
    display.setTextColor(previousTextColor, previousBackgroundColor);
}

}  // namespace CYDUI
