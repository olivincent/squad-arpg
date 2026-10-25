// ./tests/combat/test_Skill.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <vector>

#include "aides.hpp"
#include "combat/TypeDArme.hpp"
#include "base/Cercle.hpp"
#include "combat/Skill.hpp"

namespace {

void ecoulerTemps(Skill& skill, float duree) {
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        skill.mettreAJour(s_dureeTic);
    }
}

// Lance le skill d'un trait (temps d'exécution nul) : vrai si le coup part.
bool lancer(Skill& skill, Generateur& generateur) {
    return skill.commencer() && skill.avancer(0.0f, generateur);
}

}  // namespace

// Origine en (100, 100), portée 80 px, puissance 25, recharge 5 s.

TEST_CASE("Skill.getType renvoie le type donne au constructeur") {
    Skill degats({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f,
                  .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact});
    Skill soin({.type = TypeSkill::Soin, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact});

    REQUIRE(degats.getType() == TypeSkill::DegatsSurCible);
    REQUIRE(soin.getType() == TypeSkill::Soin);
}

TEST_CASE("Skill.getRayonZone renvoie le rayon donne au constructeur, distinct de la portee") {
    Skill zone({.type = TypeSkill::DegatsSurCible, .portee = 250.0f, .puissance = 15, .rechargeMin = 8.0f,
                .rechargeMax = 8.0f, .rayonZone = 60.0f, .nature = NatureDAttaque::Contact});
    Skill monoCible({.type = TypeSkill::Soin, .portee = 40.0f, .puissance = 30, .rechargeMin = 8.0f,
                     .rechargeMax = 8.0f, .nature = NatureDAttaque::Contact});

    REQUIRE(zone.getRayonZone() == 60.0f);
    REQUIRE(zone.estAPortee(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})));
    REQUIRE(monoCible.getRayonZone() == 0.0f);
}

TEST_CASE("Skill de provocation : la puissance est la duree, avec zone et recharge") {
    Generateur generateur = generateurDeTest();
    Skill provocation({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f,
                       .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact});
    const Cercle origine = pointCible({100.0f, 100.0f});

    REQUIRE(provocation.getType() == TypeSkill::Provocation);
    REQUIRE(provocation.getRayonZone() == 150.0f);
    REQUIRE(provocation.estAPortee(origine, origine));
    REQUIRE_FALSE(provocation.estAPortee(origine, pointCible({200.0f, 100.0f})));
    REQUIRE(provocation.getPuissance() == 4);
    REQUIRE(lancer(provocation, generateur));
    REQUIRE(provocation.getProgressionRecharge() == 0.0f);
}

TEST_CASE("Skill refuse de partir pendant la recharge, puis repart une fois ecoulee") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::Soin, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact});

    REQUIRE(lancer(skill, generateur));
    REQUIRE_FALSE(lancer(skill, generateur));

    ecoulerTemps(skill, 4.8f);
    REQUIRE_FALSE(lancer(skill, generateur));

    ecoulerTemps(skill, 0.4f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 pret, 0 juste apres le declenchement") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f,
                 .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact});

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    REQUIRE(lancer(skill, generateur));
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 6.0f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.estAPortee renvoie vrai pour un point proche") {
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({150.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie faux pour un point lointain") {
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE_FALSE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie vrai pour une distance exactement egale a la portee") {
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({180.0f, 100.0f})));
}

TEST_CASE("Skill refuse de partir apres une duree simulee trop courte") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE(lancer(skill, generateur));
    ecoulerTemps(skill, 0.3f);
    REQUIRE_FALSE(lancer(skill, generateur));
}

