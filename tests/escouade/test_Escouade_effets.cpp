// ./tests/escouade/test_Escouade_effets.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/EffetActif.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/ValeursEffectives.hpp"
#include "escouade/Escouade.hpp"

// Escouade en (50, 50), orientée vers +x : le guerrier (place 1) en (85, 60), le prêtre (place 2) en (35, 35), le mage (place 3) en
// (35, 85). Le prêtre de ces tests a un buff d'armure à la place de son soin (mode Allie, qui vise l'allié le plus blessé).

namespace {
const ModificateurDeStatistique s_malusAttendu{.statistique = StatistiqueModifiable::Armure, .pourcentage = s_malusDArmureDeTest};
const ModificateurDeStatistique s_bonusAttendu{.statistique = StatistiqueModifiable::Armure, .pourcentage = s_bonusDArmureDeTest};

// Une escouade dont le guerrier est blessé (c'est lui que le buff du prêtre vise) et dont le prêtre a ce Savoir.
Escouade uneEscouadeAuBuff(int savoir = s_valeurDeReference) {
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f},
                                      {guerrierDeTest(), avecCaracteristique(pretreQuiBenit(), Caracteristique::Savoir, savoir), mageDeTest()});
    infliger(escouade, IdMonstre{1}, {85.0f, 60.0f}, 30);
    return escouade;
}

// Les effets du guerrier une fois le buff parti.
EffetsActifs effetsDuGuerrierApresLeBuff(Escouade& escouade) {
    static_cast<void>(declencherTout(escouade, s_adresseSoin, {}));
    return *escouade.effetsDuPersonnage(Place{1});
}
}  // namespace

TEST_CASE("effets : le coup d'un skill qui affaiblit porte l'effet actif de ce skill") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierQuiAffaiblit(), pretreDeTest(), mageDeTest()});

    const std::vector<Coup> coups = declencherEtAttendre(escouade, s_adresseFrappe, {MonstreVu{IdMonstre{4}, cibleMonstre({85.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].attaque.effets.size() == 1);
    const EffetActif& effet = coups[0].attaque.effets[0];
    REQUIRE(effet.source == IdSkill::FrappePuissante);
    REQUIRE(effet.dureeRestante == static_cast<float>(s_dureeMalusDArmureDeTest));  // coefficient de durée 0 : l'Intellect n'y change rien
    REQUIRE(std::get<ModificateurDeStatistique>(effet.sorte) == s_malusAttendu);
}

TEST_CASE("effets : le coup d'un skill sans effet actif n'en porte aucun, l'attaque de base non plus") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::vector<Coup> coups = declencherEtAttendre(escouade, s_adresseFrappe, {MonstreVu{IdMonstre{4}, cibleMonstre({85.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].attaque.effets.empty());
    REQUIRE(attaqueDeBaseDe(escouade, Place{1})->effets.empty());
}

TEST_CASE("effets : un buff parti est pose sur l'allie vise, pas sur les autres") {
    Escouade escouade = uneEscouadeAuBuff();

    const EffetsActifs duGuerrier = effetsDuGuerrierApresLeBuff(escouade);

    REQUIRE(duGuerrier.liste().size() == 1);
    REQUIRE(duGuerrier.liste()[0].source == IdSkill::Soin);
    REQUIRE(duGuerrier.liste()[0].dureeRestante == static_cast<float>(s_dureeBonusDArmureDeTest));
    REQUIRE(std::get<ModificateurDeStatistique>(duGuerrier.liste()[0].sorte) == s_bonusAttendu);
    REQUIRE(escouade.effetsDuPersonnage(Place{2})->liste().empty());
    REQUIRE(escouade.effetsDuPersonnage(Place{3})->liste().empty());
}

TEST_CASE("effets : la duree d'un buff profite du bonus de duree de l'Intellect du lanceur") {
    // Le prêtre de ces tests tient son bonus de durée du Savoir : 110 points, soit 100 au-dessus de la référence, 0,3 × 100 = +30 %.
    Escouade escouade = uneEscouadeAuBuff(s_valeurDeReference + 100);

    const EffetsActifs duGuerrier = effetsDuGuerrierApresLeBuff(escouade);

    REQUIRE(duGuerrier.liste().size() == 1);
    REQUIRE_THAT(duGuerrier.liste()[0].dureeRestante, Catch::Matchers::WithinAbs(13.0, 0.001));
}

TEST_CASE("effets : le bonus de duree est plafonne a +100 %") {
    Escouade escouade = uneEscouadeAuBuff(100000);

    const EffetsActifs duGuerrier = effetsDuGuerrierApresLeBuff(escouade);

    REQUIRE(duGuerrier.liste().size() == 1);
    REQUIRE(duGuerrier.liste()[0].dureeRestante == 2.0f * static_cast<float>(s_dureeBonusDArmureDeTest));
}

TEST_CASE("effets : une place invalide n'a pas d'effets") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.effetsDuPersonnage(Place{9}).has_value());
}
