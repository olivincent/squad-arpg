// ./src/interface/AffichageNiveau.hpp
#pragma once
#include <string>

// Le texte du niveau de l'escouade, en anglais (`docs/glossaire.md`) : « Level 3 ». En haut de l'écran de jeu et dans l'inventaire.
[[nodiscard]] std::string ligneDeNiveau(int niveau);

// Le signal qu'un personnage au moins a des points libres à dépenser (`Escouade::aDesPointsLibres`), à côté du niveau : « Points to spend ».
[[nodiscard]] std::string ligneDePointsADepenser();