TEST_CASE("Skill repart apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE(lancer(skill, generateur));
    ecoulerTemps(skill, 0.6f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill.relancerRecharge empeche de frapper avant une recharge complete") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    skill.relancerRecharge(generateur);
    REQUIRE_FALSE(lancer(skill, generateur));
    ecoulerTemps(skill, 0.6f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 prete, 0 juste apres un coup, entre les deux ensuite") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    skill.relancerRecharge(generateur);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 0.3f);
    const float progression = skill.getProgressionRecharge();
    REQUIRE(progression > 0.0f);
    REQUIRE(progression < 1.0f);

    ecoulerTemps(skill, 0.3f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.getProgressionRecharge ne depasse jamais 1, meme apres un long temps ecoule") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact});

    skill.relancerRecharge(generateur);
    skill.mettreAJour(10.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("La nature d'un skill vient de sa definition, Contact par defaut") {
    const Skill parDefaut({.type = TypeSkill::DegatsSurCible, .portee = 20.0f, .puissance = 5, .rechargeMin = 1.0f, .rechargeMax = 1.0f, .nature = NatureDAttaque::Contact});
    const Skill tir({.type = TypeSkill::DegatsSurCible, .portee = 120.0f, .puissance = 5, .rechargeMin = 1.0f, .rechargeMax = 1.0f,
                     .nature = NatureDAttaque::Projectile});
    const Skill zone({.type = TypeSkill::Zone, .portee = 250.0f, .puissance = 5, .rechargeMin = 1.0f, .rechargeMax = 1.0f,
                      .rayonZone = 60.0f, .nature = NatureDAttaque::Zone});

    REQUIRE(parDefaut.getNature() == NatureDAttaque::Contact);
    REQUIRE(tir.getNature() == NatureDAttaque::Projectile);
    REQUIRE(zone.getNature() == NatureDAttaque::Zone);
}

TEST_CASE("Skill de zone : le depart relance la recharge, refuse tant qu'elle court") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::Zone, .portee = 250.0f, .puissance = 15, .rechargeMin = 8.0f, .rechargeMax = 8.0f,
                 .rayonZone = 60.0f, .nature = NatureDAttaque::Zone});

    REQUIRE(lancer(skill, generateur));
    REQUIRE(skill.getPuissance() == 15);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);
    REQUIRE_FALSE(lancer(skill, generateur));

    ecoulerTemps(skill, 8.2f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill a recharge fixe (min == max) donne exactement cette valeur, sans tirer de nombre") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f,
                 .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact});

    REQUIRE(lancer(skill, generateur));

    REQUIRE(skill.getRechargeRestante() == 5.0f);
    REQUIRE(generateur == intact);  // aucun nombre tiré : l'état du générateur n'a pas bougé
}

TEST_CASE("Skill a recharge en intervalle : chaque recharge est dans l'intervalle et elles ne sont pas toutes egales") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 0.3f,
                 .rechargeMax = 0.7f, .nature = NatureDAttaque::Contact});
    std::vector<float> recharges;

    for (int i = 0; i < 200; ++i) {
        skill.mettreAJour(1.0f);  // recharge écoulée
        REQUIRE(lancer(skill, generateur));
        recharges.push_back(skill.getRechargeRestante());
    }

    for (const float recharge : recharges) {
        REQUIRE(recharge >= 0.3f);
        REQUIRE(recharge <= 0.7f);
    }
    REQUIRE(std::any_of(recharges.begin(), recharges.end(), [&](float r) { return r != recharges.front(); }));
}

TEST_CASE("Skill : deux generateurs de meme graine donnent la meme suite de recharges") {
    Generateur premier = generateurDeTest();
    Generateur second = generateurDeTest();
    Skill skillA({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 0.3f,
                  .rechargeMax = 0.7f, .nature = NatureDAttaque::Contact});
    Skill skillB({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 0.3f,
                  .rechargeMax = 0.7f, .nature = NatureDAttaque::Contact});

    for (int i = 0; i < 20; ++i) {
        skillA.mettreAJour(1.0f);
        skillB.mettreAJour(1.0f);
        REQUIRE(lancer(skillA, premier));
        REQUIRE(lancer(skillB, second));
        REQUIRE(skillA.getRechargeRestante() == skillB.getRechargeRestante());
    }
}

TEST_CASE("Skill.relancerRecharge tire aussi dans l'intervalle") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 0.3f,
                 .rechargeMax = 0.7f, .nature = NatureDAttaque::Contact});

    skill.relancerRecharge(generateur);

    REQUIRE(skill.getRechargeRestante() >= 0.3f);
    REQUIRE(skill.getRechargeRestante() <= 0.7f);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);
}

// Machine à états : prêt, en exécution, en recharge.

TEST_CASE("Skill.commencer passe en execution depuis pret seulement") {
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f, .tempsExecution = 0.5f, .nature = NatureDAttaque::Contact});

    REQUIRE(skill.getPhase() == PhaseSkill::Pret);
    REQUIRE(skill.commencer());
    REQUIRE(skill.getPhase() == PhaseSkill::EnExecution);
    REQUIRE_FALSE(skill.commencer());
    REQUIRE(skill.getPhase() == PhaseSkill::EnExecution);
}

