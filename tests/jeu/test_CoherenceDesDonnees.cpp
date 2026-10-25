// ./tests/jeu/test_CoherenceDesDonnees.cpp
#include <catch2/catch_test_macros.hpp>

#include "escouade/Specialite.hpp"
#include "objets/CatalogueObjets.hpp"

// Ces tests vérifient les données des catalogues, pas du code : chaque donnée ajoutée plus tard (monstres,
// compositions de groupes) y trouve sa vérification.

TEST_CASE("chaque classe atteint les seuils de tout son equipement de depart") {
    for (const Specialite& classe : catalogue()) {
        for (const ObjetDeDepart& objet : classe.definition.equipementDeDepart) {
            INFO(classe.nom << " porte " << definitionObjet(objet.objet).nom);
            REQUIRE(seuilsAtteints(objet.objet, classe.definition.caracteristiques));
        }
    }
}
