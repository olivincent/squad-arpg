// ./tests/monstres/test_Monstre_tireur.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_monstre.hpp"
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/NatureDAttaque.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/TileMap.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"

// Le rôle de tireur (étape 107.2) : frappe de loin, recule d'une longueur de recul quand un personnage le colle, au plus une fois
// par délai. Le tireur du catalogue : rayon 13, portée 120 px, contact à 20 px, recul de 80 px, délai de 8 s. Les personnages de ces
// tests ont le rayon d'un monstre d'infanterie (15 px) : deux centres à 40 px sont à 12 px bord à bord, donc au contact.

namespace {
const DefinitionDeMonstre& definitionDuTireur() {
    return definitionDeMonstre(CategorieDeMonstre::Tireur);
}

Monstre unTireur(sf::Vector2f centre) {
    return Monstre(centre, IdMonstre{1}, 0, definitionDuTireur());
}

// Le tireur voit ces personnages (le premier est sa proie) pendant `duree` secondes.
void ecoulerAvec(Monstre& monstre, float duree, const std::vector<Cercle>& personnages, const TileMap& carte) {
    std::vector<PersonnageVu> candidats;
    for (std::size_t i = 0; i < personnages.size(); ++i) {
        candidats.push_back({Place::depuisIndice(i), personnages[i]});
    }
    const std::optional<PersonnageVu> proie = candidats.empty() ? std::nullopt : std::optional<PersonnageVu>(candidats.front());
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, {}, {});
    }
}
}  // namespace

TEST_CASE("tireur : une attaque de projectile sans zone de controle") {
    const Monstre tireur = unTireur({1000.0f, 1000.0f});

    REQUIRE(tireur.getZoneDeControle() == 0.0f);
    REQUIRE(tireur.getAttaque().nature == NatureDAttaque::Projectile);
}

TEST_CASE("tireur : frappe a distance, sans contact") {
    Generateur generateur = generateurDeTest();
    Monstre tireur = unTireur({1000.0f, 1000.0f});
    const Cercle personnage = cibleMonstre({1100.0f, 1000.0f});  // 100 px : 72 px bord à bord, dans sa portée de 120

    ecoulerTemps(tireur, 0.2f, personnage, carteOuverte());  // détection, puis arrêt dès qu'il a une cible de frappe

    REQUIRE(engager(tireur, personnage, Place{1}, carteOuverte()));
    int degats = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && degats == 0; ++i) {
        degats = degatsOuZero(tireur.avancerAttaque(s_dureeTic, generateur));
    }
    REQUIRE(degats == tireur.getAttaque().degats.total());
    verifierPointProche(tireur.getCentre(), {1000.0f, 1000.0f});
}

TEST_CASE("tireur : un personnage au contact le fait reculer de 80 px dans la direction opposee") {
    Monstre tireur = unTireur({1000.0f, 1000.0f});

    ecoulerAvec(tireur, 1.0f, {cibleMonstre({1040.0f, 1000.0f})}, carteOuverte());

    verifierPointProche(tireur.getCentre(), {920.0f, 1000.0f});
}

TEST_CASE("tireur : sans personnage au contact, il ne recule pas") {
    Monstre tireur = unTireur({1000.0f, 1000.0f});

    ecoulerAvec(tireur, 1.0f, {cibleMonstre({1060.0f, 1000.0f})}, carteOuverte());  // 32 px bord à bord : hors contact

    verifierPointProche(tireur.getCentre(), {1000.0f, 1000.0f});
}

TEST_CASE("tireur : il recule devant le personnage le plus proche") {
    Monstre tireur = unTireur({1000.0f, 1000.0f});
    // L'un à 12 px bord à bord (à droite), l'autre à 18 px (en dessous) : tous deux au contact.
    ecoulerAvec(tireur, 1.0f, {cibleMonstre({1000.0f, 1046.0f}), cibleMonstre({1040.0f, 1000.0f})}, carteOuverte());

    verifierPointProche(tireur.getCentre(), {920.0f, 1000.0f});
}

