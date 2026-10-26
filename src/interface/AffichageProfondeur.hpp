// ./src/interface/AffichageProfondeur.hpp
#pragma once
#include <string>

// Le texte de la profondeur en haut de l'écran de jeu, en anglais (`docs/glossaire.md`) : « Depth 2 / 5 ».
[[nodiscard]] std::string ligneDeProfondeur(int profondeur);
