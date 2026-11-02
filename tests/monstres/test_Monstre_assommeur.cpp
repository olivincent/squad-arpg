// ./tests/monstres/test_Monstre_assommeur.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_monstre.hpp"
#include "base/Cercle.hpp"
#include "combat/Controle.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monstres/RoleDeMonstre.hpp"

using Catch::Approx;

// Le coup assommant de l'infanterie lourde (étape 113.7) : le premier monstre qui pose un effet actif. Son attaque de base porte un
// étourdissement de 1 s, au plus une fois toutes les 10 s. L'infanterie lourde de ces tests est sans esquive ni butin, en (100, 100) ; sa
// cible, un cercle collé à elle.

namespace {
DefinitionDeMonstre lourdSansHasard() {
    DefinitionDeMonstre definition = definitionDeMonstre(CategorieDeMonstre::InfanterieLourde);
    definition.chanceDeButin = 0;
    definition.statistiques.esquiveContact = 0;
    definition.statistiques.esquiveProjectile = 0;
    return definition;
}

const Cercle s_cible = pointCible({115.0f, 100.0f});

// Engage un coup sur la cible et rend l'attaque quand elle part (vide si elle ne part pas en 3 s).
std::optional<Attaque> frapper(Monstre& monstre, Generateur& generateur) {
    if (!engager(monstre, s_cible, Place{1}, carteOuverte())) {
        return std::nullopt;
    }
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        if (std::optional<Attaque> attaque = monstre.avancerAttaque(s_dureeTic, generateur)) {
            return attaque;
        }
    }
    return std::nullopt;
}

bool portaitUnEtourdissement(const Attaque& attaque) {
    return std::ranges::any_of(attaque.effets, [](const EffetActif& effet) {
        const auto* controle = std::get_if<Controle>(&effet.sorte);
        return controle != nullptr && controle->etat == EtatInflige::Etourdissement;
    });
}
}  // namespace

TEST_CASE("assommeur : seule l'infanterie lourde l'est, avec 1 s et 10 s") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const bool assommeur = std::holds_alternative<RoleAssommeur>(definitionDeMonstre(categorie).role);
        REQUIRE(assommeur == (categorie == CategorieDeMonstre::InfanterieLourde));
    }
    const RoleAssommeur& role = std::get<RoleAssommeur>(definitionDeMonstre(CategorieDeMonstre::InfanterieLourde).role);
    REQUIRE(role.dureeDeLEtourdissement == 1.0f);
    REQUIRE(role.delaiEntreCoups == 10.0f);
}

TEST_CASE("assommeur : son premier coup porte un etourdissement de son role, identifie par CoupAssommant") {
    Generateur generateur = generateurDeTest();
    Monstre lourd({100.0f, 100.0f}, IdMonstre{1}, 0, lourdSansHasard());

    const std::optional<Attaque> attaque = frapper(lourd, generateur);

    REQUIRE(attaque.has_value());
    REQUIRE(attaque->effets.size() == 1);
    const EffetActif& effet = attaque->effets.front();
    REQUIRE(effet.source == IdSkill::CoupAssommant);
    REQUIRE(effet.dureeRestante == Approx(1.0f));
    REQUIRE(std::get<Controle>(effet.sorte).etat == EtatInflige::Etourdissement);
    REQUIRE(attaque->degats.total() == lourd.getAttaque().degats.total());  // les dégâts du coup ne changent pas
}

TEST_CASE("assommeur : au plus un etourdissement toutes les 10 s") {
    Generateur generateur = generateurDeTest();
    Monstre lourd({100.0f, 100.0f}, IdMonstre{1}, 0, lourdSansHasard());
    REQUIRE(portaitUnEtourdissement(*frapper(lourd, generateur)));

    // Les coups suivants, pendant les 10 s : aucun étourdissement. Chaque coup dure 0,4 s d'exécution puis 1,2 à 1,8 s de recharge.
    int coupsSansEtourdissement = 0;
    float ecoule = 0.0f;
    while (ecoule < 8.0f) {
        ecoulerTemps(lourd, 2.0f, std::nullopt, carteOuverte());  // la recharge
        ecoule += 2.0f;
        const std::optional<Attaque> attaque = frapper(lourd, generateur);
        REQUIRE(attaque.has_value());
        REQUIRE_FALSE(portaitUnEtourdissement(*attaque));
        ++coupsSansEtourdissement;
    }
    REQUIRE(coupsSansEtourdissement >= 3);

    ecoulerTemps(lourd, 4.0f, std::nullopt, carteOuverte());  // plus de 10 s depuis le premier
    REQUIRE(portaitUnEtourdissement(*frapper(lourd, generateur)));
}

TEST_CASE("assommeur : en Disruption, son coup n'etourdit pas et le delai ne repart pas") {
    Generateur generateur = generateurDeTest();
    Monstre lourd({100.0f, 100.0f}, IdMonstre{1}, 0, lourdSansHasard());
    static_cast<void>(lourd.recevoirDegats(attaqueQuiControle(unControle(EtatInflige::Disruption, 6.0f)), generateur));

    REQUIRE_FALSE(portaitUnEtourdissement(*frapper(lourd, generateur)));  // il frappe, sans étourdir

    ecoulerTemps(lourd, 6.0f, std::nullopt, carteOuverte());  // la Disruption a pris fin, aucun délai n'était parti
    REQUIRE(portaitUnEtourdissement(*frapper(lourd, generateur)));
}

TEST_CASE("assommeur : un coup d'opportunite (getAttaque) n'etourdit pas") {
    Monstre lourd({100.0f, 100.0f}, IdMonstre{1}, 0, lourdSansHasard());

    REQUIRE(lourd.getAttaque().effets.empty());
}

TEST_CASE("assommeur : les autres monstres ne portent aucun effet") {
    Generateur generateur = generateurDeTest();
    Monstre infanterie({100.0f, 100.0f}, IdMonstre{1}, 0, monstreSansHasard());

    const std::optional<Attaque> attaque = frapper(infanterie, generateur);

    REQUIRE(attaque.has_value());
    REQUIRE(attaque->effets.empty());
}

TEST_CASE("assommeur : la profondeur ne change pas son etourdissement") {
    const DefinitionDeMonstre renforcee = renforcer(definitionDeMonstre(CategorieDeMonstre::InfanterieLourde), 4);

    const RoleAssommeur& role = std::get<RoleAssommeur>(renforcee.role);
    REQUIRE(role.dureeDeLEtourdissement == 1.0f);
    REQUIRE(role.delaiEntreCoups == 10.0f);
}

TEST_CASE("assommeur : etourdi lui-meme, son coup engage est annule et rien ne part") {
    Generateur generateur = generateurDeTest();
    Monstre lourd({100.0f, 100.0f}, IdMonstre{1}, 0, lourdSansHasard());
    REQUIRE(engager(lourd, s_cible, Place{1}, carteOuverte()));

    static_cast<void>(lourd.recevoirDegats(attaqueQuiControle(unControle(EtatInflige::Etourdissement, 2.0f)), generateur));

    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        REQUIRE_FALSE(lourd.avancerAttaque(s_dureeTic, generateur).has_value());
    }
}
