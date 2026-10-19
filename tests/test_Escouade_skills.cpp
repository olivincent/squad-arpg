// ./tests/test_Escouade_skills.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "EtatSkill.hpp"
#include "EntiteVue.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"
#include "TypeSkill.hpp"

namespace {

// Le monstre d'identifiant 1 tel que l'escouade le voit, seul dans la liste.
std::vector<EntiteVue> unSeulMonstre(Cercle cercle) {
    return {EntiteVue{1, cercle}};
}

// Déclenche le skill de cette place et renvoie le total des dégâts de la
// liste de coups.
int declencher(Escouade& escouade, AdresseSkill adresse, const std::vector<EntiteVue>& monstres) {
    int total = 0;
    Generateur generateur = generateurDeTest();
    for (const Coup& coup : declencherEtAttendre(escouade, adresse, monstres, carteOuverte(), generateur)) {
        total += coup.degats;
    }
    return total;
}

}  // namespace


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

// Escouade en (50, 50), orientée vers +x : les trois places sont en
// (85, 60), (35, 35) et (35, 85). Des dégâts d'origine égale à
// l'une de ces positions touchent la place correspondante.

TEST_CASE("choisirCible (soin) est vide quand tout le monde est a pleine vie") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

TEST_CASE("choisirCible (soin) designe le seul blesse") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("choisirCible (soin) designe celui qui a le moins de points de vie") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 30);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 50);

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("choisirCible (soin) choisit la plus petite place en cas d'egalite") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 20);  // prêtre (place 2), sans armure : perd 20
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 30);  // guerrier (place 1), armure 50 : 30 x 2/3 = 20

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("choisirCible (soin) ignore les morts") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 200);
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("choisirCible (soin) est vide quand tout le monde est mort") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    for (const sf::Vector2f origine : {sf::Vector2f(85.0f, 60.0f), sf::Vector2f(35.0f, 35.0f), sf::Vector2f(35.0f, 85.0f)}) {
        escouade.recevoirDegats(1, origine, 200);
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
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    REQUIRE(declencher(escouade, s_adresseSoin, {}) == 0);

    // 80 + 30 plafonné à 100 : plus personne à soigner.
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

TEST_CASE("prendreSoins rend le soin applique (place, points reellement rendus, position) puis se vide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);  // le mage (place 3) perd 20 points

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
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 20);

    declencher(escouade, s_adresseSoin, {});

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("Le soin ne part pas a pleine vie et ne declenche alors pas la recharge") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    declencher(escouade, s_adresseSoin, {});

    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte()).has_value());
}

TEST_CASE("Le soin est en recharge juste apres son lancement") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 60);

    declencher(escouade, s_adresseSoin, {});   // 40 -> 70
    declencher(escouade, s_adresseSoin, {});   // refusé : recharge

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("Un soigneur mort ne soigne pas") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 200);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    declencher(escouade, s_adresseSoin, {});

    REQUIRE(placeAlliePourSoin(escouade) == 3);
}

TEST_CASE("Le skill de degats est refuse sans monstre verrouille meme si une cible est donnee") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, s_adresseFrappe, {}) == 0);
}

// Skill de soin en attente. Carte dégagée dédiée ; escouade immobile en
// (50, 50), prêtre au soin court. Le blessé est la place 1 (guerrier, en (85, 60)),
// à environ 55,9 px du soigneur (35, 35) : hors de la portée de ce soin
// (33 px bord à bord, soit 53 px entre centres). Le mage (35, 85), à 50 px, est à portée.

TEST_CASE("Un soin hors de portee est mis en attente : le soigneur marche, soigne, puis revient a sa place") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 20);
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
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    escouade.recevoirDegats(1, {85.0f, 60.0f}, 200);
    static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur)); // le monstre fantôme n'existe pas : il n'est pas un agresseur
    simuler(escouade, carte, 2.0f);

    // Le soigneur est revenu à sa place et son skill est prêt : un nouveau
    // blessé à portée est soigné aussitôt.
    const std::optional<EntiteVue> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    verifierPointProche(soigneur->cercle.centre, {35.0f, 35.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);
    declencher(escouade, s_adresseSoin, {});
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Le soin en attente est annule si le soigneur meurt") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    escouade.recevoirDegats(1, {35.0f, 35.0f}, 200);
    simuler(escouade, carte, 2.0f);

    REQUIRE(placeAlliePourSoin(escouade) == 1);
}

