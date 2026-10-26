// ./src/jeu/ObjetsAuSol.cpp
#include "jeu/ObjetsAuSol.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <utility>
#include "escouade/Escouade.hpp"
#include "monde/TileMap.hpp"
#include "monde/Vision.hpp"

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

void ObjetsAuSol::poser(const Objet& objet, sf::Vector2f origine, const TileMap& carte) {
    m_objets.push_back({m_prochainId++, objet, positionPour(carte, origine)});
}

bool ObjetsAuSol::retirer(int id) {
    return std::erase_if(m_objets, [&](const ObjetAuSol& objet) { return objet.id == id; }) > 0;
}

const ObjetAuSol* ObjetsAuSol::trouver(int id) const {
    const auto it = std::ranges::find(m_objets, id, &ObjetAuSol::id);
    return it == m_objets.end() ? nullptr : &*it;
}

const ObjetAuSol* ObjetsAuSol::sous(sf::Vector2f point, const TileMap& carte, const Vision& vision) const {
    const ObjetAuSol* trouve = nullptr;
    float meilleureDistance = s_rayonDeSurvolDObjet;
    for (const ObjetAuSol& objet : m_objets) {
        const sf::Vector2f ecart = objet.position - point;
        const float distance = std::hypot(ecart.x, ecart.y);
        if (distance <= meilleureDistance && vision.etat(carte.caseContenant(objet.position)) != EtatCase::Inconnue) {
            trouve = &objet;
            meilleureDistance = distance;
        }
    }
    return trouve;
}

sf::Vector2f ObjetsAuSol::positionPour(const TileMap& carte, sf::Vector2f origine) const {
    const sf::Vector2i depart = carte.caseContenant(origine);
    const auto estOccupee = [&](sf::Vector2i coordCase) {
        return std::ranges::any_of(m_objets, [&](const ObjetAuSol& objet) {
            return carte.caseContenant(objet.position) == coordCase;
        });
    };
    for (const sf::Vector2i decalage : decalagesParProximite()) {
        const sf::Vector2i candidate = depart + decalage;
        const Tile terrain = carte.getTile(candidate.x, candidate.y);
        if (!estBloquante(terrain) && terrain != Tile::Escalier && !estOccupee(candidate)) {
            return carte.centreDeCase(candidate);
        }
    }
    return carte.centreDeCase(depart);
}

const std::vector<ObjetAuSol>& ObjetsAuSol::tous() const {
    return m_objets;
}

bool jeterUnObjet(Escouade& escouade, ObjetsAuSol& sol, const TileMap& carte, const PositionDObjet& position) {
    const std::optional<Objet> objet = escouade.retirerObjet(position);
    if (!objet.has_value()) {
        return false;
    }
    sol.poser(*objet, escouade.getCentre(), carte);
    return true;
}
