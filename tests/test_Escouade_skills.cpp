// ./tests/test_Escouade_skills.cpp
#include <catch2/catch_test_macros.hpp>
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
#include "MonstreVu.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"
#include "TypeSkill.hpp"

namespace {

// Le monstre d'identifiant 1 tel que l'escouade le voit, seul dans la liste.
std::vector<MonstreVu> unSeulMonstre(Cercle cercle) {
    return {MonstreVu{1, cercle}};
}

// Déclenche le skill de cette place et renvoie le total des dégâts de la
// liste de coups.
int declencher(Escouade& escouade, std::size_t place, const std::vector<MonstreVu>& monstres) {
    int total = 0;
    Generateur generateur = generateurDeTest();
    for (const Coup& coup : declencherEtAttendre(escouade, place, monstres, carteOuverte(), generateur)) {
        total += coup.degats;
    }
    return total;
}

}  // namespace


// Pour les tests du skill : centre initial (60, 60), mêlées en (85, 35) et
// (85, 85). Une cible de la taille d'un monstre en (85, 60) est à 0 px bord
// à bord de chacun d'eux, donc à portée du skill (20 px, comme la mêlée).

// Sans verrou, le skill de dégâts vise le monstre que son lanceur frapperait
// en autodéfense, avec la portée du skill (20 px). Le lanceur de la place 1
// est en (85, 35), rayon 10 : un monstre (rayon 15) à 45 px ou moins est à portée.

TEST_CASE("Sans verrou, le skill de degats part sur un monstre a portee, sans creer de verrou") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});

    const std::vector<Coup> coups = declencherEtAttendre(escouade, 1, {MonstreVu{4, cibleMonstre({85.0f, 60.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 4);
    REQUIRE(coups[0].degats == 25);
    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("Sans verrou, le skill de degats vise le plus proche parmi plusieurs monstres a portee") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});

    // Le monstre 1 (premier de la liste) est à 15 px bord à bord, le 2 à 0.
    const std::vector<Coup> coups = declencherEtAttendre(escouade,
        1, {MonstreVu{1, cibleMonstre({125.0f, 35.0f})}, MonstreVu{2, cibleMonstre({110.0f, 35.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 2);
}

TEST_CASE("Sans verrou, le skill de degats est refuse sans monstre a portee, et sans recharge") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f}))) == 0);
    REQUIRE(declencher(escouade, 1, {}) == 0);
    // Pas de mise en attente ni de recharge : un monstre à portée est frappé aussitôt.
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 25);
}

TEST_CASE("Avec un verrou, le skill de degats vise le monstre verrouille, pas le plus proche") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const std::vector<Coup> coups = declencherEtAttendre(escouade,
        1, {MonstreVu{1, cibleMonstre({125.0f, 35.0f})}, MonstreVu{2, cibleMonstre({110.0f, 35.0f})}}, carteOuverte(), generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Avec un verrou absent de la liste des monstres, le skill de degats est refuse") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(9);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie les degats du seul personnage qui a un skill") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Les deux mêlées sont à portée, mais un seul a un skill : 25, pas 50.
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 25);
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une cible hors de portee du skill") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f}))) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie 0 au second appel immediat (recharge du skill)") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cible)) == 25);
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cible)) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une place sans skill ou hors formation") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(declencher(escouade, 2, unSeulMonstre(cible)) == 0);
    REQUIRE(declencher(escouade, 0, unSeulMonstre(cible)) == 0);
    REQUIRE(declencher(escouade, 5, unSeulMonstre(cible)) == 0);
}

// Escouade en (50, 50), orientée vers +x : les quatre places sont en
// (85, 35), (85, 85), (35, 35) et (35, 85). Des dégâts d'origine égale à
// l'une de ces positions touchent la place correspondante.

TEST_CASE("choisirCible (soin) est vide quand tout le monde est a pleine vie") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}, carteOuverte()).has_value());
}

TEST_CASE("choisirCible (soin) designe le seul blesse") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("choisirCible (soin) designe celui qui a le moins de points de vie") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 30);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 50);

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("choisirCible (soin) choisit la plus petite place en cas d'egalite") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 20);  // tireur, sans armure : perd 20
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 30);  // mêlée, armure 50 : 30 x 2/3 = 20

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("choisirCible (soin) ignore les morts") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 200);
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("choisirCible (soin) est vide quand tout le monde est mort") {
    Escouade escouade({50.0f, 50.0f});
    for (const sf::Vector2f origine : {sf::Vector2f(85.0f, 35.0f), sf::Vector2f(85.0f, 85.0f),
                                       sf::Vector2f(35.0f, 35.0f), sf::Vector2f(35.0f, 85.0f)}) {
        escouade.recevoirDegats(1, origine, 200);
    }

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}, carteOuverte()).has_value());
}