TEST_CASE("Skill.avancer fait partir le coup une seule fois a la fin de l'execution") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f, .tempsExecution = 0.5f, .nature = NatureDAttaque::Contact});
    REQUIRE(skill.commencer());

    int departs = 0;
    for (int i = 0; i < nombreDeTics(0.4f); ++i) {
        departs += skill.avancer(s_dureeTic, generateur) ? 1 : 0;
    }
    REQUIRE(departs == 0);
    REQUIRE(skill.getPhase() == PhaseSkill::EnExecution);

    for (int i = 0; i < nombreDeTics(0.4f); ++i) {
        departs += skill.avancer(s_dureeTic, generateur) ? 1 : 0;
    }
    REQUIRE(departs == 1);
    REQUIRE(skill.getPhase() == PhaseSkill::EnRecharge);
}

TEST_CASE("Skill.avancer tire la recharge au depart du coup puis revient a pret") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f, .tempsExecution = 0.5f, .nature = NatureDAttaque::Contact});
    REQUIRE(skill.commencer());
    REQUIRE_FALSE(skill.avancer(0.4f, generateur));
    REQUIRE(skill.getRechargeRestante() == 0.0f);

    REQUIRE(skill.avancer(0.2f, generateur));
    REQUIRE(skill.getRechargeRestante() == 1.0f);

    REQUIRE_FALSE(skill.avancer(0.6f, generateur));
    REQUIRE(skill.getPhase() == PhaseSkill::EnRecharge);
    REQUIRE_FALSE(skill.commencer());

    REQUIRE_FALSE(skill.avancer(0.6f, generateur));
    REQUIRE(skill.getPhase() == PhaseSkill::Pret);
    REQUIRE(skill.commencer());
}

TEST_CASE("Skill.annuler revient a pret sans consommer de recharge") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f, .tempsExecution = 0.5f, .nature = NatureDAttaque::Contact});
    REQUIRE(skill.commencer());
    REQUIRE_FALSE(skill.avancer(0.2f, generateur));

    skill.annuler();

    REQUIRE(skill.getPhase() == PhaseSkill::Pret);
    REQUIRE(skill.getRechargeRestante() == 0.0f);
    REQUIRE(skill.commencer());
}

TEST_CASE("Skill.annuler ne touche pas une recharge en cours") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f, .nature = NatureDAttaque::Contact});
    REQUIRE(skill.commencer());
    REQUIRE(skill.avancer(0.0f, generateur));

    skill.annuler();

    REQUIRE(skill.getPhase() == PhaseSkill::EnRecharge);
    REQUIRE(skill.getRechargeRestante() == 1.0f);
}

TEST_CASE("Skill a execution nulle : le coup part des le premier avancer") {
    Generateur generateur = generateurDeTest();
    Skill skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f, .nature = NatureDAttaque::Contact});
    REQUIRE(skill.commencer());

    REQUIRE(skill.avancer(s_dureeTic, generateur));
    REQUIRE(skill.getPhase() == PhaseSkill::EnRecharge);
}

TEST_CASE("Skill.convientA : sans exigence, toute arme et les mains nues conviennent") {
    const Skill libre({.type = TypeSkill::DegatsSurCible, .portee = 20.0f, .puissance = 5, .rechargeMin = 1.0f, .rechargeMax = 1.0f,
                       .nature = NatureDAttaque::Contact});

    REQUIRE(libre.getArmesExigees().empty());
    REQUIRE(libre.convientA(std::nullopt));
    REQUIRE(libre.convientA(TypeDArme::Arc));
}

TEST_CASE("Skill.convientA : avec des exigences, seules les armes de la liste conviennent, jamais les mains nues") {
    const Skill exigeant({.type = TypeSkill::DegatsSurCible, .portee = 20.0f, .puissance = 5, .rechargeMin = 1.0f, .rechargeMax = 1.0f,
                          .nature = NatureDAttaque::Contact, .armesExigees = {TypeDArme::EpeeLongue, TypeDArme::Masse}});

    REQUIRE(exigeant.getArmesExigees().size() == 2);
    REQUIRE(exigeant.convientA(TypeDArme::EpeeLongue));
    REQUIRE(exigeant.convientA(TypeDArme::Masse));
    REQUIRE_FALSE(exigeant.convientA(TypeDArme::Dague));
    REQUIRE_FALSE(exigeant.convientA(TypeDArme::Arc));
    REQUIRE_FALSE(exigeant.convientA(std::nullopt));
}
