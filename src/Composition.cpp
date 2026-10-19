// ./src/Composition.cpp
#include "Composition.hpp"
#include <cstddef>

namespace {
constexpr float s_decalageDeRang = 25.0f;     // distance de chaque rang à l'ancre, selon x
constexpr float s_ecartDansLeRang = 50.0f;    // entre deux personnages d'un même rang, selon y
}  // namespace

std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs) {
    std::size_t nombreAvant = 0;
    for (const Rang rang : rangs) {
        if (rang == Rang::Avant) {
            ++nombreAvant;
        }
    }
    const std::size_t nombreArriere = rangs.size() - nombreAvant;

    std::vector<sf::Vector2f> decalages;
    std::size_t rangAvant = 0;    // indice du prochain dans son rang
    std::size_t rangArriere = 0;
    for (const Rang rang : rangs) {
        const bool devant = rang == Rang::Avant;
        const std::size_t k = devant ? rangAvant++ : rangArriere++;
        const std::size_t n = devant ? nombreAvant : nombreArriere;
        const float y = (static_cast<float>(k) - static_cast<float>(n - 1) / 2.0f) * s_ecartDansLeRang;
        decalages.push_back({devant ? s_decalageDeRang : -s_decalageDeRang, y});
    }
    return decalages;
}
