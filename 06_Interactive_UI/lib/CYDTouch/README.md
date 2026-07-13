# CYDTouch 0.1.0

CYDTouch lit le contrôleur résistif XPT2046 sur un bus SPI dédié et transforme ses mesures brutes en coordonnées écran. Cette copie conserve sans modification l'API et la calibration du projet `05_Touch_Input`.

```cpp
SPIClass touchSpi(HSPI);
CYDTouch::TouchController touch(touchSpi, 33, 36);

touchSpi.begin(25, 39, 32, 33);
touch.setScreenSize(320, 240);
touch.setCalibration(200, 3800, 200, 3800);
touch.begin();
```

CYDTouch ne connaît ni TFT_eSPI ni CYDUI. Les valeurs `200–3800` sont reprises du sprint précédent mais restent à confirmer physiquement pour chaque dalle.
