// ./tests/test_Escouade_agresseurs.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "Agresseurs.hpp"
#include "Cercle.hpp"
#include "Escouade.hpp"
#include "MonstreVu.hpp"
#include "TileMap.hpp"


TEST_CASE("proieDuMonstre renvoie un centre plus proche du point que le centre de l'escouade") {
    const Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f pointLointain{1000.0f, 1000.0f};
    const std::optional<Cercle> cerclePersonnage = escouade.proieDuMonstre(1, pointLointain);
    REQUIRE(cerclePersonnage.has_value());

    const float distancePersonnage = (cerclePersonnage->centre - pointLointain).length();
    const float distanceEscouade = (escouade.getCentre() - pointLointain).length();

    REQUIRE(distancePersonnage < distanceEscouade);
}

TEST_CASE("proieDuMonstre ignore les degats insuffisants pour tuer") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> avant = escouade.proieDuMonstre(1, point);
    REQUIRE(avant.has_value());

    escouade.recevoirDegats(1, point, 50);
    const std::optional<Cercle> apres = escouade.proieDuMonstre(1, point);
    REQUIRE(apres.has_value());

    REQUIRE(apres->centre == avant->centre);
}

TEST_CASE("proieDuMonstre change de cible une fois le plus proche mort") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> avant = escouade.proieDuMonstre(1, point);
    REQUIRE(avant.has_value());

    escouade.recevoirDegats(1, point, 200);
    const std::optional<Cercle> apres = escouade.proieDuMonstre(1, point);
    REQUIRE(apres.has_value());

    REQUIRE_FALSE(apres->centre == avant->centre);
}

TEST_CASE("proieDuMonstre est vide une fois tous les personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, point, 200);
    }

    REQUIRE_FALSE(escouade.proieDuMonstre(1, point).has_value());

    escouade.recevoirDegats(1, point, 200);
    REQUIRE_FALSE(escouade.proieDuMonstre(1, point).has_value());
}

TEST_CASE("Un coup qui fait des degats enregistre son auteur comme agresseur") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);

    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(agresseurs.size() == 1);
    REQUIRE(agresseurs[0].monstre.id == 7);
    verifierPointProche(agresseurs[0].monstre.cercle.centre, {300.0f, 60.0f});
    REQUIRE(agresseurs[0].tempsRestant == 3.0f);
}

TEST_CASE("Un coup a zero degat n'enregistre aucun agresseur") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 0);

    REQUIRE(escouade.getAgresseurs().empty());
}

TEST_CASE("Un coup d'un agresseur deja connu ne le duplique pas et remet son temps au maximum") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.recevoirDegats(8, {310.0f, 60.0f}, 10);
    escouade.recevoirDegats(7, {320.0f, 60.0f}, 10);

    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(agresseurs.size() == 2);
    REQUIRE(agresseurs[0].monstre.id == 7);
    verifierPointProche(agresseurs[0].monstre.cercle.centre, {320.0f, 60.0f});
    REQUIRE(agresseurs[1].monstre.id == 8);
}

TEST_CASE("Sans personnage vivant, un coup n'enregistre aucun agresseur") {
    Escouade escouade({50.0f, 50.0f});
    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, {50.0f, 50.0f}, 200);
    }
    escouade.recevoirDegats(9, {50.0f, 50.0f}, 10);

    REQUIRE_FALSE(escouade.estVivante());
    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(std::none_of(agresseurs.begin(), agresseurs.end(), [](const Agresseurs::Agresseur& a) { return a.monstre.id == 9; }));
}

TEST_CASE("Un agresseur est oublie 3 s apres son dernier coup") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);

    escouade.mettreAJour(2.9f, carte, {});
    REQUIRE(escouade.getAgresseurs().size() == 1);
    escouade.mettreAJour(0.2f, carte, {});
    REQUIRE(escouade.getAgresseurs().empty());
}

TEST_CASE("Un nouveau coup repousse l'oubli de l'agresseur") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.mettreAJour(2.0f, carte, {});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.mettreAJour(2.0f, carte, {});

    REQUIRE(escouade.getAgresseurs().size() == 1);
}

TEST_CASE("attaquer retire l'agresseur absent de la liste et met a jour le cercle des autres") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.recevoirDegats(8, {310.0f, 60.0f}, 10);

    static_cast<void>(escouade.attaquer({MonstreVu{8, cibleMonstre({320.0f, 70.0f})}}, carte)); // avance l'état : les coups ne sont pas l'objet du test

    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(agresseurs.size() == 1);
    REQUIRE(agresseurs[0].monstre.id == 8);
    verifierPointProche(agresseurs[0].monstre.cercle.centre, {320.0f, 70.0f});
    REQUIRE(agresseurs[0].monstre.cercle.rayon == 15.0f);
}