TEST_CASE("Un ordre de deplacement n'annule pas un soin en attente") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});

    escouade.allerVers({300.0f, 300.0f}, carte);
    simulerAvecAttaque(escouade, carte, 5.0f, generateur);

    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Un seul skill en attente a la fois") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouadeAuSoinCourt({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 60.0f}, 20);
    declencher(escouade, s_adresseSoin, {});   // en attente

    // Le skill de dégâts de la place 1, à portée d'une cible, est refusé.
    escouade.verrouiller(1);
    REQUIRE(declencher(escouade, s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
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
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
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

// Provocation (touche 2) : lanceur (guerrier) en (235, 210), rayon 150 px bord à bord.

TEST_CASE("Provocation : provoque tous les monstres dans le rayon, sans degats") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({335.0f, 210.0f})},
        EntiteVue{2, cibleMonstre({235.0f, 310.0f})},
        EntiteVue{3, cibleMonstre({800.0f, 210.0f})},
    };

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 2);
    REQUIRE(provocations[0].id == 1);
    REQUIRE(provocations[1].id == 2);
    REQUIRE(provocations[0].place == 1);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : la zone se mesure depuis le centre du lanceur, bord du monstre compris") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Centre du lanceur (235, 210), rayon de zone 150, rayon du monstre 15 :
    // le monstre est touché jusqu'à 165 px de distance entre centres.
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({400.0f, 210.0f})},
        EntiteVue{2, cibleMonstre({401.0f, 210.0f})},
    };

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 1);
}

TEST_CASE("Provocation : sans monstre dans le rayon, elle part quand meme et consomme sa recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> loin = {EntiteVue{1, cibleMonstre({800.0f, 210.0f})}};
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));

    // Recharge en cours : un monstre qui arrive n'est pas provoqué.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, {EntiteVue{1, cibleMonstre({335.0f, 210.0f})}}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
}


TEST_CASE("Provocation : un nouvel appui pendant la recharge est ignore, puis accepte apres 10 s") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({335.0f, 210.0f})}};

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    escouade.mettreAJour(3.0f, carte, {});
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations()[0].tempsRestant < 1.5f);  // pas remis à 4

    escouade.mettreAJour(7.1f, carte, {});
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations()[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : refusee si le lanceur est mort") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {235.0f, 210.0f}, 200);  // tue la place 1

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, {EntiteVue{1, cibleMonstre({335.0f, 210.0f})}}, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
}

TEST_CASE("Provocation : refusee si un autre skill est en attente") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({335.0f, 210.0f})}};
    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carteOuverte(), generateur).empty());  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
}

// Skill de zone (touche 4) : lanceur (mage) en (185, 235), 15 dégâts, recharge 8 s,
// zone de 60 px de rayon ; monstres de rayon 15, donc touchés jusqu'à 75 px
// de distance entre centres.

TEST_CASE("skillEstPret : vrai pour un lanceur vivant dont le skill est charge") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("skillEstPret : faux pour une place invalide") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE_FALSE(escouade.skillEstPret(AdresseSkill{0, IdSkill::FrappePuissante}));
    REQUIRE_FALSE(escouade.skillEstPret(AdresseSkill{5, IdSkill::FrappePuissante}));
}

TEST_CASE("skillEstPret : faux si le lanceur est mort") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 200);  // tue la place 3

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("skillEstPret : faux pendant la recharge, vrai apres 8 s") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));

    escouade.mettreAJour(8.1f, carte, {});
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : les monstres dans la zone sont touches, celui hors zone est epargne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({300.0f, 300.0f})},
        EntiteVue{2, cibleMonstre({330.0f, 300.0f})},
        EntiteVue{3, cibleMonstre({500.0f, 300.0f})},
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre == 1);
    REQUIRE(coups[0].degats == s_degatsZoneDeTest);
    REQUIRE(coups[1].idMonstre == 2);
    REQUIRE(coups[1].degats == s_degatsZoneDeTest);
}

