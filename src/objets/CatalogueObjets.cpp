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

// Armes (valeurs provisoires, à tester). Une attaque de contact engage (zone de contrôle), un projectile non.
constexpr float s_porteeDeContact = 20.0f;        // bord à bord
constexpr float s_porteeDeProjectile = 120.0f;    // bord à bord
constexpr float s_executionArmeLegere = 0.15f;    // en secondes
constexpr float s_executionArmeLourde = 0.3f;
constexpr float s_rechargeTirMin = 0.4f;          // en secondes, les armes de tir ont le même intervalle
constexpr float s_rechargeTirMax = 0.6f;
constexpr int s_degatsEpeeLongue = 15;
constexpr int s_degatsMasse = 15;
constexpr int s_degatsDague = 14;
constexpr int s_degatsEpeeADeuxMains = 60;
constexpr int s_degatsSceptre = 15;
constexpr int s_degatsBaton = 15;
constexpr int s_degatsArc = 15;
constexpr int s_degatsMainsNues = 6;

DonneesDArme armeDeMelee(TypeDArme type, int mains, int degats, float rechargeMin, float rechargeMax, float execution) {
    return {.type = type, .mains = mains,
            .attaque = {.degats = degats, .rechargeMin = rechargeMin, .rechargeMax = rechargeMax,
                        .tempsExecution = execution, .portee = s_porteeDeContact, .nature = NatureDAttaque::Contact}};
}

DonneesDArme armeDeTir(TypeDArme type, int mains, int degats) {
    return {.type = type, .mains = mains,
            .attaque = {.degats = degats, .rechargeMin = s_rechargeTirMin, .rechargeMax = s_rechargeTirMax,
                        .tempsExecution = s_executionArmeLegere, .portee = s_porteeDeProjectile,
                        .nature = NatureDAttaque::Projectile}};
}

std::vector<DefinitionObjet> construireLeCatalogue() {
    return {
        {IdObjet::CasqueDeFer, "Casque de fer", CategorieObjet::Casque, s_armureCasqueDeFer},
        {IdObjet::CuirasseDeMailles, "Cuirasse de mailles", CategorieObjet::Armure, s_armureCuirasseDeMailles},
        {IdObjet::JambieresDeFer, "Jambières de fer", CategorieObjet::Jambieres, s_armureJambieresDeFer},
        {IdObjet::BouclierRond, "Bouclier rond", CategorieObjet::Bouclier, s_armureBouclierRond, {}, {}, std::nullopt,
         DonneesDeBouclier{.chanceDeBlocage = s_chanceDeBlocageBouclierRond, .montantBloque = s_montantBloqueBouclierRond}},
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
        {IdObjet::EpeeLongue, "Épée longue", CategorieObjet::Arme, 0, {}, {},
         armeDeMelee(TypeDArme::EpeeLongue, 1, s_degatsEpeeLongue, 0.3f, 0.7f, s_executionArmeLegere)},
        {IdObjet::Masse, "Masse", CategorieObjet::Arme, 0, {}, {},
         armeDeMelee(TypeDArme::Masse, 1, s_degatsMasse, 0.3f, 0.7f, s_executionArmeLegere)},
        {IdObjet::Dague, "Dague", CategorieObjet::Arme, 0, {}, {},
         armeDeMelee(TypeDArme::Dague, 1, s_degatsDague, 0.4f, 0.8f, s_executionArmeLegere)},
        {IdObjet::EpeeADeuxMains, "Épée à deux mains", CategorieObjet::Arme, 0, {}, {},
         armeDeMelee(TypeDArme::EpeeADeuxMains, 2, s_degatsEpeeADeuxMains, 1.2f, 2.8f, s_executionArmeLourde)},
        {IdObjet::Sceptre, "Sceptre", CategorieObjet::Arme, 0, {}, {}, armeDeTir(TypeDArme::Sceptre, 1, s_degatsSceptre)},
        {IdObjet::Baton, "Bâton", CategorieObjet::Arme, 0, {}, {}, armeDeTir(TypeDArme::Baton, 2, s_degatsBaton)},
        {IdObjet::Arc, "Arc", CategorieObjet::Arme, 0, {}, {}, armeDeTir(TypeDArme::Arc, 2, s_degatsArc)},
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

const DescriptionDAttaque& mainsNues() {
    static const DescriptionDAttaque s_mainsNues = {.degats = s_degatsMainsNues, .rechargeMin = s_rechargeTirMin,
                                                    .rechargeMax = s_rechargeTirMax, .tempsExecution = s_executionArmeLegere,
                                                    .portee = s_porteeDeContact, .nature = NatureDAttaque::Contact};
    return s_mainsNues;
}

bool estADeuxMains(IdObjet id) {
    const std::optional<DonneesDArme>& arme = definitionObjet(id).arme;
    return arme.has_value() && arme->mains == 2;
}

std::vector<Objet> sacDeDepart() {
    return {{IdObjet::GantsDeCuir},         {IdObjet::BottesDeCuir},         {IdObjet::CasqueDeCuir},
            {IdObjet::TuniqueMatelassee},   {IdObjet::CollierDeProtection},  {IdObjet::AnneauDeProtection},
            {IdObjet::AnneauDeProtection}, {IdObjet::AnneauDePuissance},     {IdObjet::CollierDEndurance},
            {IdObjet::Dague},               {IdObjet::EpeeADeuxMains}};
}
