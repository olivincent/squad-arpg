// ./tests/escouade/test_Escouade_effets.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
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
    REQUIRE(effet.rang == 1);  // le rang de l'effet dans la liste du skill : le principal (les dégâts) est le rang 0
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

// La bénédiction : un buff pour toute l'escouade (mode `Escouade`), sans cible ni portée.

namespace {
constexpr AdresseSkill s_adresseBenediction{Place{2}, IdSkill::Benediction};

Escouade uneEscouadeBenie(int duree = s_dureeBonusDArmureDeTest, float recharge = s_rechargeBonusDArmureDeTest) {
    return uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), pretreQuiBenitToutLeMonde(duree, recharge), mageDeTest()});
}
}  // namespace

TEST_CASE("benediction : touche chaque membre vivant, a pleine vie, sans cible a choisir") {
    Escouade escouade = uneEscouadeBenie();
    REQUIRE(escouade.modeDuSkill(s_adresseBenediction) == ModeDeCible::Escouade);

    static_cast<void>(declencherTout(escouade, s_adresseBenediction, {}));

    for (const Place place : {Place{1}, Place{2}, Place{3}}) {
        const EffetsActifs effets = *escouade.effetsDuPersonnage(place);
        REQUIRE(effets.liste().size() == 1);
        REQUIRE(effets.liste()[0].source == IdSkill::Benediction);
        REQUIRE(effets.liste()[0].dureeRestante == static_cast<float>(s_dureeBonusDArmureDeTest));
    }
}

TEST_CASE("benediction : un membre mort n'est pas beni") {
    Escouade escouade = uneEscouadeBenie();
    infliger(escouade, IdMonstre{1}, {35.0f, 85.0f}, s_coupMortel);  // le mage (place 3)
    REQUIRE_FALSE(escouade.placeEstVivante(Place{3}));

    static_cast<void>(declencherTout(escouade, s_adresseBenediction, {}));

    REQUIRE(escouade.effetsDuPersonnage(Place{1})->liste().size() == 1);
    REQUIRE(escouade.effetsDuPersonnage(Place{2})->liste().size() == 1);
    REQUIRE(escouade.effetsDuPersonnage(Place{3})->liste().empty());
}

TEST_CASE("benediction : l'armure affichee monte de 20 %, et l'armure de l'equipement ne change pas") {
    Escouade escouade = uneEscouadeBenie();
    const int avant = *escouade.armureDuPersonnage(Place{1});
    const int deLEquipement = escouade.equipementActifDuPersonnage(Place{1}).armureTotale();
    REQUIRE(avant == deLEquipement);
    REQUIRE(avant > 0);

    static_cast<void>(declencherTout(escouade, s_adresseBenediction, {}));

    REQUIRE(*escouade.armureDuPersonnage(Place{1}) == static_cast<int>(std::lround(avant * 1.2)));
    REQUIRE(escouade.equipementActifDuPersonnage(Place{1}).armureTotale() == deLEquipement);
}

TEST_CASE("benediction : l'armure revient a sa valeur quand l'effet expire") {
    Escouade escouade = uneEscouadeBenie();
    const int avant = *escouade.armureDuPersonnage(Place{1});
    static_cast<void>(declencherTout(escouade, s_adresseBenediction, {}));
    REQUIRE(*escouade.armureDuPersonnage(Place{1}) > avant);

    simuler(escouade, carteOuverte(), static_cast<float>(s_dureeBonusDArmureDeTest) + 0.5f);

    REQUIRE(escouade.effetsDuPersonnage(Place{1})->liste().empty());
    REQUIRE(*escouade.armureDuPersonnage(Place{1}) == avant);
}

TEST_CASE("benediction : un buff ne dure jamais plus longtemps que sa recharge") {
    // 10 s de base, mais une recharge de 6 s : la durée est ramenée à 6 s.
    Escouade escouade = uneEscouadeBenie(10, 6.0f);

    static_cast<void>(declencherTout(escouade, s_adresseBenediction, {}));

    REQUIRE(escouade.effetsDuPersonnage(Place{1})->liste()[0].dureeRestante == 6.0f);
}

TEST_CASE("benediction : meme avec le bonus de duree au plafond, la duree ne depasse pas la recharge") {
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), avecCaracteristique(pretreQuiBenitToutLeMonde(), Caracteristique::Savoir, 100000), mageDeTest()});

    static_cast<void>(declencherTout(escouade, s_adresseBenediction, {}));

    REQUIRE(escouade.effetsDuPersonnage(Place{1})->liste()[0].dureeRestante == s_rechargeBonusDArmureDeTest);  // 10 × 2 = 20 = la recharge
}

// Les effets dans le temps (étape 112) : le montant par tick se fige à la pose.

