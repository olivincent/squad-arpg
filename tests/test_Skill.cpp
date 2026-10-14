// ./tests/test_Skill.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Skill.hpp"

namespace {

void ecoulerTemps(Skill& skill, float duree) {
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        skill.mettreAJour(s_dureeTic);
    }
}

}  // namespace

// Origine en (100, 100), portée 80 px, puissance 25, recharge 5 s.

TEST_CASE("Skill.getType renvoie le type donne au constructeur") {
    Skill degats(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);
    Skill soin(TypeSkill::Soin, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(degats.getType() == TypeSkill::Degats);
    REQUIRE(soin.getType() == TypeSkill::Soin);
}

TEST_CASE("Skill.getRayonZone renvoie le rayon donne au constructeur, distinct de la portee") {
    Skill zone(TypeSkill::Degats, 250.0f, 15, 8.0f, 60.0f);
    Skill monoCible(TypeSkill::Soin, 40.0f, 30, 8.0f, 0.0f);

    REQUIRE(zone.getRayonZone() == 60.0f);
    REQUIRE(zone.estAPortee(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})));
    REQUIRE(monoCible.getRayonZone() == 0.0f);
}

TEST_CASE("Skill de provocation : la puissance est la duree, avec zone et recharge") {
    Skill provocation(TypeSkill::Provocation, 0.0f, 4, 10.0f, 150.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});

    REQUIRE(provocation.getType() == TypeSkill::Provocation);
    REQUIRE(provocation.getRayonZone() == 150.0f);
    REQUIRE(provocation.estAPortee(origine, origine));
    REQUIRE_FALSE(provocation.estAPortee(origine, pointCible({200.0f, 100.0f})));
    REQUIRE(provocation.declencher(origine, origine) == 4);
    REQUIRE(provocation.getProgressionRecharge() == 0.0f);
}

TEST_CASE("Skill.declencher renvoie la puissance pour une cible a portee") {
    Skill skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(skill.declencher(pointCible({100.0f, 100.0f}), pointCible({150.0f, 100.0f})) == 25);
}

TEST_CASE("Skill.declencher renvoie 0 pour une cible hors de portee") {
    Skill skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(skill.declencher(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})) == 0);
}

TEST_CASE("Skill.declencher renvoie 0 pendant la recharge, puis de nouveau la puissance") {
    Skill skill(TypeSkill::Soin, 80.0f, 25, 5.0f, 0.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 25);
    REQUIRE(skill.declencher(origine, cible) == 0);

    ecoulerTemps(skill, 4.8f);
    REQUIRE(skill.declencher(origine, cible) == 0);

    ecoulerTemps(skill, 0.4f);
    REQUIRE(skill.declencher(origine, cible) == 25);
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 pret, 0 juste apres le declenchement") {
    Skill skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 0.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    REQUIRE(skill.declencher(pointCible({100.0f, 100.0f}), pointCible({150.0f, 100.0f})) == 25);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 6.0f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.estAPortee renvoie vrai pour un point proche") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({150.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie faux pour un point lointain") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE_FALSE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie vrai pour une distance exactement egale a la portee") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    REQUIRE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({180.0f, 100.0f})));
}

TEST_CASE("Skill.declencher renvoie 0 au second appel immediat (recharge en cours)") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 5);
    REQUIRE(skill.declencher(origine, cible) == 0);
}

TEST_CASE("Skill.declencher renvoie 0 apres une duree simulee trop courte") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 5);
    ecoulerTemps(skill, 0.3f);
    REQUIRE(skill.declencher(origine, cible) == 0);
}

TEST_CASE("Skill.declencher renvoie de nouveau les degats apres une duree simulee genereuse") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, cible) == 5);
    ecoulerTemps(skill, 0.6f);
    REQUIRE(skill.declencher(origine, cible) == 5);
}

TEST_CASE("Un declenchement de Skill tente hors de portee ne declenche pas la recharge") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});
    const Cercle loin = pointCible({300.0f, 100.0f});
    const Cercle proche = pointCible({150.0f, 100.0f});

    REQUIRE(skill.declencher(origine, loin) == 0);
    REQUIRE(skill.declencher(origine, proche) == 5);
}

TEST_CASE("Skill.relancerRecharge empeche de frapper avant une recharge complete") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);
    const Cercle origine = pointCible({100.0f, 100.0f});
    const Cercle cible = pointCible({150.0f, 100.0f});

    skill.relancerRecharge(); // jamais utilisée, pourtant elle recharge
    REQUIRE(skill.declencher(origine, cible) == 0);
    ecoulerTemps(skill, 0.6f);
    REQUIRE(skill.declencher(origine, cible) == 5);
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 prete, 0 juste apres un coup, entre les deux ensuite") {
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

TEST_CASE("Skill.getProgressionRecharge ne depasse jamais 1, meme apres un long temps ecoule") {
    Skill skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.0f);

    skill.relancerRecharge();
    skill.mettreAJour(10.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.declencherSansCible renvoie la puissance sans verifier la portee") {
    Skill skill(TypeSkill::Provocation, 0.0f, 4, 10.0f, 150.0f);

    REQUIRE(skill.declencherSansCible() == 4);
}

TEST_CASE("Skill.declencherSansCible relance la recharge et refuse tant qu'elle court") {
    Skill skill(TypeSkill::Zone, 250.0f, 15, 8.0f, 60.0f);

    REQUIRE(skill.declencherSansCible() == 15);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);
    REQUIRE(skill.declencherSansCible() == 0);

    ecoulerTemps(skill, 8.2f);
    REQUIRE(skill.declencherSansCible() == 15);
}
