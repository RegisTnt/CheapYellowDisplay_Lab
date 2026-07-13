#ifndef CYDUI_BUTTON_H
#define CYDUI_BUTTON_H

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace CYDUI
{

/**
 * @brief Bouton graphique statique dessiné avec TFT_eSPI.
 *
 * Le composant mémorise uniquement sa géométrie et son libellé. Il n'interprète
 * volontairement aucun événement tactile dans ce sprint.
 */
class Button
{
public:
    /**
     * @brief Construit un bouton sans allocation dynamique.
     *
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

    /**
     * @brief Dessine le fond, la bordure et le texte centré du bouton.
     *
     * @param display Écran TFT_eSPI utilisé pour le dessin, sans transfert de propriété.
     */
    void draw(TFT_eSPI& display) const;

private:
    int16_t x_;
    int16_t y_;
    int16_t width_;
    int16_t height_;
    const char* text_;
};

}  // namespace CYDUI

#endif