TEST_CASE("effets dans le temps : le coup d'un skill qui fait saigner porte le saignement, de rang 1, avec ses piles") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierQuiSaigne(), pretreDeTest(), mageDeTest()});

    const std::vector<Coup> coups = declencherEtAttendre(escouade, s_adresseFrappe, {MonstreVu{IdMonstre{4}, cibleMonstre({85.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].attaque.effets.size() == 1);
    const EffetActif& effet = coups[0].attaque.effets[0];
    REQUIRE(effet.source == IdSkill::FrappePuissante);
    REQUIRE(effet.rang == 1);
    REQUIRE(effet.dureeRestante == static_cast<float>(s_dureeDeSaignementDeTest));  // coefficient de durée 0 : l'Intellect n'y change rien
    REQUIRE(effet.pilesMax == s_pilesDeSaignementDeTest);
    REQUIRE(effet.piles == 1);
    const DegatsDansLeTemps& saignement = std::get<DegatsDansLeTemps>(effet.sorte);
    REQUIRE(saignement.element == Element::Physique);
    REQUIRE(saignement.montant == s_montantDeSaignementDeTest);  // Puissance de référence : aucun bonus
}

TEST_CASE("effets dans le temps : le montant par tick est celui des caracteristiques du lanceur au moment de la pose") {
    Generateur generateur = generateurDeTest();
    // 100 points de Puissance au-dessus de la référence : 1 % chacun, le montant double.
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {avecCaracteristique(guerrierQuiSaigne(), Caracteristique::Puissance, s_valeurDeReference + 100), pretreDeTest(), mageDeTest()});

    const std::vector<Coup> coups = declencherEtAttendre(escouade, s_adresseFrappe, {MonstreVu{IdMonstre{4}, cibleMonstre({85.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(std::get<DegatsDansLeTemps>(coups[0].attaque.effets[0].sorte).montant == 2 * s_montantDeSaignementDeTest);
}

TEST_CASE("effets dans le temps : un soin dans le temps est pose sur l'allie vise, sans soin immediat") {
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), pretreQuiRegenere(), mageDeTest()});
    infliger(escouade, IdMonstre{1}, {85.0f, 60.0f}, 30);  // le guerrier est blessé : c'est lui que le prêtre vise
    const int vieAvant = *escouade.pointsDeVieDuPersonnage(Place{1});

    static_cast<void>(declencherTout(escouade, s_adresseSoin, {}));

    REQUIRE(*escouade.pointsDeVieDuPersonnage(Place{1}) == vieAvant);  // rien tout de suite : tout vient des ticks
    const EffetsActifs effets = *escouade.effetsDuPersonnage(Place{1});
    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste()[0].source == IdSkill::Soin);
    REQUIRE(effets.liste()[0].rang == 0);
    REQUIRE(effets.liste()[0].dureeRestante == static_cast<float>(s_dureeDeSoinDansLeTempsDeTest));
    REQUIRE(std::get<SoinDansLeTemps>(effets.liste()[0].sorte).montant == s_montantDeSoinDansLeTempsDeTest);
    REQUIRE(escouade.effetsDuPersonnage(Place{2})->liste().empty());
    REQUIRE(escouade.effetsDuPersonnage(Place{3})->liste().empty());
}

TEST_CASE("effets dans le temps : un soin dans le temps ne dure jamais plus longtemps que la recharge du skill") {
    DefinitionPersonnage pretre = pretreQuiRegenere();
    pretre.skills.front() = SkillConnu{IdSkill::Soin, Skill({.mode = ModeDeCible::Allie,
                                                             .portee = s_porteeSoinDeTest,
                                                             .rechargeMin = 5.0f,
                                                             .rechargeMax = 5.0f,
                                                             .effets = {EffetSoinDansLeTemps{.montant = 4, .duree = 20}},
                                                             .tempsExecution = s_activationSoinDeTest})};
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), pretre, mageDeTest()});
    infliger(escouade, IdMonstre{1}, {85.0f, 60.0f}, 30);

    static_cast<void>(declencherTout(escouade, s_adresseSoin, {}));

    REQUIRE(escouade.effetsDuPersonnage(Place{1})->liste()[0].dureeRestante == 5.0f);
}

TEST_CASE("effets dans le temps : les ticks d'un soin rendent des points de vie au personnage et sont rendus a la partie") {
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), pretreQuiRegenere(), mageDeTest()});
    infliger(escouade, IdMonstre{1}, {85.0f, 60.0f}, 60);
    static_cast<void>(declencherTout(escouade, s_adresseSoin, {}));
    const int vieAvant = *escouade.pointsDeVieDuPersonnage(Place{1});
    static_cast<void>(escouade.prendreLesTicks());

    for (int i = 0; i < 61; ++i) {  // une seconde et un tic : le premier tick a lieu à une seconde, sans dépendre des flottants
        escouade.mettreAJour(1.0f / 60.0f, carteOuverte(), {});
    }

    REQUIRE(*escouade.pointsDeVieDuPersonnage(Place{1}) == vieAvant + s_montantDeSoinDansLeTempsDeTest);
    const std::vector<TickSurUnAllie> ticks = escouade.prendreLesTicks();
    REQUIRE(ticks.size() == 1);
    REQUIRE(ticks[0].place == Place{1});
    REQUIRE(std::get<TickDeSoin>(ticks[0].tick).montant == s_montantDeSoinDansLeTempsDeTest);
    REQUIRE(escouade.prendreLesTicks().empty());
}