TEST_CASE("tireur : il n'engage pas de coup pendant son recul") {
    Monstre tireur = unTireur({1000.0f, 1000.0f});
    const Cercle personnage = cibleMonstre({1040.0f, 1000.0f});

    ecoulerAvec(tireur, 0.1f, {personnage}, carteOuverte());  // le recul est parti, il n'est pas arrivé

    REQUIRE_FALSE(engager(tireur, personnage, Place{1}, carteOuverte()));
    REQUIRE_FALSE(tireur.cibleEngagee().has_value());
}

TEST_CASE("tireur : au plus un recul par tranche de 8 s, le premier tout de suite") {
    Monstre tireur = unTireur({1000.0f, 1000.0f});

    ecoulerAvec(tireur, 1.0f, {cibleMonstre({1040.0f, 1000.0f})}, carteOuverte());
    verifierPointProche(tireur.getCentre(), {920.0f, 1000.0f});

    // Le personnage le recolle : tant que le délai court, il reste.
    ecoulerAvec(tireur, 6.0f, {cibleMonstre({960.0f, 1000.0f})}, carteOuverte());
    verifierPointProche(tireur.getCentre(), {920.0f, 1000.0f});

    // Le délai écoulé, il recule de nouveau.
    ecoulerAvec(tireur, 2.0f, {cibleMonstre({960.0f, 1000.0f})}, carteOuverte());
    verifierPointProche(tireur.getCentre(), {840.0f, 1000.0f});
}

TEST_CASE("tireur : le recul attend la fin du coup en cours") {
    Generateur generateur = generateurDeTest();
    Monstre tireur = unTireur({1000.0f, 1000.0f});
    const Cercle loin = cibleMonstre({1100.0f, 1000.0f});
    ecoulerTemps(tireur, 0.2f, loin, carteOuverte());
    REQUIRE(engager(tireur, loin, Place{1}, carteOuverte()));  // 0,4 s d'exécution

    ecoulerAvec(tireur, 0.3f, {cibleMonstre({1040.0f, 1000.0f})}, carteOuverte());  // un personnage le colle
    verifierPointProche(tireur.getCentre(), {1000.0f, 1000.0f});
    REQUIRE(tireur.cibleEngagee().has_value());

    for (int i = 0; i < nombreDeTics(0.5f); ++i) {
        static_cast<void>(tireur.avancerAttaque(s_dureeTic, generateur));  // le coup part : son départ est testé ailleurs
    }
    REQUIRE_FALSE(tireur.cibleEngagee().has_value());
    ecoulerAvec(tireur, 1.0f, {cibleMonstre({1040.0f, 1000.0f})}, carteOuverte());
    verifierPointProche(tireur.getCentre(), {920.0f, 1000.0f});
}

TEST_CASE("tireur : il ne recule pas en retour au poste") {
    const TileMap carte = carteLongue();
    Monstre tireur({100.0f, 100.0f}, IdMonstre{1}, 0, definitionDuTireur());
    amenerEnRetourHorsTerritoire(tireur, carte);
    const float avant = tireur.getCentre().x;

    // Un personnage le colle par la gauche : un recul l'enverrait vers la droite, le retour vers son poste (à gauche).
    ecoulerAvec(tireur, 0.3f, {cibleMonstre({avant - 40.0f, 100.0f})}, carte);

    REQUIRE(tireur.getCentre().x < avant);
}

TEST_CASE("tireur : un recul impossible n'a pas lieu et ne consomme pas le delai") {
    // Deux colonnes de mur à gauche : le recul vers la gauche tombe dans un mur.
    std::vector<std::string> plan(10, "..........");
    for (std::string& ligne : plan) {
        ligne[0] = '#';
        ligne[1] = '#';
    }
    const TileMap carte(plan, 40.0f);
    Monstre tireur = unTireur({100.0f, 200.0f});

    ecoulerAvec(tireur, 1.0f, {cibleMonstre({140.0f, 200.0f})}, carte);
    verifierPointProche(tireur.getCentre(), {100.0f, 200.0f});

    // Le personnage passe au-dessus : le recul vers le bas est possible, tout de suite (le délai n'a pas été consommé).
    ecoulerAvec(tireur, 1.0f, {cibleMonstre({100.0f, 160.0f})}, carte);
    verifierPointProche(tireur.getCentre(), {100.0f, 280.0f});
}
