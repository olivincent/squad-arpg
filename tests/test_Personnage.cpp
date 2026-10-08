// ./tests/test_Personnage.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "Personnage.hpp"
#include "Attaque.hpp"
#include "Cercle.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"

namespace {

// Cible ponctuelle (rayon nul), pour les tests qui ne portent pas sur
// la forme de la cible elle-même.
Cercle pointCible(sf::Vector2f centre) {
    return {centre, 0.0f};
}

// Carte dégagée dédiée aux tests de recharge : le personnage ne bouge
// pas (cible = son propre centre), seul le temps doit s'écouler.
TileMap carteDegagee() {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    return TileMap(plan, 40.0f);
}

void ecoulerTemps(Personnage& personnage, const TileMap& carte, sf::Vector2f centre, float duree) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        personnage.mettreAJour(pas, centre, carte, {});
        tempsRestant -= pas;
    }
}

}  // namespace

// Personnage centré en (100, 100), portée 80 px.

TEST_CASE("estAPortee renvoie vrai pour un point proche") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    REQUIRE(personnage.estAPortee(pointCible({150.0f, 100.0f})));
}

TEST_CASE("estAPortee renvoie faux pour un point lointain") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    REQUIRE_FALSE(personnage.estAPortee(pointCible({300.0f, 100.0f})));
}

TEST_CASE("frapper renvoie les dégâts pour une cible à portée") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    REQUIRE(personnage.frapper(pointCible({150.0f, 100.0f})) == 5);
}

TEST_CASE("frapper renvoie 0 pour une cible hors de portée") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    REQUIRE(personnage.frapper(pointCible({300.0f, 100.0f})) == 0);
}

TEST_CASE("frapper renvoie de nouveau les dégâts après une durée simulée généreuse") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    const TileMap carte = carteDegagee();
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(personnage.frapper(cible) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(personnage.frapper(cible) == 5);
}

TEST_CASE("Un personnage est vivant a la creation") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est encore vivant apres 80 degats") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    personnage.recevoirDegats(80);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est mort apres 100 degats") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    personnage.recevoirDegats(100);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("getCentre renvoie le centre donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    REQUIRE(personnage.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le personnage, de rayon la moitie du cote") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    const Cercle cercle = personnage.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 10.0f);
}

TEST_CASE("Un personnage est mort apres un coup qui depasse le reste de ses points de vie") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);

    personnage.recevoirDegats(80);
    personnage.recevoirDegats(50);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("Un personnage mort ne frappe plus") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    const Cercle cible = pointCible({150.0f, 100.0f});

    personnage.recevoirDegats(100);

    REQUIRE(personnage.frapper(cible) == 0);
}

TEST_CASE("Un personnage mort ne bouge plus") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    const TileMap carte = carteDegagee();
    const sf::Vector2f centreInitial = personnage.getCentre();

    personnage.recevoirDegats(100);
    ecoulerTemps(personnage, carte, {500.0f, 500.0f}, 2.0f); // cible lointaine, durée généreuse

    REQUIRE(personnage.getCentre() == centreInitial);
}

// Skill : portée 80 px comme l'attaque de base, 25 dégâts, 5 s de recharge.

TEST_CASE("declencherSkill renvoie les degats du skill pour une cible a portee") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), Skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f)));

    REQUIRE(personnage.declencherSkill(pointCible({150.0f, 100.0f})) == 25);
}

TEST_CASE("declencherSkill renvoie 0 pour un personnage sans skill, sans toucher a l'attaque de base") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(personnage.declencherSkill(cible) == 0);
    REQUIRE(personnage.frapper(cible) == 5);
}

TEST_CASE("declencherSkill renvoie 0 pour un personnage mort") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), Skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f)));

    personnage.recevoirDegats(100);

    REQUIRE(personnage.declencherSkill(pointCible({150.0f, 100.0f})) == 0);
}

TEST_CASE("declencherSkill renvoie 0 hors de portee, sans toucher a l'attaque de base") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), Skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f)));

    REQUIRE(personnage.declencherSkill(pointCible({300.0f, 100.0f})) == 0);
    REQUIRE(personnage.frapper(pointCible({150.0f, 100.0f})) == 5);
}

TEST_CASE("declencherSkill renvoie 0 pendant sa recharge, puis de nouveau les degats une fois ecoulee") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), Skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f)));
    const TileMap carte = carteDegagee();
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(personnage.declencherSkill(cible) == 25);
    REQUIRE(personnage.declencherSkill(cible) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 4.8f);
    REQUIRE(personnage.declencherSkill(cible) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.4f);
    REQUIRE(personnage.declencherSkill(cible) == 25);
}

TEST_CASE("Apres un skill reussi, l'attaque de base attend une recharge complete") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), Skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f)));
    const TileMap carte = carteDegagee();
    const Cercle cible = pointCible({150.0f, 100.0f});

    // Attaque de base lancée, puis 0,3 s écoulées : il ne lui reste que
    // 0,2 s de recharge. Le skill la relance au complet (0,5 s) : 0,3 s
    // plus tard, elle ne doit toujours pas partir, alors qu'elle l'aurait
    // fait sans le skill.
    REQUIRE(personnage.frapper(cible) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.declencherSkill(cible) == 25);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.frapper(cible) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.frapper(cible) == 5);
}

TEST_CASE("mettreAJour s'arrête avant d'entrer dans un obstacle") {
    const TileMap carte = carteDegagee();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    const std::vector<Cercle> obstacles = {{{140.0f, 100.0f}, 15.0f}};

    for (int i = 0; i < 60; ++i) {
        personnage.mettreAJour(1.0f / 60.0f, {170.0f, 100.0f}, carte, obstacles);
    }

    // distanceBordABord est bornée à 0 : on compare donc les distances
    // entre centres et la somme des rayons.
    const float ecartCentres = (personnage.getCentre() - obstacles[0].centre).length();
    REQUIRE(ecartCentres >= personnage.getCercle().rayon + obstacles[0].rayon - 0.001f);
    REQUIRE(personnage.getCentre().x > 100.0f);
    REQUIRE(personnage.getCentre().x < 140.0f);
}

TEST_CASE("recevoirSoin rend des points de vie") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    personnage.recevoirDegats(50);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == 80);
}

TEST_CASE("recevoirSoin ne dépasse pas le maximum") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    personnage.recevoirDegats(10);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

TEST_CASE("recevoirSoin est sans effet sur un mort") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Attaque(80.0f, 5, 0.5f), std::nullopt);
    personnage.recevoirDegats(100);

    personnage.recevoirSoin(30);

    REQUIRE_FALSE(personnage.estVivant());
    REQUIRE(personnage.getPointsDeVie() == 0);
}
