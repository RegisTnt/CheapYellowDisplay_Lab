#ifndef CYDTOUCH_TOUCH_CONTROLLER_H
#define CYDTOUCH_TOUCH_CONTROLLER_H

#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

namespace CYDTouch
{

/** @brief Mesure tactile brute et position convertie en pixels écran. */
struct TouchPoint
{
    int16_t x;
    int16_t y;
    int16_t rawX;
    int16_t rawY;
    int16_t pressure;
    bool touched;
};

/**
 * @brief Adapte un XPT2046 à un écran en orientation paysage.
 *
 * La classe utilise un bus SPI injecté, convertit les mesures brutes et ne
 * contient aucune dépendance graphique ou logique d'interaction.
 */
class TouchController
{
public:
    TouchController(SPIClass& spi, uint8_t chipSelectPin, uint8_t interruptPin);

    /** @brief Initialise le XPT2046 sur le bus SPI injecté. */
    bool begin();

    /** @brief Lit et convertit un contact éventuel. */
    bool read(TouchPoint& point);

    /** @brief Définit les dimensions de l'écran après rotation. */
    void setScreenSize(int16_t width, int16_t height);

    /** @brief Définit les quatre bornes brutes de calibration. */
    void setCalibration(
        int16_t rawMinX,
        int16_t rawMaxX,
        int16_t rawMinY,
        int16_t rawMaxY
    );

private:
    static int16_t mapAndClamp(
        int16_t rawValue,
        int16_t rawStart,
        int16_t rawEnd,
        int16_t screenSize
    );

    SPIClass& spi_;
    XPT2046_Touchscreen touchscreen_;
    int16_t screenWidth_;
    int16_t screenHeight_;
    int16_t rawMinX_;
    int16_t rawMaxX_;
    int16_t rawMinY_;
    int16_t rawMaxY_;
};

}  // namespace CYDTouch

#endif
