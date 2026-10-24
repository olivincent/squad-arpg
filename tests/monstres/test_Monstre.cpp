// ./tests/monstres/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"
#include "aides_monstre.hpp"

// Monstre centré en (100, 100), taille 30x30 : bords en x: [85, 115),
// y: [85, 115).

TEST_CASE("contient renvoie vrai pour un point au centre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.contient({100.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point a l'exterieur") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.contient({200.0f, 200.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord gauche") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.contient({85.0f, 100.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord haut") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.contient({100.0f, 85.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord droit") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.contient({115.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord bas") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.contient({100.0f, 115.0f}));
}

TEST_CASE("getCentre renvoie le centre du monstre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le monstre, de rayon la moitie du cote") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const Cercle cercle = monstre.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 15.0f);
}

TEST_CASE("getId renvoie l'identifiant de chaque monstre") {
    const Monstre premier({100.0f, 100.0f}, 1, 0, 0);
    const Monstre second({200.0f, 200.0f}, 2, 0, 0);

    REQUIRE(premier.getId() == 1);
    REQUIRE(second.getId() == 2);
}

// Le monstre a tous ses points de vie à la création. Leur nombre n'est pas injectable : les tests
// le lisent sur le monstre neuf (getPointsDeVie), sans l'écrire.

TEST_CASE("Le monstre est vivant a la creation") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est encore vivant apres des degats inferieurs a ses points de vie") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie() - 20);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres autant de degats que ses points de vie") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie());

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres un coup qui depasse les points de vie restants") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie() - 20); // reste 20 points de vie
    monstre.recevoirDegats(50); // dépasse largement le reste

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre reste mort apres un coup supplementaire") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie());
    monstre.recevoirDegats(1);

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Monstre.recevoirDegats applique son armure") {
    Monstre monstre({100.0f, 100.0f}, 1, 20, 0);
    const int maximum = monstre.getPointsDeVie();

    monstre.recevoirDegats(50);

    REQUIRE(monstre.getPointsDeVie() == maximum - 30);  // 50 - 20 = 30
}

TEST_CASE("Monstre.recevoirDegats sans armure retire le montant entier") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const int maximum = monstre.getPointsDeVie();

    monstre.recevoirDegats(5);

    REQUIRE(monstre.getPointsDeVie() == maximum - 5);
}

TEST_CASE("Monstre.recevoirDegats d'un coup de 1 retire 1 malgre l'armure et reveille le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 20, 0);
    const int maximum = monstre.getPointsDeVie();
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    monstre.recevoirDegats(1);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getPointsDeVie() == maximum - 1);
    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.getGroupe rend le groupe recu a la construction") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 7);

    REQUIRE(monstre.getGroupe() == 7);
}

TEST_CASE("un monstre a une zone de controle de 20 px et une attaque de 100 degats") {
    const Monstre monstre({100.0f, 100.0f}, 1, 20, 0);

    REQUIRE(monstre.getZoneDeControle() == 20.0f);
    REQUIRE(Monstre::s_zoneDeControle == 20.0f);
    REQUIRE(monstre.getPuissanceAttaque() == 100);
}

TEST_CASE("lire la puissance de l'attaque ne change ni le coup engage ni la recharge") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const TileMap carte = carteDegagee10x10();
    Generateur generateur = generateurDeTest();
    REQUIRE(engager(monstre, pointCible({125.0f, 100.0f}), 1, carte));

    const int puissance = monstre.getPuissanceAttaque();

    REQUIRE(monstre.cibleEngagee() == 1);
    REQUIRE(monstre.avancerAttaque(0.4f, generateur) == puissance);  // le coup engagé part normalement
    REQUIRE(monstre.getPuissanceAttaque() == puissance);
    REQUIRE_FALSE(engager(monstre, pointCible({125.0f, 100.0f}), 1, carte));  // la recharge court toujours
}

TEST_CASE("Monstre.recevoirDegats renvoie les degats subis apres armure") {
    Monstre sansArmure({100.0f, 100.0f}, 1, 0, 0);
    Monstre arme({100.0f, 100.0f}, 2, 20, 0);

    REQUIRE(sansArmure.recevoirDegats(50) == 50);
    REQUIRE(arme.recevoirDegats(50) == 30);  // 50 - 20 = 30
}

TEST_CASE("Monstre.recevoirDegats sur un monstre mort renvoie zero") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    monstre.recevoirDegats(monstre.getPointsDeVie());

    REQUIRE(monstre.recevoirDegats(10) == 0);
}
