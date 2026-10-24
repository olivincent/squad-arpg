// ./src/monstres/DefinitionDeMonstre.cpp
#include "monstres/DefinitionDeMonstre.hpp"
#include "combat/TypeSkill.hpp"

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr float s_cote = 30.0f;                // côté du carré, en pixels
constexpr float s_vitesse = 160.0f;            // en pixels par seconde
constexpr int s_pointsDeVieMax = 1000;
constexpr int s_armure = 0;
constexpr float s_rayonDetection = 200.0f;     // en pixels
constexpr float s_rayonTerritoire = 600.0f;    // en pixels
constexpr float s_zoneDeControle = 20.0f;      // en pixels, bord à bord
constexpr float s_regenerationRetour = 0.5f;   // fraction des points de vie maximum rendue par seconde en retour
constexpr float s_porteeAttaque = 15.0f;       // bord à bord
constexpr int s_degatsAttaque = 100;
constexpr float s_rechargeAttaqueMin = 0.7f;   // en secondes
constexpr float s_rechargeAttaqueMax = 1.3f;
constexpr float s_executionAttaque = 0.3f;     // en secondes, immobile avant que le coup parte
constexpr int s_capaciteOffensive = 100;       // celle d'un personnage d'Adresse 10
constexpr int s_capaciteDefensive = 100;
}  // namespace

DefinitionDeMonstre monstreOrdinaire() {
    return {.cote = s_cote,
            .vitesse = s_vitesse,
            .pointsDeVieMax = s_pointsDeVieMax,
            .armure = s_armure,
            .rayonDetection = s_rayonDetection,
            .rayonTerritoire = s_rayonTerritoire,
            .zoneDeControle = s_zoneDeControle,
            .regenerationRetour = s_regenerationRetour,
            .attaque = {.type = TypeSkill::Degats, .portee = s_porteeAttaque, .puissance = s_degatsAttaque,
                        .rechargeMin = s_rechargeAttaqueMin, .rechargeMax = s_rechargeAttaqueMax,
                        .tempsExecution = s_executionAttaque},
            .statistiques = {.capaciteOffensive = s_capaciteOffensive, .capaciteDefensive = s_capaciteDefensive}};
}
