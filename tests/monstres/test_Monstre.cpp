// ./tests/monstres/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "combat/Blocage.hpp"
#include "combat/Defense.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"
#include "aides_monstre.hpp"

// Monstre centré en (100, 100), taille 30x30 : bords en x: [85, 115),
// y: [85, 115).

TEST_CASE("contient renvoie vrai pour un point au centre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE(monstre.contient({100.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point a l'exterieur") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE_FALSE(monstre.contient({200.0f, 200.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord gauche") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE(monstre.contient({85.0f, 100.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord haut") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE(monstre.contient({100.0f, 85.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord droit") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE_FALSE(monstre.contient({115.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord bas") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE_FALSE(monstre.contient({100.0f, 115.0f}));
}

TEST_CASE("getCentre renvoie le centre du monstre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le monstre, de rayon la moitie du cote") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const Cercle cercle = monstre.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 15.0f);
}

TEST_CASE("getId renvoie l'identifiant de chaque monstre") {
    const Monstre premier({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const Monstre second({200.0f, 200.0f}, 2, 0, monstreAvecArmure(0));

    REQUIRE(premier.getId() == 1);
    REQUIRE(second.getId() == 2);
}

// Le monstre a tous ses points de vie à la création. Leur nombre n'est pas injectable : les tests
// le lisent sur le monstre neuf (getPointsDeVie), sans l'écrire.

TEST_CASE("Le monstre est vivant a la creation") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est encore vivant apres des degats inferieurs a ses points de vie") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    subir(monstre, monstre.getPointsDeVie() - 20);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres autant de degats que ses points de vie") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    subir(monstre, monstre.getPointsDeVie());

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres un coup qui depasse les points de vie restants") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    subir(monstre, monstre.getPointsDeVie() - 20); // reste 20 points de vie
    subir(monstre, 50); // dépasse largement le reste

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre reste mort apres un coup supplementaire") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));

    subir(monstre, monstre.getPointsDeVie());
    subir(monstre, 1);

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Monstre.recevoirDegats applique son armure") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(20));
    const int maximum = monstre.getPointsDeVie();

    subir(monstre, 50);

    REQUIRE(monstre.getPointsDeVie() == maximum - 30);  // 50 - 20 = 30
}

TEST_CASE("Monstre.recevoirDegats sans armure retire le montant entier") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const int maximum = monstre.getPointsDeVie();

    subir(monstre, 5);

    REQUIRE(monstre.getPointsDeVie() == maximum - 5);
}

TEST_CASE("Monstre.recevoirDegats d'un coup de 1 retire 1 malgre l'armure et reveille le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(20));
    const int maximum = monstre.getPointsDeVie();
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    subir(monstre, 1);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getPointsDeVie() == maximum - 1);
    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.getGroupe rend le groupe recu a la construction") {
    Monstre monstre({100.0f, 100.0f}, 1, 7, monstreAvecArmure(0));

    REQUIRE(monstre.getGroupe() == 7);
}

TEST_CASE("un monstre a une zone de controle de 20 px et l'attaque de sa definition") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(20));

    REQUIRE(monstre.getZoneDeControle() == 20.0f);
    REQUIRE(monstre.getAttaque().degats == definitionDeMonstre(CategorieDeMonstre::Infanterie).attaque.puissance);
}

TEST_CASE("un monstre prend toutes ses valeurs de sa definition") {
    DefinitionDeMonstre definition = definitionDeMonstre(CategorieDeMonstre::Infanterie);
    definition.cote = 50.0f;
    definition.pointsDeVieMax = 300;
    definition.armure = 40;
    definition.zoneDeControle = 35.0f;
    definition.attaque.puissance = 77;
    definition.statistiques = {.capaciteOffensive = 130, .capaciteDefensive = 90};
    Monstre monstre({100.0f, 100.0f}, 1, 0, definition);

    REQUIRE(monstre.getCercle().rayon == 25.0f);
    REQUIRE(monstre.getPointsDeVie() == 300);
    REQUIRE(monstre.getZoneDeControle() == 35.0f);
    REQUIRE(monstre.getAttaque().degats == 77);
    REQUIRE(monstre.getStatistiques() == StatistiquesDeCombat{130, 90});
    REQUIRE(subir(monstre, 100) == appliquerArmure(100, 40));
}

