// ./tests/escouade/test_Intention.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>

#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"
#include "escouade/Intention.hpp"

namespace {

// Valeurs de test, indépendantes de celles du jeu : aucune géométrie n'est calculée, les cercles et
// les points ne servent qu'à reconnaître ce que choisirIntention transmet.
constexpr float s_porteeDeTest = 20.0f;
constexpr float s_porteeDuSkillDeTest = 50.0f;
const sf::Vector2f s_placeDeFormation(100.0f, 100.0f);
const sf::Vector2f s_pointDuSkill(400.0f, 100.0f);
const EntiteVue s_monstreDeLaPlace{3, {{150.0f, 100.0f}, 15.0f}};
const EntiteVue s_degagement{4, {{160.0f, 120.0f}, 15.0f}};
const EntiteVue s_verrou{5, {{200.0f, 100.0f}, 15.0f}};
const EntiteVue s_agresseur{6, {{130.0f, 60.0f}, 0.0f}};  // tout juste enregistré : rayon nul

// Un personnage vivant, ancre à l'arrêt, sans rien d'autre : il va à sa place de formation.
SituationDuPersonnage auRepos() {
    return {
        .vivant = true,
        .enExecution = false,
        .monstreDeLaPlace = std::nullopt,
        .skillEnAttente = std::nullopt,
        .degagement = std::nullopt,
        .verrou = std::nullopt,
        .verrouAtteint = false,
        .ancreArretee = true,
        .agresseur = std::nullopt,
        .monstreAPortee = false,
        .placeDeFormation = s_placeDeFormation,
        .porteeAttaqueDeBase = s_porteeDeTest,
    };
}

// Toutes les règles 1 à 4 s'appliqueraient : seule la priorité départage.
SituationDuPersonnage avecToutesLesRegles() {
    SituationDuPersonnage situation = auRepos();
    situation.skillEnAttente = Intention{AllerAuPoint{s_pointDuSkill}};
    situation.degagement = s_degagement;
    situation.verrou = s_verrou;
    situation.agresseur = s_agresseur;
    return situation;
}

Intention approcher(EntiteVue monstre, float portee = s_porteeDeTest) {
    return ApprocherMonstre{monstre.id, monstre.cercle, portee};
}

}  // namespace

TEST_CASE("choisirIntention : sans autre regle, le personnage va a sa place de formation") {
    REQUIRE(choisirIntention(auRepos()) == Intention{AllerAuPoint{s_placeDeFormation}});
}

TEST_CASE("choisirIntention, regle 0 : en execution, il reste sur place et garde sa place, quelle qu'elle soit") {
    SituationDuPersonnage situation = auRepos();
    situation.enExecution = true;
    REQUIRE(choisirIntention(situation) == Intention{ResterSurPlace{std::nullopt}});

    situation.monstreDeLaPlace = s_monstreDeLaPlace.id;
    REQUIRE(choisirIntention(situation) == Intention{ResterSurPlace{s_monstreDeLaPlace.id}});
}

TEST_CASE("choisirIntention : la regle 0 passe avant toutes les autres") {
    SituationDuPersonnage situation = avecToutesLesRegles();
    situation.enExecution = true;
    situation.monstreDeLaPlace = s_monstreDeLaPlace.id;

    REQUIRE(choisirIntention(situation) == Intention{ResterSurPlace{s_monstreDeLaPlace.id}});
}

TEST_CASE("choisirIntention, regle 1 : le lanceur suit l'intention de son skill en attente") {
    SituationDuPersonnage situation = auRepos();
    situation.skillEnAttente = Intention{AllerAuPoint{s_pointDuSkill}};
    REQUIRE(choisirIntention(situation) == Intention{AllerAuPoint{s_pointDuSkill}});

    situation.skillEnAttente = approcher(s_verrou, s_porteeDuSkillDeTest);
    REQUIRE(choisirIntention(situation) == approcher(s_verrou, s_porteeDuSkillDeTest));
}