TEST_CASE("Zone : un monstre qui touche tout juste le bord de la zone est touche") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({375.0f, 300.0f})},
        EntiteVue{2, cibleMonstre({300.0f, 376.0f})},
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Zone : une zone vide part quand meme et lance la recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {EntiteVue{1, cibleMonstre({700.0f, 300.0f})}}, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : un point a portee (250 px bord a bord) part tout de suite") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Lanceur (mage, place 3) en (185, 235), rayon 10 : (435, 235) est à 240 px du bord.
    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {435.0f, 235.0f}, {EntiteVue{1, cibleMonstre({435.0f, 235.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : un point hors de portee met le sort en attente, sans coup ni recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({650.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    // Un sort est en attente : un autre skill est refusé.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : le lanceur se rapproche, puis le sort part au point fixe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({650.0f, 235.0f})},
        EntiteVue{2, cibleMonstre({650.0f, 400.0f})},
    };
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 300) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.degats == s_degatsZoneDeTest) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
        ++tics;
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
    REQUIRE(tics > 5);  // le lanceur a dû marcher
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
    // Plus rien en attente : un autre skill peut maintenant partir.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : elle touche les monstres presents au depart, pas ceux du clic") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, {EntiteVue{1, cibleMonstre({650.0f, 235.0f})}}, carte, generateur).empty());

    const std::vector<EntiteVue> ailleurs = {
        EntiteVue{1, cibleMonstre({650.0f, 500.0f})},
        EntiteVue{2, cibleMonstre({660.0f, 235.0f})},
    };
    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, ailleurs, carte, generateur)) {
            if (coup.degats == s_degatsZoneDeTest) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 2);
}

TEST_CASE("Zone en attente : la mort du lanceur l'annule sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({650.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 200);  // tue la place 3

    for (int i = 0; i < 300; ++i) {
        REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
        escouade.mettreAJour(0.016f, carte, {});
    }

    // Plus rien en attente : une provocation est acceptée.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : verrouiller, deverrouiller et un ordre de deplacement ne l'annulent pas") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({650.0f, 235.0f})},
        EntiteVue{2, cibleMonstre({300.0f, 400.0f})},
    };
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    escouade.verrouiller(2);
    escouade.verrouiller(1);
    escouade.deverrouiller();
    escouade.allerVers({200.0f, 500.0f}, carte);

    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.degats == s_degatsZoneDeTest) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
}

TEST_CASE("Zone : refusee pendant la recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({300.0f, 300.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur).size() == 1);

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur).empty());
}

TEST_CASE("Zone : refusee si le lanceur est mort") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 200);  // tue la place 3

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {EntiteVue{1, cibleMonstre({300.0f, 300.0f})}}, carte, generateur).empty());
}

TEST_CASE("Zone : refusee si un skill est en attente") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({300.0f, 300.0f})}};
    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carteOuverte(), generateur).empty());  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : refusee pour une place invalide ou dont le skill n'est pas de zone") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({235.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, AdresseSkill{0, IdSkill::FrappePuissante}, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, AdresseSkill{5, IdSkill::FrappePuissante}, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseFrappe, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseProvocation, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseSoin, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
    REQUIRE(escouade.skillEstPret(s_adresseSoin));
}

TEST_CASE("declencherSkill sans point refuse un skill de zone, et choisirCible ne renvoie rien") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({185.0f, 235.0f})}};

    REQUIRE(declencherEtAttendre(escouade, s_adresseExplosion, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Zone, s_adresseExplosion, monstres, carteOuverte()).has_value());
}

TEST_CASE("Zone : relance la recharge de l'attaque de base du lanceur") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const TileMap carte = carteDegageeEntouree();
    // Ce monstre est à portée de tir des places 2 et 3 (120 px) et hors de
    // portée du guerrier : sans le skill, les deux tireurs frapperaient.
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({285.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur).empty());

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].degats == s_degatsBaseDeTest);
}

TEST_CASE("rayonZoneDuSkill : celui de l'explosion et de la provocation, 0 sans zone ou place invalide") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(escouade.rayonZoneDuSkill(s_adresseExplosion) == s_rayonZoneDeTest);
    REQUIRE(escouade.rayonZoneDuSkill(s_adresseProvocation) == s_rayonProvocationDeTest);
    REQUIRE(escouade.rayonZoneDuSkill(s_adresseFrappe) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(AdresseSkill{0, IdSkill::FrappePuissante}) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(AdresseSkill{5, IdSkill::FrappePuissante}) == 0.0f);
}

