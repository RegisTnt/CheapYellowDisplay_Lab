#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "AppData.h"

class PortalClient
{
public:
    PortalClient();
    ~PortalClient();

    void begin(const char* baseUrl, bool demoMode, uint8_t demoScenario);
    void update(uint32_t nowMs, bool wifiConnected, time_t currentTime);
    bool requestPedestrian(uint32_t nowMs);
    bool requestVehicle(uint32_t nowMs);
    bool canSend(uint32_t nowMs) const;
    PortalSnapshot snapshot() const;

private:
    enum class Action : uint8_t { ReadState, Pedestrian, Vehicle };

    static void taskEntry(void* context);
    bool start(Action action, uint32_t nowMs);
    void execute();
    bool readState(PortalPosition& position, int& statusCode);
    bool sendCommand(Action action, int& statusCode);
    void loadDemo(uint8_t scenario);

    mutable SemaphoreHandle_t mutex_ = nullptr;
    PortalSnapshot snapshot_ = {};
    String baseUrl_;
    bool demoMode_ = false;
    uint8_t demoScenario_ = 0;
    volatile bool taskRunning_ = false;
    volatile Action pendingAction_ = Action::ReadState;
    uint8_t consecutiveFailures_ = 0;
    uint32_t nextPollMs_ = 0;
    uint32_t cooldownUntilMs_ = 0;
    time_t taskTime_ = 0;
};