TEST_CASE("choisirIntention : la regle 1 passe avant le degagement, le verrou et l'agresseur") {
    REQUIRE(choisirIntention(avecToutesLesRegles()) == Intention{AllerAuPoint{s_pointDuSkill}});
}

TEST_CASE("choisirIntention, regle 2 : il approche le monstre dont il se degage, avec la portee de l'attaque de base") {
    SituationDuPersonnage situation = auRepos();
    situation.degagement = s_degagement;

    REQUIRE(choisirIntention(situation) == approcher(s_degagement));
}

TEST_CASE("choisirIntention : la regle 2 passe avant le verrou et l'agresseur") {
    SituationDuPersonnage situation = avecToutesLesRegles();
    situation.skillEnAttente.reset();

    REQUIRE(choisirIntention(situation) == approcher(s_degagement));
}

TEST_CASE("choisirIntention, regle 3 : un verrou atteint le fait rester sur place, autour du verrou") {
    SituationDuPersonnage situation = auRepos();
    situation.verrou = s_verrou;
    situation.verrouAtteint = true;

    REQUIRE(choisirIntention(situation) == Intention{ResterSurPlace{s_verrou.id}});
}

TEST_CASE("choisirIntention, regle 3 : un verrou pas encore atteint, il l'approche") {
    SituationDuPersonnage situation = auRepos();
    situation.verrou = s_verrou;

    REQUIRE(choisirIntention(situation) == approcher(s_verrou));
}

TEST_CASE("choisirIntention : la regle 3 passe avant l'agresseur") {
    SituationDuPersonnage situation = avecToutesLesRegles();
    situation.skillEnAttente.reset();
    situation.degagement.reset();
    situation.monstreAPortee = true;

    REQUIRE(choisirIntention(situation) == approcher(s_verrou));
}

TEST_CASE("choisirIntention, regle 3 : un mort ignore le verrou et l'agresseur, il va a sa place de formation") {
    SituationDuPersonnage situation = auRepos();
    situation.vivant = false;
    situation.verrou = s_verrou;
    situation.verrouAtteint = true;
    situation.agresseur = s_agresseur;

    REQUIRE(choisirIntention(situation) == Intention{AllerAuPoint{s_placeDeFormation}});
}

TEST_CASE("choisirIntention, regle 4 : sans monstre a portee, il approche l'agresseur avec son cercle tel quel") {
    SituationDuPersonnage situation = auRepos();
    situation.agresseur = s_agresseur;

    REQUIRE(choisirIntention(situation) == approcher(s_agresseur));
}

TEST_CASE("choisirIntention, regle 4 : avec un monstre a portee, il reste sur place, autour de l'agresseur") {
    SituationDuPersonnage situation = auRepos();
    situation.agresseur = s_agresseur;
    situation.monstreAPortee = true;

    REQUIRE(choisirIntention(situation) == Intention{ResterSurPlace{s_agresseur.id}});
}

TEST_CASE("choisirIntention, regle 4 : l'ancre en marche, l'agresseur est ignore") {
    SituationDuPersonnage situation = auRepos();
    situation.agresseur = s_agresseur;
    situation.ancreArretee = false;

    REQUIRE(choisirIntention(situation) == Intention{AllerAuPoint{s_placeDeFormation}});
}

TEST_CASE("choisirIntention : un monstre a portee sans agresseur ne retient personne sur place") {
    SituationDuPersonnage situation = auRepos();
    situation.monstreAPortee = true;

    REQUIRE(choisirIntention(situation) == Intention{AllerAuPoint{s_placeDeFormation}});
}

TEST_CASE("monstreDeLIntention : le monstre approche, celui de la place gardee, rien pour un point") {
    REQUIRE(monstreDeLIntention(approcher(s_verrou)) == s_verrou.id);
    REQUIRE(monstreDeLIntention(Intention{ResterSurPlace{s_agresseur.id}}) == s_agresseur.id);
    REQUIRE_FALSE(monstreDeLIntention(Intention{ResterSurPlace{std::nullopt}}).has_value());
    REQUIRE_FALSE(monstreDeLIntention(Intention{AllerAuPoint{s_pointDuSkill}}).has_value());
}
