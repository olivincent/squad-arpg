// ./src/ObjetInteractif.cpp
#include "ObjetInteractif.hpp"
#include <algorithm>
#include <cassert>
#include <utility>
#include "TileMap.hpp"

namespace {
bool sontSurUneMemeLigne(const std::vector<sf::Vector2i>& cases) {
    return std::ranges::all_of(cases, [&](sf::Vector2i c) { return c.y == cases.front().y; });
}

bool sontSurUneMemeColonne(const std::vector<sf::Vector2i>& cases) {
    return std::ranges::all_of(cases, [&](sf::Vector2i c) { return c.x == cases.front().x; });
}
}  // namespace

ObjetInteractif::ObjetInteractif(int identifiant, Porte porte, float tailleCase) : id(identifiant), nature(std::move(porte)) {
    const std::vector<sf::Vector2i>& cases = std::get<Porte>(nature).cases;
    assert(!cases.empty());
    assert(sontSurUneMemeLigne(cases) || sontSurUneMemeColonne(cases));

    sf::Vector2i min = cases.front();
    sf::Vector2i max = cases.front();
    for (sf::Vector2i c : cases) {
        min = {std::min(min.x, c.x), std::min(min.y, c.y)};
        max = {std::max(max.x, c.x), std::max(max.y, c.y)};
    }
    zone = sf::FloatRect({static_cast<float>(min.x) * tailleCase, static_cast<float>(min.y) * tailleCase},
                         {static_cast<float>(max.x - min.x + 1) * tailleCase, static_cast<float>(max.y - min.y + 1) * tailleCase});
}

std::optional<sf::Vector2f> pointDApproche(const Porte& porte, const TileMap& carte, sf::Vector2f depuis) {
    const bool horizontale = sontSurUneMemeLigne(porte.cases);
    const sf::Vector2i decalage = horizontale ? sf::Vector2i(0, 1) : sf::Vector2i(1, 0);

    std::optional<sf::Vector2f> meilleur;
    float meilleureDistance = 0.0f;
    for (const int cote : {-1, 1}) {
        sf::Vector2f somme(0.0f, 0.0f);
        bool libre = true;
        for (sf::Vector2i c : porte.cases) {
            const sf::Vector2i voisine = c + decalage * cote;
            if (estBloquante(carte.getTile(voisine.x, voisine.y))) {
                libre = false;
                break;
            }
            somme += carte.centreDeCase(voisine);
        }
        if (!libre) {
            continue;
        }
        const sf::Vector2f point = somme / static_cast<float>(porte.cases.size());
        const float distance = (point - depuis).lengthSquared();
        if (!meilleur.has_value() || distance < meilleureDistance) {
            meilleur = point;
            meilleureDistance = distance;
        }
    }
    return meilleur;
}
