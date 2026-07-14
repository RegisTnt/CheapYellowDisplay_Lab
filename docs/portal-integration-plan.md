# Plan d'intégration de PortailControl sur Cheap Yellow Display

Ce document prépare le futur projet `07_Portal_Remote`. Il ne crée aucune connexion Wi-Fi, aucune requête HTTP et aucune commande réelle dans le laboratoire actuel.

## Point de départ audité

- Projet interactif le plus récent : `06_Interactive_UI` (`main` au commit `4664a72`).
- Carte : ESP32-2432S028 / ESP32-WROOM, écran paysage 320 × 240 piloté par TFT_eSPI 2.5.43 et `ILI9341_2_DRIVER`.
- `CYDUI` : version 0.3.0, bouton à états `Normal`, `Pressed`, `Confirmed`, `Disabled`.
- `CYDTouch` : version 0.1.0, XPT2046 1.4 sur HSPI.
- Tactile : SCLK 25, MISO 39, MOSI 32, CS 33, IRQ 36 ; calibration brute provisoire X/Y `200–3800`, rotation 1, encore à valider physiquement sur chaque dalle.
- Interaction : clic validé au relâchement dans la zone, lecture toutes les 30 ms, anti-rebond global 250 ms, confirmation visuelle 220 ms. Aucun callback métier.
- Réseau : absent de tous les projets actuels. Aucun secret Wi-Fi n'est présent dans le code compilable audité.

Les libellés actuels `OUVRIR` et `PIETON` sont seulement démonstratifs : aucun clic ne déclenche de réseau ou de portail.

## Contrat PortailControl à consommer

Adresse de base configurable, par exemple `http://portail.local`, jamais codée en dur dans une bibliothèque. Une configuration locale future devra fournir le SSID, le mot de passe et l'adresse de base sans les versionner.

| Usage | Requête | Réponse attendue | Politique client |
|---|---|---|---|
| Lire le capteur | `GET /etat` | `text/plain`: `ferme` ou `ouvert` | Timeout court, pas de cache ; `ouvert` signifie « non confirmé fermé ». |
| Impulsion piétonne | `GET /pieton` | HTTP 200, texte | Effet physique. Confirmation humaine et aucun retry. |
| Impulsion complète | `GET /voiture` | HTTP 200, texte | Effet physique. Ne pas l'étiqueter ouvrir/fermer ; aucun retry. |
| Lire l'historique | `GET /log.txt` | texte CSV `horodatage;état` | Lecture optionnelle, réseau direct. |

Le HTTP 200 d'une commande confirme seulement que le handler a répondu. Il ne confirme ni l'activation effective du relais ni le mouvement. L'état physique est partiellement observable via `/etat` seulement.

## Architecture proposée

```text
CYDTouch ──> PortalRemoteApp <── CYDUI
                    │
                    v
               PortalClient
                    │
                    v
                CYDNetwork
                    │
                    v
              PortailControl
```

### CYDUI

- Dessiner les composants et leurs états visuels.
- Exposer les zones de contact sans connaître Wi-Fi, HTTP ou le portail.
- Ajouter au besoin des composants de dialogue, badge réseau et statut, tout en gardant l'API graphique indépendante.

### CYDTouch

- Lire le XPT2046 et convertir les coordonnées brutes en pixels.
- Ne connaître ni HTTP ni la logique métier.
- Conserver la calibration configurable ; valider les quatre coins physiquement avant usage portail.

### CYDNetwork

- Gérer connexion et reconnexion Wi-Fi de manière contrôlée.
- Exposer un état structuré : déconnecté, connexion, connecté, erreur.
- Fournir adresse IP locale et RSSI.
- Ne jamais stocker ni rejouer une requête de portail.
- La reconnexion Wi-Fi peut être automatique ; la répétition d'une commande portail ne le peut pas.

### PortalClient

- Conserver l'adresse de base configurable et la normaliser sans slash terminal.
- Fournir `readState()` et une méthode explicite par commande, par exemple `sendPedestrianPulse()` et `sendFullPulse()`.
- Utiliser un timeout (proposition : 5 s) et désactiver le cache.
- Retourner un résultat structuré, sans texte d'interface :

