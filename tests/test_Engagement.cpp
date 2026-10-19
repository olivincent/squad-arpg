// ./tests/test_Engagement.cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "Engagement.hpp"

namespace {

// Toutes les entités ont un rayon de 10 px : A engage B quand leurs centres sont à
// 10 + 10 + zone = 40 px ou moins (zone de 20 px).
constexpr float s_rayon = 10.0f;
constexpr float s_zone = 20.0f;

EntiteDeControle entite(int id, float x, float y, float zone = 0.0f) {
    return {id, Cercle{{x, y}, s_rayon}, zone};
}

// Un attaquant (zone de 20 px) en (0, 0), les cibles à la position donnée.
EntiteDeControle attaquant(float x = 0.0f, float y = 0.0f) {
    return entite(1, x, y, s_zone);
}

bool memeCoup(const CoupDOpportunite& coup, int idAttaquant, int idCible) {
    return coup.idAttaquant == idAttaquant && coup.idCible == idCible;
}

}  // namespace

TEST_CASE("engage : zone positive et ecart bord a bord dans la zone") {
    REQUIRE(engage(attaquant(), entite(2, 40.0f, 0.0f)));         // écart 20 = zone : engagé
    REQUIRE_FALSE(engage(attaquant(), entite(2, 40.5f, 0.0f)));   // écart 20,5 : hors zone
    REQUIRE(engage(attaquant(), entite(2, 5.0f, 0.0f)));          // chevauchement : écart 0
    REQUIRE_FALSE(engage(entite(1, 0.0f, 0.0f, 0.0f), entite(2, 20.0f, 0.0f)));  // zone 0 : n'engage personne
}

TEST_CASE("engage n'est pas symetrique") {
    const EntiteDeControle melee = attaquant();
    const EntiteDeControle tireur = entite(2, 30.0f, 0.0f);  // zone 0

    REQUIRE(engage(melee, tireur));
    REQUIRE_FALSE(engage(tireur, melee));
}

TEST_CASE("la cible qui sort de la zone en s'eloignant recoit un coup gratuit") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 60.0f, 0.0f)}};

    const std::vector<CoupDOpportunite> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 2));
}

TEST_CASE("l'attaquant qui s'eloigne lui-meme ne frappe pas") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant(-40.0f, 0.0f)}, {entite(2, 30.0f, 0.0f)}};  // A recule de 40 px, B ne bouge pas

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("les deux s'eloignent : le plus rapide a fui, a egalite la cible a fui") {
    SECTION("la cible s'eloigne davantage") {
        const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
        const PhotoDesCamps apres{{attaquant(-10.0f, 0.0f)}, {entite(2, 60.0f, 0.0f)}};  // A -10, B +30

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
    SECTION("l'attaquant s'eloigne davantage") {
        const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
        const PhotoDesCamps apres{{attaquant(-30.0f, 0.0f)}, {entite(2, 40.0f, 0.0f)}};  // A -30, B +10

        REQUIRE(coupsDOpportunite(avant, apres).empty());
    }
    SECTION("egalite") {
        const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
        const PhotoDesCamps apres{{attaquant(-20.0f, 0.0f)}, {entite(2, 50.0f, 0.0f)}};  // A -20, B +20

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
}

TEST_CASE("le deplacement est projete sur la droite attaquant-cible") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};

    SECTION("un pas de cote de la cible, l'attaquant immobile : egalite, la cible a fui") {
        const PhotoDesCamps apres{{attaquant()}, {entite(2, 30.0f, 50.0f)}};

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
    SECTION("l'attaquant recule de 5 px, la cible fait un pas de cote : l'attaquant a davantage fui") {
        const PhotoDesCamps apres{{attaquant(-5.0f, 0.0f)}, {entite(2, 30.0f, 60.0f)}};

        REQUIRE(coupsDOpportunite(avant, apres).empty());
    }
    SECTION("la cible recule de 5 px en faisant un pas de cote, l'attaquant immobile : la cible a fui") {
        const PhotoDesCamps apres{{attaquant()}, {entite(2, 35.0f, 60.0f)}};

        REQUIRE(coupsDOpportunite(avant, apres).size() == 1);
    }
}

TEST_CASE("un attaquant qui engage encore un autre adversaire a la fin ne frappe pas") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, -30.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 80.0f, 0.0f), entite(3, -30.0f, 0.0f)}};  // 2 fuit, 3 reste

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("deux fuyards simultanes : un seul coup, sur le plus proche a la fin") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, -30.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 100.0f, 0.0f), entite(3, -60.0f, 0.0f)}};

    const std::vector<CoupDOpportunite> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 3));
}

