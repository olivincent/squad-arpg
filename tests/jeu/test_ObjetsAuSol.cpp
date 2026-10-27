// ./tests/jeu/test_ObjetAuSol.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "jeu/ObjetsAuSol.hpp"
#include "monde/TileMap.hpp"
#include "objets/Objet.hpp"

// `ObjetsAuSol::positionPour` (étape 97.3, rangée dans la classe à l'étape 98.1) : où tombe un objet. Cases de 40 px, le centre de la case (x, y) est (40x + 20, 40y + 20).
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

const Objet s_gants{IdObjet::GantsDeTissu};

// Pose `nombre` objets l'un après l'autre à partir de cette origine, chacun voyant les précédents.
ObjetsAuSol poser(const TileMap& carte, sf::Vector2f origine, int nombre) {
    ObjetsAuSol sol;
    for (int i = 0; i < nombre; ++i) {
        sol.poser(s_gants, origine, carte);
    }
    return sol;
}
}  // namespace

TEST_CASE("positionPour : le premier objet tombe au centre de la case de l'origine") {
    const TileMap carte(s_plan, s_tailleCase);

    REQUIRE(ObjetsAuSol{}.positionPour(carte, {135.0f, 75.0f}) == centre(3, 1));
}

TEST_CASE("positionPour : sur une case prise, l'objet va sur une voisine, les quatre cotes avant les diagonales") {
    const TileMap carte(s_plan, s_tailleCase);
    const sf::Vector2f origine = centre(4, 2);

    const ObjetsAuSol sol = poser(carte, origine, 5);
    const std::vector<ObjetAuSol>& auSol = sol.tous();

    REQUIRE(auSol[0].position == origine);
    const std::set<std::pair<float, float>> premieresVoisines = {{centre(4, 1).x, centre(4, 1).y}, {centre(3, 2).x, centre(3, 2).y},
                                                                  {centre(5, 2).x, centre(5, 2).y}, {centre(4, 3).x, centre(4, 3).y}};
    std::set<std::pair<float, float>> posees;
    for (std::size_t i = 1; i < auSol.size(); ++i) {
        posees.insert({auSol[i].position.x, auSol[i].position.y});
    }
    REQUIRE(posees == premieresVoisines);
}

TEST_CASE("positionPour : jamais dans un mur, jamais sur une case prise tant qu'il en reste") {
    const TileMap carte(s_plan, s_tailleCase);
    // L'origine contre le mur du haut : les voisines du dessus sont des murs. Le sol compte 7 x 3 cases, toutes à moins de
    // trois cases de l'origine.
    const ObjetsAuSol sol = poser(carte, centre(4, 1), 21);
    const std::vector<ObjetAuSol>& auSol = sol.tous();

    std::set<std::pair<int, int>> cases;
    for (const ObjetAuSol& objet : auSol) {
        const sf::Vector2i coordCase = carte.caseContenant(objet.position);
        REQUIRE(carte.getTile(coordCase.x, coordCase.y) == Tile::Floor);
        REQUIRE(objet.position == carte.centreDeCase(coordCase));
        cases.insert({coordCase.x, coordCase.y});
    }
    REQUIRE(cases.size() == 21);  // une case par objet : tout le sol est pris
}

TEST_CASE("positionPour : une porte fermee n'est pas une case libre") {
    TileMap carte(s_plan, s_tailleCase);
    carte.placerPorte({2, 2});
    carte.placerPorte({3, 1});

    const ObjetsAuSol sol = poser(carte, centre(2, 1), 8);
    const std::vector<ObjetAuSol>& auSol = sol.tous();

    for (const ObjetAuSol& objet : auSol) {
        const sf::Vector2i coordCase = carte.caseContenant(objet.position);
        REQUIRE(carte.getTile(coordCase.x, coordCase.y) != Tile::PorteFermee);
    }
}

TEST_CASE("positionPour : quand tout autour est pris ou bloque, l'objet se pose sur la case de l'origine") {
    // Une case de sol seule au milieu des murs : rien d'autre à essayer.
    const TileMap carte({"###", "#.#", "###"}, s_tailleCase);
    ObjetsAuSol sol;
    sol.poser(s_gants, centre(1, 1), carte);

    REQUIRE(sol.positionPour(carte, centre(1, 1)) == centre(1, 1));
}

