#include <Arduino.h>
#include <Button.h>
#include <TFT_eSPI.h>

TFT_eSPI tft;

void setup()
{
    Serial.begin(115200);
    delay(500);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("CYDUI Components", tft.width() / 2, 12, 4);

    static CYDUI::Button openButton(15, 65, 135, 55, "OUVRIR");
    static CYDUI::Button pedestrianButton(170, 65, 135, 55, "PIETON");
    static CYDUI::Button settingsButton(70, 145, 180, 50, "PARAMETRES");

    openButton.draw(tft);
    pedestrianButton.draw(tft);
    settingsButton.draw(tft);

    Serial.println("CYDUI button demo ready.");
}

void loop()
{
}
