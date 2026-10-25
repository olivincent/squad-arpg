// ./tests/jeu/test_ObjetAuSol.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "jeu/ObjetAuSol.hpp"
#include "monde/TileMap.hpp"
#include "objets/Objet.hpp"

// `positionPourUnObjet` (étape 97.3) : où tombe un objet. Cases de 40 px, le centre de la case (x, y) est (40x + 20, 40y + 20).
namespace {
constexpr float s_tailleCase = 40.0f;

const std::vector<std::string> s_plan = {
    "#########",
    "#.......#",
    "#.......#",
    "#.......#",
    "#########",
};

sf::Vector2f centre(int x, int y) {
    return {s_tailleCase * static_cast<float>(x) + s_tailleCase / 2.0f, s_tailleCase * static_cast<float>(y) + s_tailleCase / 2.0f};
}

ObjetAuSol objetEn(sf::Vector2f position, int id = 1) {
    return {id, Objet{IdObjet::GantsDeCuir}, position};
}

// Pose `nombre` objets l'un après l'autre à partir de cette origine, chacun voyant les précédents.
std::vector<ObjetAuSol> poser(const TileMap& carte, sf::Vector2f origine, int nombre) {
    std::vector<ObjetAuSol> auSol;
    for (int i = 0; i < nombre; ++i) {
        const sf::Vector2f position = positionPourUnObjet(carte, origine, auSol);
        auSol.push_back(objetEn(position, i + 1));
    }
    return auSol;
}
}  // namespace

TEST_CASE("positionPourUnObjet : le premier objet tombe au centre de la case de l'origine") {
    const TileMap carte(s_plan, s_tailleCase);

    REQUIRE(positionPourUnObjet(carte, {135.0f, 75.0f}, {}) == centre(3, 1));
}

TEST_CASE("positionPourUnObjet : sur une case prise, l'objet va sur une voisine, les quatre cotes avant les diagonales") {
    const TileMap carte(s_plan, s_tailleCase);
    const sf::Vector2f origine = centre(4, 2);

    const std::vector<ObjetAuSol> auSol = poser(carte, origine, 5);

    REQUIRE(auSol[0].position == origine);
    const std::set<std::pair<float, float>> premieresVoisines = {{centre(4, 1).x, centre(4, 1).y}, {centre(3, 2).x, centre(3, 2).y},
                                                                  {centre(5, 2).x, centre(5, 2).y}, {centre(4, 3).x, centre(4, 3).y}};
    std::set<std::pair<float, float>> posees;
    for (std::size_t i = 1; i < auSol.size(); ++i) {
        posees.insert({auSol[i].position.x, auSol[i].position.y});
    }
    REQUIRE(posees == premieresVoisines);
}

TEST_CASE("positionPourUnObjet : jamais dans un mur, jamais sur une case prise tant qu'il en reste") {
    const TileMap carte(s_plan, s_tailleCase);
    // L'origine contre le mur du haut : les voisines du dessus sont des murs. Le sol compte 7 x 3 cases, toutes à moins de
    // trois cases de l'origine.
    const std::vector<ObjetAuSol> auSol = poser(carte, centre(4, 1), 21);

    std::set<std::pair<int, int>> cases;
    for (const ObjetAuSol& objet : auSol) {
        const sf::Vector2i coordCase = carte.caseContenant(objet.position);
        REQUIRE(carte.getTile(coordCase.x, coordCase.y) == Tile::Floor);
        REQUIRE(objet.position == carte.centreDeCase(coordCase));
        cases.insert({coordCase.x, coordCase.y});
    }
    REQUIRE(cases.size() == 21);  // une case par objet : tout le sol est pris
}

TEST_CASE("positionPourUnObjet : une porte fermee n'est pas une case libre") {
    TileMap carte(s_plan, s_tailleCase);
    carte.placerPorte({2, 2});
    carte.placerPorte({3, 1});

    const std::vector<ObjetAuSol> auSol = poser(carte, centre(2, 1), 8);

    for (const ObjetAuSol& objet : auSol) {
        const sf::Vector2i coordCase = carte.caseContenant(objet.position);
        REQUIRE(carte.getTile(coordCase.x, coordCase.y) != Tile::PorteFermee);
    }
}

TEST_CASE("positionPourUnObjet : quand tout autour est pris ou bloque, l'objet se pose sur la case de l'origine") {
    // Une case de sol seule au milieu des murs : rien d'autre à essayer.
    const TileMap carte({"###", "#.#", "###"}, s_tailleCase);
    const ObjetAuSol dejaLa = objetEn(centre(1, 1));

    REQUIRE(positionPourUnObjet(carte, centre(1, 1), {dejaLa}) == centre(1, 1));
}

TEST_CASE("positionPourUnObjet : meme resultat pour les memes arguments") {
    const TileMap carte(s_plan, s_tailleCase);

    REQUIRE(poser(carte, centre(4, 2), 6) == poser(carte, centre(4, 2), 6));
}