// Soin (place 3, en (35, 35), portée 40 px bord à bord = 60 px entre
// centres, 30 points de vie, recharge 8 s). Place 4 en (35, 85) : 50 px,
// à portée. Place 2 en (85, 85) : environ 70,7 px, hors de portée.

TEST_CASE("Le soin de la place 3 soigne un allie a portee, sans combat engage") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    REQUIRE(declencher(escouade, 3, {}) == 0);

    // 80 + 30 plafonné à 100 : plus personne à soigner.
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}, carteOuverte()).has_value());
}

TEST_CASE("Le soin ne part pas si la cible est hors de portee") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);

    declencher(escouade, 3, {});

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("Le soin ne part pas a pleine vie et ne declenche alors pas la recharge") {
    Escouade escouade({50.0f, 50.0f});
    declencher(escouade, 3, {});

    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}, carteOuverte()).has_value());
}

TEST_CASE("Le soin est en recharge juste apres son lancement") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 60);

    declencher(escouade, 3, {});   // 40 -> 70
    declencher(escouade, 3, {});   // refusé : recharge

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("Un soigneur mort ne soigne pas") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 200);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    declencher(escouade, 3, {});

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("Le skill de degats est refuse sans monstre verrouille meme si une cible est donnee") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, 1, {}) == 0);
}

// Skill de soin en attente. Carte dégagée dédiée ; escouade immobile en
// (50, 50). Le blessé est la place 2 (85, 85), à environ 70,7 px du
// soigneur (35, 35) : hors de la portée du soin (60 px entre centres).

TEST_CASE("Un soin hors de portee est mis en attente : le soigneur marche, soigne, puis revient a sa place") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur)); // le monstre fantôme n'existe pas : il n'est pas un agresseur

    declencher(escouade, 3, {});
    REQUIRE(placeAlliePourSoin(escouade) == 2);   // pas encore soigné

    simulerAvecAttaque(escouade, carte, 1.5f, generateur);
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());  // soigné en route (marche, puis 0,6 s d'activation)

    simuler(escouade, carte, 2.0f);
    const std::optional<Cercle> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    REQUIRE(soigneur.has_value());
    verifierPointProche(soigneur->centre, {35.0f, 35.0f});
}

TEST_CASE("Le soin en attente est annule si la cible meurt, sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    escouade.recevoirDegats(1, {85.0f, 85.0f}, 200);
    static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur)); // le monstre fantôme n'existe pas : il n'est pas un agresseur
    simuler(escouade, carte, 2.0f);

    // Le soigneur est revenu à sa place et son skill est prêt : un nouveau
    // blessé à portée est soigné aussitôt.
    const std::optional<Cercle> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    verifierPointProche(soigneur->centre, {35.0f, 35.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);
    declencher(escouade, 3, {});
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Le soin en attente est annule si le soigneur meurt") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    escouade.recevoirDegats(1, {35.0f, 35.0f}, 200);
    simuler(escouade, carte, 2.0f);

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("Un ordre de deplacement n'annule pas un soin en attente") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    escouade.allerVers({300.0f, 300.0f}, carte);
    simulerAvecAttaque(escouade, carte, 5.0f, generateur);

    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Un seul skill en attente a la fois") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});   // en attente

    // Le skill de dégâts de la place 1, à portée d'une cible, est refusé.
    escouade.verrouiller(1);
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
}

// Skill de dégâts en attente. Aucune attaque de base ne dépasse 20 dégâts
// en un tic (4 personnages x 5) : un tic qui rapporte 25 ou plus contient
// donc forcément le skill.

TEST_CASE("Un skill de degats hors de portee est mis en attente, puis part a l'arrivee du lanceur") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    const Cercle cible = cibleMonstre({300.0f, 60.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cible);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cible)) == 0);

    int degatsMax = 0;
    for (int i = 0; i < 600; ++i) {
        const int degats = attaquerCible(escouade, carte, cible);
        degatsMax = std::max(degatsMax, degats);
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }
    REQUIRE(degatsMax >= 25);
}