TEST_CASE("positionPour : meme resultat pour les memes arguments") {
    const TileMap carte(s_plan, s_tailleCase);

    REQUIRE(poser(carte, centre(4, 2), 6).tous() == poser(carte, centre(4, 2), 6).tous());
}

TEST_CASE("ObjetsAuSol : poser donne des identifiants qui ne sont jamais reutilises, meme apres un retrait") {
    const TileMap carte(s_plan, s_tailleCase);
    ObjetsAuSol sol = poser(carte, centre(4, 2), 3);
    const std::vector<ObjetAuSol> avant = sol.tous();

    REQUIRE(sol.retirer(avant.back().id));
    sol.poser(s_gants, centre(4, 2), carte);

    REQUIRE(sol.tous().size() == 3);
    std::set<int> identifiants;
    for (const ObjetAuSol& objet : sol.tous()) {
        identifiants.insert(objet.id);
    }
    REQUIRE(identifiants.size() == 3);
    REQUIRE(sol.tous().back().id > avant.back().id);
}

TEST_CASE("ObjetsAuSol : retirer et trouver passent par l'identifiant, pas par l'indice") {
    const TileMap carte(s_plan, s_tailleCase);
    ObjetsAuSol sol = poser(carte, centre(4, 2), 3);
    const ObjetAuSol premier = sol.tous()[0];
    const ObjetAuSol dernier = sol.tous()[2];

    REQUIRE(sol.retirer(premier.id));

    REQUIRE(sol.tous().size() == 2);
    REQUIRE(sol.trouver(premier.id) == nullptr);
    REQUIRE(sol.trouver(dernier.id) != nullptr);
    REQUIRE(*sol.trouver(dernier.id) == dernier);
}

TEST_CASE("ObjetsAuSol : retirer un identifiant absent ne change rien") {
    const TileMap carte(s_plan, s_tailleCase);
    ObjetsAuSol sol = poser(carte, centre(4, 2), 2);
    const std::vector<ObjetAuSol> avant = sol.tous();

    REQUIRE_FALSE(sol.retirer(999));

    REQUIRE(sol.tous() == avant);
}

TEST_CASE("positionPour : un objet evite la case de l'escalier, meme quand l'origine est l'escalier") {
    const TileMap carte({"#########", "#.......#", "#...>...#", "#.......#", "#########"}, s_tailleCase);
    const sf::Vector2f escalier = centre(4, 2);

    const sf::Vector2f position = ObjetsAuSol{}.positionPour(carte, escalier);

    REQUIRE(position != escalier);
    REQUIRE(carte.getTile(carte.caseContenant(position).x, carte.caseContenant(position).y) == Tile::Floor);
}

TEST_CASE("positionPour : les objets d'un tas autour de l'escalier ne le prennent jamais tant qu'il reste de la place") {
    const TileMap carte({"#########", "#.......#", "#...>...#", "#.......#", "#########"}, s_tailleCase);
    ObjetsAuSol sol;

    for (int i = 0; i < 20; ++i) {  // le sol compte 7 x 3 = 21 cases, dont l'escalier
        sol.poser(s_gants, centre(4, 2), carte);
    }

    for (const ObjetAuSol& objet : sol.tous()) {
        REQUIRE(carte.caseContenant(objet.position) != sf::Vector2i(4, 2));
    }
}

TEST_CASE("positionPour : quand tout autour est pris, l'objet se pose sur l'escalier faute de mieux") {
    // Une seule case de sol en dehors de l'escalier.
    const TileMap carte({"####", "#>.#", "####"}, s_tailleCase);
    ObjetsAuSol sol;
    sol.poser(s_gants, centre(1, 1), carte);  // prend la case (2, 1) : l'escalier est évité
    REQUIRE(carte.caseContenant(sol.tous().front().position) == sf::Vector2i(2, 1));

    REQUIRE(sol.positionPour(carte, centre(1, 1)) == centre(1, 1));
}
