// ./tests/escouade/test_Escouade_absorption.cpp
// Un skill qui pose un bouclier d'absorption (étape 114.5) : l'Aegis du paladin, posé sur l'allié le plus blessé.
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <variant>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/EffetActif.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "combat/ValeursEffectives.hpp"
#include "escouade/Escouade.hpp"

// Escouade en (50, 50) : le guerrier (place 1) en (85, 60), le prêtre (place 2) en (35, 35), le mage (place 3) en (35, 85). Le prêtre de ces
// tests a un Aegis à la place de son soin (mode Allie, qui vise l'allié le plus blessé).

namespace {
constexpr AdresseSkill s_adresseAegis{Place{2}, IdSkill::Aegis};
constexpr int s_montant = 60;
constexpr int s_duree = 8;
constexpr float s_recharge = 15.0f;

DefinitionPersonnage pretreQuiProtege(int volonte = s_valeurDeReference) {
    DefinitionPersonnage pretre = pretreDeTest();
    pretre.skills.front() = SkillConnu{
        IdSkill::Aegis,
        Skill({.mode = ModeDeCible::Allie,
               .portee = s_porteeSoinDeTest,
               .rechargeMin = s_recharge,
               .rechargeMax = s_recharge,
               .effets = {EffetAbsorption{.famille = FamilleDAbsorption::Aegis,
                                          .montant = s_montant,
                                          .duree = s_duree,
                                          .coefficients = Coefficients::seul(Caracteristique::Volonte, 1.0f),
                                          .coefficientDeDuree = 1.0f}},
               .tempsExecution = s_activationSoinDeTest})};
    return avecCaracteristique(pretre, Caracteristique::Volonte, volonte);
}

// Une escouade dont le guerrier est blessé (c'est lui que l'Aegis vise).
Escouade uneEscouadeProtegee(int volonte = s_valeurDeReference) {
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), pretreQuiProtege(volonte), mageDeTest()});
    infliger(escouade, IdMonstre{1}, {85.0f, 60.0f}, 30);
    return escouade;
}
}  // namespace

TEST_CASE("aegis : l'Aegis parti est pose sur l'allie le plus blesse, pas sur les autres") {
    Escouade escouade = uneEscouadeProtegee();

    static_cast<void>(declencherTout(escouade, s_adresseAegis, {}));

    const EffetsActifs duGuerrier = *escouade.effetsDuPersonnage(Place{1});
    REQUIRE(duGuerrier.liste().size() == 1);
    REQUIRE(duGuerrier.liste()[0].source == IdSkill::Aegis);
    REQUIRE(duGuerrier.pointsDAbsorption().aegis == s_montant);  // Volonté de référence : aucun bonus
    REQUIRE(duGuerrier.liste()[0].dureeRestante == static_cast<float>(s_duree));
    REQUIRE(escouade.effetsDuPersonnage(Place{2})->liste().empty());
    REQUIRE(escouade.effetsDuPersonnage(Place{3})->liste().empty());
}

TEST_CASE("aegis : le montant suit la Volonte du lanceur, 1 % par point") {
    Escouade escouade = uneEscouadeProtegee(s_valeurDeReference + 50);  // +50 % : 90 points

    static_cast<void>(declencherTout(escouade, s_adresseAegis, {}));

    REQUIRE(escouade.effetsDuPersonnage(Place{1})->pointsDAbsorption().aegis == 90);
}

TEST_CASE("aegis : la duree profite de l'Intellect mais ne depasse jamais la recharge") {
    Escouade auPlafond = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), avecCaracteristique(pretreQuiProtege(), Caracteristique::Savoir, 100000), mageDeTest()});
    infliger(auPlafond, IdMonstre{1}, {85.0f, 60.0f}, 30);

    static_cast<void>(declencherTout(auPlafond, s_adresseAegis, {}));

    // 8 s avec le bonus plafonné à +100 % feraient 16 s : ramenées à la recharge, 15 s.
    REQUIRE(auPlafond.effetsDuPersonnage(Place{1})->liste()[0].dureeRestante == s_recharge);
}
