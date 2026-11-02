// ./tests/monstres/test_Monstre_ward.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_monstre.hpp"
#include "base/Cercle.hpp"
#include "combat/AllieVu.hpp"
#include "combat/EffetActif.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "monde/TileMap.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monstres/RoleDeMonstre.hpp"

// La deuxième capacité du soigneur (étape 114.6) : un Ward de 70 points pendant 8 s sur un allié de son groupe, jamais sur lui-même, recharge
// de 12 s, activation de 0,8 s. Le soigneur de ces tests : `soigneurSansHasard()` en (1000, 1000), avec un personnage à (1100, 1000) qui lui
// sert de proie ; les alliés sont des monstres de 300 points de vie.

namespace {
const sf::Vector2f s_centre{1000.0f, 1000.0f};
const Cercle s_personnage = cibleMonstre({1100.0f, 1000.0f});

Monstre unSoigneur() {
    return Monstre(s_centre, IdMonstre{1}, 0, soigneurSansHasard());
}

AllieVu unAllie(int id, sf::Vector2f centre, int pointsDeVie, bool aUnWard = false) {
    return {IdMonstre{id}, cibleMonstre(centre), pointsDeVie, 300, aUnWard};
}

void ecouler(Monstre& monstre, float duree, const std::vector<AllieVu>& allies) {
    const PersonnageVu proie{Place{1}, s_personnage};
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        monstre.mettreAJour(s_dureeTic, proie, {proie}, allies, carteOuverte(), {}, {});
    }
}

// Un allié en pleine santé, à 100 px : dans la portée de 150 (70 px bord à bord).
const std::vector<AllieVu> s_unAllieEnForme = {unAllie(2, {900.0f, 1000.0f}, 300)};

// Le soigneur, son Ward engagé sur l'allié en forme.
Monstre unSoigneurQuiProtege() {
    Monstre soigneur = unSoigneur();
    ecouler(soigneur, 0.1f, s_unAllieEnForme);
    REQUIRE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));
    return soigneur;
}
}  // namespace

TEST_CASE("ward : le soigneur engage un Ward sur un allie meme en pleine forme, et pas de soin") {
    Monstre soigneur = unSoigneur();
    ecouler(soigneur, 0.1f, s_unAllieEnForme);

    REQUIRE_FALSE(soigneur.engagerSoin(s_unAllieEnForme, carteOuverte()));
    REQUIRE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));

    REQUIRE(soigneur.cibleDuWardEngage() == IdMonstre{2});
    REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());
}

TEST_CASE("ward : sans allie, pas de Ward (il ne se protege jamais lui-meme)") {
    Monstre soigneur = unSoigneur();
    ecouler(soigneur, 0.1f, {});

    REQUIRE_FALSE(soigneur.engagerWard({}, carteOuverte()));
}

TEST_CASE("ward : un allie qui a deja un Ward n'en recoit pas un deuxieme") {
    Monstre soigneur = unSoigneur();
    const std::vector<AllieVu> allies = {unAllie(2, {900.0f, 1000.0f}, 300, true)};
    ecouler(soigneur, 0.1f, allies);

    REQUIRE_FALSE(soigneur.engagerWard(allies, carteOuverte()));
}

TEST_CASE("ward : un allie hors de portee n'est pas protege, le soigneur ne marche pas vers lui pour cela") {
    Monstre soigneur = unSoigneur();
    const std::vector<AllieVu> allies = {unAllie(2, {600.0f, 1000.0f}, 300)};  // 370 px bord à bord

    ecouler(soigneur, 0.1f, allies);

    REQUIRE_FALSE(soigneur.engagerWard(allies, carteOuverte()));
}

TEST_CASE("ward : le Ward s'active 0,8 s, immobile, puis part avec 70 points de Ward pendant 8 s") {
    Generateur generateur = generateurDeTest();
    Monstre soigneur = unSoigneurQuiProtege();

    for (int i = 0; i < nombreDeTics(0.7f); ++i) {
        REQUIRE_FALSE(soigneur.avancerWard(s_dureeTic, generateur).has_value());
    }
    REQUIRE(soigneur.cibleDuWardEngage().has_value());

    int departs = 0;
    std::optional<WardDeMonstre> ward;
    for (int i = 0; i < nombreDeTics(0.3f); ++i) {
        if (const std::optional<WardDeMonstre> parti = soigneur.avancerWard(s_dureeTic, generateur)) {
            ++departs;
            ward = parti;
        }
    }
    REQUIRE(departs == 1);
    REQUIRE(ward->cible == IdMonstre{2});
    REQUIRE(ward->effet.source == SourceDEffet{IdCapacite::WardDuSoigneur});
    REQUIRE(ward->effet.dureeRestante == 8.0f);
    REQUIRE(std::get<Absorption>(ward->effet.sorte) == Absorption{.famille = FamilleDAbsorption::Ward, .points = 70});
    REQUIRE_FALSE(soigneur.cibleDuWardEngage().has_value());
}

