// ./tests/combat/test_Skill.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <vector>

#include "aides.hpp"
#include "combat/TypeDArme.hpp"
#include "base/Cercle.hpp"
#include "combat/Skill.hpp"
#include "combat/ModeDeCible.hpp"
#include "combat/EffetDeSkill.hpp"

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

TEST_CASE("Skill.getEffet renvoie l'effet donne au constructeur, une alternative par sorte d'effet") {
    Skill degats({.mode = ModeDeCible::Monstre,
                  .portee = 80.0f,
                  .rechargeMin = 5.0f,
                  .rechargeMax = 5.0f,
                  .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});
    Skill soin({.mode = ModeDeCible::Allie,
                .portee = 80.0f,
                .rechargeMin = 5.0f,
                .rechargeMax = 5.0f,
                .effet = EffetSoin{.montant = 25}});

    REQUIRE(std::holds_alternative<EffetDegats>(degats.getEffet()));
    REQUIRE(std::get<EffetDegats>(degats.getEffet()).montant == 25);
    REQUIRE(std::holds_alternative<EffetSoin>(soin.getEffet()));
    REQUIRE(std::get<EffetSoin>(soin.getEffet()).montant == 25);
}

TEST_CASE("Skill.getMode renvoie le mode de cible donne au constructeur, quel que soit ce que fait le skill") {
    // Le mode (comment on vise) ne se déduit pas du type (ce que le skill fait) : un skill de dégâts peut viser un monstre ou un point.
    Skill surUnMonstre({.mode = ModeDeCible::Monstre,
                        .portee = 80.0f,
                        .rechargeMin = 5.0f,
                        .rechargeMax = 5.0f,
                        .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});
    Skill surUnAllie({.mode = ModeDeCible::Allie,
                      .portee = 80.0f,
                      .rechargeMin = 5.0f,
                      .rechargeMax = 5.0f,
                      .effet = EffetSoin{.montant = 25}});
    Skill surSoi({.mode = ModeDeCible::Soi,
                  .portee = 0.0f,
                  .rechargeMin = 10.0f,
                  .rechargeMax = 10.0f,
                  .effet = EffetProvocation{.duree = 4},
                  .rayonZone = 150.0f});
    Skill surUnPoint({.mode = ModeDeCible::Point,
                      .portee = 250.0f,
                      .rechargeMin = 8.0f,
                      .rechargeMax = 8.0f,
                      .effet = EffetDegats{.montant = 15, .nature = NatureDAttaque::Zone},
                      .rayonZone = 60.0f});

    REQUIRE(surUnMonstre.getMode() == ModeDeCible::Monstre);
    REQUIRE(surUnAllie.getMode() == ModeDeCible::Allie);
    REQUIRE(surSoi.getMode() == ModeDeCible::Soi);
    REQUIRE(surUnPoint.getMode() == ModeDeCible::Point);
}

TEST_CASE("Skill.getRayonZone renvoie le rayon donne au constructeur, distinct de la portee") {
    Skill zone({.mode = ModeDeCible::Monstre,
                .portee = 250.0f,
                .rechargeMin = 8.0f,
                .rechargeMax = 8.0f,
                .effet = EffetDegats{.montant = 15, .nature = NatureDAttaque::Contact},
                .rayonZone = 60.0f});
    Skill monoCible({.mode = ModeDeCible::Allie,
                     .portee = 40.0f,
                     .rechargeMin = 8.0f,
                     .rechargeMax = 8.0f,
                     .effet = EffetSoin{.montant = 30}});

    REQUIRE(zone.getRayonZone() == std::optional<float>(60.0f));
    REQUIRE(zone.estAPortee(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})));
    REQUIRE_FALSE(monoCible.getRayonZone().has_value());  // sans rayon, l'effet touche la cible seule
}

TEST_CASE("Skill de provocation : la puissance est la duree, avec zone et recharge") {
    Generateur generateur = generateurDeTest();
    Skill provocation({.mode = ModeDeCible::Soi,
                       .portee = 0.0f,
                       .rechargeMin = 10.0f,
                       .rechargeMax = 10.0f,
                       .effet = EffetProvocation{.duree = 4},
                       .rayonZone = 150.0f});
    const Cercle origine = pointCible({100.0f, 100.0f});

    REQUIRE(std::holds_alternative<EffetProvocation>(provocation.getEffet()));
    REQUIRE(provocation.getRayonZone() == std::optional<float>(150.0f));
    REQUIRE(provocation.estAPortee(origine, origine));
    REQUIRE_FALSE(provocation.estAPortee(origine, pointCible({200.0f, 100.0f})));
    REQUIRE(std::get<EffetProvocation>(provocation.getEffet()).duree == 4);
    REQUIRE(lancer(provocation, generateur));
    REQUIRE(provocation.getProgressionRecharge() == 0.0f);
}