```cpp
enum class PortalResultKind {
    StateConfirmedClosed,
    StateNotConfirmedClosed,
    CommandAcceptedByHttp,
    HttpRejected,
    TimeoutUnknown,
    NetworkUnavailable,
    InvalidResponse
};

struct PortalResult {
    PortalResultKind kind;
    int httpStatus;
};
```

- Ne jamais effectuer de retry automatique sur une commande, y compris après timeout.
- Ne contenir aucune confirmation, animation, bouton ni navigation.
- Ne pas transformer `ouvert` en direction mécanique garantie.

### PortalRemoteApp

- Gérer les écrans, le statut réseau et la dernière lecture.
- Demander une confirmation en deux étapes avant toute impulsion réelle.
- Désactiver les commandes pendant l'envoi et pendant un anti-double-clic configurable (proposition initiale : 3 s).
- Afficher séparément : requête transmise, réponse HTTP reçue, capteur fermé confirmé.
- Après réponse, planifier une seule lecture de `/etat`; cette lecture peut être retentée comme lecture seulement, jamais la commande.
- Afficher clairement timeout, refus HTTP, réponse invalide et Wi-Fi indisponible.
- Ne déclencher aucune commande au démarrage, à la reconnexion, au retour d'écran ou après une veille.

## Séquence sûre d'une commande future

1. L'utilisateur touche puis relâche un bouton.
2. `PortalRemoteApp` affiche le nom exact de l'impulsion et demande confirmation.
3. L'utilisateur confirme ; l'application verrouille les deux commandes.
4. Elle vérifie l'état Wi-Fi. En cas d'échec, elle s'arrête sans file d'attente.
5. `PortalClient` effectue une unique requête avec timeout.
6. L'UI affiche « réponse HTTP reçue » ou « résultat inconnu/refusé », sans prétendre que le portail a bougé.
7. Une lecture `/etat` séparée actualise le capteur.
8. Le verrou est levé après le délai anti-double-appui ; une nouvelle impulsion exige une nouvelle action humaine complète.

## Configuration et secrets

Le futur projet devra versionner `include/config.example.h` ou `include/secrets.example.h` et ignorer la copie réelle. Exemple de paramètres :

```cpp
#define WIFI_SSID "replace-me"
#define WIFI_PASSWORD "replace-me"
#define PORTAL_BASE_URL "http://portail.local"
```

Ne jamais versionner mot de passe Wi-Fi, mot de passe HTTP/OTA, clé API, jeton, adresse publique ou certificat privé. Une adresse LAN ou mDNS peut être configurable, mais aucune ouverture Internet ne doit être supposée.

## Tests prévus avant activation réelle

1. Tests unitaires du parsing `ferme`, `ouvert`, corps vide et corps invalide.
2. Tests contre un serveur simulé local : 200, 4xx, 5xx, timeout et coupure réseau.
3. Vérification qu'une commande n'est appelée qu'une fois par confirmation.
4. Vérification qu'aucune commande n'est conservée après redémarrage ou reconnexion.
5. Tests UI avec un `PortalClient` factice, sans Wi-Fi vers le portail.
6. Validation physique de la calibration tactile.
7. Revue humaine de l'adresse de base et des libellés.
8. Seulement dans une tâche ultérieure et avec présence humaine : essai réel unique, portail visible et arrêt d'urgence accessible.

## Cahier des charges proposé pour `07_Portal_Remote`

Créer un projet PlatformIO indépendant basé sur `06_Interactive_UI`, sans modifier les laboratoires précédents. Livrer les cinq couches ci-dessus, configuration locale ignorée, simulateur HTTP ou client factice activé par défaut, écran 320 × 240 avec état réseau/capteur, deux commandes par impulsion et dialogue de confirmation. Le build par défaut ne doit joindre aucun portail réel. Un drapeau de compilation explicite, absent du dépôt et activé manuellement, devra être requis pour autoriser les commandes réelles. Les critères d'acceptation prioritaires sont : zéro commande au démarrage, zéro retry de commande, zéro file hors ligne, un seul envoi par confirmation, distinction des trois niveaux de confirmation et compilation reproductible sans secret.
