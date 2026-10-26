// ./tests/jeu/test_CalibrageDeLExperience.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

#include "base/Aleatoire.hpp"
#include "combat/Progression.hpp"
#include "jeu/Expedition.hpp"
#include "monde/GenerateurDonjon.hpp"
#include "monde/Niveau.hpp"
#include "monstres/DefinitionDeMonstre.hpp"

// La cible de calibrage de la courbe d'expérience (étape 99.3) : en tuant tous les monstres de chaque étage, l'escouade arrive
// vers le niveau 3 à 4 à la fin du 1er étage et au niveau 10 à 11 à la fin du 5e. Seul `s_experienceDeLaCourbe` s'ajuste.
namespace {
constexpr int s_nombreDeGraines = 300;  // des cartes générées : assez pour que la moyenne soit stable

// L'expérience moyenne d'un étage de cette profondeur : celle de tous ses monstres, renforcés par la profondeur.
double experienceMoyenneDUnEtage(int profondeur) {
    long total = 0;
    for (int graine = 1; graine <= s_nombreDeGraines; ++graine) {
        Generateur generateur(graineDEtage(static_cast<Generateur::result_type>(graine), profondeur));
        const Niveau niveau = genererNiveau(generateur, ParametresDonjon{});
        for (const MonstreDuNiveau& monstre : niveau.monstres) {
            total += renforcer(definitionDeMonstre(monstre.categorie), profondeur).experience;
        }
    }
    return static_cast<double>(total) / s_nombreDeGraines;
}
}  // namespace

TEST_CASE("calibrage : tous les monstres tues, niveau 3 a 4 apres le 1er etage et 10 a 11 apres le 5e") {
    double cumul = 0.0;
    std::vector<int> niveaux;
    for (int profondeur = 1; profondeur <= s_profondeurMaximale; ++profondeur) {
        cumul += experienceMoyenneDUnEtage(profondeur);
        niveaux.push_back(niveauPour(static_cast<int>(cumul)));
    }

    REQUIRE(niveaux.size() == static_cast<std::size_t>(s_profondeurMaximale));
    REQUIRE(niveaux.front() >= 3);
    REQUIRE(niveaux.front() <= 4);
    REQUIRE(niveaux.back() >= 10);
    REQUIRE(niveaux.back() <= 11);
    for (std::size_t i = 1; i < niveaux.size(); ++i) {
        REQUIRE(niveaux[i] > niveaux[i - 1]);  // chaque étage fait monter
    }
}
