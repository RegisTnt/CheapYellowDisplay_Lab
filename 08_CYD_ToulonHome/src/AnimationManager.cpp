#include "AnimationManager.h"

#include <cmath>

namespace
{
constexpr uint32_t FRAME_INTERVAL_MS = 33;
constexpr uint32_t FEEDBACK_DURATION_MS = 650;

float approach(float value, float target, float amount)
{
    if (value < target) return min(value + amount, target);
    if (value > target) return max(value - amount, target);
    return value;
}
}

void AnimationManager::press(Button button)
{
    pressed_ = button;
}

void AnimationManager::release()
{
    pressed_ = Button::None;
}

void AnimationManager::commandResult(bool accepted, uint32_t nowMs)
{
    feedbackAccepted_ = accepted;
    feedbackUntilMs_ = nowMs + FEEDBACK_DURATION_MS;
}

bool AnimationManager::update(uint32_t nowMs, bool portalAlertActive)
{
    if (nowMs - lastFrameMs_ < FRAME_INTERVAL_MS)
    {
        return false;
    }
    lastFrameMs_ = nowMs;

    const float previousPedestrian = pedestrianDepth_;
    const float previousVehicle = vehicleDepth_;
    const uint8_t previousGlow = portalGlow_;
    pedestrianDepth_ = approach(pedestrianDepth_, pressed_ == Button::Pedestrian ? 1.0F : 0.0F, 0.24F);
    vehicleDepth_ = approach(vehicleDepth_, pressed_ == Button::Vehicle ? 1.0F : 0.0F, 0.24F);

    if (portalAlertActive)
    {
        const float wave = (sinf(static_cast<float>(nowMs) * 0.002F) + 1.0F) * 0.5F;
        portalGlow_ = static_cast<uint8_t>(90 + wave * 165);
    }
    else
    {
        portalGlow_ = 255;
    }

    return previousPedestrian != pedestrianDepth_
        || previousVehicle != vehicleDepth_
        || previousGlow != portalGlow_;
}

AnimationManager::Button AnimationManager::pressedButton() const
{
    return pressed_;
}

float AnimationManager::pressDepth(Button button) const
{
    return button == Button::Pedestrian ? pedestrianDepth_ : vehicleDepth_;
}

uint8_t AnimationManager::portalGlow() const
{
    return portalGlow_;
}

bool AnimationManager::feedbackActive(uint32_t nowMs) const
{
    return static_cast<int32_t>(feedbackUntilMs_ - nowMs) > 0;
}

bool AnimationManager::feedbackAccepted() const
{
    return feedbackAccepted_;
}
