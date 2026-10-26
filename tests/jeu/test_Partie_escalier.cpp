// ./tests/jeu/test_Partie_escalier.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "jeu/Partie.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Niveau.hpp"
#include "monde/TileMap.hpp"
#include "objets/Objet.hpp"

// L'escalier dans la partie (étape 98.4) : le clic, la marche, l'arrivée. Niveau dégagé de 15 × 15 cases de 40 px ; l'ancre
// part en (210, 210).
namespace {
const sf::Vector2i s_caseProche = {8, 8};   // centre (340, 340) : vu dès le départ
const sf::Vector2i s_caseLointaine = {14, 1};  // centre (580, 60) : hors de la vision des personnages au départ
const sf::Vector2f s_coinLointain = {570.0f, 570.0f};  // un monstre endormi, pour que la partie ne soit pas gagnée

sf::Vector2f centreDe(sf::Vector2i coordCase) {
    return {(static_cast<float>(coordCase.x) + 0.5f) * 40.0f, (static_cast<float>(coordCase.y) + 0.5f) * 40.0f};
}

// Le niveau dégagé, avec l'escalier sur cette case.
Niveau niveauAvecEscalier(const std::vector<MonstreDuNiveau>& monstres, sf::Vector2i coordCase) {
    Niveau niveau = niveauDegageEnGroupes(monstres);
    niveau.plan[static_cast<std::size_t>(coordCase.y)][static_cast<std::size_t>(coordCase.x)] = s_caractereDEscalier;
    return niveau;
}

Partie partieAvecEscalier(sf::Vector2i coordCase = s_caseProche, const std::vector<MonstreDuNiveau>& monstres = {{s_coinLointain, 0}}) {
    return Partie(niveauAvecEscalier(monstres, coordCase), expeditionDeTest(), s_graineDeTest, catalogueDeTest(monstreSansHasard()));
}

float distance(sf::Vector2f a, sf::Vector2f b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

// Avance jusqu'à ce que l'escalier soit pris, au plus `secondes`.
void avancerJusquALaDescente(Partie& partie, float secondes) {
    for (int i = 0; i < nombreDeTics(secondes) && !partie.aPrisLEscalier(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
}
}  // namespace

TEST_CASE("escalier : un clic sur l'escalier visible est un ordre de descente") {
    Partie partie = partieAvecEscalier();

    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);
    REQUIRE(partie.cliquer(centreDe(s_caseProche) + sf::Vector2f(15.0f, -15.0f)) == ResultatClic::Escalier);  // n'importe où dans la case
    REQUIRE(partie.cliquer(centreDe(s_caseProche) + sf::Vector2f(25.0f, 0.0f)) == ResultatClic::Sol);         // la case voisine
}

TEST_CASE("escalier : une partie sans escalier n'en a pas a cliquer") {
    Partie partie = Partie(niveauDegageEnGroupes({{s_coinLointain, 0}}), expeditionDeTest(), s_graineDeTest, catalogueDeTest(monstreSansHasard()));

    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Sol);
}

TEST_CASE("escalier : l'escalier d'une case jamais vue ne se clique pas, une fois explore il se clique") {
    Partie partie = partieAvecEscalier(s_caseLointaine);
    REQUIRE(partie.cliquer(centreDe(s_caseLointaine)) == ResultatClic::Sol);  // inconnue : cachée

    // L'escouade s'approche : la case est vue.
    partie.ordonnerDeplacement(centreDe({12, 4}));
    avancer(partie, nombreDeTics(8.0f));

    REQUIRE(partie.cliquer(centreDe(s_caseLointaine)) == ResultatClic::Escalier);
}

TEST_CASE("escalier : l'escouade marche jusqu'a l'escalier, et le prend quand l'ancre arrive") {
    Partie partie = partieAvecEscalier();
    REQUIRE_FALSE(partie.aPrisLEscalier());
    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);

    partie.mettreAJour(s_dureeTic);
    REQUIRE_FALSE(partie.aPrisLEscalier());  // l'ancre est à 180 px

    avancerJusquALaDescente(partie, 10.0f);

    REQUIRE(partie.aPrisLEscalier());
    REQUIRE(distance(partie.getEscouade().getCentre(), centreDe(s_caseProche)) < s_distanceDeDescente);
}