TEST_CASE("l'attaque d'un monstre porte la capacite offensive de sa definition") {
    DefinitionDeMonstre definition = definitionDeMonstre(CategorieDeMonstre::Infanterie);
    definition.statistiques.capaciteOffensive = 130;
    const Monstre monstre({100.0f, 100.0f}, 1, 0, definition);

    REQUIRE(monstre.getAttaque().degats == definition.attaque.puissance);
    REQUIRE(monstre.getAttaque().capaciteOffensive == 130);
}

TEST_CASE("l'attaque de l'infanterie est une attaque de contact") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, definitionDeMonstre(CategorieDeMonstre::Infanterie));

    REQUIRE(monstre.getAttaque().nature == NatureDAttaque::Contact);
}

TEST_CASE("l'infanterie esquive 5 % des coups de contact et 5 % des projectiles") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, definitionDeMonstre(CategorieDeMonstre::Infanterie));

    REQUIRE(monstre.getStatistiques().esquiveContact == 5);
    REQUIRE(monstre.getStatistiques().esquiveProjectile == 5);
}

TEST_CASE("un coup esquive par un monstre ne lui retire rien mais le reveille quand meme") {
    const TileMap carte = carteDegagee10x10();
    DefinitionDeMonstre definition = monstreSansHasard();
    definition.statistiques.esquiveContact = 50;
    const Attaque attaque{.degats = 100, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    Generateur generateur = generateurDeTest();
    int esquives = 0;

    // Esquivé ou non, un coup réveille le monstre ; esquivé, il ne lui retire rien. À 50 %, la chance de n'en voir
    // aucun esquivé sur 100 coups est de 2^-100, quelle que soit la graine.
    for (int essai = 0; essai < 100; ++essai) {
        Monstre monstre({100.0f, 100.0f}, 1, 0, definition);
        const int vieAvant = monstre.getPointsDeVie();

        const bool esquive = monstre.recevoirDegats(attaque, generateur).esquive;

        if (esquive) {
            ++esquives;
            REQUIRE(monstre.getPointsDeVie() == vieAvant);
        }
        ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);  // une proie hors détection
        REQUIRE(monstre.getCentre().x > 100.0f);  // l'attaque a eu lieu : le monstre poursuit
    }
    REQUIRE(esquives > 0);
}

TEST_CASE("un monstre au bouclier : le blocage se recupere avec le temps qui passe") {
    const TileMap carte = carteDegagee10x10();
    DefinitionDeMonstre definition = definitionDeMonstre(CategorieDeMonstre::Infanterie);
    definition.statistiques = {.capaciteDefensive = 100, .chanceDeBlocage = 100, .montantBloque = 40};
    Monstre monstre({100.0f, 100.0f}, 1, 0, definition);
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = 100, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};

    REQUIRE(monstre.recevoirDegats(attaque, generateur).bloque == 40);
    REQUIRE(monstre.recevoirDegats(attaque, generateur).bloque == 0);  // en récupération

    ecoulerTemps(monstre, s_recuperationDuBlocage + 0.1f, std::nullopt, carte);

    REQUIRE(monstre.recevoirDegats(attaque, generateur).bloque == 40);
}

TEST_CASE("lire la puissance de l'attaque ne change ni le coup engage ni la recharge") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    const TileMap carte = carteDegagee10x10();
    Generateur generateur = generateurDeTest();
    REQUIRE(engager(monstre, pointCible({125.0f, 100.0f}), 1, carte));

    const int puissance = monstre.getAttaque().degats;

    REQUIRE(monstre.cibleEngagee() == 1);
    REQUIRE(degatsOuZero(monstre.avancerAttaque(0.4f, generateur)) == puissance);  // le coup engagé part normalement
    REQUIRE(monstre.getAttaque().degats == puissance);
    REQUIRE_FALSE(engager(monstre, pointCible({125.0f, 100.0f}), 1, carte));  // la recharge court toujours
}

TEST_CASE("Monstre.recevoirDegats renvoie les degats subis apres armure") {
    Monstre sansArmure({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    Monstre arme({100.0f, 100.0f}, 2, 0, monstreAvecArmure(20));

    REQUIRE(subir(sansArmure, 50) == 50);
    REQUIRE(subir(arme, 50) == 30);  // 50 - 20 = 30
}

TEST_CASE("Monstre.recevoirDegats sur un monstre mort renvoie zero") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, monstreAvecArmure(0));
    subir(monstre, monstre.getPointsDeVie());

    REQUIRE(subir(monstre, 10) == 0);
}
