// ./tests/combat/test_Caracteristiques.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Caracteristiques.hpp"

TEST_CASE("Caracteristiques : par defaut, toutes a zero") {
    const Caracteristiques vide;

    for (const Caracteristique caracteristique : s_caracteristiques) {
        REQUIRE(vide.valeur(caracteristique) == 0);
    }
}

TEST_CASE("Caracteristiques : les valeurs suivent l'ordre de l'enum") {
    const Caracteristiques caracteristiques({1, 2, 3, 4, 5});

    REQUIRE(caracteristiques.valeur(Caracteristique::Puissance) == 1);
    REQUIRE(caracteristiques.valeur(Caracteristique::Endurance) == 2);
    REQUIRE(caracteristiques.valeur(Caracteristique::Adresse) == 3);
    REQUIRE(caracteristiques.valeur(Caracteristique::Savoir) == 4);
    REQUIRE(caracteristiques.valeur(Caracteristique::Volonte) == 5);
}

TEST_CASE("Caracteristiques::seule ne touche qu'une caracteristique") {
    const Caracteristiques bonus = Caracteristiques::seule(Caracteristique::Savoir, 3);

    for (const Caracteristique caracteristique : s_caracteristiques) {
        REQUIRE(bonus.valeur(caracteristique) == (caracteristique == Caracteristique::Savoir ? 3 : 0));
    }
}

TEST_CASE("Caracteristiques : l'addition se fait caracteristique par caracteristique") {
    const Caracteristiques a({1, 2, 3, 4, 5});
    const Caracteristiques b({10, 0, -3, 0, 1});

    const Caracteristiques somme = a + b;

    REQUIRE(somme == Caracteristiques({11, 2, 0, 4, 6}));
    REQUIRE(a == Caracteristiques({1, 2, 3, 4, 5}));  // les opérandes ne changent pas
}

TEST_CASE("Caracteristiques : += accumule") {
    Caracteristiques total;
    total += Caracteristiques::seule(Caracteristique::Puissance, 3);
    total += Caracteristiques::seule(Caracteristique::Puissance, 2);

    REQUIRE(total.valeur(Caracteristique::Puissance) == 5);
    REQUIRE(total.valeur(Caracteristique::Endurance) == 0);
}

TEST_CASE("pointsDeVieMax : la base plus 20 par point d'Endurance, les autres caracteristiques sont sans effet") {
    const Caracteristiques caracteristiques({99, 14, 99, 99, 99});

    REQUIRE(pointsDeVieMax(720, caracteristiques) == 720 + s_pvParPointDEndurance * 14);
    REQUIRE(pointsDeVieMax(720, Caracteristiques()) == 720);
}
