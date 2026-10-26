// ./src/interface/AffichageProfondeur.cpp
#include "interface/AffichageProfondeur.hpp"
#include "jeu/Expedition.hpp"

std::string ligneDeProfondeur(int profondeur) {
    return "Depth " + std::to_string(profondeur) + " / " + std::to_string(s_profondeurMaximale);
}
