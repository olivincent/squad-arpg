// ./tests/jeu/test_Partie_soigneur.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Niveau.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"

// Le soin d'un soigneur dans une partie (étape 107.4) : `Partie` applique le soin au monstre visé et le signale. Le soigneur partage
// le groupe d'une infanterie de 2000 points de vie que l'escouade blesse.

TEST_CASE("un soigneur soigne un membre blesse de son groupe, jamais lui-meme") {
    DefinitionDeMonstre infanterie = monstreSansHasard();
    infanterie.pointsDeVieMax = 2000;  // assez pour passer sous 75 % sans mourir
    CatalogueDeMonstres catalogue = catalogueDeTest(infanterie);
    catalogue[static_cast<std::size_t>(CategorieDeMonstre::Soigneur)].pointsDeVieMax = 100000;  // l'escouade le frappe aussi
    const Niveau niveau{std::vector<std::string>(15, std::string(15, '.')), 40.0f, {200.0f, 200.0f},
                        {{{270.0f, 210.0f}, 0}, {{330.0f, 260.0f}, 0, CategorieDeMonstre::Soigneur}}};
    Partie partie(niveau, expeditionDeTest(compositionDeTest()), s_graineDeTest, catalogue);
    const IdMonstre idInfanterie = partie.getMonstres()[0].getId();
    const IdMonstre idSoigneur = partie.getMonstres()[1].getId();

    int soins = 0;
    for (int i = 0; i < nombreDeTics(20.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            const IdMonstre* cible = std::get_if<IdMonstre>(&evenement.cible);
            const Soin* soin = std::get_if<Soin>(&evenement.detail);
            if (cible != nullptr && soin != nullptr) {
                ++soins;
                REQUIRE(*cible == idInfanterie);
                REQUIRE(*cible != idSoigneur);
                REQUIRE(soin->montant == 75);
                REQUIRE_FALSE(soin->critique);
            }
        }
    }

    REQUIRE(soins >= 1);
}
