// ./tests/test_Skill.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

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
    Skill degats(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);
    Skill soin(TypeSkill::Soin, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(degats.getType() == TypeSkill::Degats);
    REQUIRE(soin.getType() == TypeSkill::Soin);
}

TEST_CASE("Skill.getRayonZone renvoie le rayon donne au constructeur, distinct de la portee") {
    Skill zone(TypeSkill::Degats, 250.0f, 15, 8.0f, 60.0f);
    Skill monoCible(TypeSkill::Soin, 40.0f, 30, 8.0f, 0.0f);

    REQUIRE(zone.getRayonZone() == 60.0f);
    REQUIRE(zone.estAPortee(point({100.0f, 100.0f}), point({300.0f, 100.0f})));
    REQUIRE(monoCible.getRayonZone() == 0.0f);
}

TEST_CASE("Skill de provocation : la puissance est la durée, avec zone et recharge") {
    Skill provocation(TypeSkill::Provocation, 0.0f, 4, 10.0f, 150.0f);
    const Cercle origine = point({100.0f, 100.0f});

    REQUIRE(provocation.getType() == TypeSkill::Provocation);
    REQUIRE(provocation.getRayonZone() == 150.0f);
    REQUIRE(provocation.estAPortee(origine, origine));
    REQUIRE_FALSE(provocation.estAPortee(origine, point({200.0f, 100.0f})));
    REQUIRE(provocation.declencher(origine, origine) == 4);
    REQUIRE(provocation.getProgressionRecharge() == 0.0f);
}

TEST_CASE("Skill.declencher renvoie la puissance pour une cible à portée") {
    Skill skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(skill.declencher(point({100.0f, 100.0f}), point({150.0f, 100.0f})) == 25);
}

TEST_CASE("Skill.declencher renvoie 0 pour une cible hors de portée") {
    Skill skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(skill.declencher(point({100.0f, 100.0f}), point({300.0f, 100.0f})) == 0);
}

TEST_CASE("Skill.declencher renvoie 0 pendant la recharge, puis de nouveau la puissance") {
    Skill skill(TypeSkill::Soin, 80.0f, 25, 5.0f, 0.0f);
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
    Skill skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    static_cast<void>(skill.declencher(point({100.0f, 100.0f}), point({150.0f, 100.0f})));
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 6.0f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.estAPortee renvoie vrai pour un point proche") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE(skill.estAPortee(point({100.0f, 100.0f}), point({150.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie faux pour un point lointain") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE_FALSE(skill.estAPortee(point({100.0f, 100.0f}), point({300.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie vrai pour une distance exactement égale à la portée") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE(skill.estAPortee(point({100.0f, 100.0f}), point({180.0f, 100.0f})));
}

TEST_CASE("Skill.declencher renvoie 0 au second appel immédiat (recharge en cours)") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = point({100.0f, 100.0f});
    const Cercle cible = point({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 5);
    REQUIRE(skill.declencher(origine, cible) == 0);
}

TEST_CASE("Skill.declencher renvoie 0 après une durée simulée trop courte") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = point({100.0f, 100.0f});
    const Cercle cible = point({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 5);
    ecoulerTemps(skill, 0.3f);
    REQUIRE(skill.declencher(origine, cible) == 0);
}

TEST_CASE("Skill.declencher renvoie de nouveau les dégâts après une durée simulée généreuse") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = point({100.0f, 100.0f});
    const Cercle cible = point({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 5);
    ecoulerTemps(skill, 0.6f);
    REQUIRE(skill.declencher(origine, cible) == 5);
}

TEST_CASE("Un déclenchement de Skill tenté hors de portée ne déclenche pas la recharge") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = point({100.0f, 100.0f});
    const Cercle loin = point({300.0f, 100.0f});
    const Cercle proche = point({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, loin) == 0);
    REQUIRE(skill.declencher(origine, proche) == 5);
}

TEST_CASE("Skill.relancerRecharge empêche de frapper avant une recharge complète") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = point({100.0f, 100.0f});
    const Cercle cible = point({150.0f, 100.0f});

    skill.relancerRecharge(); // jamais utilisée, pourtant elle recharge
    REQUIRE(skill.declencher(origine, cible) == 0);
    ecoulerTemps(skill, 0.6f);
    REQUIRE(skill.declencher(origine, cible) == 5);
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 prête, 0 juste après un coup, entre les deux ensuite") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    skill.relancerRecharge();
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 0.3f);
    const float progression = skill.getProgressionRecharge();
    REQUIRE(progression > 0.0f);
    REQUIRE(progression < 1.0f);

    ecoulerTemps(skill, 0.3f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.getProgressionRecharge ne dépasse jamais 1, même après un long temps écoulé") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    skill.relancerRecharge();
    skill.mettreAJour(10.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}
