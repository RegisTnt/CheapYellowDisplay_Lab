#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup()
{
    // Le moniteur série utilise la même vitesse que platformio.ini.
    Serial.begin(115200);
    delay(500);

    // Le rétroéclairage du CYD est commandé par la broche TFT_BL.
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    tft.init();

    // setRotation(1) place l'écran en paysage : 320 px de large
    // (axe x) et 240 px de haut (axe y).
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // L'origine (0, 0) est en haut à gauche. x augmente vers la droite
    // et y vers le bas. Les dimensions restent disponibles avec width()/height().

    // Les couleurs TFT sont des constantes RGB 16 bits, par exemple TFT_CYAN.
    // TC_DATUM ancre le texte par son centre supérieur, ce qui simplifie
    // son centrage horizontal à la coordonnée x = tft.width() / 2.
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("CYD Graphics Lab", tft.width() / 2, 8, 4);

    // Les fonctions draw... dessinent un contour ; les fonctions fill...
    // remplissent toute la surface de la forme.
    tft.drawFastHLine(15, 42, tft.width() - 30, TFT_DARKGREY);
    tft.drawRect(15, 55, 90, 55, TFT_WHITE);
    tft.fillRect(115, 55, 90, 55, TFT_BLUE);
    tft.drawRoundRect(215, 55, 90, 55, 8, TFT_GREEN);
    tft.drawCircle(60, 155, 28, TFT_YELLOW);
    tft.fillCircle(160, 155, 28, TFT_RED);
    tft.drawTriangle(
        235, 180,
        275, 125,
        310, 180,
        TFT_MAGENTA
    );

    // Chaque forme reçoit des coordonnées x et y, puis des dimensions
    // (largeur et hauteur) ou un rayon selon la primitive utilisée.
    constexpr int progressPercent = 64;
    constexpr int progressInnerWidth = 274;
    tft.drawRoundRect(20, 205, 280, 22, 6, TFT_WHITE);
    tft.fillRoundRect(
        23,
        208,
        progressInnerWidth * progressPercent / 100,
        16,
        4,
        TFT_GREEN
    );

    // MC_DATUM place le point d'ancrage au centre du texte : les coordonnées
    // données à drawString() correspondent donc au centre de la barre.
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("64 %", 160, 216, 2);

    Serial.println("Test graphique lance : primitives affichees sur le TFT.");
}

void loop()
{
}
