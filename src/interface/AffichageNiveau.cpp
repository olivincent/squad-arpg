// ./src/interface/AffichageNiveau.cpp
#include "interface/AffichageNiveau.hpp"

std::string ligneDeNiveau(int niveau) {
    return "Level " + std::to_string(niveau);
}

std::string ligneDePointsADepenser() {
    return "Points to spend";
}
