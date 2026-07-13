# CYDUI 0.2.0

CYDUI fournit des composants graphiques simples pour le Cheap Yellow Display. La version **0.2.0** fait évoluer `CYDUI::Button` avec une détection géométrique et trois états visuels : `Normal`, `Pressed` et `Disabled`.

## API Button

- `draw(display)` dessine le style correspondant à l'état courant ;
- `contains(x, y)` teste les limites incluses et retourne `false` si le bouton est désactivé ;
- `setState(state)` et `state()` modifient ou lisent l'état visuel ;
- `setEnabled(enabled)` et `isEnabled()` contrôlent la disponibilité du bouton.

```cpp
CYDUI::Button button(20, 60, 130, 55, "OUVRIR");

if (button.contains(point.x, point.y)) {
    button.setState(CYDUI::ButtonState::Pressed);
    button.draw(tft);
}
```

## Choix de conception et limites

Le bouton mémorise seulement sa géométrie, son libellé et son état. Il ne connaît ni CYDTouch, ni le portail, ni une action métier. La lecture tactile, la capture du bouton et la validation du clic restent dans l'application. Aucun callback ou mécanisme d'événements n'est intégré.
