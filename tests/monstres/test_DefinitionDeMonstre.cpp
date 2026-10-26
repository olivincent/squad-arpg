// ./tests/monstres/test_DefinitionDeMonstre.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Element.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monstres/DefinitionDeMonstre.hpp"

// Le renforcement par la profondeur (étape 98.3).
namespace {
// Tout ce que le renforcement ne doit pas toucher.
void verifierQueLeResteNeChangePas(const DefinitionDeMonstre& avant, const DefinitionDeMonstre& apres) {
    REQUIRE(apres.cote == avant.cote);
    REQUIRE(apres.couleur == avant.couleur);
    REQUIRE(apres.vitesse == avant.vitesse);
    REQUIRE(apres.armure == avant.armure);
    REQUIRE(apres.rayonDetection == avant.rayonDetection);
    REQUIRE(apres.rayonTerritoire == avant.rayonTerritoire);
    REQUIRE(apres.zoneDeControle == avant.zoneDeControle);
    REQUIRE(apres.regenerationRetour == avant.regenerationRetour);
    REQUIRE(apres.attaque.type == avant.attaque.type);
    REQUIRE(apres.attaque.portee == avant.attaque.portee);
    REQUIRE(apres.attaque.rechargeMin == avant.attaque.rechargeMin);
    REQUIRE(apres.attaque.rechargeMax == avant.attaque.rechargeMax);
    REQUIRE(apres.attaque.rayonZone == avant.attaque.rayonZone);
    REQUIRE(apres.attaque.tempsExecution == avant.attaque.tempsExecution);
    REQUIRE(apres.attaque.nature == avant.attaque.nature);
    REQUIRE(apres.statistiques.esquiveContact == avant.statistiques.esquiveContact);
    REQUIRE(apres.statistiques.esquiveProjectile == avant.statistiques.esquiveProjectile);
    REQUIRE(apres.statistiques.chanceDeBlocage == avant.statistiques.chanceDeBlocage);
    REQUIRE(apres.statistiques.montantBloque == avant.statistiques.montantBloque);
    REQUIRE(apres.statistiques.resistances == avant.statistiques.resistances);
    REQUIRE(apres.chanceDeButin == avant.chanceDeButin);
    REQUIRE(apres.nombreDObjets == avant.nombreDObjets);
}
}  // namespace

TEST_CASE("renforcer : au premier etage, la definition est identique") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(categorie);

        const DefinitionDeMonstre renforcee = renforcer(definition, 1);

        REQUIRE(renforcee.pointsDeVieMax == definition.pointsDeVieMax);
        REQUIRE(renforcee.attaque.puissance == definition.attaque.puissance);
        REQUIRE(renforcee.statistiques == definition.statistiques);
        verifierQueLeResteNeChangePas(definition, renforcee);
    }
}

TEST_CASE("renforcer : au troisieme etage, points de vie x 1,5, degats x 1,3, capacites + 20") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(categorie);

        const DefinitionDeMonstre renforcee = renforcer(definition, 3);

        REQUIRE(renforcee.pointsDeVieMax == definition.pointsDeVieMax * 3 / 2);
        REQUIRE(renforcee.attaque.puissance == (definition.attaque.puissance * 13 + 5) / 10);
        REQUIRE(renforcee.statistiques.capaciteOffensive == definition.statistiques.capaciteOffensive + 20);
        REQUIRE(renforcee.statistiques.capaciteDefensive == definition.statistiques.capaciteDefensive + 20);
        verifierQueLeResteNeChangePas(definition, renforcee);
    }
}

TEST_CASE("renforcer : valeurs connues de l'infanterie, de la nuee et de la creature au deuxieme et au cinquieme etage") {
    const DefinitionDeMonstre& infanterie = definitionDeMonstre(CategorieDeMonstre::Infanterie);  // 300 PV, 30 dégâts, 100 / 100
    REQUIRE(renforcer(infanterie, 2).pointsDeVieMax == 375);
    REQUIRE(renforcer(infanterie, 2).attaque.puissance == 35);  // 34,5 arrondi au plus proche
    REQUIRE(renforcer(infanterie, 2).statistiques.capaciteOffensive == 110);
    REQUIRE(renforcer(infanterie, 5).pointsDeVieMax == 600);
    REQUIRE(renforcer(infanterie, 5).attaque.puissance == 48);
    REQUIRE(renforcer(infanterie, 5).statistiques.capaciteDefensive == 140);

    const DefinitionDeMonstre& nuee = definitionDeMonstre(CategorieDeMonstre::Nuee);  // 60 PV, 10 dégâts
    REQUIRE(renforcer(nuee, 5).pointsDeVieMax == 120);
    REQUIRE(renforcer(nuee, 5).attaque.puissance == 16);

    const DefinitionDeMonstre& creature = definitionDeMonstre(CategorieDeMonstre::CreatureMonstrueuse);  // 1500 PV, 70 dégâts
    REQUIRE(renforcer(creature, 5).pointsDeVieMax == 3000);
    REQUIRE(renforcer(creature, 5).attaque.puissance == 112);
    REQUIRE(renforcer(creature, 5).statistiques.resistances[Element::Feu] == creature.statistiques.resistances[Element::Feu]);
}

TEST_CASE("renforcer : plus la profondeur est grande, plus le monstre est fort") {
    const DefinitionDeMonstre& definition = definitionDeMonstre(CategorieDeMonstre::Infanterie);
    for (int profondeur = 1; profondeur < 5; ++profondeur) {
        const DefinitionDeMonstre moins = renforcer(definition, profondeur);
        const DefinitionDeMonstre plus = renforcer(definition, profondeur + 1);

        REQUIRE(plus.pointsDeVieMax > moins.pointsDeVieMax);
        REQUIRE(plus.attaque.puissance > moins.attaque.puissance);
        REQUIRE(plus.statistiques.capaciteOffensive > moins.statistiques.capaciteOffensive);
        REQUIRE(plus.statistiques.capaciteDefensive > moins.statistiques.capaciteDefensive);
    }
}

TEST_CASE("renforcer : ne modifie pas la definition recue, meme renforcee deux fois depuis la meme") {
    const DefinitionDeMonstre& definition = definitionDeMonstre(CategorieDeMonstre::Infanterie);
    const int vieAvant = definition.pointsDeVieMax;

    const DefinitionDeMonstre premiere = renforcer(definition, 4);
    const DefinitionDeMonstre seconde = renforcer(definition, 4);

    REQUIRE(definition.pointsDeVieMax == vieAvant);
    REQUIRE(premiere.pointsDeVieMax == seconde.pointsDeVieMax);
}