TEST_CASE("Un skill de degats en attente est annule par deverrouiller, sans recharge") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    escouade.deverrouiller();
    escouade.verrouiller(1);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 25);
}

TEST_CASE("Un skill de degats en attente est annule par un changement de cible, pas par la meme cible") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    escouade.verrouiller(1);   // même cible : toujours en attente
    REQUIRE(escouade.declencherSkill(1, unSeulMonstre(cibleMonstre({85.0f, 60.0f})), carteOuverte(), generateur).empty());

    escouade.verrouiller(2);   // autre cible : annulé
    REQUIRE(declencher(escouade, 1, {MonstreVu{2, cibleMonstre({85.0f, 60.0f})}}) == 25);
}

TEST_CASE("Un soin est refuse pendant qu'un skill de degats est en attente") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    declencher(escouade, 3, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

// Provocation (touche 2) : lanceur en (235, 235), rayon 150 px bord à bord.

TEST_CASE("Provocation : provoque tous les monstres dans le rayon, sans degats") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({335.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({235.0f, 335.0f})},
        MonstreVu{3, cibleMonstre({800.0f, 235.0f})},
    };

    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 2);
    REQUIRE(provocations[0].id == 1);
    REQUIRE(provocations[1].id == 2);
    REQUIRE(provocations[0].place == 2);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : la zone se mesure depuis le centre du lanceur, bord du monstre compris") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    // Centre du lanceur (235, 235), rayon de zone 150, rayon du monstre 15 :
    // le monstre est touché jusqu'à 165 px de distance entre centres.
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({400.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({401.0f, 235.0f})},
    };

    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 1);
}

TEST_CASE("Provocation : sans monstre dans le rayon, elle part quand meme et consomme sa recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> loin = {MonstreVu{1, cibleMonstre({800.0f, 235.0f})}};
    REQUIRE(escouade.skillEstPret(2));

    REQUIRE(declencherEtAttendre(escouade, 2, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));

    // Recharge en cours : un monstre qui arrive n'est pas provoqué.
    REQUIRE(declencherEtAttendre(escouade, 2, {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
}


TEST_CASE("Provocation : un nouvel appui pendant la recharge est ignore, puis accepte apres 10 s") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}};

    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());
    escouade.mettreAJour(3.0f, carte, {});
    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations()[0].tempsRestant < 1.5f);  // pas remis à 4

    escouade.mettreAJour(7.1f, carte, {});
    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations()[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : refusee si le lanceur est mort") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {235.0f, 235.0f}, 200);  // tue la place 2

    REQUIRE(declencherEtAttendre(escouade, 2, {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}}, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
}

TEST_CASE("Provocation : refusee si un autre skill est en attente") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}};
    REQUIRE(declencherEtAttendre(escouade, 1, monstres, carteOuverte(), generateur).empty());  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
}

// Skill de zone (touche 4) : lanceur en (185, 235), 15 dégâts, recharge 8 s,
// zone de 60 px de rayon ; monstres de rayon 15, donc touchés jusqu'à 75 px
// de distance entre centres.

TEST_CASE("skillEstPret : vrai pour un lanceur vivant dont le skill est charge") {
    Escouade escouade({200.0f, 200.0f});

    REQUIRE(escouade.skillEstPret(4));
}

TEST_CASE("skillEstPret : faux pour une place invalide") {
    Escouade escouade({200.0f, 200.0f});

    REQUIRE_FALSE(escouade.skillEstPret(0));
    REQUIRE_FALSE(escouade.skillEstPret(5));
}

TEST_CASE("skillEstPret : faux si le lanceur est mort") {
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 200);  // tue la place 4

    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("skillEstPret : faux pendant la recharge, vrai apres 8 s") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, {}, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(4));

    escouade.mettreAJour(8.1f, carte, {});
    REQUIRE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : les monstres dans la zone sont touches, celui hors zone est epargne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({300.0f, 300.0f})},
        MonstreVu{2, cibleMonstre({330.0f, 300.0f})},
        MonstreVu{3, cibleMonstre({500.0f, 300.0f})},
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre == 1);
    REQUIRE(coups[0].degats == 15);
    REQUIRE(coups[1].idMonstre == 2);
    REQUIRE(coups[1].degats == 15);
}

