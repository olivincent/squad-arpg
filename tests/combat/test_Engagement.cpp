// ./tests/combat/test_Engagement.cpp
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <vector>

#include "combat/Engagement.hpp"

namespace {

// Toutes les entités ont un rayon de 10 px : A engage B quand leurs centres sont à
// 10 + 10 + zone = 40 px ou moins (zone de 20 px).
constexpr float s_rayon = 10.0f;
constexpr float s_zone = 20.0f;

// La règle est la même dans les deux sens ; ces tests prennent celui d'un monstre qui frappe un personnage de l'escouade.
// Les numéros des cas deviennent des identifiants de monstre (les attaquants) et des places (les cibles).
using Photo = PhotoDesCamps<IdMonstre, Place>;
using Coups = std::vector<CoupDOpportunite<IdMonstre, Place>>;

MonstreDeControle unMonstre(int id, float x, float y, float zone = 0.0f) {
    return {IdMonstre{id}, Cercle{{x, y}, s_rayon}, zone};
}

// Une cible : le personnage de cette place.
PersonnageDeControle entite(int place, float x, float y, float zone = 0.0f) {
    return {Place{static_cast<std::size_t>(place)}, Cercle{{x, y}, s_rayon}, zone};
}

// Un attaquant (zone de 20 px) en (0, 0), les cibles à la position donnée.
MonstreDeControle attaquant(float x = 0.0f, float y = 0.0f) {
    return unMonstre(1, x, y, s_zone);
}

bool memeCoup(const CoupDOpportunite<IdMonstre, Place>& coup, int idAttaquant, int idCible) {
    return coup.idAttaquant == IdMonstre{idAttaquant} && coup.idCible == Place{static_cast<std::size_t>(idCible)};
}

}  // namespace

TEST_CASE("engage : zone positive et ecart bord a bord dans la zone") {
    REQUIRE(engage(attaquant(), entite(2, 40.0f, 0.0f)));         // écart 20 = zone : engagé
    REQUIRE_FALSE(engage(attaquant(), entite(2, 40.5f, 0.0f)));   // écart 20,5 : hors zone
    REQUIRE(engage(attaquant(), entite(2, 5.0f, 0.0f)));          // chevauchement : écart 0
    REQUIRE_FALSE(engage(entite(1, 0.0f, 0.0f, 0.0f), entite(2, 20.0f, 0.0f)));  // zone 0 : n'engage personne
}

TEST_CASE("engage n'est pas symetrique") {
    const MonstreDeControle melee = attaquant();
    const PersonnageDeControle tireur = entite(2, 30.0f, 0.0f);  // zone 0

    REQUIRE(engage(melee, tireur));
    REQUIRE_FALSE(engage(tireur, melee));
}

TEST_CASE("la cible qui sort de la zone en s'eloignant recoit un coup gratuit") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 60.0f, 0.0f)}};

    const Coups coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 2));
}

