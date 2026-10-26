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

// La barre d'expérience et F9 (étape 99.5).
TEST_CASE("progressionVersLeNiveauSuivant : 0 juste au seuil, la moitie au milieu, jamais 1") {
    for (int niveau = 1; niveau <= 20; ++niveau) {
        const int debut = experiencePourAtteindre(niveau);
        const int fin = experiencePourAtteindre(niveau + 1);
        INFO("niveau " << niveau);
        REQUIRE(progressionVersLeNiveauSuivant(debut) == 0.0f);
        REQUIRE(progressionVersLeNiveauSuivant(fin - 1) < 1.0f);
        REQUIRE(progressionVersLeNiveauSuivant(fin - 1) > 0.0f);
    }
    REQUIRE(progressionVersLeNiveauSuivant(experiencePourAtteindre(2) / 2) == 0.5f);
}

TEST_CASE("experienceManquante : de quoi atteindre exactement le niveau suivant, au moins 1") {
    REQUIRE(experienceManquante(0) == experiencePourAtteindre(2));
    for (int experience = 0; experience < 3000; experience += 7) {
        const int manquante = experienceManquante(experience);
        REQUIRE(manquante >= 1);
        REQUIRE(niveauPour(experience + manquante) == niveauPour(experience) + 1);
        REQUIRE(niveauPour(experience + manquante - 1) == niveauPour(experience));
        REQUIRE(experience + manquante == experiencePourAtteindre(niveauPour(experience) + 1));
    }
}

// Les points libres (étape 100.1).
TEST_CASE("pointsLibresRecus : rien au niveau 1, puis trois par niveau gagne") {
    REQUIRE(pointsLibresRecus(1) == 0);
    REQUIRE(pointsLibresRecus(2) == s_pointsLibresParNiveau);
    REQUIRE(pointsLibresRecus(5) == 4 * s_pointsLibresParNiveau);
    REQUIRE(s_pointsLibresParNiveau == 3);
}

TEST_CASE("pointsLibresDisponibles : les recus moins la somme de ce qui est depense") {
    const Caracteristiques depenses({2, 0, 1, 0, 0});

    REQUIRE(pointsLibresDisponibles(1, Caracteristiques()) == 0);
    REQUIRE(pointsLibresDisponibles(2, Caracteristiques()) == 3);
    REQUIRE(pointsLibresDisponibles(5, Caracteristiques()) == 12);
    REQUIRE(pointsLibresDisponibles(5, depenses) == 9);
    REQUIRE(pointsLibresDisponibles(2, depenses) == 0);  // tout depense
}

TEST_CASE("depensePermise : jusqu'aux points disponibles, pas un de plus") {
    const Caracteristiques dejaDepense = Caracteristiques::seule(Caracteristique::Puissance, 2);  // niveau 3 : 6 recus, 4 libres

    REQUIRE(depensePermise(3, dejaDepense, Caracteristiques({0, 4, 0, 0, 0})));
    REQUIRE(depensePermise(3, dejaDepense, Caracteristiques({1, 1, 1, 1, 0})));
    REQUIRE(depensePermise(3, dejaDepense, Caracteristiques()));
    REQUIRE_FALSE(depensePermise(3, dejaDepense, Caracteristiques({0, 5, 0, 0, 0})));
    REQUIRE_FALSE(depensePermise(1, Caracteristiques(), Caracteristiques::seule(Caracteristique::Savoir, 1)));
}

TEST_CASE("depensePermise : une valeur negative est refusee, meme si la somme est permise") {
    REQUIRE_FALSE(depensePermise(5, Caracteristiques(), Caracteristiques({-1, 0, 0, 0, 0})));
    REQUIRE_FALSE(depensePermise(5, Caracteristiques(), Caracteristiques({4, -1, 0, 0, 0})));  // somme 3, mais une reprise
}
