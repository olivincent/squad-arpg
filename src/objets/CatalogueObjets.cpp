// ./src/objets/CatalogueObjets.cpp
#include "objets/CatalogueObjets.hpp"
#include <cassert>
#include <cstddef>

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr int s_armureCasqueDeFer = 10;
constexpr int s_armureCuirasseDeMailles = 20;
constexpr int s_armureJambieresDeFer = 10;
constexpr int s_armureBouclierRond = 10;
constexpr int s_armureGantsDeCuir = 4;
constexpr int s_armureBottesDeCuir = 4;
constexpr int s_armureCasqueDeCuir = 5;
constexpr int s_armureTuniqueMatelassee = 8;
constexpr int s_armureCollierDeProtection = 6;
constexpr int s_armureAnneauDeProtection = 3;

std::vector<DefinitionObjet> construireLeCatalogue() {
    return {
        {IdObjet::CasqueDeFer, "Casque de fer", CategorieObjet::Casque, s_armureCasqueDeFer},
        {IdObjet::CuirasseDeMailles, "Cuirasse de mailles", CategorieObjet::Armure, s_armureCuirasseDeMailles},
        {IdObjet::JambieresDeFer, "Jambières de fer", CategorieObjet::Jambieres, s_armureJambieresDeFer},
        {IdObjet::BouclierRond, "Bouclier rond", CategorieObjet::Bouclier, s_armureBouclierRond},
        {IdObjet::GantsDeCuir, "Gants de cuir", CategorieObjet::Gants, s_armureGantsDeCuir},
        {IdObjet::BottesDeCuir, "Bottes de cuir", CategorieObjet::Bottes, s_armureBottesDeCuir},
        {IdObjet::CasqueDeCuir, "Casque de cuir", CategorieObjet::Casque, s_armureCasqueDeCuir},
        {IdObjet::TuniqueMatelassee, "Tunique matelassée", CategorieObjet::Armure, s_armureTuniqueMatelassee},
        {IdObjet::CollierDeProtection, "Collier de protection", CategorieObjet::Collier,
         s_armureCollierDeProtection},
        {IdObjet::AnneauDeProtection, "Anneau de protection", CategorieObjet::Anneau,
         s_armureAnneauDeProtection},
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
            {IdObjet::AnneauDeProtection}};
}
