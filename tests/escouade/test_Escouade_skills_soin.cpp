// ./tests/escouade/test_Escouade_skills_soin.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/TileMap.hpp"
#include "combat/TypeSkill.hpp"

// Escouade en (50, 50), orientée vers +x : les trois places sont en
// (85, 60), (35, 35) et (35, 85). Des dégâts d'origine égale à
// l'une de ces positions touchent la place correspondante.

TEST_CASE("choisirCible (soin) est vide quand tout le monde est a pleine vie") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

TEST_CASE("choisirCible (soin) designe le seul blesse") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 85.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("choisirCible (soin) designe celui qui a le moins de points de vie") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 30);
    infliger(escouade, 1, {35.0f, 85.0f}, 50);

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("choisirCible (soin) choisit la plus petite place en cas d'egalite") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 35.0f}, 20);  // prêtre (place 2), sans armure : perd 20
    infliger(escouade, 1, {85.0f, 60.0f}, 30);  // guerrier (place 1), armure 50 : 30 x 2/3 = 20

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("choisirCible (soin) ignore les morts") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 85.0f}, 200);
    infliger(escouade, 1, {85.0f, 60.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("choisirCible (soin) est vide quand tout le monde est mort") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    for (const sf::Vector2f origine : {sf::Vector2f(85.0f, 60.0f), sf::Vector2f(35.0f, 35.0f), sf::Vector2f(35.0f, 85.0f)}) {
        infliger(escouade, 1, origine, 200);
    }

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

// Soin (prêtre, place 2, en (35, 35), portée 40 px bord à bord = 60 px entre
// centres, 30 points de vie, recharge 8 s). Place 3 (mage) en (35, 85) : 50 px,
// à portée. Place 1 (guerrier) en (85, 60) : environ 55,9 px, à portée aussi.
// Dans le triangle, aucun allié n'est hors de portée : les tests d'un soin hors de portée
// utilisent un prêtre au soin plus court (voir uneEscouadeAuSoinCourt).

TEST_CASE("Le soin du pretre soigne un allie a portee, sans combat engage") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 85.0f}, 20);

    REQUIRE(declencher(escouade, s_adresseSoin, {}) == 0);

    // 80 + 30 plafonné à 100 : plus personne à soigner.
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

TEST_CASE("prendreSoins rend le soin applique (place, points reellement rendus, position) puis se vide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 85.0f}, 20);  // le mage (place 3) perd 20 points

    declencher(escouade, s_adresseSoin, {});
    const std::vector<Escouade::SoinApplique> soins = escouade.prendreSoins();

    REQUIRE(soins.size() == 1);
    REQUIRE(soins[0].place == 3);
    REQUIRE(soins[0].montant == 20);  // 30 de soin, mais il ne manquait que 20 points
    verifierPointProche(soins[0].position, *escouade.centreDuPersonnage(3));
    REQUIRE(escouade.prendreSoins().empty());
}

TEST_CASE("centreDuPersonnage : le centre de la place, vide si la place est invalide") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    verifierPointProche(*escouade.centreDuPersonnage(3), {35.0f, 85.0f});
    REQUIRE_FALSE(escouade.centreDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.centreDuPersonnage(4).has_value());
}

TEST_CASE("Le soin ne part pas si la cible est hors de portee") {
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 20);

    declencher(escouade, s_adresseSoin, {});

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("Le soin ne part pas a pleine vie et ne declenche alors pas la recharge") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    declencher(escouade, s_adresseSoin, {});

    infliger(escouade, 1, {35.0f, 85.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

TEST_CASE("Le soin est en recharge juste apres son lancement") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 85.0f}, 60);

    declencher(escouade, s_adresseSoin, {});   // 40 -> 70
    declencher(escouade, s_adresseSoin, {});   // refusé : recharge

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("Un soigneur mort ne soigne pas") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {35.0f, 35.0f}, 200);
    infliger(escouade, 1, {35.0f, 85.0f}, 20);

    declencher(escouade, s_adresseSoin, {});

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

// Skill de soin en attente. Carte dégagée dédiée ; escouade immobile en
// (50, 50), prêtre au soin court. Le blessé est la place 1 (guerrier, en (85, 60)),
// à environ 55,9 px du soigneur (35, 35) : hors de la portée de ce soin
// (33 px bord à bord, soit 53 px entre centres). Le mage (35, 85), à 50 px, est à portée.

TEST_CASE("Un soin hors de portee est mis en attente : le soigneur marche, soigne, puis revient a sa place") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 20);
    static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur)); // le monstre fantôme n'existe pas : il n'est pas un agresseur

    declencher(escouade, s_adresseSoin, {});
    REQUIRE(placeAlliePourSoin(escouade) == 1);   // pas encore soigné

    simulerAvecAttaque(escouade, carte, 1.5f, generateur);
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());  // soigné en route (marche, puis 0,6 s d'activation)

    simuler(escouade, carte, 2.0f);
    const std::optional<EntiteVue> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    REQUIRE(soigneur.has_value());
    verifierPointProche(soigneur->cercle.centre, {35.0f, 35.0f});
}

TEST_CASE("Le soin en attente est annule si la cible meurt, sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    infliger(escouade, 1, {85.0f, 60.0f}, 200);
    static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur)); // le monstre fantôme n'existe pas : il n'est pas un agresseur
    simuler(escouade, carte, 2.0f);

    // Le soigneur est revenu à sa place et son skill est prêt : un nouveau
    // blessé à portée est soigné aussitôt.
    const std::optional<EntiteVue> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    verifierPointProche(soigneur->cercle.centre, {35.0f, 35.0f});
    infliger(escouade, 1, {35.0f, 85.0f}, 20);
    declencher(escouade, s_adresseSoin, {});
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Le soin en attente est annule si le soigneur meurt") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    infliger(escouade, 1, {35.0f, 35.0f}, 200);
    simuler(escouade, carte, 2.0f);

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("Un ordre de deplacement n'annule pas un soin en attente") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    escouade.allerVers({300.0f, 300.0f}, carte);
    simulerAvecAttaque(escouade, carte, 5.0f, generateur);

    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Un seul skill en attente a la fois") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});   // en attente

    // Le skill de dégâts de la place 1, à portée d'une cible, est refusé.
    escouade.verrouiller(1);
    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
}