TEST_CASE("etatDuSkill est vide pour une place invalide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.etatDuSkill(AdresseSkill{0, IdSkill::FrappePuissante}).has_value());
    REQUIRE_FALSE(escouade.etatDuSkill(AdresseSkill{5, IdSkill::FrappePuissante}).has_value());
}

TEST_CASE("etatDuSkill donne le type du skill de chaque place") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->type == TypeSkill::Degats);
    REQUIRE(escouade.etatDuSkill(s_adresseProvocation)->type == TypeSkill::Provocation);
    REQUIRE(escouade.etatDuSkill(s_adresseSoin)->type == TypeSkill::Soin);
    REQUIRE(escouade.etatDuSkill(s_adresseExplosion)->type == TypeSkill::Zone);
    REQUIRE(escouade.etatDuSkill(s_adresseExplosion)->id == IdSkill::Explosion);
}

TEST_CASE("etatDuSkill d'un skill pret : progression 1, temps restant 0") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::optional<EtatSkill> etat = escouade.etatDuSkill(s_adresseProvocation);

    REQUIRE(etat.has_value());
    REQUIRE(etat->progression == 1.0f);
    REQUIRE(etat->tempsRestant == 0.0f);
    REQUIRE(etat->lanceurVivant);
}

TEST_CASE("etatDuSkill juste apres un declenchement : recharge complete") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, {}, carteOuverte(), generateur).empty()); // la provocation part même sans monstre
    const std::optional<EtatSkill> etat = escouade.etatDuSkill(s_adresseProvocation);

    REQUIRE(etat.has_value());
    REQUIRE(etat->progression == 0.0f);
    REQUIRE(etat->tempsRestant > 0.0f);
}

TEST_CASE("etatDuSkill d'un lanceur mort : lanceurVivant est faux") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    for (int i = 0; i < 3; ++i) {
        escouade.recevoirDegats(1, {1000.0f, 1000.0f}, 200);
    }

    const std::optional<EtatSkill> etat = escouade.etatDuSkill(s_adresseProvocation);

    REQUIRE(etat.has_value());
    REQUIRE_FALSE(etat->lanceurVivant);
}

// Temps d'activation des skills (étape 58) : dégâts 0,3 s, provocation 0,2 s,
// soin 0,6 s, zone 0,8 s. Escouade en (50, 50) : places en (85, 60),
// (35, 35) et (35, 85).

namespace {

// Appelle attaquer() par tics pendant cette durée (sans mettreAJour : les
// recharges ne courent pas) et renvoie tous les coups partis.
std::vector<Coup> attaquerPendant(Escouade& escouade, const std::vector<EntiteVue>& monstres, float duree, Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        const std::vector<Coup> coupsDuTic = escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur);
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
    }
    return coups;
}

// Nombre de coups d'exactement ce montant (écarte les attaques de base).
int compter(const std::vector<Coup>& coups, int degats) {
    return static_cast<int>(std::count_if(coups.begin(), coups.end(), [&](const Coup& coup) { return coup.degats == degats; }));
}

// Position du personnage de cette place, lue par la proie d'un monstre qu'il provoque.
sf::Vector2f positionDeLaPlace(Escouade& escouade, std::size_t place) {
    escouade.provoquer(99, place, 1000.0f);
    return escouade.proieDuMonstre(99, {0.0f, 0.0f})->cercle.centre;
}

}  // namespace

TEST_CASE("Un skill de degats part a la fin de son activation de 0,3 s, pas avant") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));
    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);

    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.25f, generateur), s_degatsFrappeDeTest) == 0);
    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.1f, generateur), s_degatsFrappeDeTest) == 1);
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);
    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->progression < 1.0f);  // la recharge a démarré au départ
}

TEST_CASE("Un skill en activation refuse un nouvel appui sur la meme touche") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());

    REQUIRE(compter(attaquerPendant(escouade, monstres, 1.0f, generateur), s_degatsFrappeDeTest) == 1);
}

