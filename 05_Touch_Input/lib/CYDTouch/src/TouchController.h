#ifndef CYDTOUCH_TOUCH_CONTROLLER_H
#define CYDTOUCH_TOUCH_CONTROLLER_H

#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

namespace CYDTouch
{

/**
 * @brief Mesure tactile brute et position convertie en pixels écran.
 *
 * La pression est la valeur Z fournie par le XPT2046. Les coordonnées brutes
 * doivent être calibrées avant d'être considérées comme précises.
 */
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
 * La classe lit un contrôleur connecté à un bus SPI injecté par référence,
 * convertit ses mesures brutes et ne contient aucune logique graphique ou UI.
 * Le tactile interactif, les événements et les actions sont hors de ce sprint.
 */
class TouchController
{
public:
    /**
     * @brief Construit le contrôleur sans posséder ni créer le bus SPI.
     *
     * @param spi Bus SPI dédié, initialisé explicitement par l'application.
     * @param chipSelectPin Broche CS du XPT2046.
     * @param interruptPin Broche IRQ du XPT2046.
     */
    TouchController(
        SPIClass& spi,
        uint8_t chipSelectPin,
        uint8_t interruptPin
    );

    /**
     * @brief Initialise le XPT2046 sur le bus SPI dédié injecté.
     * @return true lorsque l'initialisation de la bibliothèque a réussi.
     */
    bool begin();

    /**
     * @brief Lit une mesure et la convertit en coordonnées écran bornées.
     *
     * @param point Structure remplie avec coordonnées, pression et état.
     * @return true si un contact valide est détecté, sinon false.
     */
    bool read(TouchPoint& point);

    /**
     * @brief Définit les dimensions de l'écran après rotation.
     * @param width Largeur en pixels, normalement 320 en paysage.
     * @param height Hauteur en pixels, normalement 240 en paysage.
     */
    void setScreenSize(int16_t width, int16_t height);

    /**
     * @brief Définit les bornes brutes utilisées pour la conversion.
     *
     * Une paire de bornes inversée permet de corriger un axe en miroir.
     *
     * @param rawMinX Mesure X correspondant au bord gauche.
     * @param rawMaxX Mesure X correspondant au bord droit.
     * @param rawMinY Mesure Y correspondant au bord supérieur.
     * @param rawMaxY Mesure Y correspondant au bord inférieur.
     */
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
