// ./src/objets/CatalogueAffixes.cpp
#include "objets/CatalogueAffixes.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>

namespace {
// Les catégories des suffixes de caractéristique : toutes.
const std::vector<CategorieObjet> s_toutesLesCategories = {
    CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Gants,   CategorieObjet::Bottes,
    CategorieObjet::Bouclier, CategorieObjet::Collier, CategorieObjet::Anneau, CategorieObjet::Arme};

std::vector<DefinitionDAffixe> construireLeCatalogue() {
    return {
        {IdAffixe::Flamboyant, "Blazing", PositionDAffixe::Prefixe, EffetDAffixe::DegatsDeFeu, {CategorieObjet::Arme}},
        {IdAffixe::Sanctifie, "Hallowed", PositionDAffixe::Prefixe, EffetDAffixe::DegatsSacres, {CategorieObjet::Arme}},
        {IdAffixe::Acere, "Keen", PositionDAffixe::Prefixe, EffetDAffixe::PenetrationDArmure, {CategorieObjet::Arme}},
        {IdAffixe::Renforce, "Reinforced", PositionDAffixe::Prefixe, EffetDAffixe::Armure,
         {CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Bouclier}},
        {IdAffixe::DuBoeuf, "of the Ox", PositionDAffixe::Suffixe, EffetDAffixe::Puissance, s_toutesLesCategories},
        {IdAffixe::DeLOurs, "of the Bear", PositionDAffixe::Suffixe, EffetDAffixe::Endurance, s_toutesLesCategories},
        {IdAffixe::DuRenard, "of the Fox", PositionDAffixe::Suffixe, EffetDAffixe::Adresse, s_toutesLesCategories},
        {IdAffixe::DeLaChouette, "of the Owl", PositionDAffixe::Suffixe, EffetDAffixe::Savoir, s_toutesLesCategories},
        {IdAffixe::DuLion, "of the Lion", PositionDAffixe::Suffixe, EffetDAffixe::Volonte, s_toutesLesCategories},
        {IdAffixe::DEsquive, "of Evasion", PositionDAffixe::Suffixe, EffetDAffixe::EsquiveContact,
         {CategorieObjet::Bottes, CategorieObjet::Armure}},
        {IdAffixe::DeDeviation, "of Deflection", PositionDAffixe::Suffixe, EffetDAffixe::EsquiveProjectile,
         {CategorieObjet::Casque, CategorieObjet::Bouclier}},
        {IdAffixe::DeBlocage, "of Blocking", PositionDAffixe::Suffixe, EffetDAffixe::ChanceDeBlocage, {CategorieObjet::Bouclier}},
    };
}
}  // namespace

const std::vector<DefinitionDAffixe>& catalogueAffixes() {
    static const std::vector<DefinitionDAffixe> s_catalogue = construireLeCatalogue();
    return s_catalogue;
}

const DefinitionDAffixe& definitionAffixe(IdAffixe id) {
    const DefinitionDAffixe& trouvee = catalogueAffixes()[static_cast<std::size_t>(id)];
    assert(trouvee.id == id);  // le catalogue suit l'ordre de IdAffixe
    return trouvee;
}

bool affixePermis(IdAffixe id, CategorieObjet categorie) {
    const std::vector<CategorieObjet>& permises = definitionAffixe(id).categories;
    return std::ranges::find(permises, categorie) != permises.end();
}