TEST_CASE("Skill refuse de partir pendant la recharge, puis repart une fois ecoulee") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Allie,
                 .portee = 80.0f,
                 .rechargeMin = 5.0f,
                 .rechargeMax = 5.0f,
                 .effet = EffetSoin{.montant = 25}});

    REQUIRE(lancer(skill, generateur));
    REQUIRE_FALSE(lancer(skill, generateur));

    ecoulerTemps(skill, 4.8f);
    REQUIRE_FALSE(lancer(skill, generateur));

    ecoulerTemps(skill, 0.4f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 pret, 0 juste apres le declenchement") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 5.0f,
                 .rechargeMax = 5.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});

    REQUIRE(skill.getProgressionRecharge() == 1.0f);

    REQUIRE(lancer(skill, generateur));
    REQUIRE(skill.getProgressionRecharge() == 0.0f);

    ecoulerTemps(skill, 6.0f);
    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("Skill.estAPortee renvoie vrai pour un point proche") {
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    REQUIRE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({150.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie faux pour un point lointain") {
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    REQUIRE_FALSE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({300.0f, 100.0f})));
}

TEST_CASE("Skill.estAPortee renvoie vrai pour une distance exactement egale a la portee") {
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    REQUIRE(skill.estAPortee(pointCible({100.0f, 100.0f}), pointCible({180.0f, 100.0f})));
}

TEST_CASE("Skill refuse de partir apres une duree simulee trop courte") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    REQUIRE(lancer(skill, generateur));
    ecoulerTemps(skill, 0.3f);
    REQUIRE_FALSE(lancer(skill, generateur));
}

TEST_CASE("Skill repart apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    REQUIRE(lancer(skill, generateur));
    ecoulerTemps(skill, 0.6f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill.relancerRecharge empeche de frapper avant une recharge complete") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    skill.relancerRecharge(generateur);
    REQUIRE_FALSE(lancer(skill, generateur));
    ecoulerTemps(skill, 0.6f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill.getProgressionRecharge vaut 1 prete, 0 juste apres un coup, entre les deux ensuite") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

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
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.5f,
                 .rechargeMax = 0.5f,
                 .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    skill.relancerRecharge(generateur);
    skill.mettreAJour(10.0f);

    REQUIRE(skill.getProgressionRecharge() == 1.0f);
}

TEST_CASE("La nature d'un skill vient de sa definition, Contact par defaut") {
    const Skill parDefaut({.mode = ModeDeCible::Monstre,
                           .portee = 20.0f,
                           .rechargeMin = 1.0f,
                           .rechargeMax = 1.0f,
                           .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});
    const Skill tir({.mode = ModeDeCible::Monstre,
                     .portee = 120.0f,
                     .rechargeMin = 1.0f,
                     .rechargeMax = 1.0f,
                     .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Projectile}});
    const Skill zone({.mode = ModeDeCible::Point,
                      .portee = 250.0f,
                      .rechargeMin = 1.0f,
                      .rechargeMax = 1.0f,
                      .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Zone},
                      .rayonZone = 60.0f});

    REQUIRE(std::get<EffetDegats>(parDefaut.getEffet()).nature == NatureDAttaque::Contact);
    REQUIRE(std::get<EffetDegats>(tir.getEffet()).nature == NatureDAttaque::Projectile);
    REQUIRE(std::get<EffetDegats>(zone.getEffet()).nature == NatureDAttaque::Zone);
}

TEST_CASE("Skill de zone : le depart relance la recharge, refuse tant qu'elle court") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Point,
                 .portee = 250.0f,
                 .rechargeMin = 8.0f,
                 .rechargeMax = 8.0f,
                 .effet = EffetDegats{.montant = 15, .nature = NatureDAttaque::Zone},
                 .rayonZone = 60.0f});

    REQUIRE(lancer(skill, generateur));
    REQUIRE(std::get<EffetDegats>(skill.getEffet()).montant == 15);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);
    REQUIRE_FALSE(lancer(skill, generateur));

    ecoulerTemps(skill, 8.2f);
    REQUIRE(lancer(skill, generateur));
}

TEST_CASE("Skill a recharge fixe (min == max) donne exactement cette valeur, sans tirer de nombre") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 5.0f,
                 .rechargeMax = 5.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});

    REQUIRE(lancer(skill, generateur));

    REQUIRE(skill.getRechargeRestante() == 5.0f);
    REQUIRE(generateur == intact);  // aucun nombre tiré : l'état du générateur n'a pas bougé
}

TEST_CASE("Skill a recharge en intervalle : chaque recharge est dans l'intervalle et elles ne sont pas toutes egales") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.3f,
                 .rechargeMax = 0.7f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});
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
    Skill skillA({.mode = ModeDeCible::Monstre,
                  .portee = 80.0f,
                  .rechargeMin = 0.3f,
                  .rechargeMax = 0.7f,
                  .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});
    Skill skillB({.mode = ModeDeCible::Monstre,
                  .portee = 80.0f,
                  .rechargeMin = 0.3f,
                  .rechargeMax = 0.7f,
                  .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});

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
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 0.3f,
                 .rechargeMax = 0.7f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});

    skill.relancerRecharge(generateur);

    REQUIRE(skill.getRechargeRestante() >= 0.3f);
    REQUIRE(skill.getRechargeRestante() <= 0.7f);
    REQUIRE(skill.getProgressionRecharge() == 0.0f);
}

