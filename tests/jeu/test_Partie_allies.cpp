// ./tests/jeu/test_Partie_allies.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/AllieVu.hpp"
#include "monde/Niveau.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"

// La vue des alliés du groupe (étape 107.3) : ce que `Partie` passe à un monstre de ses voisins de groupe.

namespace {
// Trois monstres loin de l'escouade : deux dans le groupe 0 (les indices 0 et 1), un dans le groupe 1 (l'indice 2).
Partie unePartieDeTroisMonstres() {
    return unePartie(niveauDegageEnGroupes({{{480.0f, 480.0f}, 0}, {{520.0f, 520.0f}, 0}, {{560.0f, 480.0f}, 1}}), s_graineDeTest);
}
}  // namespace

TEST_CASE("alliesDuGroupe : les autres monstres du meme groupe, avec leur cercle et leurs points de vie") {
    const Partie partie = unePartieDeTroisMonstres();
    const Monstre& premier = partie.getMonstres()[0];
    const Monstre& deuxieme = partie.getMonstres()[1];

    const std::vector<AllieVu> allies = partie.alliesDuGroupe(premier);

    REQUIRE(allies.size() == 1);
    REQUIRE(allies.front().id == deuxieme.getId());
    verifierPointProche(allies.front().cercle.centre, deuxieme.getCentre());
    REQUIRE(allies.front().cercle.rayon == deuxieme.getCercle().rayon);
    REQUIRE(allies.front().pointsDeVie == deuxieme.getPointsDeVie());
    REQUIRE(allies.front().pointsDeVieMax == deuxieme.getPointsDeVieMax());
}

TEST_CASE("alliesDuGroupe : jamais le monstre lui-meme ni un monstre d'un autre groupe") {
    const Partie partie = unePartieDeTroisMonstres();

    for (const Monstre& monstre : partie.getMonstres()) {
        for (const AllieVu& allie : partie.alliesDuGroupe(monstre)) {
            REQUIRE(allie.id != monstre.getId());
        }
    }
    REQUIRE(partie.alliesDuGroupe(partie.getMonstres()[2]).empty());  // seul dans son groupe
}

TEST_CASE("alliesDuGroupe : un monstre mort n'y est plus") {
    Partie partie = unePartie(niveauDegageEnGroupes({{{235.0f, 210.0f}, 0}, {{560.0f, 560.0f}, 0}}), s_graineDeTest);
    REQUIRE(partie.alliesDuGroupe(partie.getMonstres().back()).size() == 1);
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && partie.getMonstres().size() > 1; ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.getMonstres().size() == 1);
    REQUIRE(partie.alliesDuGroupe(partie.getMonstres().front()).empty());
}

TEST_CASE("alliesDuGroupe : un monstre qui rentre a son poste n'y est pas") {
    // Deux monstres du même groupe sur un couloir : le premier abandonne et rentre avant le second (son poste est plus loin à l'est).
    const Niveau niveau{std::vector<std::string>(15, std::string(60, '.')), 40.0f, {550.0f, 290.0f},
                        {{s_posteDuFuyard, 0}, {{s_posteDuFuyard.x + 200.0f, s_posteDuFuyard.y}, 0}}};
    Partie partie = unePartie(niveau, s_graineDeTest);
    const Monstre& premier = partie.getMonstres()[0];
    const Monstre& deuxieme = partie.getMonstres()[1];
    REQUIRE(partie.alliesDuGroupe(deuxieme).size() == 1);

    partie.ordonnerDeplacement({1500.0f, 300.0f});
    for (int i = 0; i < nombreDeTics(15.0f) && premier.estCiblable(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE_FALSE(premier.estCiblable());
    REQUIRE(deuxieme.estCiblable());
    REQUIRE(partie.alliesDuGroupe(deuxieme).empty());
}