TEST_CASE("Zone : un monstre qui touche tout juste le bord de la zone est touche") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({375.0f, 300.0f})},
        MonstreVu{2, cibleMonstre({300.0f, 376.0f})},
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Zone : une zone vide part quand meme et lance la recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});

    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, {MonstreVu{1, cibleMonstre({700.0f, 300.0f})}}, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un point a portee (250 px bord a bord) part tout de suite") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    // Lanceur (place 4) en (185, 235), rayon 10 : (435, 235) est à 240 px du bord.
    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, 4, {435.0f, 235.0f}, {MonstreVu{1, cibleMonstre({435.0f, 235.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un point hors de portee met le sort en attente, sans coup ni recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({650.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, 4, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(4));
    // Un sort est en attente : un autre skill est refusé.
    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : le lanceur se rapproche, puis le sort part au point fixe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({650.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({650.0f, 400.0f})},
    };
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 300) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.degats == 15) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
        ++tics;
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
    REQUIRE(tics > 5);  // le lanceur a dû marcher
    REQUIRE_FALSE(escouade.skillEstPret(4));
    // Plus rien en attente : un autre skill peut maintenant partir.
    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : elle touche les monstres presents au depart, pas ceux du clic") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {650.0f, 235.0f}, {MonstreVu{1, cibleMonstre({650.0f, 235.0f})}}, carte, generateur).empty());

    const std::vector<MonstreVu> ailleurs = {
        MonstreVu{1, cibleMonstre({650.0f, 500.0f})},
        MonstreVu{2, cibleMonstre({660.0f, 235.0f})},
    };
    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, ailleurs, carte, generateur)) {
            if (coup.degats == 15) {
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
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({650.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {650.0f, 235.0f}, monstres, carte, generateur).empty());
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 200);  // tue la place 4

    for (int i = 0; i < 300; ++i) {
        REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
        escouade.mettreAJour(0.016f, carte, {});
    }

    // Plus rien en attente : une provocation (place 2) est acceptée.
    REQUIRE(declencherEtAttendre(escouade, 2, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : verrouiller, deverrouiller et un ordre de deplacement ne l'annulent pas") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({650.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({300.0f, 400.0f})},
    };
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    escouade.verrouiller(2);
    escouade.verrouiller(1);
    escouade.deverrouiller();
    escouade.allerVers({200.0f, 500.0f}, carte);

    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.degats == 15) {
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
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({300.0f, 300.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, monstres, carte, generateur).size() == 1);

    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, monstres, carte, generateur).empty());
}

TEST_CASE("Zone : refusee si le lanceur est mort") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 200);  // tue la place 4

    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, {MonstreVu{1, cibleMonstre({300.0f, 300.0f})}}, carte, generateur).empty());
}

TEST_CASE("Zone : refusee si un skill est en attente") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({300.0f, 300.0f})}};
    REQUIRE(declencherEtAttendre(escouade, 1, monstres, carteOuverte(), generateur).empty());  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : refusee pour une place invalide ou dont le skill n'est pas de zone") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({235.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, 0, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, 5, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, 1, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, 2, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, 3, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(1));
    REQUIRE(escouade.skillEstPret(2));
    REQUIRE(escouade.skillEstPret(3));
}

TEST_CASE("declencherSkill sans point refuse un skill de zone, et choisirCible ne renvoie rien") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({185.0f, 235.0f})}};

    REQUIRE(declencherEtAttendre(escouade, 4, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.skillEstPret(4));
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Zone, 4, monstres, carteOuverte()).has_value());
}

TEST_CASE("Zone : relance la recharge de l'attaque de base du lanceur") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({200.0f, 200.0f});
    const TileMap carte = carteDegageeEntouree();
    // Ce monstre est à portée de tir des places 3 et 4 (120 px) et hors de
    // portée des mêlées : sans le skill, les deux tireurs frapperaient.
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({285.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, 4, {300.0f, 300.0f}, {}, carte, generateur).empty());

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].degats == 5);
}

TEST_CASE("rayonZoneDuSkill : 60 pour la place 4, 150 pour la provocation, 0 sans zone ou place invalide") {
    Escouade escouade({200.0f, 200.0f});

    REQUIRE(escouade.rayonZoneDuSkill(4) == 60.0f);
    REQUIRE(escouade.rayonZoneDuSkill(2) == 150.0f);
    REQUIRE(escouade.rayonZoneDuSkill(1) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(0) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(5) == 0.0f);
}

