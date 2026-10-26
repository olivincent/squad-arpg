// ./tests/jeu/test_Partie_clics.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Partie.hpp"

TEST_CASE("cliquer sur un monstre le verrouille") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}, {300.0f, 400.0f}}), s_graineDeTest);

    REQUIRE(partie.cliquer({300.0f, 400.0f}) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee() == 2);
}

TEST_CASE("cliquer au sol renvoie Sol et leve le verrou") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({400.0f, 400.0f}) == ResultatClic::Monstre);

    REQUIRE(partie.cliquer({100.0f, 500.0f}) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

namespace {

// Salle de 30 × 11 cases (40 px) ; l'escouade part de (200, 200), son centre est en (210, 210).
Niveau niveauLong(const std::vector<MonstreDuNiveau>& monstres) {
    return {std::vector<std::string>(11, std::string(30, '.')), 40.0f, {200.0f, 200.0f}, monstres};
}

// Un monstre à 610 px de l'escouade : hors de sa vision (320 px) et de sa détection (200 px).
const sf::Vector2f s_monstreAuLoin{820.0f, 220.0f};

}  // namespace

TEST_CASE("vision : un monstre de la salle de depart est cliquable des la construction") {
    Partie partie = unePartie(niveauLong({{{300.0f, 210.0f}, 0}}), s_graineDeTest);  // aucun mettreAJour

    REQUIRE(partie.cliquer({300.0f, 210.0f}) == ResultatClic::Monstre);
}

TEST_CASE("vision : un monstre hors de vue ne peut pas etre clique, le clic est un clic au sol") {
    Partie partie = unePartie(niveauLong({{s_monstreAuLoin, 0}}), s_graineDeTest);

    REQUIRE(partie.cliquer(s_monstreAuLoin) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("vision : un monstre cache par un mur n'est pas cliquable") {
    Niveau niveau = niveauLong({{{500.0f, 200.0f}, 0}});  // à 290 px : dans le rayon, mais derrière le mur
    for (std::string& ligne : niveau.plan) {
        ligne[8] = '#';
    }
    Partie partie = unePartie(niveau, s_graineDeTest);

    REQUIRE(partie.cliquer({500.0f, 200.0f}) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("vision : le meme monstre devient cliquable quand l'escouade s'en approche") {
    Partie partie = unePartie(niveauLong({{s_monstreAuLoin, 0}}), s_graineDeTest);
    REQUIRE(partie.cliquer(s_monstreAuLoin) == ResultatClic::Sol);

    partie.ordonnerDeplacement({540.0f, 210.0f});  // à 280 px du monstre : vu, mais pas détecté
    avancer(partie, 150);

    REQUIRE(partie.cliquer(s_monstreAuLoin) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee() == 1);
}

namespace {

// Salle de 30 × 11 cases coupée par un mur en colonne 25, percé d'une porte de 2 cases en (25, 4)
// et (25, 5), soit la zone [1000, 1040] × [160, 240]. Un monstre au sud-ouest, loin de tout.
Niveau niveauPorteLointaine() {
    Niveau niveau = niveauLong({{{60.0f, 400.0f}, 0}});
    for (std::string& ligne : niveau.plan) {
        ligne[25] = '#';
    }
    niveau.plan[4][25] = '.';
    niveau.plan[5][25] = '.';
    niveau.portes = {PorteDuNiveau{{{25, 4}, {25, 5}}}};
    return niveau;
}

constexpr sf::Vector2f s_surLaPorteLointaine{1020.0f, 200.0f};

}  // namespace

TEST_CASE("vision : une porte dans l'inconnu n'est pas cliquable, le clic est un clic au sol") {
    Partie partie = unePartie(niveauPorteLointaine(), s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorteLointaine) == ResultatClic::Sol);
}

TEST_CASE("vision : une porte vue devient cliquable") {
    Partie partie = unePartie(niveauPorteLointaine(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorteLointaine) == ResultatClic::Sol);

    partie.ordonnerDeplacement({800.0f, 200.0f});  // à 5 cases de la porte
    avancer(partie, 250);

    REQUIRE(partie.cliquer(s_surLaPorteLointaine) == ResultatClic::Porte);
}

TEST_CASE("estVisible : la salle de depart est visible, un point lointain non") {
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);

    REQUIRE(partie.estVisible(partie.getEscouade().getCentre()));
    REQUIRE(partie.estVisible({235.0f, 170.0f}));
    REQUIRE_FALSE(partie.estVisible({580.0f, 580.0f}));  // à plus de 320 px de tout personnage
}
