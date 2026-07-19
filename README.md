# Cheap Yellow Display Lab

Ce dépôt rassemble des laboratoires progressifs autour de la carte **ESP32-2432S028**, couramment appelée **Cheap Yellow Display** ou **CYD**. Le premier projet valide toute la chaîne minimale : compilation PlatformIO, téléversement USB, communication série, configuration matérielle TFT et affichage d'un Hello World.

## Projets disponibles

- `01_Hello_world` valide la chaîne PlatformIO, le port série et un premier affichage sur le CYD.
- `02_Hello_Display` propose une étape d'affichage simple et indépendante.
- `02_Graphics` présente progressivement les primitives graphiques de TFT_eSPI.
- `04_UI_Components` introduit un premier composant graphique orienté objet et la bibliothèque locale CYDUI.
- `05_Touch_Input` assure la lecture, la calibration et la visualisation des coordonnées tactiles du contrôleur XPT2046.
- `06_Interactive_UI` propose des boutons tactiles interactifs avec états visuels et détection de clic sans logique métier.
- `07_Toulon_Weather` affiche la météo de Toulon, les prévisions, le Soleil et la phase de la Lune dans un tableau de bord méditerranéen.

## Matériel et outils

- une carte ESP32-2432S028 avec son câble USB de données ;
- un ordinateur Windows, macOS ou Linux ;
- [Visual Studio Code](https://code.visualstudio.com/) ;
- l'extension PlatformIO IDE, qui installe PlatformIO Core ;
- un pilote USB-série CH340 ou CP210x si le système ne reconnaît pas la carte ;
- Git pour récupérer le dépôt.

Le projet utilise le framework Arduino et la bibliothèque **TFT_eSPI 2.5.43**. Cette bibliothèque fournit le pilote rapide de l'écran, le dialogue SPI, les couleurs, les polices et les primitives de dessin. Sa configuration matérielle est centralisée dans `platformio.ini` afin de ne pas modifier les fichiers internes de la bibliothèque.

## Installation

1. Cloner le dépôt puis l'ouvrir dans VS Code.
2. Installer les extensions recommandées proposées par VS Code, en particulier PlatformIO IDE.
3. Ouvrir le dossier `01_Hello_world` comme projet PlatformIO.
4. Connecter le CYD avec un câble USB de données.
5. Attendre la première installation des outils Espressif et des dépendances.

## Compiler et téléverser

Dans un terminal placé dans `01_Hello_world` :

```powershell
pio run
pio run --target upload
pio device monitor
```

On peut aussi utiliser les commandes **Build**, **Upload** et **Serial Monitor** de PlatformIO dans VS Code. Le moniteur doit être réglé à **115 200 bauds**. Pour le fermer dans le terminal, utiliser `Ctrl+C`.

Si plusieurs ports sont connectés, afficher leur liste avec `pio device list`. Il est possible d'ajouter temporairement `upload_port = COMx` et `monitor_port = COMx` à l'environnement PlatformIO ; il vaut mieux ne pas versionner un numéro de port propre à une machine.

## Résultat attendu

Après le téléversement, le rétroéclairage s'allume et l'écran en orientation paysage présente un fond bleu foncé avec :

```text
Cheap Yellow Display
Hello World !
ESP32-2432S028
```

Le moniteur série affiche également :

```text
CYD initialise : Hello World affiche sur l'ecran TFT.
```

Le téléversement et le rendu final restent à confirmer physiquement sur chaque carte et câble utilisés.

## Comprendre `platformio.ini`

| Ligne | Rôle |
|---|---|
| `[env:esp32dev]` | Définit l'environnement de construction. |
| `platform = espressif32` | Installe la chaîne d'outils destinée aux ESP32. |
| `board = esp32dev` | Utilise la définition générique ESP32 Dev Module. |
| `framework = arduino` | Sélectionne l'API Arduino. |
| `monitor_speed = 115200` | Règle le moniteur série à la même vitesse que le programme. |
| `lib_deps` | Télécharge la version exacte 2.5.43 de TFT_eSPI. |
| `build_flags` | Transmet à TFT_eSPI le pilote, les dimensions, les broches, les polices et la fréquence SPI. |

La carte est déclarée comme `esp32dev` parce que PlatformIO ne fournit pas nécessairement une définition dédiée à toutes les variantes du Cheap Yellow Display. La configuration propre au CYD est donc explicitée séparément dans les options de compilation.

## Broches TFT du prototype

| Fonction | Valeur | Rôle |
|---|---:|---|
| Contrôleur | ILI9341_2 | Variante du pilote TFT utilisée par TFT_eSPI |
| Dimensions natives | 240 × 320 | Largeur et hauteur avant rotation |
| `TFT_MISO` | GPIO 12 | Données SPI de l'écran vers l'ESP32 |
| `TFT_MOSI` | GPIO 13 | Données SPI de l'ESP32 vers l'écran |
| `TFT_SCLK` | GPIO 14 | Horloge SPI |
| `TFT_CS` | GPIO 15 | Sélection du contrôleur TFT |
| `TFT_DC` | GPIO 2 | Sélection commande/données |
| `TFT_RST` | -1 | Reset TFT non connecté à un GPIO dédié |
| `TFT_BL` | GPIO 21 | Commande du rétroéclairage, active à `HIGH` |
| Fréquence SPI | 40 MHz | Horloge utilisée pour le TFT |

Ces valeurs correspondent à la configuration matérielle validée pour ce prototype et ne doivent pas être changées sans vérifier la révision exacte de la carte.

## Dépannage

- **Aucun port série** : essayer un câble USB de données, un autre port USB, puis installer le pilote CH340 ou CP210x adapté.
- **Mauvais port COM** : comparer `pio device list` avant et après avoir branché la carte.
- **Téléversement bloqué sur `Connecting...`** : fermer le moniteur série et utiliser le bouton `BOOT` pendant le début de la connexion.
- **Texte illisible dans le moniteur** : sélectionner 115 200 bauds et réinitialiser la carte.
- **Écran noir** : contrôler l'alimentation et la configuration du rétroéclairage GPIO 21 actif à `HIGH`.
- **Écran éclairé mais vide** : contrôler les broches SPI, le pilote `ILI9341_2_DRIVER` et la fréquence de 40 MHz.
- **Image tournée** : le prototype impose `setRotation(1)` ; une autre révision peut nécessiter une vérification physique.
- **Compilation impossible avec `pio` introuvable** : ouvrir un terminal PlatformIO dans VS Code ou ajouter PlatformIO Core au `PATH`.

La documentation détaillée se trouve dans [01_Hello_world/README.md](01_Hello_world/README.md), [docs/hardware.md](docs/hardware.md) et [docs/roadmap.md](docs/roadmap.md).
