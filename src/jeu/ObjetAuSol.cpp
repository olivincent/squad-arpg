// ./src/jeu/ObjetAuSol.cpp
#include "jeu/ObjetAuSol.hpp"
#include <algorithm>
#include <cstdlib>
#include <utility>
#include "monde/TileMap.hpp"

namespace {
constexpr int s_rayonDeRecherche = 3;  // en cases : jusqu'où chercher une case libre autour de l'origine

// Les décalages en cases du carré [-r, r]², du plus proche au plus éloigné : d'abord par couronne (distance de
// Tchebychev), puis, dans une couronne, par distance à vol d'oiseau (les quatre voisines avant les diagonales).
const std::vector<sf::Vector2i>& decalagesParProximite() {
    static const std::vector<sf::Vector2i> decalages = [] {
        std::vector<sf::Vector2i> liste;
        for (int dy = -s_rayonDeRecherche; dy <= s_rayonDeRecherche; ++dy) {
            for (int dx = -s_rayonDeRecherche; dx <= s_rayonDeRecherche; ++dx) {
                liste.push_back({dx, dy});
            }
        }
        std::ranges::stable_sort(liste, {}, [](sf::Vector2i d) {
            return std::pair{std::max(std::abs(d.x), std::abs(d.y)), d.x * d.x + d.y * d.y};
        });
        return liste;
    }();
    return decalages;
}
}  // namespace

sf::Vector2f positionPourUnObjet(const TileMap& carte, sf::Vector2f origine, const std::vector<ObjetAuSol>& auSol) {
    const sf::Vector2i depart = carte.caseContenant(origine);
    const auto estOccupee = [&](sf::Vector2i coordCase) {
        return std::ranges::any_of(auSol, [&](const ObjetAuSol& objet) {
            return carte.caseContenant(objet.position) == coordCase;
        });
    };
    for (const sf::Vector2i decalage : decalagesParProximite()) {
        const sf::Vector2i candidate = depart + decalage;
        if (!estBloquante(carte.getTile(candidate.x, candidate.y)) && !estOccupee(candidate)) {
            return carte.centreDeCase(candidate);
        }
    }
    return carte.centreDeCase(depart);
}
