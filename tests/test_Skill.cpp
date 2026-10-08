// ./tests/test_Skill.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "Attaque.hpp"
#include "Cercle.hpp"
#include "Skill.hpp"

namespace {

void ecoulerTemps(Skill& skill, float duree) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        skill.mettreAJour(pas);
        tempsRestant -= pas;
    }
}

Cercle point(sf::Vector2f centre) {
    return {centre, 0.0f};
}

}  // namespace

// Origine en (100, 100), portée 80 px, puissance 25, recharge 5 s.

TEST_CASE("Skill.getType renvoie le type donné au constructeur") {
    Skill degats(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f));
    Skill soin(TypeSkill::Soin, Attaque(80.0f, 25, 5.0f));

    REQUIRE(degats.getType() == TypeSkill::Degats);
    REQUIRE(soin.getType() == TypeSkill::Soin);
}

TEST_CASE("Skill.declencher renvoie la puissance pour une cible à portée") {
    Skill skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f));

    REQUIRE(skill.declencher(point({100.0f, 100.0f}), point({150.0f, 100.0f})) == 25);
}

TEST_CASE("Skill.declencher renvoie 0 pour une cible hors de portée") {
    Skill skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f));

    REQUIRE(skill.declencher(point({100.0f, 100.0f}), point({300.0f, 100.0f})) == 0);
}

TEST_CASE("Skill.declencher renvoie 0 pendant la recharge, puis de nouveau la puissance") {
    Skill skill(TypeSkill::Soin, Attaque(80.0f, 25, 5.0f));
    const Cercle origine = point({100.0f, 100.0f});
    const Cercle cible = point({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 25);
    REQUIRE(skill.declencher(origine, cible) == 0);

    ecoulerTemps(skill, 4.8f);
    REQUIRE(skill.declencher(origine, cible) == 0);

    ecoulerTemps(skill, 0.4f);
    REQUIRE(skill.declencher(origine, cible) == 25);
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 prêt, 0 juste après le déclenchement") {
    Skill skill(TypeSkill::Degats, Attaque(80.0f, 25, 5.0f));

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    skill.declencher(point({100.0f, 100.0f}), point({150.0f, 100.0f}));
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 6.0f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}
