#pragma once

// Copier vers ToulonHomeConfig.h et renseigner les valeurs réelles.
// ToulonHomeConfig.h est ignoré par Git.
#define WIFI_SSID "MonReseau"
#define WIFI_PASSWORD "MonMotDePasse"
#define PORTAL_BASE_URL "http://portail.local"
#define OTA_PASSWORD "change-me"

// Le build reproductible par défaut n'envoie aucune requête réelle.
#define DEMO_MODE

// 0 soleil/fermé, 1 pluie/ouvert, 2 nuit/fermé,
// 3 API portail indisponible, 4 Wi-Fi perdu, 5 portail ouvert.
#define DEMO_SCENARIO 0
