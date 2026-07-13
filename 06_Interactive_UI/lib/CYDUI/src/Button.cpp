#include "Button.h"

namespace CYDUI
{

namespace
{

struct ButtonStyle
{
    uint16_t background;
    uint16_t border;
    uint16_t text;
    int16_t textOffset;
};

ButtonStyle styleFor(ButtonState state)
{
    switch (state)
    {
        case ButtonState::Pressed:
            return {TFT_CYAN, TFT_WHITE, TFT_NAVY, 1};
        case ButtonState::Disabled:
            return {TFT_DARKGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, 0};
        case ButtonState::Normal:
        default:
            return {TFT_NAVY, TFT_CYAN, TFT_WHITE, 0};
    }
}

}  // namespace

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
      text_(text),
      state_(ButtonState::Normal)
{
}

void Button::draw(TFT_eSPI& display) const
{
    const ButtonStyle style = styleFor(state_);
    const int16_t radius = height_ / 6;
    const int16_t centerX = x_ + width_ / 2 + style.textOffset;
    const int16_t centerY = y_ + height_ / 2 + style.textOffset;
    const uint8_t previousDatum = display.getTextDatum();
    const uint16_t previousTextColor = static_cast<uint16_t>(display.textcolor);
    const uint16_t previousBackgroundColor = static_cast<uint16_t>(display.textbgcolor);

    display.fillRoundRect(x_, y_, width_, height_, radius, style.background);
    display.drawRoundRect(x_, y_, width_, height_, radius, style.border);
    display.setTextDatum(MC_DATUM);
    display.setTextColor(style.text, style.background);
    display.drawString(text_ != nullptr ? text_ : "", centerX, centerY, 2);

    display.setTextDatum(previousDatum);
    display.setTextColor(previousTextColor, previousBackgroundColor);
}

bool Button::contains(int16_t x, int16_t y) const
{
    if (!isEnabled() || width_ <= 0 || height_ <= 0)
    {
        return false;
    }

    const int32_t right = static_cast<int32_t>(x_) + width_ - 1;
    const int32_t bottom = static_cast<int32_t>(y_) + height_ - 1;
    return x >= x_ && static_cast<int32_t>(x) <= right
        && y >= y_ && static_cast<int32_t>(y) <= bottom;
}

void Button::setState(ButtonState state)
{
    state_ = state;
}

ButtonState Button::state() const
{
    return state_;
}

void Button::setEnabled(bool enabled)
{
    state_ = enabled ? ButtonState::Normal : ButtonState::Disabled;
}

bool Button::isEnabled() const
{
    return state_ != ButtonState::Disabled;
}

}  // namespace CYDUI
