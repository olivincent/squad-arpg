// ./src/interface/ClicInterface.cpp
#include "interface/ClicInterface.hpp"
#include "interface/BarreDeRaccourcis.hpp"

std::optional<ClicInterface> clicSurInterface(sf::Vector2f point, sf::Vector2f tailleVue) {
    const std::optional<std::size_t> emplacement = emplacementSous(point, tailleVue);
    if (!emplacement.has_value()) {
        return std::nullopt;
    }
    return ClicInterface{Barre::Raccourcis, *emplacement};
}
