// ./src/escouade/TailleDeLEscouade.hpp
#pragma once
#include <cstddef>

// Le nombre de personnages d'une escouade choisie par le joueur : la seule constante de cette idée. La sélection, l'écran
// d'inventaire, la barre de raccourcis et la composition s'y rapportent. `Escouade` et `Expedition` n'en dépendent pas
// (elles acceptent N personnages). En-tête à part : l'interface en a besoin sans dépendre de `SelectionDEscouade`.
inline constexpr std::size_t s_tailleDeLEscouade = 3;
