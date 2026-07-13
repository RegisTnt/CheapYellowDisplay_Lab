#include <Arduino.h>
#include <Button.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TouchController.h>

#include <cstring>

namespace
{

constexpr uint8_t TOUCH_MOSI_PIN = 32;
constexpr uint8_t TOUCH_MISO_PIN = 39;
constexpr uint8_t TOUCH_SCLK_PIN = 25;
constexpr uint8_t TOUCH_CS_PIN = 33;
constexpr uint8_t TOUCH_IRQ_PIN = 36;

// Calibration reprise telle quelle de 05_Touch_Input, encore à valider sur la dalle.
constexpr int16_t TOUCH_RAW_MIN_X = 200;
constexpr int16_t TOUCH_RAW_MAX_X = 3800;
constexpr int16_t TOUCH_RAW_MIN_Y = 200;
constexpr int16_t TOUCH_RAW_MAX_Y = 3800;

constexpr uint32_t TOUCH_READ_INTERVAL_MS = 30;
constexpr uint32_t MIN_CLICK_INTERVAL_MS = 250;

TFT_eSPI tft;
SPIClass touchSpi(HSPI);
CYDTouch::TouchController touchController(touchSpi, TOUCH_CS_PIN, TOUCH_IRQ_PIN);

CYDUI::Button openButton(15, 58, 135, 52, "OUVRIR");
CYDUI::Button pedestrianButton(170, 58, 135, 52, "PIETON");
CYDUI::Button settingsButton(70, 130, 180, 48, "PARAMETRES");

struct ButtonBinding
{
    CYDUI::Button* button;
    const char* name;
};

ButtonBinding buttons[] = {
    {&openButton, "OUVRIR"},
    {&pedestrianButton, "PIETON"},
    {&settingsButton, "PARAMETRES"}
};

bool touchReady = false;
char currentStatus[32] = "";

ButtonBinding* buttonAt(int16_t x, int16_t y)
{
    for (ButtonBinding& binding : buttons)
    {
        if (binding.button->contains(x, y))
        {
            return &binding;
        }
    }
    return nullptr;
}

void setStatus(const char* text)
{
    if (strcmp(currentStatus, text) == 0)
    {
        return;
    }

    strncpy(currentStatus, text, sizeof(currentStatus) - 1);
    currentStatus[sizeof(currentStatus) - 1] = '\0';

    tft.fillRect(10, 204, 300, 28, TFT_BLACK);
    tft.drawRoundRect(10, 204, 300, 28, 5, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(currentStatus, 160, 218, 2);
}

void drawInterface()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("CYD Interactive UI", tft.width() / 2, 8, 4);
    tft.drawFastHLine(10, 42, tft.width() - 20, TFT_DARKGREY);

    for (ButtonBinding& binding : buttons)
    {
        binding.button->draw(tft);
    }
    setStatus("Aucun bouton");
}

void showButtonStatus(const char* prefix, const ButtonBinding& binding)
{
    char status[32];
    snprintf(status, sizeof(status), "%s : %s", prefix, binding.name);
    setStatus(status);
}

void setButtonState(ButtonBinding& binding, CYDUI::ButtonState state)
{
    if (binding.button->state() == state)
    {
        return;
    }
    binding.button->setState(state);
    binding.button->draw(tft);
}

}  // namespace

void setup()
{
    Serial.begin(115200);
    delay(100);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    tft.init();
    tft.setRotation(1);

    touchSpi.begin(TOUCH_SCLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS_PIN);
    touchController.setScreenSize(tft.width(), tft.height());
    touchController.setCalibration(
        TOUCH_RAW_MIN_X,
        TOUCH_RAW_MAX_X,
        TOUCH_RAW_MIN_Y,
        TOUCH_RAW_MAX_Y
    );
    touchReady = touchController.begin();

    drawInterface();
    if (!touchReady)
    {
        setStatus("Erreur tactile");
        Serial.println("XPT2046 initialization failed.");
        return;
    }

    Serial.println("CYD interactive UI ready.");
}

void loop()
{
    static uint32_t lastReadTime = 0;
    static uint32_t lastClickTime = 0;
    static bool previousTouched = false;
    static bool gestureCancelled = false;
    static CYDTouch::TouchPoint previousPoint = {0, 0, 0, 0, 0, false};
    static ButtonBinding* capturedButton = nullptr;

    if (!touchReady)
    {
        return;
    }

    const uint32_t now = millis();
    if (now - lastReadTime < TOUCH_READ_INTERVAL_MS)
    {
        return;
    }
    lastReadTime = now;

    CYDTouch::TouchPoint point;
    touchController.read(point);

    if (point.touched && !previousTouched)
    {
        capturedButton = buttonAt(point.x, point.y);
        gestureCancelled = false;
        if (capturedButton != nullptr)
        {
            setButtonState(*capturedButton, CYDUI::ButtonState::Pressed);
            showButtonStatus("Appui", *capturedButton);
        }
    }
    else if (point.touched && previousTouched && capturedButton != nullptr)
    {
        if (!capturedButton->button->contains(point.x, point.y) && !gestureCancelled)
        {
            gestureCancelled = true;
            setButtonState(*capturedButton, CYDUI::ButtonState::Normal);
            showButtonStatus("Annule", *capturedButton);
        }
    }
    else if (!point.touched && previousTouched)
    {
        if (capturedButton != nullptr)
        {
            setButtonState(*capturedButton, CYDUI::ButtonState::Normal);
            const bool releasedInside = capturedButton->button->contains(
                previousPoint.x,
                previousPoint.y
            );
            const bool debounceElapsed = lastClickTime == 0
                || now - lastClickTime >= MIN_CLICK_INTERVAL_MS;

            if (!gestureCancelled && releasedInside && debounceElapsed)
            {
                lastClickTime = now;
                showButtonStatus("Clic", *capturedButton);
                Serial.print("Button clicked: ");
                Serial.println(capturedButton->name);
            }
            else if (!gestureCancelled && !releasedInside)
            {
                showButtonStatus("Annule", *capturedButton);
            }
            else if (!gestureCancelled)
            {
                setStatus("Aucun bouton");
            }
        }
        else
        {
            setStatus("Aucun bouton");
        }

        capturedButton = nullptr;
        gestureCancelled = false;
    }

    previousTouched = point.touched;
    if (point.touched)
    {
        previousPoint = point;
    }
}
