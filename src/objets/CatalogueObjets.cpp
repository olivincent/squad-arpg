// ./src/objets/CatalogueObjets.cpp
#include "objets/CatalogueObjets.hpp"
#include <cassert>
#include <cstddef>

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr int s_armureCasqueDeFer = 2;
constexpr int s_armureCuirasseDeMailles = 4;
constexpr int s_armureJambieresDeFer = 2;
constexpr int s_armureBouclierRond = 1;
constexpr int s_armureGantsDeCuir = 1;
constexpr int s_armureBottesDeCuir = 1;
constexpr int s_armureCasqueDeCuir = 1;
constexpr int s_armureTuniqueMatelassee = 2;
constexpr int s_armureCollierDeProtection = 1;
constexpr int s_armureAnneauDeProtection = 1;
constexpr int s_bonusAnneauDePuissance = 3;    // Puissance
constexpr int s_bonusCollierDEndurance = 3;    // Endurance
constexpr int s_esquiveBottesDeCuir = 5;       // en pour cent, contre le contact
constexpr int s_esquiveCasqueDeCuir = 5;       // en pour cent, contre les projectiles
constexpr int s_chanceDeBlocageBouclierRond = 30;       // en pour cent
constexpr int s_montantBloqueBouclierRond = 12;         // points absorbés par blocage
constexpr float s_recuperationBouclierRond = 3.0f;      // en secondes

std::vector<DefinitionObjet> construireLeCatalogue() {
    return {
        {IdObjet::CasqueDeFer, "Casque de fer", CategorieObjet::Casque, s_armureCasqueDeFer},
        {IdObjet::CuirasseDeMailles, "Cuirasse de mailles", CategorieObjet::Armure, s_armureCuirasseDeMailles},
        {IdObjet::JambieresDeFer, "Jambières de fer", CategorieObjet::Jambieres, s_armureJambieresDeFer},
        {IdObjet::BouclierRond, "Bouclier rond", CategorieObjet::Bouclier, s_armureBouclierRond, {},
         {.chanceDeBlocage = s_chanceDeBlocageBouclierRond, .montantBloque = s_montantBloqueBouclierRond,
          .recuperationDuBlocage = s_recuperationBouclierRond}},
        {IdObjet::GantsDeCuir, "Gants de cuir", CategorieObjet::Gants, s_armureGantsDeCuir},
        {IdObjet::BottesDeCuir, "Bottes de cuir", CategorieObjet::Bottes, s_armureBottesDeCuir, {},
         {.esquiveContact = s_esquiveBottesDeCuir}},
        {IdObjet::CasqueDeCuir, "Casque de cuir", CategorieObjet::Casque, s_armureCasqueDeCuir, {},
         {.esquiveProjectile = s_esquiveCasqueDeCuir}},
        {IdObjet::TuniqueMatelassee, "Tunique matelassée", CategorieObjet::Armure, s_armureTuniqueMatelassee},
        {IdObjet::CollierDeProtection, "Collier de protection", CategorieObjet::Collier,
         s_armureCollierDeProtection},
        {IdObjet::AnneauDeProtection, "Anneau de protection", CategorieObjet::Anneau,
         s_armureAnneauDeProtection},
        {IdObjet::AnneauDePuissance, "Anneau de puissance", CategorieObjet::Anneau, 0,
         Caracteristiques::seule(Caracteristique::Puissance, s_bonusAnneauDePuissance)},
        {IdObjet::CollierDEndurance, "Collier d'endurance", CategorieObjet::Collier, 0,
         Caracteristiques::seule(Caracteristique::Endurance, s_bonusCollierDEndurance)},
    };
}
}  // namespace

const std::vector<DefinitionObjet>& catalogueObjets() {
    static const std::vector<DefinitionObjet> s_catalogue = construireLeCatalogue();
    return s_catalogue;
}

const DefinitionObjet& definitionObjet(IdObjet id) {
    const DefinitionObjet& trouvee = catalogueObjets()[static_cast<std::size_t>(id)];
    assert(trouvee.id == id);  // le catalogue suit l'ordre de IdObjet
    return trouvee;
}

std::vector<Objet> sacDeDepart() {
    return {{IdObjet::GantsDeCuir},         {IdObjet::BottesDeCuir},         {IdObjet::CasqueDeCuir},
            {IdObjet::TuniqueMatelassee},   {IdObjet::CollierDeProtection},  {IdObjet::AnneauDeProtection},
            {IdObjet::AnneauDeProtection}, {IdObjet::AnneauDePuissance},     {IdObjet::CollierDEndurance}};
}
