# 07 - Toulon Weather

## Objectif

Ce projet PlatformIO autonome affiche sur le Cheap Yellow Display la météo actuelle de Toulon, la nuit à venir, demain, le Soleil, la phase de la Lune et l'heure de dernière mise à jour. La première version utilise un écran synthétique unique, plus lisible qu'une navigation multi-page sur 320 × 240.

## Matériel et bibliothèques

- ESP32-2432S028R et son écran ILI9341 320 × 240 ;
- câble USB de données ;
- TFT_eSPI 2.5.43 ;
- ArduinoJson 7.4.3 ;
- bibliothèques ESP32 intégrées `WiFi`, `HTTPClient` et `WiFiClientSecure`.

Le tactile n'est pas nécessaire. La configuration TFT reprend exactement les broches et paramètres déjà validés dans les projets précédents.

## Configuration Wi-Fi

Copier `include/config.example.h` vers `include/config.h`, puis renseigner :

```cpp
#define WIFI_SSID "MonReseau"
#define WIFI_PASSWORD "MonMotDePasse"
```

Pour utiliser Open-Meteo, commenter `#define WEATHER_DEMO_MODE`. Le fichier `config.h` est ignoré par Git et ne doit jamais être committé. Si le fichier est absent, le projet utilise automatiquement `config.example.h` et compile en mode démo.

## Source météo

La source est l'[API de prévision Open-Meteo](https://open-meteo.com/en/docs), gratuite et sans clé, aux coordonnées de Toulon `43.1242, 5.9280` et dans le fuseau `Europe/Paris`.

```text
https://api.open-meteo.com/v1/forecast
  ?latitude=43.1242&longitude=5.9280
  &current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m
  &hourly=temperature_2m,weather_code,precipitation_probability
  &daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,sunrise,sunset
  &timezone=Europe%2FParis&forecast_days=3
```

Les réponses sont filtrées pendant le parsing ArduinoJson afin de ne conserver que ces champs.

## Cette nuit et demain

« Cette nuit » couvre **22 h aujourd'hui à 6 h demain**. Le client calcule la température moyenne, la minimale, le code WMO le plus fréquent et la probabilité maximale de pluie sur ces neuf points horaires. Si la plage est incomplète, les valeurs quotidiennes de demain servent de repli. Demain utilise directement les minima, maxima, code quotidien et probabilité maximale fournis par l'API.

## Heure locale et phase lunaire

NTP utilise `pool.ntp.org` et `time.nist.gov`. La règle POSIX `CET-1CEST,M3.5.0,M10.5.0/3` gère automatiquement l'heure d'été et l'heure d'hiver de `Europe/Paris`, sans décalage UTC fixe.

La phase lunaire est calculée localement à partir d'une nouvelle Lune de référence (jour julien `2451550.1`) et d'une lunaison moyenne de **29,53059 jours**. L'illumination suit une approximation cosinusoïdale. Ce calcul est adapté à un tableau de bord mais pas à l'astronomie de précision.

## Interface

Le thème méditerranéen utilise un en-tête azur et quatre cartes arrondies : cette nuit, demain, Lune et Soleil. Les icônes météo et lunaire sont dessinées avec les primitives TFT_eSPI ; aucune image ou police distante n'est chargée.

```text
┌────────────── TOULON / actuel ──────────────┐
│ CETTE NUIT               │ DEMAIN           │
│ température, ciel, pluie │ min/max, ciel    │
├──────────────────────────┼──────────────────┤
│ LUNE                     │ SOLEIL           │
│ phase et illumination    │ lever / coucher  │
└──────────────────────────┴──────────────────┘
```

## Rafraîchissement et mode hors ligne

La météo est demandée au démarrage, après reconnexion et toutes les **15 minutes**. Après une erreur, une nouvelle tentative a lieu après une minute. Les connexions ont des délais bornés à 4–5 secondes. Les dernières données valides restent en mémoire et l'écran affiche `HORS LIGNE` ou `DONNEES ANCIENNES` au lieu d'être effacé ou de redémarrer.

La connexion Wi-Fi est retentée toutes les 30 secondes sans boucle bloquante dans `setup()`.

## Mode démonstration

`WEATHER_DEMO_MODE` teste l'interface sans Wi-Fi. Choisir `WEATHER_DEMO_SCENARIO` :

| Valeur | Cas |
|---:|---|
| 0 | Soleil, valeurs estivales normales |
| 1 | Pluie forte |
| 2 | Météo indisponible |
| 3 | Températures négatives et neige |
| 4 | Température supérieure à 40 °C |

Les libellés lunaires longs sont abrégés uniquement à l'affichage. Pour contrôler les changements d'heure et les phases proches d'une nouvelle ou pleine Lune, appeler `calculateMoonPhase()` avec des dates fixes et vérifier `localtime_r()` autour des transitions de mars et octobre.

## Compilation et téléversement

```powershell
pio run
pio run --target upload
pio device monitor
```

Le moniteur série utilise 115 200 bauds.

## Limites connues

- Les valeurs de nuit reposent sur une plage horaire fixe et non sur le coucher/lever réel.
- Le calcul lunaire est approximatif.
- La connexion HTTPS utilise actuellement `setInsecure()` : le trafic est chiffré, mais le certificat du serveur n'est pas authentifié.
- Les données restent uniquement en RAM et sont perdues après redémarrage.
- Le téléchargement HTTP est synchrone mais borné ; l'écran conserve les dernières données pendant l'opération.
