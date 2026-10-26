// ./tests/jeu/test_Partie_portes.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Partie.hpp"

namespace {

// Deux pièces de 15 × 11 cases (40 px) séparées par un mur en colonne 7
// (x: [280, 320)) percé d'une porte de 2 cases en (7, 4) et (7, 5), soit la zone
// [280, 320] × [160, 240]. L'escouade part de (200, 200), côté ouest.
Niveau niveauAvecPorte(const std::vector<MonstreDuNiveau>& monstres = {{{60.0f, 60.0f}, 0}}) {
    std::vector<std::string> plan(11, std::string(15, '.'));
    for (int y = 0; y < 11; ++y) {
        plan[static_cast<std::size_t>(y)][7] = '#';
    }
    plan[4][7] = '.';
    plan[5][7] = '.';
    Niveau niveau{plan, 40.0f, {200.0f, 200.0f}, monstres};
    niveau.portes = {PorteDuNiveau{{{7, 4}, {7, 5}}}};
    return niveau;
}

constexpr sf::Vector2f s_surLaPorte{300.0f, 200.0f};
constexpr sf::Vector2f s_coteEst{420.0f, 200.0f};

}  // namespace

TEST_CASE("porte : fermee a la construction, l'escouade ne la traverse pas") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);

    partie.ordonnerDeplacement(s_coteEst);
    avancer(partie, 300);

    REQUIRE(partie.getEscouade().getCentre().x < 280.0f);
}

TEST_CASE("porte : un clic dessus renvoie Objet et leve le verrou") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer({60.0f, 60.0f}) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee().has_value());

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("porte : l'escouade s'avance, la porte s'ouvre et le chemin passe") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    avancer(partie, 120);

    // Ouverte : un clic dessus compte comme un clic au sol.
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);
    // L'escouade s'est arrêtée devant, sans franchir la porte.
    REQUIRE(partie.getEscouade().getCentre().x < 300.0f);

    partie.ordonnerDeplacement(s_coteEst);
    avancer(partie, 300);
    REQUIRE(partie.getEscouade().getCentre().x > 400.0f);
}

TEST_CASE("porte : un ordre de deplacement annule l'interaction") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    partie.ordonnerDeplacement({100.0f, 200.0f});
    avancer(partie, 120);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);  // toujours fermée
}

TEST_CASE("porte : un clic au sol annule l'interaction") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    REQUIRE(partie.cliquer({100.0f, 200.0f}) == ResultatClic::Sol);
    avancer(partie, 120);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);
}

TEST_CASE("porte : un monstre ciblable sur la porte a la priorite") {
    Partie partie = unePartie(niveauAvecPorte({{s_surLaPorte, 0}}), s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Monstre);
}

TEST_CASE("porte : sans cote libre, le clic compte comme un clic au sol") {
    Niveau niveau = niveauAvecPorte();
    for (const int y : {4, 5}) {
        niveau.plan[static_cast<std::size_t>(y)][6] = '#';
        niveau.plan[static_cast<std::size_t>(y)][8] = '#';
    }
    Partie partie = unePartie(niveau, s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);
}

TEST_CASE("porte : une partie figee n'ouvre rien") {
    Partie partie = unePartie(niveauAvecPorte({}), s_graineDeTest);  // aucun monstre : gagnée
    REQUIRE(partie.estGagnee());
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    avancer(partie, 120);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);
}

namespace {

// La place avant de la formation (le guerrier) est-elle plus proche du point que les deux places arrière ?
bool formationFaitFace(const Partie& partie, sf::Vector2f point) {
    const std::vector<sf::Vector2f>& places = partie.getEscouade().getPlacesDeFormation();
    const float avant = (places[0] - point).length();
    const float arriereMin = std::min((places[1] - point).length(), (places[2] - point).length());
    return avant < arriereMin;
}

// Monstre loin de la porte et de l'escouade, pour que la partie ne soit pas gagnée.
const std::vector<MonstreDuNiveau> s_monstreLointain = {{{560.0f, 400.0f}, 0}};

// Escouade à l'est de la porte (centre (360, 200)), tournée vers l'est : ses tireurs sont
// collés à la porte, qu'elle a dans le dos.
Niveau niveauCollePorteDeDos() {
    Niveau niveau = niveauAvecPorte(s_monstreLointain);
    niveau.departEscouade = {350.0f, 190.0f};
    return niveau;
}

}  // namespace

TEST_CASE("porte : un personnage deja a portee ouvre la porte sur place, sans deplacement") {
    Partie partie = unePartie(niveauCollePorteDeDos(), s_graineDeTest);
    const sf::Vector2f centreAvant = partie.getEscouade().getCentre();

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);  // déjà ouverte, avant tout mettreAJour
    avancer(partie, 30);
    verifierPointProche(partie.getEscouade().getCentre(), centreAvant, 0.01f);
}

TEST_CASE("porte : de dos et collee a la porte, la formation fait face a la porte apres l'ouverture sur place") {
    Partie partie = unePartie(niveauCollePorteDeDos(), s_graineDeTest);
    partie.mettreAJour(s_dureeTic);
    REQUIRE_FALSE(formationFaitFace(partie, s_surLaPorte));

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);
    avancer(partie, 60);

    REQUIRE(formationFaitFace(partie, s_surLaPorte));
}

TEST_CASE("porte : apres l'ouverture par approche, la formation est tournee vers la porte") {
    Niveau niveau = niveauAvecPorte(s_monstreLointain);
    niveau.departEscouade = {250.0f, 80.0f};  // au nord-ouest : l'approche se fait vers le sud
    Partie partie = unePartie(niveau, s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);
    avancer(partie, 200);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);  // ouverte
    REQUIRE(formationFaitFace(partie, s_surLaPorte));
}

TEST_CASE("porte : loin de la porte, la porte reste fermee tant que l'escouade n'est pas arrivee") {
    Partie partie = unePartie(niveauAvecPorte(s_monstreLointain), s_graineDeTest);  // départ (210, 210), à 70 px

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Porte);  // pas encore ouverte
    avancer(partie, 120);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);
}
