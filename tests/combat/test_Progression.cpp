// ./tests/combat/test_Progression.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Caracteristiques.hpp"
#include "combat/Progression.hpp"

// La courbe de l'expérience (étape 99.3) : k × n × (n − 1).
TEST_CASE("experiencePourAtteindre : k x n x (n - 1), 0 pour le niveau 1") {
    REQUIRE(experiencePourAtteindre(1) == 0);
    REQUIRE(experiencePourAtteindre(2) == 2 * s_experienceDeLaCourbe);
    REQUIRE(experiencePourAtteindre(3) == 6 * s_experienceDeLaCourbe);
    REQUIRE(experiencePourAtteindre(10) == 90 * s_experienceDeLaCourbe);
}

TEST_CASE("niveauPour : niveau 1 sans experience, et juste avant le seuil du niveau 2") {
    REQUIRE(niveauPour(0) == 1);
    REQUIRE(niveauPour(experiencePourAtteindre(2) - 1) == 1);
}

TEST_CASE("niveauPour : juste a un seuil on a le niveau, juste avant on a le precedent") {
    for (int niveau = 2; niveau <= 60; ++niveau) {
        INFO("niveau " << niveau);
        REQUIRE(niveauPour(experiencePourAtteindre(niveau) - 1) == niveau - 1);
        REQUIRE(niveauPour(experiencePourAtteindre(niveau)) == niveau);
        REQUIRE(niveauPour(experiencePourAtteindre(niveau + 1) - 1) == niveau);
    }
}

TEST_CASE("niveauPour : une experience enorme donne un niveau sans plafond") {
    REQUIRE(niveauPour(experiencePourAtteindre(500)) == 500);
}

TEST_CASE("niveauPour : ne baisse jamais quand l'experience monte") {
    int precedent = 1;
    for (int experience = 0; experience < 5000; ++experience) {
        const int niveau = niveauPour(experience);
        REQUIRE(niveau >= precedent);
        precedent = niveau;
    }
}

// La croissance automatique (étape 99.4).
TEST_CASE("croissanceAuNiveau : rien au niveau 1") {
    const PaireDeCroissance paire = {Caracteristique::Puissance, Caracteristique::Endurance};

    REQUIRE(croissanceAuNiveau(1, paire) == Caracteristiques());
}

TEST_CASE("croissanceAuNiveau : les niveaux 2, 3 et 4 donnent la premiere, la seconde, puis la premiere de la paire") {
    const PaireDeCroissance paire = {Caracteristique::Adresse, Caracteristique::Volonte};

    REQUIRE(croissanceAuNiveau(2, paire) == Caracteristiques::seule(Caracteristique::Adresse, 1));
    REQUIRE(croissanceAuNiveau(3, paire) == Caracteristiques({0, 0, 1, 0, 1}));
    REQUIRE(croissanceAuNiveau(4, paire) == Caracteristiques({0, 0, 2, 0, 1}));
    REQUIRE(croissanceAuNiveau(5, paire) == Caracteristiques({0, 0, 2, 0, 2}));
}

TEST_CASE("croissanceAuNiveau : chaque niveau gagne exactement un point, dans la paire seulement") {
    const PaireDeCroissance paire = {Caracteristique::Puissance, Caracteristique::Savoir};
    for (int niveau = 1; niveau <= 30; ++niveau) {
        const Caracteristiques croissance = croissanceAuNiveau(niveau, paire);
        int total = 0;
        for (const Caracteristique caracteristique : s_caracteristiques) {
            total += croissance.valeur(caracteristique);
        }
        REQUIRE(total == niveau - 1);
        REQUIRE(croissance.valeur(Caracteristique::Endurance) == 0);
        REQUIRE(croissance.valeur(Caracteristique::Adresse) == 0);
        REQUIRE(croissance.valeur(Caracteristique::Volonte) == 0);
    }
}