TEST_CASE("etatDuSkill est vide pour une place invalide") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.etatDuSkill(0).has_value());
    REQUIRE_FALSE(escouade.etatDuSkill(5).has_value());
}

TEST_CASE("etatDuSkill donne le type du skill de chaque place") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.etatDuSkill(1)->type == TypeSkill::Degats);
    REQUIRE(escouade.etatDuSkill(2)->type == TypeSkill::Provocation);
    REQUIRE(escouade.etatDuSkill(3)->type == TypeSkill::Soin);
    REQUIRE(escouade.etatDuSkill(4)->type == TypeSkill::Zone);
}

TEST_CASE("etatDuSkill d'un skill pret : progression 1, temps restant 0") {
    Escouade escouade({50.0f, 50.0f});

    const std::optional<EtatSkill> etat = escouade.etatDuSkill(2);

    REQUIRE(etat.has_value());
    REQUIRE(etat->progression == 1.0f);
    REQUIRE(etat->tempsRestant == 0.0f);
    REQUIRE(etat->lanceurVivant);
}

TEST_CASE("etatDuSkill juste apres un declenchement : recharge complete") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(declencherEtAttendre(escouade, 2, {}, carteOuverte(), generateur).empty()); // la provocation part même sans monstre
    const std::optional<EtatSkill> etat = escouade.etatDuSkill(2);

    REQUIRE(etat.has_value());
    REQUIRE(etat->progression == 0.0f);
    REQUIRE(etat->tempsRestant > 0.0f);
}

TEST_CASE("etatDuSkill d'un lanceur mort : lanceurVivant est faux") {
    Escouade escouade({50.0f, 50.0f});
    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, {1000.0f, 1000.0f}, 200);
    }

    const std::optional<EtatSkill> etat = escouade.etatDuSkill(2);

    REQUIRE(etat.has_value());
    REQUIRE_FALSE(etat->lanceurVivant);
}

// Temps d'activation des skills (étape 58) : dégâts 0,3 s, provocation 0,2 s,
// soin 0,6 s, zone 0,8 s. Escouade en (50, 50) : places en (85, 35), (85, 85),
// (35, 35) et (35, 85).

namespace {

// Appelle attaquer() par tics pendant cette durée (sans mettreAJour : les
// recharges ne courent pas) et renvoie tous les coups partis.
std::vector<Coup> attaquerPendant(Escouade& escouade, const std::vector<MonstreVu>& monstres, float duree, Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        const std::vector<Coup> coupsDuTic = escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur);
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
    }
    return coups;
}

// Nombre de coups d'exactement ce montant (écarte les attaques de base, de 5).
int compter(const std::vector<Coup>& coups, int degats) {
    return static_cast<int>(std::count_if(coups.begin(), coups.end(), [&](const Coup& coup) { return coup.degats == degats; }));
}

// Position du personnage de cette place, lue par la proie d'un monstre qu'il provoque.
sf::Vector2f positionDeLaPlace(Escouade& escouade, std::size_t place) {
    escouade.provoquer(99, place, 1000.0f);
    return escouade.proieDuMonstre(99, {0.0f, 0.0f})->centre;
}

}  // namespace

TEST_CASE("Un skill de degats part a la fin de son activation de 0,3 s, pas avant") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(1, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(1));
    REQUIRE(escouade.etatDuSkill(1)->enActivation);

    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.25f, generateur), 25) == 0);
    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.1f, generateur), 25) == 1);
    REQUIRE_FALSE(escouade.etatDuSkill(1)->enActivation);
    REQUIRE(escouade.etatDuSkill(1)->progression < 1.0f);  // la recharge a démarré au départ
}

TEST_CASE("Un skill en activation refuse un nouvel appui sur la meme touche") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(1, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(1, monstres, carteOuverte(), generateur).empty());

    REQUIRE(compter(attaquerPendant(escouade, monstres, 1.0f, generateur), 25) == 1);
}

TEST_CASE("Le lanceur reste sur place pendant l'activation, un ordre de deplacement ne l'annule pas") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 35.0f}, 30);  // un blessé à soigner
    REQUIRE(escouade.declencherSkill(3, {}, carte, generateur).empty());
    const sf::Vector2f avant = positionDeLaPlace(escouade, 3);

    escouade.allerVers({300.0f, 300.0f}, carte);
    simulerAvecAttaque(escouade, carte, 0.4f, generateur);

    verifierPointProche(positionDeLaPlace(escouade, 3), avant);   // le soigneur s'est arrêté
    REQUIRE(escouade.getCentre().x > 70.0f);                      // le reste de l'escouade est parti
    REQUIRE(placeAlliePourSoin(escouade) == 1);                   // pas encore soigné

    simulerAvecAttaque(escouade, carte, 0.5f, generateur);
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());      // soigné malgré l'ordre
}

