// ./src/monstres/DefinitionDeMonstre.cpp
#include "monstres/DefinitionDeMonstre.hpp"
#include <cstddef>
#include "combat/NatureDAttaque.hpp"
#include "combat/TypeSkill.hpp"

namespace {
// Ce que toutes les catégories partagent. Valeurs provisoires, à ajuster après test.
constexpr float s_rayonDetection = 200.0f;     // en pixels
constexpr float s_rayonTerritoire = 600.0f;    // en pixels
constexpr float s_zoneDeControle = 20.0f;      // en pixels, bord à bord
constexpr float s_regenerationRetour = 0.5f;   // fraction des points de vie maximum rendue par seconde en retour
constexpr float s_porteeAttaque = 15.0f;       // bord à bord

// Ce qui change d'une catégorie à l'autre : une ligne du tableau de `docs/classes-et-combat.md` (section 11).
struct Profil {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    sf::Color couleur;
    float rayon;              // du cercle de collision, en pixels (le carré a pour côté le double)
    float vitesse;            // en pixels par seconde
    int pointsDeVieMax;
    int armure;
    int degats;               // avant l'armure de la cible
    float rechargeMin;        // en secondes
    float rechargeMax;
    float execution;          // en secondes, immobile avant que le coup parte
    int capaciteOffensive;    // celle d'un personnage d'Adresse 10 : 100
    int capaciteDefensive;
    int esquive;              // en pour cent, contre le contact comme contre les projectiles
};

constexpr Profil s_nuee = {.couleur = {115, 165, 25}, .rayon = 9.0f, .vitesse = 190.0f, .pointsDeVieMax = 60,
                           .armure = 0, .degats = 10, .rechargeMin = 0.6f, .rechargeMax = 1.0f, .execution = 0.2f,
                           .capaciteOffensive = 90, .capaciteDefensive = 80, .esquive = 10};
constexpr Profil s_infanterie = {.couleur = {255, 0, 0}, .rayon = 15.0f, .vitesse = 160.0f, .pointsDeVieMax = 300,
                                 .armure = 3, .degats = 30, .rechargeMin = 0.7f, .rechargeMax = 1.3f,
                                 .execution = 0.3f, .capaciteOffensive = 100, .capaciteDefensive = 100, .esquive = 5};
constexpr Profil s_infanterieLourde = {.couleur = {110, 125, 140}, .rayon = 17.0f, .vitesse = 110.0f,
                                       .pointsDeVieMax = 450, .armure = 10, .degats = 45, .rechargeMin = 1.2f,
                                       .rechargeMax = 1.8f, .execution = 0.4f, .capaciteOffensive = 100,
                                       .capaciteDefensive = 120, .esquive = 0};
// Rayon 16 et non 30 : un chemin suit les centres des cases, donc à une demi-case (20 px) du mur de sa voie, et un
// cercle de rayon 20 ou plus s'y bloque (mesuré : 19 passe, 20 non). 16 laisse 4 px de marge. À rétablir quand le
// chemin tiendra compte du rayon des monstres (`docs/monstre.md`).
constexpr Profil s_creatureMonstrueuse = {.couleur = {90, 30, 140}, .rayon = 16.0f, .vitesse = 140.0f,
                                          .pointsDeVieMax = 1500, .armure = 6, .degats = 70, .rechargeMin = 1.5f,
                                          .rechargeMax = 2.5f, .execution = 0.5f, .capaciteOffensive = 120,
                                          .capaciteDefensive = 110, .esquive = 0};

DefinitionDeMonstre definitionDuProfil(const Profil& profil) {
    return {.cote = 2.0f * profil.rayon,
            .couleur = profil.couleur,
            .vitesse = profil.vitesse,
            .pointsDeVieMax = profil.pointsDeVieMax,
            .armure = profil.armure,
            .rayonDetection = s_rayonDetection,
            .rayonTerritoire = s_rayonTerritoire,
            .zoneDeControle = s_zoneDeControle,
            .regenerationRetour = s_regenerationRetour,
            .attaque = {.type = TypeSkill::DegatsSurCible, .portee = s_porteeAttaque, .puissance = profil.degats,
                        .rechargeMin = profil.rechargeMin, .rechargeMax = profil.rechargeMax,
                        .tempsExecution = profil.execution, .nature = NatureDAttaque::Contact},
            .statistiques = {.capaciteOffensive = profil.capaciteOffensive,
                             .capaciteDefensive = profil.capaciteDefensive,
                             .esquiveContact = profil.esquive, .esquiveProjectile = profil.esquive}};
}
}  // namespace

const CatalogueDeMonstres& catalogueDeMonstres() {
    static const CatalogueDeMonstres catalogue = {definitionDuProfil(s_nuee), definitionDuProfil(s_infanterie),
                                                  definitionDuProfil(s_infanterieLourde),
                                                  definitionDuProfil(s_creatureMonstrueuse)};
    return catalogue;
}

const DefinitionDeMonstre& definitionDeMonstre(const CatalogueDeMonstres& catalogue, CategorieDeMonstre categorie) {
    return catalogue[static_cast<std::size_t>(categorie)];
}

const DefinitionDeMonstre& definitionDeMonstre(CategorieDeMonstre categorie) {
    return definitionDeMonstre(catalogueDeMonstres(), categorie);
}
