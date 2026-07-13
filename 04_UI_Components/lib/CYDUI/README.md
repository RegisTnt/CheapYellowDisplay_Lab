# CYDUI

CYDUI est une petite bibliothèque graphique locale pour l'ESP32-2432S028. Sa version **0.1.0** introduit un unique composant réutilisable : `CYDUI::Button`.

## API actuelle

```cpp
#include <Button.h>

CYDUI::Button button(20, 60, 130, 55, "OUVRIR");
button.draw(tft);
```

Le bouton conserve sa position, ses dimensions et un pointeur `const char*` vers son libellé. La méthode `draw()` reçoit l'écran TFT_eSPI par référence et dessine un fond arrondi, une bordure et un texte centré.

## Limites de la version 0.1.0

La bibliothèque ne gère ni tactile, ni état pressé, ni action, ni callback, ni navigation. Elle ne réalise aucune allocation dynamique et ne possède pas l'écran.

## Évolutions envisagées

De futurs sprints pourront étudier une méthode `contains(x, y)`, les états normal, pressé et désactivé, un thème, un label, une barre de progression et le tactile. Ces fonctions ne sont pas implémentées dans cette version.
