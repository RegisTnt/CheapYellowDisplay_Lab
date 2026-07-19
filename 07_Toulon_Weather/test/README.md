# Matrice de test

Le mode démo propose cinq scénarios via `WEATHER_DEMO_SCENARIO` : soleil, pluie, données indisponibles, températures négatives et canicule. Les phases longues, les dates de nouvelle/pleine Lune et les transitions été/hiver sont à vérifier avec des dates fixes passées à `calculateMoonPhase()` et avec la règle POSIX documentée dans le README.
