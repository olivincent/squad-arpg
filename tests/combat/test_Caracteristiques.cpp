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

TEST_CASE("pointsDeVieMax : la base commune plus 6 par point d'Endurance, les autres caracteristiques sont sans effet") {
    const Caracteristiques caracteristiques({99, 14, 99, 99, 99});

    REQUIRE(pointsDeVieMax(caracteristiques, 1) == s_pointsDeVieDeBase + s_pvParPointDEndurance * 14);
    REQUIRE(pointsDeVieMax(Caracteristiques(), 1) == s_pointsDeVieDeBase);
}

TEST_CASE("Coefficients : par defaut tous a zero, seul() n'en pose qu'un") {
    const Coefficients vide;
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);

    for (const Caracteristique caracteristique : s_caracteristiques) {
        REQUIRE(vide.valeur(caracteristique) == 0.0f);
        REQUIRE(puissance.valeur(caracteristique) == (caracteristique == Caracteristique::Puissance ? 1.0f : 0.0f));
    }
}

TEST_CASE("montantEffectif : neutre quand la caracteristique vaut la reference") {
    const Coefficients coefficients = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    const Caracteristiques neutres({s_valeurDeReference, 0, 0, 0, 0});

    REQUIRE(montantEffectif(250, coefficients, neutres) == 250);
    REQUIRE(montantEffectif(0, coefficients, neutres) == 0);
}

TEST_CASE("montantEffectif : un coefficient nul ignore les caracteristiques") {
    const Caracteristiques fortes({40, 40, 40, 40, 40});

    REQUIRE(montantEffectif(250, Coefficients(), fortes) == 250);
}

TEST_CASE("montantEffectif : chaque point au-dessus de la reference ajoute s_bonusParPoint, chaque point en dessous le retire") {
    const Coefficients coefficients = Coefficients::seul(Caracteristique::Puissance, 1.0f);

    REQUIRE(montantEffectif(1000, coefficients, Caracteristiques({s_valeurDeReference + 3, 0, 0, 0, 0})) == 1090);
    REQUIRE(montantEffectif(1000, coefficients, Caracteristiques({s_valeurDeReference - 5, 0, 0, 0, 0})) == 850);
}

TEST_CASE("montantEffectif : le coefficient pondere et les caracteristiques s'additionnent") {
    const Coefficients moitie = Coefficients::seul(Caracteristique::Puissance, 0.5f);
    REQUIRE(montantEffectif(1000, moitie, Caracteristiques({s_valeurDeReference + 10, 0, 0, 0, 0})) == 1150);

    const Coefficients deux({1.0f, 0.0f, 0.0f, 1.0f, 0.0f});
    REQUIRE(montantEffectif(1000, deux, Caracteristiques({s_valeurDeReference + 2, 0, 0, s_valeurDeReference + 4, 0})) == 1180);
}

TEST_CASE("montantEffectif : arrondi au plus proche") {
    const Coefficients coefficients = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    const Caracteristiques plusUn({s_valeurDeReference + 1, 0, 0, 0, 0});

    REQUIRE(montantEffectif(50, coefficients, plusUn) == 52);  // 51,5
    REQUIRE(montantEffectif(10, coefficients, plusUn) == 10);  // 10,3
    REQUIRE(montantEffectif(20, coefficients, plusUn) == 21);  // 20,6
}

TEST_CASE("montantEffectif : jamais negatif") {
    const Coefficients coefficients = Coefficients::seul(Caracteristique::Puissance, 1.0f);

    REQUIRE(montantEffectif(100, coefficients, Caracteristiques({-1000, 0, 0, 0, 0})) == 0);
}

TEST_CASE("pointsDeVieMax : la base, l'Endurance et 8 points par niveau au-dela du premier") {
    const Caracteristiques caracteristiques = Caracteristiques::seule(Caracteristique::Endurance, 14);

    REQUIRE(pointsDeVieMax(caracteristiques, 1) == s_pointsDeVieDeBase + s_pvParPointDEndurance * 14);
    REQUIRE(pointsDeVieMax(caracteristiques, 2) == s_pointsDeVieDeBase + s_pvParPointDEndurance * 14 + s_pvParNiveau);
    REQUIRE(pointsDeVieMax(caracteristiques, 11) == s_pointsDeVieDeBase + s_pvParPointDEndurance * 14 + 10 * s_pvParNiveau);
    REQUIRE(pointsDeVieMax(Caracteristiques(), 6) == 240 + 5 * 8);
    REQUIRE(s_pvParNiveau == 8);
}
