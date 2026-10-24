// ./tests/monstres/test_Monstre_retour.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"
#include "aides_monstre.hpp"

TEST_CASE("Monstre touche en retour dans son territoire ne se reveille pas et ne perd rien") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte, 50);
    // Laisse le retour ramener le monstre dans son territoire.
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > 300.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    const float xAvant = monstre.getCentre().x;
    const int vieAvant = monstre.getPointsDeVie();

    subir(monstre, 10);
    REQUIRE(monstre.getPointsDeVie() == vieAvant); // invulnérable
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // continue de rentrer, à l'ouest
}

TEST_CASE("Monstre en retour est invulnerable sans rien tirer") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte);
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup resultat = monstre.recevoirDegats({.montant = 50, .capaciteOffensive = 100000}, generateur);

    REQUIRE(resultat.montant == 0);
    REQUIRE_FALSE(resultat.critique);
    REQUIRE(generateur == intact);
}

TEST_CASE("Monstre.alerter en retour dans son territoire ne reveille pas le monstre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte);
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > 300.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    const float xAvant = monstre.getCentre().x;

    monstre.alerter();
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant);
    REQUIRE_FALSE(monstre.estEnPoursuite());
}

TEST_CASE("Monstre.engagerAttaque en retour n'engage aucun coup meme avec une proie a portee") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte);
    const Cercle proche{monstre.getCentre() + sf::Vector2f(20.0f, 0.0f), 10.0f};

    REQUIRE_FALSE(engager(monstre, proche, 1, carte));
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
}

TEST_CASE("Monstre.estCiblable est vrai en garde et en poursuite, faux en retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    REQUIRE(monstre.estCiblable());

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // poursuite
    REQUIRE(monstre.estEnPoursuite());
    REQUIRE(monstre.estCiblable());

    amenerEnRetourHorsTerritoire(monstre, carte);
    REQUIRE_FALSE(monstre.estCiblable());
}

// Régénération en retour : 50 % des points de vie maximum par seconde.

TEST_CASE("Monstre en retour regagne tous ses points de vie en deux secondes environ") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const int maximum = monstre.getPointsDeVie();
    amenerEnRetourHorsTerritoire(monstre, carte, maximum - 10);
    // Environ 10 : le premier tic du retour a pu rendre un point.
    const int vieEnRetour = monstre.getPointsDeVie();
    REQUIRE(vieEnRetour < maximum / 2);

    ecoulerTemps(monstre, 1.0f, pointCible({900.0f, 100.0f}), carte);
    REQUIRE(monstre.getPointsDeVie() >= vieEnRetour + maximum / 2 - 5);
    REQUIRE(monstre.getPointsDeVie() <= vieEnRetour + maximum / 2 + 5);

    ecoulerTemps(monstre, 1.0f, pointCible({900.0f, 100.0f}), carte);
    REQUIRE(monstre.getPointsDeVie() == maximum);
}

TEST_CASE("Monstre en retour ne depasse jamais son maximum") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const int maximum = monstre.getPointsDeVie();
    amenerEnRetourHorsTerritoire(monstre, carte, 10);

    ecoulerTemps(monstre, 3.0f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getPointsDeVie() == maximum);
}

TEST_CASE("Monstre en retour regagne des points de vie a 60 images par seconde") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const int maximum = monstre.getPointsDeVie();
    amenerEnRetourHorsTerritoire(monstre, carte, maximum - 10);
    const int vieEnRetour = monstre.getPointsDeVie();

    ecoulerTemps(monstre, 0.1f, pointCible({900.0f, 100.0f}), carte); // 6 images, moins d'un point chacune

    // 0,1 s à 50 % du maximum par seconde : la partie entière cumulée, pas 0 à chaque image.
    REQUIRE(monstre.getPointsDeVie() >= vieEnRetour + maximum / 20 - 1);
}

TEST_CASE("Monstre ne regagne aucun point de vie hors retour") {
    const TileMap carte = carteLongue();
    Monstre enGarde({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const int maximum = enGarde.getPointsDeVie();
    subir(enGarde, 50);
    ecoulerTemps(enGarde, 2.0f, std::nullopt, carte);
    REQUIRE(enGarde.getPointsDeVie() == maximum - 50);

    Monstre enPoursuite({100.0f, 100.0f}, 2, 0, monstreAvecArmure(0));
    ecoulerTemps(enPoursuite, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    subir(enPoursuite, 50);
    ecoulerTemps(enPoursuite, 1.0f, pointCible({250.0f, 100.0f}), carte);
    REQUIRE(enPoursuite.estEnPoursuite());
    REQUIRE(enPoursuite.getPointsDeVie() == maximum - 50);
}

TEST_CASE("Monstre touche en retour hors de son territoire continue de rentrer") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte);
    const float xAvant = monstre.getCentre().x;

    subir(monstre, 10);
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // va vers son poste, à l'ouest
}

TEST_CASE("Monstre en retour bloque par un autre monstre recalcule et rentre a son poste") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte);

    // Un autre monstre se place en travers du retour, après le choix du chemin.
    const std::vector<Cercle> autre = {{{monstre.getCentre().x - 80.0f, 100.0f}, 15.0f}};
    ecoulerTemps(monstre, 10.0f, std::nullopt, carte, autre, autre);

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.recevoirDegats en retour renvoie zero") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte, 50);

    REQUIRE(subir(monstre, 10) == 0);
}
