#ifndef CYDUI_BUTTON_H
#define CYDUI_BUTTON_H

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace CYDUI
{

/** @brief États visuels disponibles pour un bouton. */
enum class ButtonState
{
    Normal,
    Pressed,
    Disabled
};

/**
 * @brief Bouton graphique avec géométrie, libellé et état visuel.
 *
 * Le composant sait se dessiner et tester un point. Il ne lit pas le tactile,
 * ne déclenche aucune action et ne contient aucun callback.
 */
class Button
{
public:
    /**
     * @brief Construit un bouton actif dans l'état normal.
     * @param x Position horizontale du coin supérieur gauche.
     * @param y Position verticale du coin supérieur gauche.
     * @param width Largeur du bouton en pixels.
     * @param height Hauteur du bouton en pixels.
     * @param text Libellé dont la durée de vie doit dépasser celle du bouton.
     */
    Button(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        const char* text
    );

    /** @brief Dessine le bouton selon son état et restaure les réglages texte. */
    void draw(TFT_eSPI& display) const;

    /**
     * @brief Indique si un point appartient au rectangle, limites incluses.
     * @return false si la géométrie est invalide ou si le bouton est désactivé.
     */
    bool contains(int16_t x, int16_t y) const;

    /** @brief Change directement l'état visuel. */
    void setState(ButtonState state);

    /** @brief Retourne l'état visuel courant. */
    ButtonState state() const;

    /** @brief Active le bouton en état normal ou le place en état désactivé. */
    void setEnabled(bool enabled);

    /** @brief Indique si le bouton accepte les tests de zone. */
    bool isEnabled() const;

private:
    int16_t x_;
    int16_t y_;
    int16_t width_;
    int16_t height_;
    const char* text_;
    ButtonState state_;
};

}  // namespace CYDUI

#endif