TEST_CASE("Le lanceur reste sur place pendant l'activation, un ordre de deplacement ne l'annule pas") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 60.0f}, 30);  // un blessé à soigner
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carte, generateur).empty());
    const sf::Vector2f avant = positionDeLaPlace(escouade, 2);

    escouade.allerVers({300.0f, 300.0f}, carte);
    simulerAvecAttaque(escouade, carte, 0.4f, generateur);

    verifierPointProche(positionDeLaPlace(escouade, 2), avant);   // le soigneur s'est arrêté
    REQUIRE(escouade.getCentre().x > 70.0f);                      // le reste de l'escouade est parti
    REQUIRE(placeAlliePourSoin(escouade) == 1);                   // pas encore soigné

    simulerAvecAttaque(escouade, carte, 0.5f, generateur);
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());      // soigné malgré l'ordre
}

TEST_CASE("La mort du lanceur annule l'activation, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 60.0f}, 30);
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carteOuverte(), generateur).empty());

    escouade.recevoirCoup(1, 2, {35.0f, 35.0f}, 1000);
    static_cast<void>(attaquerPendant(escouade, {}, 1.0f, generateur));

    REQUIRE(placeAlliePourSoin(escouade) == 1);  // personne n'a été soigné
    REQUIRE(escouade.etatDuSkill(s_adresseSoin)->progression == 1.0f);
}

TEST_CASE("La disparition du monstre vise annule l'activation d'un skill de degats, sans recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f})), carteOuverte(), generateur).empty());

    const std::vector<Coup> coups = attaquerPendant(escouade, {}, 1.0f, generateur);  // le monstre n'est plus dans la liste

    REQUIRE(compter(coups, s_degatsFrappeDeTest) == 0);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));
}

TEST_CASE("La mort de l'allie vise annule l'activation d'un soin, sans recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 60.0f}, 30);
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carteOuverte(), generateur).empty());

    escouade.recevoirCoup(1, 1, {85.0f, 60.0f}, 1000);
    static_cast<void>(attaquerPendant(escouade, {}, 1.0f, generateur));

    REQUIRE(escouade.skillEstPret(s_adresseSoin));
}

TEST_CASE("Zone : l'effet est evalue au depart, un monstre sorti de la zone n'est pas touche, un entre l'est") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const Cercle dedans = cibleMonstre({150.0f, 100.0f});
    const Cercle dehors = cibleMonstre({300.0f, 100.0f});
    REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, {150.0f, 100.0f}, {EntiteVue{1, dedans}, EntiteVue{2, dehors}}, carteOuverte(), generateur).empty());

    // Le monstre 1 était dans la zone à l'appui, le 2 y entre pendant l'activation.
    std::vector<Coup> coups = attaquerPendant(escouade, {EntiteVue{1, dedans}, EntiteVue{2, dehors}}, 0.7f, generateur);
    const std::vector<Coup> suite = attaquerPendant(escouade, {EntiteVue{1, dehors}, EntiteVue{2, dedans}}, 0.2f, generateur);
    coups.insert(coups.end(), suite.begin(), suite.end());

    REQUIRE(compter(coups, s_degatsZoneDeTest) == 1);
    REQUIRE(std::any_of(coups.begin(), coups.end(), [](const Coup& coup) { return coup.degats == s_degatsZoneDeTest && coup.idMonstre == 2; }));
}

TEST_CASE("Provocation : l'effet est evalue au depart, apres 0,2 s") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const Cercle dedans = cibleMonstre({100.0f, 60.0f});
    const Cercle dehors = cibleMonstre({600.0f, 60.0f});
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, {EntiteVue{1, dedans}, EntiteVue{2, dehors}}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());

    static_cast<void>(attaquerPendant(escouade, {EntiteVue{1, dedans}, EntiteVue{2, dehors}}, 0.1f, generateur));
    REQUIRE(escouade.getProvocations().empty());
    static_cast<void>(attaquerPendant(escouade, {EntiteVue{1, dehors}, EntiteVue{2, dedans}}, 0.15f, generateur));

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 2);
}

TEST_CASE("Deux lanceurs peuvent activer en meme temps") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 60.0f}, 30);
    const std::vector<EntiteVue> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseSoin, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseSoin));

    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.4f, generateur), s_degatsFrappeDeTest) == 1);  // les dégâts sont partis
    REQUIRE(placeAlliePourSoin(escouade) == 1);                                         // pas encore le soin
    static_cast<void>(attaquerPendant(escouade, monstres, 0.3f, generateur));
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

