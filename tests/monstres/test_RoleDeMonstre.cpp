// ./tests/monstres/test_RoleDeMonstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <variant>

#include <optional>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "combat/AllieVu.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/ModeDeCible.hpp"
#include "combat/NatureDAttaque.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/RoleDeMonstre.hpp"

// Le rôle d'un monstre (étape 107.1) : une donnée de la définition, un état de même forme dans le monstre.

TEST_CASE("role : les quatre categories d'avant les roles sont des melees") {
    for (const CategorieDeMonstre categorie : {CategorieDeMonstre::Nuee, CategorieDeMonstre::Infanterie,
                                               CategorieDeMonstre::InfanterieLourde, CategorieDeMonstre::CreatureMonstrueuse}) {
        REQUIRE(std::holds_alternative<RoleMelee>(definitionDeMonstre(categorie).role));
    }
}

TEST_CASE("role : le tireur a un recul qui a un sens") {
    const DefinitionDeMonstre& definition = definitionDeMonstre(CategorieDeMonstre::Tireur);
    const RoleTireur* tireur = std::get_if<RoleTireur>(&definition.role);

    REQUIRE(tireur != nullptr);
    REQUIRE(tireur->distanceDeContact > 0.0f);
    REQUIRE(tireur->distanceDeRecul > 0.0f);
    REQUIRE(tireur->delaiEntreReculs > 0.0f);
    // Il frappe de loin : sa portée dépasse son contact, et il n'engage personne.
    REQUIRE(definition.attaque.portee > tireur->distanceDeContact);
    REQUIRE(definition.zoneDeControle == 0.0f);
    REQUIRE(std::get<EffetDegats>(definition.attaque.effets.front()).nature == NatureDAttaque::Projectile);
}

TEST_CASE("role : le soigneur du catalogue soigne 75 points toutes les 6 s a 150 px, apres 0,8 s") {
    const DefinitionDeMonstre& definition = definitionDeMonstre(CategorieDeMonstre::Soigneur);
    const RoleSoigneur* soigneur = std::get_if<RoleSoigneur>(&definition.role);

    REQUIRE(soigneur != nullptr);
    REQUIRE(soigneur->soin.mode == ModeDeCible::Allie);
    REQUIRE(std::get<EffetSoin>(soigneur->soin.effets.front()).montant == 75);
    REQUIRE(soigneur->soin.portee == 150.0f);
    REQUIRE(soigneur->soin.rechargeMin == 6.0f);
    REQUIRE(soigneur->soin.rechargeMax == 6.0f);
    REQUIRE(soigneur->soin.tempsExecution == 0.8f);
    REQUIRE(soigneur->seuilDeBlessure == 75);
    // Hors soin, un tireur faible : projectile de portée 120, sans zone de contrôle.
    REQUIRE(definition.attaque.portee == 120.0f);
    REQUIRE(definition.zoneDeControle == 0.0f);
    REQUIRE(std::get<EffetDegats>(definition.attaque.effets.front()).nature == NatureDAttaque::Projectile);
}

TEST_CASE("role : l'etat initial est l'alternative de meme rang que le role") {
    const RoleTireur tireur{.distanceDeContact = 20.0f, .distanceDeRecul = 80.0f, .delaiEntreReculs = 8.0f};
    const EtatDuRole etat = etatInitialDuRole(tireur);

    REQUIRE(std::holds_alternative<EtatMelee>(etatInitialDuRole(RoleMelee{})));
    REQUIRE(std::holds_alternative<EtatTireur>(etat));
    REQUIRE(std::holds_alternative<EtatSoigneur>(etatInitialDuRole(definitionDeMonstre(CategorieDeMonstre::Soigneur).role)));
    // Le tireur part avec ses données, un recul disponible tout de suite et aucun recul en cours.
    const EtatTireur& tireurInitial = std::get<EtatTireur>(etat);
    REQUIRE(tireurInitial.role.distanceDeRecul == 80.0f);
    REQUIRE(tireurInitial.delaiRestant == 0.0f);
    REQUIRE_FALSE(tireurInitial.enRecul);
}

namespace {
AllieVu unAllie(int id, int pointsDeVie, int pointsDeVieMax) {
    return {IdMonstre{id}, Cercle{{0.0f, 0.0f}, 15.0f}, pointsDeVie, pointsDeVieMax};
}
}  // namespace

TEST_CASE("choisirLeBlesse : personne sous le seuil, personne a soigner") {
    // Plein, et juste au seuil de 75 % (225 sur 300 n'est pas « sous » 75 %).
    REQUIRE_FALSE(choisirLeBlesse({}, 75).has_value());
    REQUIRE_FALSE(choisirLeBlesse({unAllie(2, 300, 300), unAllie(3, 225, 300)}, 75).has_value());
    REQUIRE(choisirLeBlesse({unAllie(2, 224, 300)}, 75).has_value());
}

TEST_CASE("choisirLeBlesse : le plus blesse en proportion, pas le plus de points perdus") {
    // 100 sur 300 (33 %) a perdu 200 points ; 10 sur 60 (17 %) n'en a perdu que 50, mais davantage en proportion.
    const std::optional<AllieVu> choisi = choisirLeBlesse({unAllie(2, 100, 300), unAllie(3, 10, 60)}, 75);

    REQUIRE(choisi.has_value());
    REQUIRE(choisi->id == IdMonstre{3});
}

TEST_CASE("choisirLeBlesse : a egalite, le plus petit identifiant") {
    // 100 sur 300 et 50 sur 150 : un tiers chacun. L'ordre de la liste n'y change rien.
    REQUIRE(choisirLeBlesse({unAllie(5, 100, 300), unAllie(3, 50, 150)}, 75)->id == IdMonstre{3});
    REQUIRE(choisirLeBlesse({unAllie(3, 50, 150), unAllie(5, 100, 300)}, 75)->id == IdMonstre{3});
}

TEST_CASE("choisirLeBlesse : jamais un mort") {
    REQUIRE_FALSE(choisirLeBlesse({unAllie(2, 0, 300)}, 75).has_value());
}

TEST_CASE("role : renforcer majore le soin du soigneur comme les degats") {
    const DefinitionDeMonstre soigneur = soigneurSansHasard();

    const DefinitionDeMonstre renforce = renforcer(soigneur, 3);  // +30 % : deux étages de 15 %

    REQUIRE(std::get<EffetSoin>(std::get<RoleSoigneur>(renforce.role).soin.effets.front()).montant == 98);  // 75 × 1,3 = 97,5
    REQUIRE(std::get<EffetSoin>(std::get<RoleSoigneur>(renforcer(soigneur, 1).role).soin.effets.front()).montant == 75);
}

TEST_CASE("role : renforcer garde le role de la definition") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(categorie);
        REQUIRE(renforcer(definition, 4).role.index() == definition.role.index());
    }
}
