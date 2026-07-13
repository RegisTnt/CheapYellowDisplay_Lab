#include <Arduino.h>
#include <TFT_eSPI.h>

// TFT_eSPI lit le contrôleur et les broches dans les build_flags de platformio.ini.
TFT_eSPI tft;

void setup()
{
    // Ouvre la liaison USB série pour afficher les messages de diagnostic.
    Serial.begin(115200);
    delay(500);

    // Le rétroéclairage du CYD est commandé par la broche GPIO 21, active à HIGH.
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    // Initialise l'écran puis le place en orientation paysage.
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_NAVY);

    // Centre les trois lignes par rapport aux coordonnées indiquées à drawString().
    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(TFT_CYAN, TFT_NAVY);
    tft.drawString("Cheap Yellow Display", tft.width() / 2, 75, 4);

    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawString("Hello World !", tft.width() / 2, 135, 4);

    tft.setTextColor(TFT_LIGHTGREY, TFT_NAVY);
    tft.drawString("ESP32-2432S028", tft.width() / 2, 190, 2);

    Serial.println("CYD initialise : Hello World affiche sur l'ecran TFT.");
}

void loop()
{
    // Ce premier test affiche une image fixe : aucune action répétitive n'est nécessaire.
}
