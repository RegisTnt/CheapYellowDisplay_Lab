# 01 — Hello World série et écran

## Objectif pédagogique

Ce premier prototype apprend à construire et téléverser un programme Arduino avec PlatformIO sur un **ESP32-2432S028**, surnommé **Cheap Yellow Display (CYD)**. Il constitue un test minimal avant d'ajouter des fonctions plus complexes.

## Ce que valide le test

- la compilation du firmware avec PlatformIO ;
- le téléversement USB vers l'ESP32 ;
- la communication série à 115 200 bauds ;
- la commande du rétroéclairage ;
- l'initialisation du contrôleur TFT ILI9341_2 ;
- l'orientation paysage et l'affichage de texte en couleur.

## Fonctionnement du programme

### `setup()`

Arduino appelle `setup()` une seule fois après le démarrage ou une réinitialisation. Le programme y initialise successivement la liaison série, le rétroéclairage et l'écran, puis dessine l'interface fixe.

- `Serial.begin(115200)` configure la liaison série à 115 200 bits par seconde. Le moniteur PlatformIO doit utiliser la même vitesse pour afficher un texte lisible.
- `pinMode()` et `digitalWrite()` configurent GPIO 21 en sortie et activent le rétroéclairage au niveau `HIGH`.
- `tft.init()` initialise le bus SPI et le contrôleur d'écran avec les paramètres fournis par `platformio.ini`.
- `tft.setRotation(1)` sélectionne l'orientation paysage. Après rotation, la surface logique mesure 320 × 240 pixels.
- `fillScreen(TFT_NAVY)` remplit tous les pixels avec un bleu foncé avant de dessiner le texte.
- `setTextDatum(MC_DATUM)` place le point d'ancrage au milieu et au centre du texte. Les coordonnées passées ensuite désignent donc le centre de chaque chaîne.
- `drawString(texte, x, y, police)` dessine une chaîne aux coordonnées demandées avec une police intégrée chargée par TFT_eSPI.

### `loop()`

Arduino répète normalement `loop()` indéfiniment. Elle est volontairement vide ici, car l'écran affiche une image statique construite une seule fois dans `setup()`.

## Couleurs TFT

TFT_eSPI fournit des constantes telles que `TFT_NAVY`, `TFT_CYAN`, `TFT_WHITE` et `TFT_LIGHTGREY`. L'écran encode les couleurs au format RGB565 : 5 bits de rouge, 6 bits de vert et 5 bits de bleu, soit 16 bits par pixel. `setTextColor(premierPlan, arrièrePlan)` fixe à la fois la couleur des caractères et celle utilisée derrière leurs pixels.

## Compiler, téléverser et observer

Depuis ce dossier :

```powershell
pio run
pio run --target upload
pio device monitor
```

Avec VS Code, les mêmes actions sont accessibles depuis la barre PlatformIO. Le résultat attendu et l'installation complète sont décrits dans le [README principal](../README.md).

## Erreurs fréquentes

- **Câble USB uniquement alimentation** : l'écran s'allume mais aucun port série n'apparaît. Utiliser un câble USB capable de transférer des données.
- **Mauvais port COM** : débrancher puis rebrancher la carte pour identifier le port apparu. Au besoin, préciser `upload_port` et `monitor_port` localement dans `platformio.ini`.
- **Pilote USB absent** : installer le pilote correspondant au convertisseur USB-série de la révision de la carte, souvent CH340 ou CP210x.
- **Blocage sur `Connecting...`** : maintenir le bouton `BOOT`, lancer le téléversement, puis relâcher le bouton quand l'écriture commence. Vérifier aussi que le moniteur série est fermé.
- **Écran noir** : vérifier l'alimentation, GPIO 21, `TFT_BACKLIGHT_ON=HIGH` et la présence de `TFT_BL=21` dans les options de compilation.
- **Écran éclairé mais vide** : le rétroéclairage fonctionne, mais pas nécessairement le bus SPI. Vérifier le pilote ILI9341_2 et toutes les broches TFT dans `platformio.ini`.
- **Orientation incorrecte** : essayer les rotations 0 à 3 pour diagnostiquer la révision, puis conserver `setRotation(1)` pour ce prototype validé en paysage.