TEST_CASE("La mort du lanceur annule l'activation, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 35.0f}, 30);
    REQUIRE(escouade.declencherSkill(3, {}, carteOuverte(), generateur).empty());

    escouade.recevoirCoup(1, 3, {35.0f, 35.0f}, 1000);
    static_cast<void>(attaquerPendant(escouade, {}, 1.0f, generateur));

    REQUIRE(placeAlliePourSoin(escouade) == 1);  // personne n'a été soigné
    REQUIRE(escouade.etatDuSkill(3)->progression == 1.0f);
}

TEST_CASE("La disparition du monstre vise annule l'activation d'un skill de degats, sans recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    REQUIRE(escouade.declencherSkill(1, unSeulMonstre(cibleMonstre({85.0f, 60.0f})), carteOuverte(), generateur).empty());

    const std::vector<Coup> coups = attaquerPendant(escouade, {}, 1.0f, generateur);  // le monstre n'est plus dans la liste

    REQUIRE(compter(coups, 25) == 0);
    REQUIRE(escouade.skillEstPret(1));
}

TEST_CASE("La mort de l'allie vise annule l'activation d'un soin, sans recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 35.0f}, 30);
    REQUIRE(escouade.declencherSkill(3, {}, carteOuverte(), generateur).empty());

    escouade.recevoirCoup(1, 1, {85.0f, 35.0f}, 1000);
    static_cast<void>(attaquerPendant(escouade, {}, 1.0f, generateur));

    REQUIRE(escouade.skillEstPret(3));
}

TEST_CASE("Zone : l'effet est evalue au depart, un monstre sorti de la zone n'est pas touche, un entre l'est") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    const Cercle dedans = cibleMonstre({150.0f, 100.0f});
    const Cercle dehors = cibleMonstre({300.0f, 100.0f});
    REQUIRE(escouade.declencherSkillDeZone(4, {150.0f, 100.0f}, {MonstreVu{1, dedans}, MonstreVu{2, dehors}}, carteOuverte(), generateur).empty());

    // Le monstre 1 était dans la zone à l'appui, le 2 y entre pendant l'activation.
    std::vector<Coup> coups = attaquerPendant(escouade, {MonstreVu{1, dedans}, MonstreVu{2, dehors}}, 0.7f, generateur);
    const std::vector<Coup> suite = attaquerPendant(escouade, {MonstreVu{1, dehors}, MonstreVu{2, dedans}}, 0.2f, generateur);
    coups.insert(coups.end(), suite.begin(), suite.end());

    REQUIRE(compter(coups, 15) == 1);
    REQUIRE(std::any_of(coups.begin(), coups.end(), [](const Coup& coup) { return coup.degats == 15 && coup.idMonstre == 2; }));
}

TEST_CASE("Provocation : l'effet est evalue au depart, apres 0,2 s") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    const Cercle dedans = cibleMonstre({100.0f, 60.0f});
    const Cercle dehors = cibleMonstre({600.0f, 60.0f});
    REQUIRE(escouade.declencherSkill(2, {MonstreVu{1, dedans}, MonstreVu{2, dehors}}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());

    static_cast<void>(attaquerPendant(escouade, {MonstreVu{1, dedans}, MonstreVu{2, dehors}}, 0.1f, generateur));
    REQUIRE(escouade.getProvocations().empty());
    static_cast<void>(attaquerPendant(escouade, {MonstreVu{1, dehors}, MonstreVu{2, dedans}}, 0.15f, generateur));

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 2);
}

TEST_CASE("Deux lanceurs peuvent activer en meme temps") {
    Generateur generateur = generateurDeTest();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, {85.0f, 35.0f}, 30);
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(1, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(3, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(1));
    REQUIRE_FALSE(escouade.skillEstPret(3));

    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.4f, generateur), 25) == 1);  // les dégâts sont partis
    REQUIRE(placeAlliePourSoin(escouade) == 1);                                         // pas encore le soin
    static_cast<void>(attaquerPendant(escouade, monstres, 0.3f, generateur));
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}