// Machine à états : prêt, en exécution, en recharge.

TEST_CASE("Skill.commencer passe en execution depuis pret seulement") {
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact},
                 .tempsExecution = 0.5f});

    REQUIRE(skill.getPhase() == PhaseSkill::Pret);
    REQUIRE(skill.commencer());
    REQUIRE(skill.getPhase() == PhaseSkill::EnExecution);
    REQUIRE_FALSE(skill.commencer());
    REQUIRE(skill.getPhase() == PhaseSkill::EnExecution);
}

TEST_CASE("Skill.avancer fait partir le coup une seule fois a la fin de l'execution") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact},
                 .tempsExecution = 0.5f});
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
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact},
                 .tempsExecution = 0.5f});
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
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact},
                 .tempsExecution = 0.5f});
    REQUIRE(skill.commencer());
    REQUIRE_FALSE(skill.avancer(0.2f, generateur));

    skill.annuler();

    REQUIRE(skill.getPhase() == PhaseSkill::Pret);
    REQUIRE(skill.getRechargeRestante() == 0.0f);
    REQUIRE(skill.commencer());
}

TEST_CASE("Skill.annuler ne touche pas une recharge en cours") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});
    REQUIRE(skill.commencer());
    REQUIRE(skill.avancer(0.0f, generateur));

    skill.annuler();

    REQUIRE(skill.getPhase() == PhaseSkill::EnRecharge);
    REQUIRE(skill.getRechargeRestante() == 1.0f);
}

TEST_CASE("Skill a execution nulle : le coup part des le premier avancer") {
    Generateur generateur = generateurDeTest();
    Skill skill({.mode = ModeDeCible::Monstre,
                 .portee = 80.0f,
                 .rechargeMin = 1.0f,
                 .rechargeMax = 1.0f,
                 .effet = EffetDegats{.montant = 25, .nature = NatureDAttaque::Contact}});
    REQUIRE(skill.commencer());

    REQUIRE(skill.avancer(s_dureeTic, generateur));
    REQUIRE(skill.getPhase() == PhaseSkill::EnRecharge);
}

TEST_CASE("Skill.convientA : sans exigence, toute arme et les mains nues conviennent") {
    const Skill libre({.mode = ModeDeCible::Monstre,
                       .portee = 20.0f,
                       .rechargeMin = 1.0f,
                       .rechargeMax = 1.0f,
                       .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});

    REQUIRE(libre.getArmesExigees().empty());
    REQUIRE(libre.convientA(std::nullopt));
    REQUIRE(libre.convientA(TypeDArme::Arc));
}

TEST_CASE("Skill.convientA : avec des exigences, seules les armes de la liste conviennent, jamais les mains nues") {
    const Skill exigeant({.mode = ModeDeCible::Monstre,
                          .portee = 20.0f,
                          .rechargeMin = 1.0f,
                          .rechargeMax = 1.0f,
                          .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact},
                          .armesExigees = {TypeDArme::EpeeAUneMain, TypeDArme::MasseAUneMain}});

    REQUIRE(exigeant.getArmesExigees().size() == 2);
    REQUIRE(exigeant.convientA(TypeDArme::EpeeAUneMain));
    REQUIRE(exigeant.convientA(TypeDArme::MasseAUneMain));
    REQUIRE_FALSE(exigeant.convientA(TypeDArme::Dague));
    REQUIRE_FALSE(exigeant.convientA(TypeDArme::Arc));
    REQUIRE_FALSE(exigeant.convientA(std::nullopt));
}

TEST_CASE("un skill ordinaire n'a pas de bonus de frappe, un skill de frappe a le sien") {
    const Skill ordinaire({.mode = ModeDeCible::Monstre,
                           .portee = 20.0f,
                           .rechargeMin = 1.0f,
                           .rechargeMax = 1.0f,
                           .effet = EffetDegats{.montant = 5, .nature = NatureDAttaque::Contact}});
    const Skill frappe({.mode = ModeDeCible::Monstre,
                        .portee = 20.0f,
                        .rechargeMin = 1.0f,
                        .rechargeMax = 1.0f,
                        .effet = EffetFrappe{.bonus = degatsDe(Element::Sacre, 45)}});

    REQUIRE_FALSE(std::holds_alternative<EffetFrappe>(ordinaire.getEffet()));
    REQUIRE(std::holds_alternative<EffetFrappe>(frappe.getEffet()));
    REQUIRE(std::get<EffetFrappe>(frappe.getEffet()).bonus[Element::Sacre] == 45);
    REQUIRE(std::get<EffetDegats>(ordinaire.getEffet()).element == Element::Physique);
}