TEST_CASE("deux fuyards a egale distance : le premier de la liste") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, -30.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 70.0f, 0.0f), entite(3, -70.0f, 0.0f)}};

    const std::vector<CoupDOpportunite> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 2));
}

TEST_CASE("une cible qui n'etait pas engagee au debut n'est pas frappee") {
    // 3 est à 200 px au début : aucune paire à rompre. Seul 2, qui fuit, est frappé.
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f), entite(3, 200.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 60.0f, 0.0f), entite(3, 300.0f, 0.0f)}};

    const std::vector<CoupDOpportunite> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 2));
}

TEST_CASE("un attaquant sans zone ne frappe jamais gratuitement") {
    const EntiteDeControle tireur = entite(1, 0.0f, 0.0f, 0.0f);
    const PhotoDesCamps avant{{tireur}, {entite(2, 30.0f, 0.0f)}};
    const PhotoDesCamps apres{{tireur}, {entite(2, 200.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("entite absente de la photo de fin : rien") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};

    SECTION("la cible est morte") {
        REQUIRE(coupsDOpportunite(avant, PhotoDesCamps{{attaquant()}, {}}).empty());
    }
    SECTION("l'attaquant est mort") {
        REQUIRE(coupsDOpportunite(avant, PhotoDesCamps{{}, {entite(2, 200.0f, 0.0f)}}).empty());
    }
    SECTION("les deux sont absents") {
        REQUIRE(coupsDOpportunite(avant, PhotoDesCamps{{}, {}}).empty());
    }
}

TEST_CASE("une paire qui reste engagee : rien") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 30.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 40.0f, 0.0f)}};  // recule un peu, écart 20 = encore dans la zone

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("l'entree dans la zone ne donne rien") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 100.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 30.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("une entite absente du debut ne frappe pas et n'est pas frappee") {
    const PhotoDesCamps avant{{}, {}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 200.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}

TEST_CASE("chaque attaquant a son propre coup, meme sur la meme cible") {
    const EntiteDeControle a1 = entite(1, 0.0f, 0.0f, s_zone);
    const EntiteDeControle a2 = entite(2, 0.0f, 20.0f, s_zone);
    const PhotoDesCamps avant{{a1, a2}, {entite(7, 30.0f, 10.0f)}};
    const PhotoDesCamps apres{{a1, a2}, {entite(7, 100.0f, 10.0f)}};

    const std::vector<CoupDOpportunite> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 2);
    REQUIRE(memeCoup(coups[0], 1, 7));
    REQUIRE(memeCoup(coups[1], 2, 7));
}

TEST_CASE("chaque attaquant est juge pour lui-meme : l'un frappe, l'autre reste au contact") {
    const EntiteDeControle a1 = entite(1, 0.0f, 0.0f, s_zone);
    const EntiteDeControle a2 = entite(2, 500.0f, 0.0f, s_zone);
    // La cible 8 est au contact de a1 et fuit ; la cible 9 est au contact de a2 et reste.
    const PhotoDesCamps avant{{a1, a2}, {entite(8, 30.0f, 0.0f), entite(9, 530.0f, 0.0f)}};
    const PhotoDesCamps apres{{a1, a2}, {entite(8, 100.0f, 0.0f), entite(9, 530.0f, 0.0f)}};

    const std::vector<CoupDOpportunite> coups = coupsDOpportunite(avant, apres);

    REQUIRE(coups.size() == 1);
    REQUIRE(memeCoup(coups[0], 1, 8));
}

TEST_CASE("deux centres confondus : personne n'a de direction, aucun coup") {
    const PhotoDesCamps avant{{attaquant()}, {entite(2, 0.0f, 0.0f)}};
    const PhotoDesCamps apres{{attaquant()}, {entite(2, 100.0f, 0.0f)}};

    REQUIRE(coupsDOpportunite(avant, apres).empty());
}
