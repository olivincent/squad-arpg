// ./tests/escouade/test_Escouade_skills_degats.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/TileMap.hpp"

// Pour les tests du skill : centre initial (60, 60), guerrier (place 1) en
// (85, 60), prêtre (place 2) en (35, 35), mage (place 3) en (35, 85). Une cible
// de la taille d'un monstre en (85, 60) est à 0 px bord à bord du guerrier,
// donc à portée de son skill (20 px, comme la mêlée).

// Sans verrou, le skill de dégâts vise le monstre que son lanceur frapperait
// en autodéfense, avec la portée du skill (20 px). Le lanceur de la place 1
// est en (85, 60), rayon 10 : un monstre (rayon 15) à 45 px ou moins est à portée.

TEST_CASE("Sans verrou, le skill de degats part sur un monstre a portee, sans creer de verrou") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::vector<Coup> coups = declencherEtAttendre(escouade, s_adresseFrappe, {EntiteVue{4, cibleMonstre({85.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 4);
    REQUIRE(coups[0].degats == s_degatsFrappeDeTest);
    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("Sans verrou, le skill de degats vise le plus proche parmi plusieurs monstres a portee") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    // Le monstre 1 (premier de la liste) est à 15 px bord à bord, le 2 à 0.
    const std::vector<Coup> coups = declencherEtAttendre(escouade,
        s_adresseFrappe, {EntiteVue{1, cibleMonstre({125.0f, 60.0f})}, EntiteVue{2, cibleMonstre({110.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 2);
}

TEST_CASE("Sans verrou, le skill de degats est refuse sans monstre a portee, et sans recharge") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({300.0f, 60.0f}))) == 0);
    REQUIRE(declencher(escouade, s_adresseFrappe, {}) == 0);
    // Pas de mise en attente ni de recharge : un monstre à portée est frappé aussitôt.
    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == s_degatsFrappeDeTest);
}

TEST_CASE("Avec un verrou, le skill de degats vise le monstre verrouille, pas le plus proche") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const std::vector<Coup> coups = declencherEtAttendre(escouade,
        s_adresseFrappe, {EntiteVue{1, cibleMonstre({125.0f, 60.0f})}, EntiteVue{2, cibleMonstre({110.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Avec un verrou absent de la liste des monstres, le skill de degats est refuse") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(9);

    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie les degats du seul lanceur du skill") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Le guerrier est à portée ; les tireurs n'ont pas de skill de dégâts : sa frappe, pas plus.
    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == s_degatsFrappeDeTest);
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une cible hors de portee du skill") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({300.0f, 60.0f}))) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie 0 au second appel immediat (recharge du skill)") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cible)) == s_degatsFrappeDeTest);
    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cible)) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une place sans skill ou hors formation") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(declencher(escouade, AdresseSkill{2, IdSkill::FrappePuissante}, unSeulMonstre(cible)) == 0);  // le prêtre n'a pas de frappe
    REQUIRE(declencher(escouade, AdresseSkill{0, IdSkill::FrappePuissante}, unSeulMonstre(cible)) == 0);
    REQUIRE(declencher(escouade, AdresseSkill{5, IdSkill::FrappePuissante}, unSeulMonstre(cible)) == 0);
}

TEST_CASE("Le skill de degats est refuse sans monstre verrouille meme si une cible est donnee") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, s_adresseFrappe, {}) == 0);
}

// Skill de dégâts en attente. Les attaques de base des 3 personnages réunies
// restent sous la frappe en un tic : un tic qui rapporte au moins la frappe
// contient donc forcément le skill.
static_assert(3 * s_degatsBaseDeTest < s_degatsFrappeDeTest);

TEST_CASE("Un skill de degats hors de portee est mis en attente, puis part a l'arrivee du lanceur") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const Cercle cible = cibleMonstre({300.0f, 60.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cible);

    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cible)) == 0);

    int degatsMax = 0;
    for (int i = 0; i < 600; ++i) {
        const int degats = attaquerCible(escouade, carte, cible);
        degatsMax = std::max(degatsMax, degats);
        escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = unSeulMonstre(cible)});
    }
    REQUIRE(degatsMax >= s_degatsFrappeDeTest);
}

TEST_CASE("Un skill de degats en attente est annule par deverrouiller, sans recharge") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    escouade.deverrouiller();
    escouade.verrouiller(1);

    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == s_degatsFrappeDeTest);
}

TEST_CASE("Un skill de degats en attente est annule par un changement de cible, pas par la meme cible") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    escouade.verrouiller(1);   // même cible : toujours en attente
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f})), carteOuverte(), generateur).empty());

    escouade.verrouiller(2);   // autre cible : annulé
    REQUIRE(declencher(escouade, s_adresseFrappe, {EntiteVue{2, cibleMonstre({85.0f, 60.0f})}}) == s_degatsFrappeDeTest);
}

TEST_CASE("Un soin est refuse pendant qu'un skill de degats est en attente") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    declencher(escouade, s_adresseSoin, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}