TEST_CASE("escalier : sans ordre, passer sur l'escalier ne le prend pas") {
    Partie partie = partieAvecEscalier();

    partie.ordonnerDeplacement(centreDe(s_caseProche));
    for (int i = 0; i < nombreDeTics(8.0f); ++i) {
        partie.ordonnerDeplacement(centreDe(s_caseProche));
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(distance(partie.getEscouade().getCentre(), centreDe(s_caseProche)) < s_distanceDeDescente);
    REQUIRE_FALSE(partie.aPrisLEscalier());
}

TEST_CASE("escalier : un nouvel ordre au sol remplace la visee de l'escalier") {
    Partie partie = partieAvecEscalier();
    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);
    avancer(partie, nombreDeTics(0.3f));  // en route

    REQUIRE(partie.cliquer({50.0f, 500.0f}) == ResultatClic::Sol);
    // Le clic seul ne détourne pas l'escouade (c'est l'ordre de déplacement du contrôleur qui le fait) : elle finit son chemin
    // jusqu'à l'escalier, sans le prendre, puisqu'on ne le lui demande plus.
    avancer(partie, nombreDeTics(8.0f));

    REQUIRE_FALSE(partie.aPrisLEscalier());
}

TEST_CASE("escalier : un ordre de deplacement (bouton tenu) remplace la visee de l'escalier") {
    Partie partie = partieAvecEscalier();
    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);

    for (int i = 0; i < nombreDeTics(8.0f); ++i) {
        partie.ordonnerDeplacement({50.0f, 500.0f});
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE_FALSE(partie.aPrisLEscalier());
    REQUIRE(distance(partie.getEscouade().getCentre(), centreDe(s_caseProche)) > s_distanceDeDescente);
}

TEST_CASE("escalier : un clic sur un monstre remplace la visee de l'escalier") {
    Partie partie = partieAvecEscalier(s_caseProche, {{s_coinLointain, 0}, {{100.0f, 300.0f}, 1}});
    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);

    REQUIRE(partie.cliquer({100.0f, 300.0f}) == ResultatClic::Monstre);
    avancer(partie, nombreDeTics(3.0f));

    REQUIRE_FALSE(partie.aPrisLEscalier());
}

TEST_CASE("escalier : avec un monstre sous le curseur, le clic vise le monstre et non l'escalier") {
    // Un monstre lourd sur l'escalier : son carré contient le centre de la case.
    Partie partie = partieAvecEscalier(s_caseProche, {{s_coinLointain, 0}, {centreDe(s_caseProche), 1, CategorieDeMonstre::InfanterieLourde}});
    REQUIRE(partie.getMonstres().back().contient(centreDe(s_caseProche)));

    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Monstre);

    avancer(partie, nombreDeTics(2.0f));
    REQUIRE_FALSE(partie.aPrisLEscalier());
}

TEST_CASE("escalier : avec un objet au sol sous le curseur, le clic vise l'objet et non l'escalier") {
    Partie partie = partieAvecEscalier();
    // 48 objets remplissent les cases libres autour de l'escalier (rayon de recherche de 3 cases), le 49e n'a plus d'autre
    // place que l'escalier lui-même.
    for (int i = 0; i < 49; ++i) {
        partie.getSol().poser(Objet{IdObjet::GantsDeCuir}, centreDe(s_caseProche), partie.getCarte());
    }
    REQUIRE(partie.getSol().tous().back().position == centreDe(s_caseProche));

    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::ObjetAuSol);

    // L'ancre arrive sur l'escalier pour ramasser l'objet : ce n'est pas une descente.
    for (int i = 0; i < nombreDeTics(10.0f) && partie.getSol().tous().size() == 49; ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.getSol().tous().size() == 48);
    REQUIRE_FALSE(partie.aPrisLEscalier());
}

TEST_CASE("escalier : on peut descendre en plein combat, sans avoir rien tue") {
    Partie partie = partieAvecEscalier(s_caseProche, {{s_coinLointain, 0}, {{300.0f, 215.0f}, 1}});
    REQUIRE(partie.cliquer(partie.getMonstres().back().getCentre()) == ResultatClic::Monstre);
    avancer(partie, nombreDeTics(0.5f));

    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);
    avancerJusquALaDescente(partie, 10.0f);

    REQUIRE(partie.aPrisLEscalier());
    REQUIRE(partie.monstresRestants() >= 1);  // le monstre du coin n'a pas été tué
}

TEST_CASE("escalier : un ordre de descente leve le verrou, comme un clic au sol") {
    Partie partie = partieAvecEscalier(s_caseProche, {{s_coinLointain, 0}, {{100.0f, 300.0f}, 1}});
    REQUIRE(partie.cliquer({100.0f, 300.0f}) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee().has_value());

    REQUIRE(partie.cliquer(centreDe(s_caseProche)) == ResultatClic::Escalier);

    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}
