# 02 - Graphics primitives

## Objectif pédagogique

Ce deuxième laboratoire présente les primitives graphiques essentielles de **TFT_eSPI** sur le Cheap Yellow Display. Il permet de comprendre comment placer du texte et des formes simples avant d'aborder, dans de futurs projets indépendants, le tactile et les composants d'interface.

## Matériel

- une carte ESP32-2432S028 (Cheap Yellow Display ou CYD) ;
- son écran TFT de 2,8 pouces ;
- un câble USB de données.

## Prérequis

- Visual Studio Code ;
- l'extension PlatformIO IDE ;
- le pilote USB adapté à la carte (souvent CH340 ou CP210x) ;
- le projet précédent compilé, téléversé et validé sur la carte.

## Compilation, téléversement et moniteur série

Ouvrir le dossier `02_Graphics` comme projet PlatformIO dans VS Code, puis utiliser successivement les actions **Build**, **Upload** et **Monitor** de la barre PlatformIO. Le moniteur série est configuré à 115 200 bauds.

Les mêmes opérations peuvent être lancées dans un terminal ouvert dans ce dossier :

```powershell
pio run
pio run --target upload
pio device monitor
```

## Résultat attendu

En orientation paysage, l'écran noir affiche le titre cyan `CYD Graphics Lab`, une ligne horizontale de séparation, plusieurs formes géométriques colorées et une barre de progression verte indiquant `64 %`. Le moniteur série confirme également le lancement du test graphique.

## Primitives expliquées

| Primitive | Rôle |
|---|---|
| `drawFastHLine(x, y, largeur, couleur)` | Trace rapidement une ligne horizontale. |
| `drawRect(x, y, largeur, hauteur, couleur)` | Dessine le contour d'un rectangle. |
| `fillRect(x, y, largeur, hauteur, couleur)` | Dessine un rectangle entièrement rempli. |
| `drawRoundRect(x, y, largeur, hauteur, rayon, couleur)` | Dessine le contour d'un rectangle aux angles arrondis. |
| `fillRoundRect(x, y, largeur, hauteur, rayon, couleur)` | Dessine une surface rectangulaire remplie aux angles arrondis. |
| `drawCircle(x, y, rayon, couleur)` | Dessine le contour d'un cercle centré en `(x, y)`. |
| `fillCircle(x, y, rayon, couleur)` | Dessine un disque rempli centré en `(x, y)`. |
| `drawTriangle(x1, y1, x2, y2, x3, y3, couleur)` | Relie trois points pour former un triangle. |
| `drawString(texte, x, y, police)` | Affiche du texte à partir du point d'ancrage choisi. |

Les couleurs comme `TFT_CYAN`, `TFT_BLUE` ou `TFT_GREEN` sont des constantes prédéfinies par TFT_eSPI. Elles représentent des couleurs RGB encodées sur 16 bits.

## Système de coordonnées

L'origine `(0, 0)` se trouve en haut à gauche de l'écran. La coordonnée **x** augmente vers la droite et la coordonnée **y** augmente vers le bas. Après `setRotation(1)`, le CYD est normalement en paysage : sa largeur vaut 320 pixels et sa hauteur 240 pixels.

`setTextDatum()` choisit le point d'ancrage utilisé pour positionner le texte. Par exemple, `TC_DATUM` utilise le centre supérieur du texte et `MC_DATUM` son centre exact. Il devient ainsi facile de centrer un libellé sur l'écran ou dans une barre.

## Contour ou remplissage

TFT_eSPI suit une convention simple :

- les fonctions dont le nom commence par `draw...` tracent uniquement le contour ;
- les fonctions dont le nom commence par `fill...` remplissent toute la surface.

## Dépannage

- **`TFT_eSPI.h: No such file or directory`** : ouvrir le bon dossier PlatformIO, vérifier la connexion réseau, puis relancer `pio run` pour installer les dépendances.
- **Dépendance absente** : vérifier que `bodmer/TFT_eSPI@^2.5.43` figure bien dans la section `lib_deps` de `platformio.ini`.
- **Écran noir** : vérifier l'alimentation et la commande du rétroéclairage sur GPIO 21, active à l'état `HIGH`.
- **Écran éclairé mais vide** : contrôler les broches SPI et les options `ILI9341_2_DRIVER` et `USER_SETUP_LOADED` dans `platformio.ini`.
- **Mauvaise orientation** : vérifier l'appel à `setRotation(1)` ; la valeur dépend du sens physique souhaité.
- **Éléments hors écran** : contrôler les coordonnées et les dimensions ; en paysage, respecter normalement `0 <= x < 320` et `0 <= y < 240`.
- **Mauvais port COM** : utiliser `pio device list`, fermer les autres moniteurs série et sélectionner le port apparu au branchement de la carte.
- **Téléversement bloqué sur `Connecting...`** : fermer le moniteur série, relancer Upload et, si nécessaire, maintenir brièvement le bouton `BOOT` pendant la connexion.

Le téléversement et le rendu doivent toujours être vérifiés physiquement sur la carte utilisée.
