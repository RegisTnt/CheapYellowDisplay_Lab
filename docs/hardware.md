# Matériel : ESP32-2432S028

## Vue d'ensemble

L'**ESP32-2432S028**, surnommé **Cheap Yellow Display (CYD)**, réunit sur un même circuit imprimé un module **ESP32-WROOM**, un écran TFT couleur et plusieurs périphériques. Le suffixe exact et l'équipement peuvent varier selon la révision ; il faut toujours comparer la sérigraphie et le schéma du vendeur avec la carte physique.

## Écran et tactile

- écran TFT couleur de 2,8 pouces ;
- résolution native de **240 × 320 pixels** ;
- contrôleur configuré comme **ILI9341_2** dans ce laboratoire ;
- interface TFT SPI à 40 MHz ;
- dalle tactile résistive généralement pilotée par un contrôleur XPT2046 séparé.

Le tactile n'est pas initialisé dans ce premier prototype. Son bus et sa calibration seront traités dans un laboratoire ultérieur.

## Autres éléments présents

- lecteur de carte microSD accessible par un bus SPI distinct ou partagé selon la révision ;
- boutons physiques de réinitialisation (`RST`) et de démarrage/flash (`BOOT`) sur les versions courantes ;
- connecteur USB pour l'alimentation, le téléversement et la liaison série ;
- connecteurs d'extension visibles sur la carte, dont le nombre, le nom et le brochage dépendent de la révision ;
- certains modèles comportent aussi une LED RGB, un capteur de lumière et une sortie audio.

Avant de raccorder un périphérique aux connecteurs, relever leur sérigraphie et vérifier le schéma de la révision : ils ne sont pas supposés libres simplement parce que ce prototype ne les utilise pas.

## Brochage TFT utilisé

| Signal | Broche ESP32 | Description |
|---|---:|---|
| `TFT_MISO` | GPIO 12 | Données reçues du contrôleur TFT |
| `TFT_MOSI` | GPIO 13 | Données envoyées au contrôleur TFT |
| `TFT_SCLK` | GPIO 14 | Horloge SPI |
| `TFT_CS` | GPIO 15 | Chip Select du TFT |
| `TFT_DC` | GPIO 2 | Sélection commande/données |
| `TFT_RST` | -1 | Reset non raccordé à une broche dédiée |
| `TFT_BL` | GPIO 21 | Rétroéclairage actif au niveau `HIGH` |

Cette table décrit uniquement la configuration validée du premier prototype. Elle ne documente pas encore les broches tactiles ou microSD.
