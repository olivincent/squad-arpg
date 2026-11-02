// ./tests/jeu/test_Partie_ward.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/EffetActif.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Niveau.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"

// Le Ward d'un soigneur dans une partie (étape 114.6) : `Partie` le pose sur un membre du groupe, jamais sur le soigneur. Le soigneur partage
// le groupe d'une infanterie de 2000 points de vie, comme dans test_Partie_soigneur.cpp.

TEST_CASE("un soigneur pose un Ward sur un membre de son groupe, jamais sur lui-meme") {
    DefinitionDeMonstre infanterie = monstreSansHasard();
    infanterie.pointsDeVieMax = 2000;
    CatalogueDeMonstres catalogue = catalogueDeTest(infanterie);
    catalogue[static_cast<std::size_t>(CategorieDeMonstre::Soigneur)].pointsDeVieMax = 100000;
    const Niveau niveau{std::vector<std::string>(15, std::string(15, '.')), 40.0f, {200.0f, 200.0f},
                        {{{270.0f, 210.0f}, 0}, {{330.0f, 260.0f}, 0, CategorieDeMonstre::Soigneur}}};
    Partie partie(niveau, expeditionDeTest(compositionDeTest()), s_graineDeTest, catalogue);
    const Monstre* soigneur = nullptr;
    const Monstre* membre = nullptr;
    for (const Monstre& monstre : partie.getMonstres()) {
        (monstre.getPointsDeVieMax() == 100000 ? soigneur : membre) = &monstre;
    }
    REQUIRE(soigneur != nullptr);
    REQUIRE(membre != nullptr);

    bool wardPose = false;
    for (int i = 0; i < nombreDeTics(20.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        REQUIRE(soigneur->getEffets().pointsDAbsorption() == PointsDAbsorption{});
        wardPose = wardPose || membre->getEffets().pointsDAbsorption().ward > 0;
    }

    REQUIRE(wardPose);
}