TEST_CASE("l'attaquant qui s'eloigne lui-meme ne frappe pas") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
    const Photo apres{{attaquant(-40.0f, 0.0f)}, {entite(2, 30.0f, 0.0f)}};  // A recule de 40 px, B ne bouge pas

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("les deux s'eloignent : le plus rapide a fui, a egalite la cible a fui") {
    SECTION("la cible s'eloigne davantage") {
        const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
        const Photo apres{{attaquant(-10.0f, 0.0f)}, {entite(2, 60.0f, 0.0f)}};  // A -10, B +30

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
    SECTION("l'attaquant s'eloigne davantage") {
        const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
        const Photo apres{{attaquant(-30.0f, 0.0f)}, {entite(2, 40.0f, 0.0f)}};  // A -30, B +10

        REQUIRE(coupsDOpportunite(avant, apres).empty());
    }
    SECTION("egalite") {
        const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
        const Photo apres{{attaquant(-20.0f, 0.0f)}, {entite(2, 50.0f, 0.0f)}};  // A -20, B +20

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
}

TEST_CASE("le deplacement est projete sur la droite attaquant-cible") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};

    SECTION("un pas de cote de la cible, l'attaquant immobile : egalite, la cible a fui") {
        const Photo apres{{attaquant()}, {entite(2, 30.0f, 50.0f)}};

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
    SECTION("l'attaquant recule de 5 px, la cible fait un pas de cote : l'attaquant a davantage fui") {
        const Photo apres{{attaquant(-5.0f, 0.0f)}, {entite(2, 30.0f, 60.0f)}};

        REQUIRE(coupsDOpportunite(avant, apres).empty());
    }
    SECTION("la cible recule de 5 px en faisant un pas de cote, l'attaquant immobile : la cible a fui") {
        const Photo apres{{attaquant()}, {entite(2, 35.0f, 60.0f)}};

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
}

TEST_CASE("un attaquant qui engage encore un autre adversaire a la fin ne frappe pas") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, -30.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 80.0f, 0.0f), entite(3, -30.0f, 0.0f)}};  // 2 fuit, 3 reste

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("deux fuyards simultanes : un seul coup, sur le plus proche a la fin") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, -30.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 100.0f, 0.0f), entite(3, -60.0f, 0.0f)}};

    const Coups coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 3));
}

TEST_CASE("deux fuyards a egale distance : le premier de la liste") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, -30.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 70.0f, 0.0f), entite(3, -70.0f, 0.0f)}};

    const Coups coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 2));
}

TEST_CASE("une cible qui n'etait pas engagee au debut n'est pas frappee") {
    // 3 est à 200 px au début : aucune paire à rompre. Seul 2, qui fuit, est frappé.
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, 200.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 60.0f, 0.0f), entite(3, 300.0f, 0.0f)}};

    const Coups coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 2));
}

