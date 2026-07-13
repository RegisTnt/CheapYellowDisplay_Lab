# 05 - Touch Input

## Objectif

Ce laboratoire présente le contrôleur tactile résistif XPT2046 et la conversion de ses coordonnées brutes en coordonnées écran. Il prépare les futures interactions avec CYDUI sans déclencher d'action sur un bouton.

## Concepts abordés

- écran tactile résistif et contrôleur XPT2046 ;
- bus SPI dédié ;
- injection de dépendance par référence ;
- coordonnées brutes et calibration ;
- transformation vers les pixels de l'écran ;
- mesure de pression ;
- séparation entre lecture d'entrée et affichage.

## Brochage de l'ESP32-2432S028R

| Signal tactile | GPIO |
|---|---:|
| MOSI | 32 |
| MISO | 39 |
| SCLK | 25 |
| CS | 33 |
| IRQ | 36 |

Sur cette variante du CYD, le tactile et l'écran utilisent deux bus SPI distincts. Le TFT conserve ses broches 12 à 15 et le programme crée une instance `HSPI` séparée pour le XPT2046. Ce brochage doit être revérifié si la carte n'est pas explicitement une ESP32-2432S028R avec ILI9341.

## Fonctionnement

Le XPT2046 mesure des positions électriques comprises approximativement entre 0 et 4095 ainsi qu'une pression Z. Ces valeurs ne correspondent pas directement aux pixels : les tolérances de la dalle rendent une calibration nécessaire.

La bibliothèque XPT2046 est réglée sur la rotation `1`, comme l'écran en paysage. Les axes bruts X et Y sont ainsi alignés sur les axes écran avant le mapping. `CYDTouch` borne chaque mesure dans l'intervalle calibré, puis la transforme vers `0..319` et `0..239`. Si un axe apparaît en miroir, ses deux bornes peuvent être inversées. Une autre révision de dalle peut aussi nécessiter de permuter les calibrations X et Y.

## API

```cpp
SPIClass touchSpi(HSPI);
CYDTouch::TouchController touch(touchSpi, 33, 36);

touchSpi.begin(25, 39, 32, 33);
touch.setScreenSize(320, 240);
touch.setCalibration(200, 3800, 200, 3800);
touch.begin();

CYDTouch::TouchPoint point;
if (touch.read(point)) {
    Serial.printf("raw=%d,%d screen=%d,%d z=%d\n",
                  point.rawX, point.rawY, point.x, point.y, point.pressure);
}
```

## Résultat attendu

L'écran sombre affiche le titre `CYD Touch Lab`, l'état `TOUCH` ou `RELEASED`, les coordonnées écran, les coordonnées brutes et la pression. Un réticule jaune suit le doigt ou le stylet. Les mêmes mesures sont écrites dans le moniteur série.

## Compilation

Ouvrir `05_Touch_Input` comme projet PlatformIO, puis utiliser **Build**, **Upload** et **Monitor**, ou lancer :

```powershell
pio run
pio run --target upload
pio device monitor
```

Le moniteur série fonctionne à 115 200 bauds.

## Calibration pratique

Les constantes `TOUCH_RAW_MIN_X`, `TOUCH_RAW_MAX_X`, `TOUCH_RAW_MIN_Y` et `TOUCH_RAW_MAX_Y` de `src/main.cpp` sont volontairement provisoires.

1. Toucher précisément le coin supérieur gauche et noter `rawX` et `rawY`.
2. Toucher le coin inférieur droit et noter `rawX` et `rawY`.
3. Reporter les valeurs dans les quatre constantes.
4. Inverser les bornes d'un axe si son déplacement est en miroir.
5. Si les directions sont permutées, échanger les calibrations X et Y.
6. Recompiler et recommencer jusqu'à ce que le réticule suive correctement le contact.

## Dépannage

- **Aucune détection tactile** : confirmer le modèle ESP32-2432S028R, le câblage IRQ/CS et un appui assez ferme avec un stylet adapté au résistif.
- **Coordonnées figées** : vérifier les cinq broches et que `touchSpi.begin(25, 39, 32, 33)` est exécuté avant `TouchController::begin()`.
- **Axes inversés** : échanger les calibrations X et Y après avoir comparé les valeurs brutes dans les quatre coins.
- **Coordonnées en miroir** : inverser la paire min/max de l'axe concerné.
- **Coordonnées hors écran** : contrôler les dimensions passées à `setScreenSize()` et les bornes de calibration ; la bibliothèque borne normalement le résultat.
- **Pression nulle** : appuyer plus fermement, contrôler IRQ et MISO, puis observer les valeurs brutes dans le moniteur.
- **Mauvais bus SPI** : utiliser l'instance `HSPI` dédiée avec SCLK 25, MISO 39 et MOSI 32.
- **Conflit TFT/tactile** : ne pas utiliser le support tactile intégré de TFT_eSPI et ne pas réutiliser son instance SPI.
- **Dépendance XPT2046 absente** : vérifier le tag officiel `v1.4` dans `lib_deps`, puis reconstruire avec un accès réseau.
- **Mauvais modèle de carte** : les variantes USB-C ou équipées d'un ST7789 peuvent nécessiter une autre configuration.
- **Tactile décalé après rotation** : confirmer `setRotation(1)` pour le TFT et refaire la calibration en paysage.

## Limites volontaires

Ce sprint n'implémente ni boutons interactifs, ni événements, ni callbacks, ni navigation, ni action métier, ni Wi-Fi, ni portail, ni LVGL.