namespace {

// Comme main.cpp : attaquer puis mettreAJour à chaque tic, le monstre étant
// un obstacle pour les personnages.
void simulerAvecMonstre(Escouade& escouade, const TileMap& carte, Cercle monstre, int tics) {
    for (int i = 0; i < tics; ++i) {
        static_cast<void>(escouade.attaquer({MonstreVu{1, monstre}}, carte)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(1.0f / 60.0f, carte, {monstre});
    }
}

float distanceDuPlusProche(const Escouade& escouade, Cercle monstre) {
    return (escouade.proieDuMonstre(1, monstre.centre)->centre - monstre.centre).length();
}

}  // namespace

// Escouade en (210, 210), orientée vers +x : mêlées en (235, 185) et
// (235, 235), tireurs en (185, 185) et (185, 235). Monstre en (180, 120) :
// le tireur arrière-gauche est à 65 px, la mêlée avant-gauche à 85 px de sa
// place (dans la laisse), l'autre mêlée à 127 px (hors laisse).

TEST_CASE("Un personnage sans monstre a portee va vers l'agresseur de sa laisse") {
    const TileMap carte = carteDegageeEntouree();
    const Cercle monstre = cibleMonstre({180.0f, 120.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);

    simulerAvecMonstre(escouade, carte, monstre, 90);

    REQUIRE(distanceDuPlusProche(escouade, monstre) < 45.0f);
}

TEST_CASE("Un agresseur hors de la laisse de tous laisse chacun a sa place") {
    const TileMap carte = carteDegageeEntouree();
    const Cercle monstre = cibleMonstre({210.0f, 380.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);

    simulerAvecMonstre(escouade, carte, monstre, 90);

    REQUIRE(distanceDuPlusProche(escouade, monstre) > 130.0f);
}

TEST_CASE("Quand l'ancre se deplace, personne ne va vers l'agresseur") {
    const TileMap carte = carteDegageeEntouree();
    const Cercle monstre = cibleMonstre({180.0f, 120.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);
    escouade.allerVers({700.0f, 210.0f}, carte);

    simulerAvecMonstre(escouade, carte, monstre, 10);

    REQUIRE(distanceDuPlusProche(escouade, monstre) > 60.0f);
}

TEST_CASE("Apres l'oubli de l'agresseur, tout le monde reprend sa place") {
    const TileMap carte = carteDegageeEntouree();
    const Cercle monstre = cibleMonstre({180.0f, 120.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);

    simulerAvecMonstre(escouade, carte, monstre, 300);

    REQUIRE(escouade.getAgresseurs().empty());
    REQUIRE(distanceDuPlusProche(escouade, monstre) > 60.0f);
}

TEST_CASE("provoquer enregistre le monstre, le provocateur et la duree") {
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 1);
    REQUIRE(provocations[0].id == 7);
    REQUIRE(provocations[0].place == 2);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
}

TEST_CASE("provoquer ignore une place invalide (0 et 5) : aucune provocation, aucun plantage") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});

    escouade.provoquer(7, 0, 4.0f);
    escouade.provoquer(8, 5, 4.0f);

    REQUIRE(escouade.getProvocations().empty());
    escouade.mettreAJour(1.0f / 60.0f, carte, {});
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.proieDuMonstre(7, {300.0f, 60.0f}).has_value());
}

TEST_CASE("Provoquer de nouveau un monstre deja provoque ne le duplique pas et remet sa duree au maximum") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.provoquer(8, 2, 4.0f);
    escouade.mettreAJour(3.0f, carte, {});
    escouade.provoquer(7, 2, 4.0f);

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 2);
    REQUIRE(provocations[0].id == 7);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
    REQUIRE(provocations[1].id == 8);
}

TEST_CASE("Une provocation prend fin apres sa duree") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);

    escouade.mettreAJour(3.9f, carte, {});
    REQUIRE(escouade.getProvocations().size() == 1);
    escouade.mettreAJour(0.2f, carte, {});
    REQUIRE(escouade.getProvocations().empty());
}

TEST_CASE("Une provocation prend fin a la mort du provocateur") {
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.provoquer(8, 1, 4.0f);

    escouade.recevoirDegats(1, {235.0f, 235.0f}, 200);  // tue la place 2
    escouade.mettreAJour(0.016f, carte, {});

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 1);
    REQUIRE(provocations[0].id == 8);
}

TEST_CASE("attaquer retire la provocation d'un monstre absent de la liste") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.provoquer(8, 2, 4.0f);

    static_cast<void>(escouade.attaquer({MonstreVu{8, cibleMonstre({300.0f, 60.0f})}}, carte)); // avance l'état : les coups ne sont pas l'objet du test

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 1);
    REQUIRE(provocations[0].id == 8);
}

// Escouade en (200, 200), orientée vers +x, personnages en (235, 185),
// (235, 235), (185, 185) et (185, 235). Un monstre en (185, 185) est tout
// près de la place 3 ; la place 2 est le provocateur.

TEST_CASE("Sans provocation, la proie d'un monstre est le personnage vivant le plus proche") {
    Escouade escouade({200.0f, 200.0f});

    verifierPointProche(escouade.proieDuMonstre(7, {170.0f, 170.0f})->centre, {185.0f, 185.0f});
}

TEST_CASE("Un monstre provoque vise le provocateur, pas le plus proche") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);

    verifierPointProche(escouade.proieDuMonstre(7, {170.0f, 170.0f})->centre, {235.0f, 235.0f});
}

TEST_CASE("La provocation d'un monstre ne change pas la proie des autres") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);

    verifierPointProche(escouade.proieDuMonstre(8, {170.0f, 170.0f})->centre, {185.0f, 185.0f});
}

TEST_CASE("Un monstre provoque dont le provocateur est mort reprend le plus proche") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.recevoirDegats(1, {235.0f, 235.0f}, 200);  // tue la place 2

    verifierPointProche(escouade.proieDuMonstre(7, {170.0f, 170.0f})->centre, {185.0f, 185.0f});
}

TEST_CASE("recevoirDegats frappe le meme personnage que proieDuMonstre") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);
    const sf::Vector2f monstre{170.0f, 170.0f};
    const Cercle proie = *escouade.proieDuMonstre(7, monstre);

    escouade.recevoirDegats(7, monstre, 200);

    const std::vector<Cercle> vivants = escouade.cerclesDesVivants();
    REQUIRE(vivants.size() == 3);
    REQUIRE(std::none_of(vivants.begin(), vivants.end(), [&](const Cercle& c) { return c.centre == proie.centre; }));
}
