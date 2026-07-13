# CYDTouch

CYDTouch est une bibliothèque locale minimale destinée au tactile résistif XPT2046 du Cheap Yellow Display. La version **0.1.0** lit les valeurs brutes, la pression et les transforme en coordonnées écran bornées.

## Dépendances et brochage

La bibliothèque dépend uniquement d'Arduino, de SPI et de `XPT2046_Touchscreen` de Paul Stoffregen. Elle ne dépend ni de TFT_eSPI ni de CYDUI.

| Signal | GPIO |
|---|---:|
| MOSI | 32 |
| MISO | 39 |
| SCLK | 25 |
| CS | 33 |
| IRQ | 36 |

L'application doit créer et initialiser le bus SPI dédié avant d'appeler `begin()`.

## API et exemple minimal

```cpp
SPIClass touchSpi(HSPI);
CYDTouch::TouchController touch(touchSpi, 33, 36);

touchSpi.begin(25, 39, 32, 33);
touch.setScreenSize(320, 240);
touch.setCalibration(200, 3800, 200, 3800);
touch.begin();

CYDTouch::TouchPoint point;
if (touch.read(point)) {
    Serial.printf("%d, %d\n", point.x, point.y);
}
```

Les bornes `200` et `3800` sont seulement des points de départ. Elles doivent être mesurées sur chaque dalle.

## Limites

La version 0.1.0 n'implémente ni composant graphique, ni événement, ni callback, ni action métier. Elle n'effectue aucune allocation dynamique et ne possède pas le bus SPI reçu par référence.

Des versions futures pourront étudier une calibration interactive, l'appui court, l'appui long, le relâchement, l'anti-rebond logiciel, la liaison avec `CYDUI::Button` et des gestes simples. Ces fonctions ne sont pas implémentées ici.
