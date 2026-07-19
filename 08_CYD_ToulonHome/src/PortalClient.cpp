#include "PortalClient.h"

#include <HTTPClient.h>
#include <WiFiClient.h>

namespace
{
constexpr uint32_t POLL_INTERVAL_MS = 2000;
constexpr uint32_t COMMAND_COOLDOWN_MS = 3000;
constexpr uint16_t STATE_TIMEOUT_MS = 1800;
constexpr uint16_t COMMAND_TIMEOUT_MS = 5000;
}

PortalClient::PortalClient()
{
    mutex_ = xSemaphoreCreateMutex();
}

PortalClient::~PortalClient()
{
    if (mutex_ != nullptr) vSemaphoreDelete(mutex_);
}

void PortalClient::begin(const char* baseUrl, bool demoMode, uint8_t demoScenario)
{
    baseUrl_ = baseUrl == nullptr ? "" : baseUrl;
    while (baseUrl_.endsWith("/")) baseUrl_.remove(baseUrl_.length() - 1);
    demoMode_ = demoMode;
    demoScenario_ = demoScenario;
    if (demoMode_) loadDemo(demoScenario_);
}

void PortalClient::update(uint32_t nowMs, bool wifiConnected, time_t currentTime)
{
    if (demoMode_)
    {
        PortalSnapshot value = snapshot();
        const bool cooldown = static_cast<int32_t>(cooldownUntilMs_ - nowMs) > 0;
        if (value.cooldown != cooldown)
        {
            value.cooldown = cooldown;
            ++value.revision;
            if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
            {
                snapshot_ = value;
                xSemaphoreGive(mutex_);
            }
        }
        return;
    }

    PortalSnapshot value = snapshot();
    const bool cooldown = static_cast<int32_t>(cooldownUntilMs_ - nowMs) > 0;
    if (value.cooldown != cooldown)
    {
        value.cooldown = cooldown;
        ++value.revision;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            snapshot_ = value;
            xSemaphoreGive(mutex_);
        }
    }

    if (!wifiConnected)
    {
        if (value.apiOnline)
        {
            value.apiOnline = false;
            ++value.revision;
            if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
            {
                snapshot_ = value;
                xSemaphoreGive(mutex_);
            }
        }
        return;
    }

    if (!taskRunning_ && static_cast<int32_t>(nowMs - nextPollMs_) >= 0)
    {
        taskTime_ = currentTime;
        start(Action::ReadState, nowMs);
    }
}

bool PortalClient::requestPedestrian(uint32_t nowMs)
{
    return start(Action::Pedestrian, nowMs);
}

bool PortalClient::requestVehicle(uint32_t nowMs)
{
    return start(Action::Vehicle, nowMs);
}

bool PortalClient::canSend(uint32_t nowMs) const
{
    const PortalSnapshot value = snapshot();
    return value.apiOnline
        && !value.busy
        && static_cast<int32_t>(nowMs - cooldownUntilMs_) >= 0;
}

PortalSnapshot PortalClient::snapshot() const
{
    PortalSnapshot value;
    if (mutex_ != nullptr && xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        value = snapshot_;
        xSemaphoreGive(mutex_);
    }
    return value;
}

void PortalClient::taskEntry(void* context)
{
    static_cast<PortalClient*>(context)->execute();
    vTaskDelete(nullptr);
}

bool PortalClient::start(Action action, uint32_t nowMs)
{
    if (taskRunning_) return false;

    if (demoMode_)
    {
        PortalSnapshot value = snapshot();
        if (!value.apiOnline || value.cooldown) return false;
        value.feedback = CommandFeedback::Sent;
        if (action == Action::Pedestrian || action == Action::Vehicle)
        {
            value.position = PortalPosition::Open;
            if (value.openSinceMs == 0) value.openSinceMs = nowMs;
            cooldownUntilMs_ = nowMs + COMMAND_COOLDOWN_MS;
            value.cooldown = true;
        }
        value.updatedAt = time(nullptr);
        ++value.revision;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            snapshot_ = value;
            xSemaphoreGive(mutex_);
        }
        return true;
    }

    if (action != Action::ReadState && !canSend(nowMs)) return false;
    pendingAction_ = action;
    taskTime_ = time(nullptr);
    taskRunning_ = true;

    PortalSnapshot value = snapshot();
    value.busy = true;
    value.feedback = CommandFeedback::None;
    ++value.revision;
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        snapshot_ = value;
        xSemaphoreGive(mutex_);
    }

    if (xTaskCreatePinnedToCore(taskEntry, "portal-http", 7168, this, 2, nullptr, 0) != pdPASS)
    {
        taskRunning_ = false;
        value.busy = false;
        ++value.revision;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            snapshot_ = value;
            xSemaphoreGive(mutex_);
        }
        return false;
    }
    return true;
}

