# 04 - UI Components

## Objectif

Ce projet introduit la programmation orientée objet avec un premier composant graphique réutilisable : `CYDUI::Button`. Il transforme plusieurs primitives TFT_eSPI en un bouton affichable, sans encore lui donner de comportement interactif.

## Concepts abordés

- encapsulation des données dans une classe ;
- responsabilité unique d'un composant ;
- séparation de la déclaration `.h` et de l'implémentation `.cpp` ;
- bibliothèque PlatformIO locale ;
- passage d'une dépendance par référence ;
- absence d'allocation dynamique ;
- distinction entre une primitive graphique et un composant UI réutilisable.

Une primitive comme `fillRoundRect()` effectue une seule opération de dessin. Le composant `Button` combine plusieurs primitives, une géométrie et un libellé derrière une API simple.

## Architecture

| Fichier | Rôle |
|---|---|
| `src/main.cpp` | Initialise le CYD et construit la démonstration avec trois boutons. |
| `lib/CYDUI/src/Button.h` | Déclare l'API publique et les attributs privés de `CYDUI::Button`. |
| `lib/CYDUI/src/Button.cpp` | Dessine le fond, la bordure et le texte centré. |
| `lib/CYDUI/library.json` | Décrit la bibliothèque locale, sa version et sa dépendance TFT_eSPI. |

PlatformIO détecte automatiquement les bibliothèques placées dans le dossier `lib/` du projet.

## API

```cpp
#include <Button.h>

CYDUI::Button button(20, 60, 130, 55, "OUVRIR");
button.draw(tft);
```

## Choix de conception

La classe ne possède pas l'écran : `draw()` reçoit une référence `TFT_eSPI&`, ce qui rend la dépendance explicite sans copie et sans accès global depuis la bibliothèque. Le libellé utilise `const char*` afin d'éviter la classe `String` et toute allocation implicite ; le texte fourni doit donc rester valide pendant la vie du bouton.

Le composant est volontairement passif. Il n'intègre ni tactile ni callback, et les trois instances de la démonstration sont créées sans `new` ou autre allocation dynamique.

## Résultat attendu

En orientation paysage, l'écran présente un fond noir, le titre cyan `CYDUI Components` et trois boutons bleu foncé aux contours cyan. Les libellés blancs `OUVRIR`, `PIETON` et `PARAMETRES` sont centrés horizontalement et verticalement.

## Compilation

Ouvrir `04_UI_Components` comme projet PlatformIO, puis utiliser **Build**, **Upload** et **Monitor**. Les mêmes actions sont disponibles dans un terminal ouvert dans ce dossier :

```powershell
pio run
pio run --target upload
pio device monitor
```

Le moniteur série utilise 115 200 bauds et doit afficher `CYDUI button demo ready.`.

## Dépannage

- **Bibliothèque CYDUI non détectée** : vérifier que `library.json`, `Button.h` et `Button.cpp` se trouvent bien sous `lib/CYDUI/`, puis supprimer `.pio/` et relancer le Build.
- **Erreur d'inclusion `Button.h`** : ouvrir `04_UI_Components` comme racine du projet PlatformIO et conserver `#include <Button.h>`.
- **TFT_eSPI absente** : contrôler `bodmer/TFT_eSPI@^2.5.43` dans `lib_deps`, puis relancer la compilation avec un accès réseau.
- **Texte non centré** : vérifier l'utilisation de `MC_DATUM` et les coordonnées du centre calculées dans `Button::draw()`.
- **Éléments hors écran** : en paysage, garder normalement les coordonnées dans une surface de 320 × 240 pixels.
- **Mauvaise rotation** : vérifier que `tft.setRotation(1)` est appelé après l'initialisation de l'écran.
- **Écran noir** : contrôler l'alimentation, le rétroéclairage GPIO 21 actif à `HIGH`, les broches SPI et le pilote `ILI9341_2_DRIVER`.

## Limites volontaires

Ce sprint n'implémente pas le tactile, l'état pressé, les actions, les callbacks, la navigation ou un gestionnaire d'écran. Il ne contient aucun autre composant UI que `Button`.
