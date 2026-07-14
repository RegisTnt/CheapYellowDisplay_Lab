# CYDUI 0.3.0

CYDUI fournit des composants graphiques simples pour le Cheap Yellow Display. La version **0.3.0** propose quatre états visuels : `Normal`, `Pressed`, `Confirmed` et `Disabled`.

## API Button

- `draw(display)` dessine le style correspondant à l'état courant ;
- `contains(x, y)` teste les limites incluses et retourne `false` si le bouton est désactivé ;
- `setState(state)` et `state()` modifient ou lisent l'état visuel ;
- `setEnabled(enabled)` et `isEnabled()` contrôlent la disponibilité du bouton ;
- `text()` expose le libellé constant sans allocation.

```cpp
CYDUI::Button button(20, 60, 130, 55, "OUVRIR");

if (button.contains(point.x, point.y)) {
    button.setState(CYDUI::ButtonState::Pressed);
    button.draw(tft);
}
```

## Choix de conception et limites

`Pressed` réduit la géométrie de deux pixels sur chaque côté et décale légèrement le texte. `Confirmed` utilise un fond vert. Avant chaque dessin, la zone complète du bouton est nettoyée afin de ne laisser aucune trace entre les géométries.

Le bouton mémorise seulement sa géométrie, son libellé et son état. Il ne connaît ni CYDTouch, ni le portail, ni une action métier. La lecture tactile, la durée de confirmation et la validation du clic restent dans l'application. Aucun callback ou mécanisme d'événements n'est intégré.