// Adresse invalide : place valide, mais skill inconnu de ce personnage.

TEST_CASE("Une adresse dont le skill est inconnu du personnage est traitee comme une place invalide") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const AdresseSkill inconnue{1, IdSkill::Soin};  // le guerrier de la place 1 n'a pas de soin

    REQUIRE_FALSE(escouade.skillEstPret(inconnue));
    REQUIRE_FALSE(escouade.etatDuSkill(inconnue).has_value());
    REQUIRE_FALSE(escouade.typeDuSkill(inconnue).has_value());
    REQUIRE(escouade.rayonZoneDuSkill(inconnue) == 0.0f);
    REQUIRE(declencherEtAttendre(escouade, inconnue, {EntiteVue{1, cibleMonstre({110.0f, 35.0f})}}, carteOuverte(), generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, inconnue, {235.0f, 235.0f}, {}, carteOuverte(), generateur).empty());
}

TEST_CASE("typeDuSkill donne le type du skill a l'adresse, vide pour une place invalide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.typeDuSkill(s_adresseFrappe) == TypeSkill::Degats);
    REQUIRE(escouade.typeDuSkill(s_adresseExplosion) == TypeSkill::Zone);
    REQUIRE_FALSE(escouade.typeDuSkill(AdresseSkill{5, IdSkill::FrappePuissante}).has_value());
}

TEST_CASE("adressesDesSkills : par place, puis dans l'ordre de la liste du personnage") {
    const Escouade escouade = uneEscouade({200.0f, 200.0f});

    const std::vector<AdresseSkill> attendues = {s_adresseFrappe, s_adresseProvocation, s_adresseSoin, s_adresseExplosion};
    REQUIRE(escouade.adressesDesSkills() == attendues);
}

TEST_CASE("couleurDuPersonnage : la couleur de la composition, vide pour une place invalide") {
    const Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(escouade.couleurDuPersonnage(1) == guerrierDeTest().couleur);
    REQUIRE(escouade.couleurDuPersonnage(3) == mageDeTest().couleur);
    REQUIRE_FALSE(escouade.couleurDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.couleurDuPersonnage(4).has_value());
}

// Un skill demandé pendant l'activation d'un autre skill du même lanceur (étape 80) : il
// est mis en attente (ou y reste) et part dès que le lanceur est libre, au lieu d'être perdu.
// Escouade en (50, 50) : guerrier en (85, 60).

namespace {

// Paladin et rôdeur de test : deux skills chacun, valeurs propres à ces tests (pas le catalogue).
constexpr int s_degatsFrappeSacreeDeTest = 20;
constexpr int s_puissanceImpositionDeTest = 25;
constexpr int s_degatsTirDeTest = 25;
constexpr int s_degatsPluieDeTest = 12;
constexpr AdresseSkill s_adresseFrappeSacree{1, IdSkill::FrappeSacree};
constexpr AdresseSkill s_adresseImposition{1, IdSkill::ImpositionDesMains};
constexpr AdresseSkill s_adresseTir{1, IdSkill::TirPercant};
constexpr AdresseSkill s_adressePluie{1, IdSkill::PluieDeFleches};

DefinitionPersonnage paladinDeTest() {
    DefinitionPersonnage paladin = guerrierDeTest();
    paladin.skills = {
        {IdSkill::FrappeSacree, Skill(TypeSkill::Degats, s_porteeMeleeDeTest, s_degatsFrappeSacreeDeTest, 5.0f, 5.0f, 0.0f, 0.3f)},
        {IdSkill::ImpositionDesMains, Skill(TypeSkill::Soin, s_porteeMeleeDeTest, s_puissanceImpositionDeTest, 10.0f, 10.0f, 0.0f, 0.5f)}};
    return paladin;
}

DefinitionPersonnage rodeurDeTest() {
    DefinitionPersonnage rodeur = mageDeTest();
    rodeur.skills = {
        {IdSkill::TirPercant, Skill(TypeSkill::Degats, 200.0f, s_degatsTirDeTest, 6.0f, 6.0f, 0.0f, 0.5f)},
        {IdSkill::PluieDeFleches, Skill(TypeSkill::Zone, 320.0f, s_degatsPluieDeTest, 8.0f, 8.0f, 45.0f, 0.8f)}};
    return rodeur;
}

// Une image d'Escouade comme dans Partie, monstres immobiles : attaquer() puis mettreAJour().
// Renvoie tous les coups partis.
std::vector<Coup> combattrePendant(Escouade& escouade, const std::vector<EntiteVue>& monstres, float duree, Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        const std::vector<Coup> coupsDuTic = escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur);
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
        escouade.mettreAJour(s_dureeTic, carteOuverte(), monstres);
    }
    return coups;
}

}  // namespace

