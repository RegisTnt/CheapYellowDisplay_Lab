# 06 - Interactive UI

## Objectif

Ce projet relie la lecture tactile CYDTouch aux composants graphiques CYDUI sans introduire de logique métier. Les boutons réagissent visuellement à l'appui et signalent un clic complet dans le moniteur série.

## Concepts abordés

- détection de zone avec limites incluses ;
- états visuels normal, pressé et désactivé ;
- transitions appui et relâchement ;
- validation ou annulation d'un clic ;
- séparation des responsabilités sans callback ;
- redessin partiel ;
- anti-rebond temporel.

## Architecture

```text
CYDTouch::TouchController
        ↓
CYDTouch::TouchPoint
        ↓
logique d'interaction dans main.cpp
        ↓
CYDUI::Button
```

CYDTouch ne connaît pas l'affichage. CYDUI ne connaît pas le tactile. Seul `main.cpp` relie les deux bibliothèques et décide si un geste constitue un clic.

## API Button 0.3.0

- `draw(display)` dessine le bouton selon son état ;
- `contains(x, y)` vérifie si un point est dans ses limites et retourne `false` s'il est désactivé ;
- `setState(state)` et `state()` modifient ou lisent l'état ;
- `setEnabled(enabled)` et `isEnabled()` désactivent ou réactivent le bouton ;
- `text()` retourne son libellé constant.

## Scénario de clic

Au premier contact dans un bouton, l'application capture ce bouton et le dessine en état `Pressed`. Une sortie de sa zone restaure l'état `Normal` et affiche `Appui annule`. Si le contact revient dans le même bouton avant le relâchement, l'état `Pressed` est restauré. Au relâchement, un clic est déclaré uniquement si la dernière position tactile se trouve dans le bouton capturé.

Le maintien ne crée pas de clic supplémentaire : la validation se produit uniquement lors de la transition contact vers relâchement. Un intervalle minimal de **250 ms** entre deux clics filtre les rebonds rapides.

## Animations et retour visuel

- `Normal` utilise un fond bleu foncé, une bordure cyan et un texte blanc.
- `Pressed` utilise un fond cyan et une bordure blanche. Sa géométrie est réduite de deux pixels sur chaque côté et son texte est légèrement décalé pour créer un effet d'enfoncement.
- `Confirmed` utilise un fond vert et reste affiché pendant **220 ms** avant le retour automatique à `Normal`.
- `Disabled` utilise des gris et refuse les tests `contains()`.

La confirmation est pilotée par `millis()` sans attente bloquante. Seul le bouton dont l'état change est redessiné, et la zone de statut n'est actualisée que si son texte change. La sortie de zone restaure immédiatement le bouton ; un retour avant relâchement réactive l'effet pressé.

## Résultat attendu

L'écran paysage affiche le titre `CYD Interactive UI`, les boutons `OUVRIR`, `PIETON` et `PARAMETRES`, puis une zone de statut. Un bouton devient cyan pendant l'appui et reprend son style normal au relâchement. Un clic valide produit par exemple `Button clicked: OUVRIR` sur le port série et `Clic : OUVRIR` à l'écran.

## Compilation

Ouvrir `06_Interactive_UI` comme projet PlatformIO, puis utiliser **Build**, **Upload** et **Monitor**, ou lancer :

```powershell
pio run
pio run --target upload
pio device monitor
```

Le moniteur série utilise 115 200 bauds.

## Validation manuelle

1. Toucher `OUVRIR` et vérifier son changement visuel.
2. Relâcher sur `OUVRIR` et vérifier le message série.
3. Vérifier le flash vert de confirmation puis le retour automatique à l'état normal.
4. Toucher `OUVRIR`, glisser hors de sa zone et vérifier l'annulation si le relâchement reste extérieur.
5. Revenir dans le bouton avant de relâcher et vérifier que l'état pressé revient.
6. Maintenir le contact, puis relâcher et vérifier qu'un seul clic est généré.
7. Refaire le test avec `PIETON` puis `PARAMETRES`.

## Dépannage

- **Bouton jamais pressé** : vérifier la calibration et afficher les coordonnées du projet `05_Touch_Input`.
- **Mauvais calibrage** : reprendre les valeurs brutes dans les quatre coins et corriger les constantes de `main.cpp`.
- **Clic déclenché plusieurs fois** : vérifier la transition `previousTouched` et l'intervalle `MIN_CLICK_INTERVAL_MS`.
- **Bouton restant pressé** : vérifier que le relâchement XPT2046 est détecté et que l'état repasse à `Normal`.
- **Clic déclaré après glissement** : vérifier que `gestureCancelled` devient vrai dès la première sortie de zone.
- **Coordonnées hors écran** : contrôler `setScreenSize()` et les quatre bornes de calibration.
- **Écran qui scintille** : ne pas appeler `fillScreen()` dans `loop()`.
- **Redessin trop fréquent** : conserver le dessin conditionnel dans `setButtonState()` et `setStatus()`.
- **Tactile non détecté** : vérifier HSPI et les broches SCLK 25, MISO 39, MOSI 32, CS 33 et IRQ 36.
- **Dépendance locale non reconnue** : confirmer les fichiers `library.json`, puis supprimer uniquement le cache `.pio/` du projet et reconstruire.

La calibration `200–3800` est reprise de `05_Touch_Input` mais reste provisoire tant qu'elle n'a pas été mesurée physiquement sur la dalle utilisée.

## Limites volontaires

Ce sprint ne contient aucune action réelle, commande de portail, connexion Wi-Fi, requête HTTP, MQTT, callback, navigation multi-écrans ou dépendance LVGL.