TEST_CASE("ward : la recharge est de 12 s") {
    Generateur generateur = generateurDeTest();
    Monstre soigneur = unSoigneurQuiProtege();
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        static_cast<void>(soigneur.avancerWard(s_dureeTic, generateur));
    }
    REQUIRE_FALSE(soigneur.cibleDuWardEngage().has_value());

    ecouler(soigneur, 11.0f, s_unAllieEnForme);
    REQUIRE_FALSE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));

    ecouler(soigneur, 1.5f, s_unAllieEnForme);
    REQUIRE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));
}

TEST_CASE("ward : un Ward annule ne consomme pas de recharge") {
    Monstre soigneur = unSoigneurQuiProtege();

    soigneur.annulerWard();

    REQUIRE_FALSE(soigneur.cibleDuWardEngage().has_value());
    REQUIRE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));
}

TEST_CASE("ward : la Disruption l'empeche et annule celui qui s'active, l'etourdissement aussi") {
    Generateur generateur = generateurDeTest();
    Monstre soigneur = unSoigneurQuiProtege();

    static_cast<void>(soigneur.recevoirDegats(attaqueQuiControle(unControle(EtatInflige::Disruption, 3.0f)), generateur));

    REQUIRE_FALSE(soigneur.cibleDuWardEngage().has_value());
    REQUIRE_FALSE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));
    ecouler(soigneur, 3.5f, s_unAllieEnForme);
    REQUIRE(soigneur.engagerWard(s_unAllieEnForme, carteOuverte()));  // aucune recharge consommée

    static_cast<void>(soigneur.recevoirDegats(attaqueQuiControle(unControle(EtatInflige::Etourdissement, 2.0f)), generateur));
    REQUIRE_FALSE(soigneur.cibleDuWardEngage().has_value());
}

TEST_CASE("ward : un Ward et un coup ne sont jamais engages ensemble") {
    Monstre soigneur = unSoigneurQuiProtege();

    REQUIRE_FALSE(engager(soigneur, s_personnage, Place{1}, carteOuverte()));
}

TEST_CASE("ward : un autre monstre n'en a pas") {
    Monstre infanterie(s_centre, IdMonstre{1}, 0, monstreSansHasard());

    REQUIRE_FALSE(infanterie.engagerWard(s_unAllieEnForme, carteOuverte()));
    REQUIRE_FALSE(infanterie.cibleDuWardEngage().has_value());
}

TEST_CASE("ward : un monstre recoit le Ward comme un effet actif") {
    Monstre protege({500.0f, 500.0f}, IdMonstre{2}, 0, monstreSansHasard());
    const EffetActif ward{.source = IdCapacite::WardDuSoigneur, .dureeRestante = 8.0f, .sorte = Absorption{.famille = FamilleDAbsorption::Ward, .points = 70}};

    protege.recevoirUnEffet(ward);

    REQUIRE(protege.getEffets().pointsDAbsorption().ward == 70);
}

// choisirLaCibleDuWard (pure)

namespace {
AllieVu unAllieDuGroupe(int id, int pointsDeVie, int pointsDeVieMax, bool aUnWard = false) {
    return {IdMonstre{id}, Cercle{{0.0f, 0.0f}, 15.0f}, pointsDeVie, pointsDeVieMax, aUnWard};
}
}  // namespace

TEST_CASE("choisirLaCibleDuWard : personne, ou tous ont deja un Ward, rien") {
    REQUIRE_FALSE(choisirLaCibleDuWard({}, 75).has_value());
    REQUIRE_FALSE(choisirLaCibleDuWard({unAllieDuGroupe(2, 100, 300, true), unAllieDuGroupe(3, 300, 300, true)}, 75).has_value());
}

TEST_CASE("choisirLaCibleDuWard : un blesse sans Ward passe avant un allie en forme") {
    const std::optional<AllieVu> choisi = choisirLaCibleDuWard({unAllieDuGroupe(2, 300, 300), unAllieDuGroupe(3, 100, 300)}, 75);

    REQUIRE(choisi.has_value());
    REQUIRE(choisi->id == IdMonstre{3});
}

TEST_CASE("choisirLaCibleDuWard : le blesse qui a deja un Ward est ecarte, au profit d'un autre") {
    const std::optional<AllieVu> choisi = choisirLaCibleDuWard({unAllieDuGroupe(2, 50, 300, true), unAllieDuGroupe(3, 250, 300)}, 75);

    REQUIRE(choisi.has_value());
    REQUIRE(choisi->id == IdMonstre{3});
}

TEST_CASE("choisirLaCibleDuWard : sans blesse, le plus entame en proportion, a egalite le plus petit identifiant") {
    const std::optional<AllieVu> proportion = choisirLaCibleDuWard({unAllieDuGroupe(2, 290, 300), unAllieDuGroupe(3, 59, 60)}, 75);
    REQUIRE(proportion.has_value());
    REQUIRE(proportion->id == IdMonstre{2});  // 96,7 % contre 98,3 % : le premier est le plus entamé

    const std::optional<AllieVu> egalite = choisirLaCibleDuWard({unAllieDuGroupe(5, 300, 300), unAllieDuGroupe(4, 300, 300)}, 75);
    REQUIRE(egalite.has_value());
    REQUIRE(egalite->id == IdMonstre{4});
}