TEST_CASE("un attaquant sans zone ne frappe jamais gratuitement") {
    const MonstreDeControle tireur = unMonstre(1, 0.0f, 0.0f, 0.0f);
    const Photo avant{{tireur}, {entite(2, 30.0f, 0.0f)}};
    const Photo apres{{tireur}, {entite(2, 200.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("entite absente de la photo de fin : rien") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};

    SECTION("la cible est morte") {
        REQUIRE(coupsDOpportunite(avant, Photo{{attaquant()}, {}}).empty());
    }
    SECTION("l'attaquant est mort") {
        REQUIRE(coupsDOpportunite(avant, Photo{{}, {entite(2, 200.0f, 0.0f)}}).empty());
    }
    SECTION("les deux sont absents") {
        REQUIRE(coupsDOpportunite(avant, Photo{{}, {}}).empty());
    }
}

TEST_CASE("une paire qui reste engagee : rien") {
    const Photo avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 40.0f, 0.0f)}};  // recule un peu, écart 20 = encore dans la zone

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("l'entree dans la zone ne donne rien") {
    const Photo avant{{attaquant()}, {entite(2, 100.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 30.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("une entite absente du debut ne frappe pas et n'est pas frappee") {
    const Photo avant{{}, {}};
    const Photo apres{{attaquant()}, {entite(2, 200.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("chaque attaquant a son propre coup, meme sur la meme cible") {
    const MonstreDeControle a1 = unMonstre(1, 0.0f, 0.0f, s_zone);
    const MonstreDeControle a2 = unMonstre(2, 0.0f, 20.0f, s_zone);
    const Photo avant{{a1, a2}, {entite(7, 30.0f, 10.0f)}};
    const Photo apres{{a1, a2}, {entite(7, 100.0f, 10.0f)}};

    const Coups coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 2);
    REQUIRE(memeCoup(coups[0], 1, 7));
    REQUIRE(memeCoup(coups[1], 2, 7));
}

TEST_CASE("chaque attaquant est juge pour lui-meme : l'un frappe, l'autre reste au contact") {
    const MonstreDeControle a1 = unMonstre(1, 0.0f, 0.0f, s_zone);
    const MonstreDeControle a2 = unMonstre(2, 500.0f, 0.0f, s_zone);
    // La cible 8 est au contact de a1 et fuit ; la cible 9 est au contact de a2 et reste.
    const Photo avant{{a1, a2}, {entite(8, 30.0f, 0.0f), entite(9, 530.0f, 0.0f)}};
    const Photo apres{{a1, a2}, {entite(8, 100.0f, 0.0f), entite(9, 530.0f, 0.0f)}};

    const Coups coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 8));
}

TEST_CASE("deux centres confondus : personne n'a de direction, aucun coup") {
    const Photo avant{{attaquant()}, {entite(2, 0.0f, 0.0f)}};
    const Photo apres{{attaquant()}, {entite(2, 100.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

// --- monstreDontSeDegager ---------------------------------------------------------------
// Monstres de zone 20 px, personnages de rayon 10 : un monstre engage un personnage dont le
// centre est à 40 px ou moins du sien.

namespace {

MonstreDeControle monstre(int id, float x, float y) {
    return unMonstre(id, x, y, s_zone);
}

PersonnageDeControle tireur(int place, float x, float y) {
    return entite(place, x, y, 0.0f);
}

PersonnageDeControle melee(int place, float x, float y) {
    return entite(place, x, y, s_zone);
}

}  // namespace

TEST_CASE("degagement : engage par un monstre tenu par un autre personnage") {
    const PersonnageDeControle mage = tireur(3, 30.0f, 0.0f);
    const PersonnageDeControle guerrier = melee(1, -30.0f, 0.0f);

    const std::optional<IdMonstre> id = monstreDontSeDegager(mage, {guerrier, mage}, {monstre(7, 0.0f, 0.0f)});

    REQUIRE(id == IdMonstre{7});
}

TEST_CASE("degagement : seul au contact, il reste") {
    const PersonnageDeControle mage = tireur(3, 30.0f, 0.0f);
    const PersonnageDeControle guerrier = melee(1, 500.0f, 0.0f);  // loin du monstre

    REQUIRE_FALSE(monstreDontSeDegager(mage, {guerrier, mage}, {monstre(7, 0.0f, 0.0f)}).has_value());
}

TEST_CASE("degagement : engage par deux monstres dont un seul est tenu, il reste") {
    const PersonnageDeControle mage = tireur(3, 0.0f, 30.0f);
    const PersonnageDeControle guerrier = melee(1, -30.0f, -10.0f);
    // Le monstre 7 (en 0,0) touche le guerrier et le mage ; le monstre 8 (en 30,30) touche seulement le mage.
    const std::vector<MonstreDeControle> monstres{monstre(7, 0.0f, 0.0f), monstre(8, 30.0f, 30.0f)};

    REQUIRE(engage(monstres[0], guerrier));
    REQUIRE(engage(monstres[0], mage));
    REQUIRE(engage(monstres[1], mage));
    REQUIRE_FALSE(engage(monstres[1], guerrier));
    REQUIRE_FALSE(monstreDontSeDegager(mage, {guerrier, mage}, monstres).has_value());
}

TEST_CASE("degagement : les deux monstres sont tenus, par le meme personnage ou par deux") {
    const PersonnageDeControle mage = tireur(3, 0.0f, 30.0f);
    const std::vector<MonstreDeControle> monstres{monstre(7, 0.0f, 0.0f), monstre(8, 30.0f, 30.0f)};

    SECTION("par le meme personnage") {
        const PersonnageDeControle guerrier = melee(1, 30.0f, 0.0f);  // dans la zone des deux monstres

        REQUIRE(engage(monstres[0], guerrier));
        REQUIRE(engage(monstres[1], guerrier));
        REQUIRE(monstreDontSeDegager(mage, {guerrier, mage}, monstres).has_value());
    }
    SECTION("par deux personnages") {
        const PersonnageDeControle guerrier = melee(1, -30.0f, -10.0f);   // tient le monstre 7
        const PersonnageDeControle paladin = melee(2, 60.0f, 30.0f);      // tient le monstre 8

        REQUIRE_FALSE(engage(monstres[0], paladin));
        REQUIRE_FALSE(engage(monstres[1], guerrier));

        REQUIRE(monstreDontSeDegager(mage, {guerrier, paladin, mage}, monstres).has_value());
    }
}

TEST_CASE("degagement : pas engage, rien") {
    const PersonnageDeControle mage = tireur(3, 300.0f, 0.0f);
    const PersonnageDeControle guerrier = melee(1, -30.0f, 0.0f);

    REQUIRE_FALSE(monstreDontSeDegager(mage, {guerrier, mage}, {monstre(7, 0.0f, 0.0f)}).has_value());
    REQUIRE_FALSE(monstreDontSeDegager(mage, {guerrier, mage}, {}).has_value());
}

TEST_CASE("degagement : le personnage lui-meme ne compte pas comme tenant le monstre") {
    const PersonnageDeControle mage = tireur(3, 30.0f, 0.0f);

    SECTION("liste qui le contient") {
        REQUIRE_FALSE(monstreDontSeDegager(mage, {mage}, {monstre(7, 0.0f, 0.0f)}).has_value());
    }
    SECTION("liste vide") {
        REQUIRE_FALSE(monstreDontSeDegager(mage, {}, {monstre(7, 0.0f, 0.0f)}).has_value());
    }
}

TEST_CASE("degagement : un personnage avec une zone de controle ne se degage jamais") {
    const PersonnageDeControle guerrier = melee(1, 30.0f, 0.0f);
    const PersonnageDeControle paladin = melee(2, -30.0f, 0.0f);

    REQUIRE_FALSE(monstreDontSeDegager(guerrier, {guerrier, paladin}, {monstre(7, 0.0f, 0.0f)}).has_value());
}

TEST_CASE("degagement : le monstre choisi est le plus proche de ceux qui l'engagent") {
    const PersonnageDeControle mage = tireur(3, 0.0f, 30.0f);
    const PersonnageDeControle guerrier = melee(1, 20.0f, 10.0f);  // dans la zone des deux monstres
    // Écarts bord à bord au mage : monstre 8 (en 40,30) : 40 - 20 = 20 ; monstre 7 (en 0,0) : 30 - 20 = 10.
    const std::vector<MonstreDeControle> monstres{monstre(8, 40.0f, 30.0f), monstre(7, 0.0f, 0.0f)};

    REQUIRE(monstreDontSeDegager(mage, {guerrier, mage}, monstres) == IdMonstre{7});
}

TEST_CASE("degagement : a egalite de distance, le premier monstre de la liste") {
    const PersonnageDeControle mage = tireur(3, 0.0f, 0.0f);
    const PersonnageDeControle guerrier = melee(1, 0.0f, 0.0f + 5.0f);
    const std::vector<MonstreDeControle> monstres{monstre(9, 30.0f, 0.0f), monstre(4, -30.0f, 0.0f)};

    REQUIRE(monstreDontSeDegager(mage, {guerrier, mage}, monstres) == IdMonstre{9});
}

TEST_CASE("le sens inverse : un personnage engage un monstre qui fuit") {
    // La règle est la même dans les deux sens : ici l'attaquant est la place 1 (zone de 20 px), la cible le monstre 7.
    const PhotoDesCamps<Place, IdMonstre> avant{{{Place{1}, Cercle{{0.0f, 0.0f}, s_rayon}, s_zone}}, {unMonstre(7, 30.0f, 0.0f)}};
    const PhotoDesCamps<Place, IdMonstre> apres{{{Place{1}, Cercle{{0.0f, 0.0f}, s_rayon}, s_zone}}, {unMonstre(7, 60.0f, 0.0f)}};

    const std::vector<CoupDOpportunite<Place, IdMonstre>> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idAttaquant == Place{1});
    REQUIRE(coups[0].idCible == IdMonstre{7});
}
