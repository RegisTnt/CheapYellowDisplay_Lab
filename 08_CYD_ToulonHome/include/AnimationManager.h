#pragma once

#include <Arduino.h>

class AnimationManager
{
public:
    enum class Button : uint8_t { None, Pedestrian, Vehicle };

    void press(Button button);
    void release();
    void commandResult(bool accepted, uint32_t nowMs);
    bool update(uint32_t nowMs, bool portalAlertActive);

    Button pressedButton() const;
    float pressDepth(Button button) const;
    uint8_t portalGlow() const;
    bool feedbackActive(uint32_t nowMs) const;
    bool feedbackAccepted() const;

private:
    Button pressed_ = Button::None;
    float pedestrianDepth_ = 0.0F;
    float vehicleDepth_ = 0.0F;
    uint8_t portalGlow_ = 255;
    uint32_t lastFrameMs_ = 0;
    uint32_t feedbackUntilMs_ = 0;
    bool feedbackAccepted_ = false;
};