TEST_CASE("Lanceur occupe : une frappe demandee pendant sa provocation attend, puis part") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_adresseProvocation)->enActivation);

    // Monstre au contact, frappe prête : elle attend la fin de la provocation.
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));

    // La provocation part (0,2 s), puis la frappe s'active et part (0,3 s).
    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.7f, generateur), s_degatsFrappeDeTest) == 1);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));  // sa recharge court
}

TEST_CASE("Lanceur occupe : une frappe en attente ne se perd pas si la cible arrive pendant la provocation") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const std::vector<EntiteVue> loin = unSeulMonstre(cibleMonstre({300.0f, 60.0f}));
    const std::vector<EntiteVue> pres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, loin, carteOuverte(), generateur).empty());  // hors de portée : en attente

    // Le monstre arrive au contact pendant l'activation de la provocation : la frappe attend encore.
    static_cast<void>(escouade.attaquer(s_dureeTic, pres, carteOuverte(), generateur));
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);

    REQUIRE(compter(attaquerPendant(escouade, pres, 1.0f, generateur), s_degatsFrappeDeTest) == 1);
}

TEST_CASE("Lanceur occupe : sans verrou, une frappe en attente est abandonnee si sa cible n'est plus atteinte") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> pres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));
    const std::vector<EntiteVue> loin = unSeulMonstre(cibleMonstre({300.0f, 60.0f}));
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, pres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, pres, carteOuverte(), generateur).empty());

    // Le monstre s'éloigne avant la fin de la provocation : sans verrou, personne ne le suit.
    REQUIRE(compter(attaquerPendant(escouade, loin, 1.0f, generateur), s_degatsFrappeDeTest) == 0);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));  // sans recharge

    // Plus rien n'attend : un autre skill est de nouveau accepté.
    escouade.recevoirCoup(1, 1, {85.0f, 60.0f}, 30);
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_adresseSoin)->enActivation);
}

TEST_CASE("Lanceur occupe : le paladin soigne apres sa frappe sacree") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f}, {paladinDeTest()});  // seul, devant : en (85, 60)
    const std::vector<EntiteVue> monstres = unSeulMonstre(cibleMonstre({110.0f, 60.0f}));
    escouade.recevoirCoup(1, 1, {110.0f, 60.0f}, 40);  // blessé : il est la cible de son soin
    const int vieBlessee = *escouade.pointsDeVieDuPersonnage(1);

    REQUIRE(escouade.declencherSkill(s_adresseFrappeSacree, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseImposition, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseImposition)->enActivation);

    // La frappe part (0,3 s), puis le soin s'active et part (0,5 s).
    REQUIRE(compter(combattrePendant(escouade, monstres, 1.2f, generateur), s_degatsFrappeSacreeDeTest) == 1);
    REQUIRE(*escouade.pointsDeVieDuPersonnage(1) == vieBlessee + s_puissanceImpositionDeTest);
}

TEST_CASE("Lanceur occupe : le rodeur lance sa pluie de fleches apres son tir percant") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f}, {rodeurDeTest()});  // seul, derrière : en (35, 60)
    const std::vector<EntiteVue> monstres = unSeulMonstre(cibleMonstre({135.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseTir, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkillDeZone(s_adressePluie, {135.0f, 60.0f}, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adressePluie)->enActivation);
    REQUIRE(escouade.skillEstPret(s_adressePluie));

    // Le tir part (0,5 s), puis la pluie s'active et part (0,8 s).
    const std::vector<Coup> coups = attaquerPendant(escouade, monstres, 1.6f, generateur);
    REQUIRE(compter(coups, s_degatsTirDeTest) == 1);
    REQUIRE(compter(coups, s_degatsPluieDeTest) == 1);
}