void PortalClient::execute()
{
    const Action action = pendingAction_;
    bool commandAccepted = true;
    int commandStatus = 0;
    if (action != Action::ReadState)
    {
        commandAccepted = sendCommand(action, commandStatus);
        cooldownUntilMs_ = millis() + COMMAND_COOLDOWN_MS;
    }

    PortalPosition position = PortalPosition::Unknown;
    int stateStatus = 0;
    const bool stateSuccess = readState(position, stateStatus);

    PortalSnapshot value = snapshot();
    value.busy = false;
    value.cooldown = static_cast<int32_t>(cooldownUntilMs_ - millis()) > 0;
    if (action != Action::ReadState)
    {
        value.feedback = commandAccepted ? CommandFeedback::Sent : CommandFeedback::Failed;
    }

    if (stateSuccess)
    {
        const bool stateChanged = !value.apiOnline || value.position != position;
        consecutiveFailures_ = 0;
        value.apiOnline = true;
        value.position = position;
        if (stateChanged)
        {
            Serial.println(position == PortalPosition::Closed
                ? "Portail: ferme confirme."
                : "Portail: ouvert.");
        }
        value.updatedAt = taskTime_ != 0 ? taskTime_ : time(nullptr);
        if (position == PortalPosition::Open)
        {
            if (value.openSinceMs == 0) value.openSinceMs = millis();
        }
        else
        {
            value.openSinceMs = 0;
        }
    }
    else
    {
        ++consecutiveFailures_;
        if (consecutiveFailures_ <= 2)
        {
            Serial.println("PortailControl: lecture /etat impossible.");
        }
        if (consecutiveFailures_ >= 2) value.apiOnline = false;
    }

    ++value.revision;
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        snapshot_ = value;
        xSemaphoreGive(mutex_);
    }
    nextPollMs_ = millis() + POLL_INTERVAL_MS;
    taskRunning_ = false;
}

bool PortalClient::readState(PortalPosition& position, int& statusCode)
{
    WiFiClient client;
    client.setTimeout(STATE_TIMEOUT_MS);
    HTTPClient http;
    http.setConnectTimeout(STATE_TIMEOUT_MS);
    http.setTimeout(STATE_TIMEOUT_MS);
    http.setReuse(false);
    if (!http.begin(client, baseUrl_ + "/etat")) return false;
    http.addHeader("Cache-Control", "no-cache");
    statusCode = http.GET();
    String body = statusCode == HTTP_CODE_OK ? http.getString() : "";
    http.end();
    body.trim();
    body.toLowerCase();
    if (statusCode != HTTP_CODE_OK || (body != "ferme" && body != "ouvert")) return false;
    position = body == "ferme" ? PortalPosition::Closed : PortalPosition::Open;
    return true;
}

bool PortalClient::sendCommand(Action action, int& statusCode)
{
    const char* path = action == Action::Pedestrian ? "/pieton" : "/voiture";
    WiFiClient client;
    client.setTimeout(COMMAND_TIMEOUT_MS);
    HTTPClient http;
    http.setConnectTimeout(COMMAND_TIMEOUT_MS);
    http.setTimeout(COMMAND_TIMEOUT_MS);
    http.setReuse(false);
    if (!http.begin(client, baseUrl_ + path)) return false;
    http.addHeader("Cache-Control", "no-cache");
    statusCode = http.GET();
    http.end();
    return statusCode == HTTP_CODE_OK;
}

void PortalClient::loadDemo(uint8_t scenario)
{
    PortalSnapshot value;
    value.apiOnline = scenario != 3 && scenario != 4;
    value.position = (scenario == 1 || scenario == 5)
        ? PortalPosition::Open
        : PortalPosition::Closed;
    const uint32_t startedAt = millis();
    value.openSinceMs = value.position == PortalPosition::Open
        ? (startedAt == 0 ? 1 : startedAt)
        : 0;
    value.updatedAt = time(nullptr);
    value.revision = 1;
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        snapshot_ = value;
        xSemaphoreGive(mutex_);
    }
}
