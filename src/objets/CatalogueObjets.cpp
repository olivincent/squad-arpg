// ./src/objets/CatalogueObjets.cpp
#include "objets/CatalogueObjets.hpp"
#include <cassert>
#include <cstddef>
#include <utility>

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
constexpr int s_penetrationDague = 30;  // en pour cent de l'armure ignorée

// Les exigences d'équipement (valeurs provisoires, « à tester ») : le seuil de chaque exigence est
// `s_seuilAuNiveau1 + pente × (niveau − 1)` (`seuilsDeLObjet`). La pente de la caractéristique principale d'un objet.
constexpr int s_penteSeuilPrincipale = 2;

// Cette définition, avec ces exigences.
DefinitionObjet avecExigences(DefinitionObjet definition, std::vector<ExigenceDeCaracteristique> exigences) {
    definition.exigences = std::move(exigences);
    return definition;
}

DonneesDArme armeDeMelee(TypeDArme type, int mains, int degats, float rechargeMin, float rechargeMax, float execution,
                         int penetration = 0) {
    return {.type = type, .mains = mains,
            .attaque = {.degats = plageFixe(degatsPhysiques(degats)), .rechargeMin = rechargeMin, .rechargeMax = rechargeMax,
                        .tempsExecution = execution, .portee = s_porteeDeContact, .nature = NatureDAttaque::Contact,
                        .penetration = penetration}};
}

DonneesDArme armeDeTir(TypeDArme type, int mains, int degats) {
    return {.type = type, .mains = mains,
            .attaque = {.degats = plageFixe(degatsPhysiques(degats)), .rechargeMin = s_rechargeTirMin, .rechargeMax = s_rechargeTirMax,
                        .tempsExecution = s_executionArmeLegere, .portee = s_porteeDeProjectile,
                        .nature = NatureDAttaque::Projectile, .penetration = 0}};
}

std::vector<DefinitionObjet> construireLeCatalogue() {
    return {
        avecExigences({IdObjet::CasqueDeFer, "Iron Helm", CategorieObjet::Casque, s_armureCasqueDeFer},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::CuirasseDeMailles, "Chain Mail", CategorieObjet::Armure, s_armureCuirasseDeMailles},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::JambieresDeFer, "Iron Greaves", CategorieObjet::Jambieres, s_armureJambieresDeFer},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::BouclierRond, "Round Shield", CategorieObjet::Bouclier, s_armureBouclierRond, {}, {}, std::nullopt,
                       DonneesDeBouclier{.chanceDeBlocage = s_chanceDeBlocageBouclierRond, .montantBloque = s_montantBloqueBouclierRond}},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        {IdObjet::GantsDeCuir, "Leather Gloves", CategorieObjet::Gants, s_armureGantsDeCuir},
        {IdObjet::BottesDeCuir, "Leather Boots", CategorieObjet::Bottes, s_armureBottesDeCuir, {},
         {.esquiveContact = s_esquiveBottesDeCuir}},
        {IdObjet::CasqueDeCuir, "Leather Cap", CategorieObjet::Casque, s_armureCasqueDeCuir, {},
         {.esquiveProjectile = s_esquiveCasqueDeCuir}},
        {IdObjet::TuniqueMatelassee, "Padded Tunic", CategorieObjet::Armure, s_armureTuniqueMatelassee},
        {IdObjet::CollierDeProtection, "Necklace of Protection", CategorieObjet::Collier,
         s_armureCollierDeProtection},
        {IdObjet::AnneauDeProtection, "Ring of Protection", CategorieObjet::Anneau,
         s_armureAnneauDeProtection},
        {IdObjet::AnneauDePuissance, "Ring of Might", CategorieObjet::Anneau, 0,
         Caracteristiques::seule(Caracteristique::Puissance, s_bonusAnneauDePuissance)},
        {IdObjet::CollierDEndurance, "Necklace of Constitution", CategorieObjet::Collier, 0,
         Caracteristiques::seule(Caracteristique::Endurance, s_bonusCollierDEndurance)},
        // Armes prototypes : leur caractéristique principale, jusqu'aux gabarits d'armes (étape 102.4).
        avecExigences({IdObjet::EpeeLongue, "Longsword", CategorieObjet::Arme, 0, {}, {},
                       armeDeMelee(TypeDArme::EpeeLongue, 1, s_degatsEpeeLongue, 0.3f, 0.7f, s_executionArmeLegere)},
                      {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::Masse, "Mace", CategorieObjet::Arme, 0, {}, {},
                       armeDeMelee(TypeDArme::Masse, 1, s_degatsMasse, 0.3f, 0.7f, s_executionArmeLegere)},
                      {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::Dague, "Dagger", CategorieObjet::Arme, 0, {}, {},
                       armeDeMelee(TypeDArme::Dague, 1, s_degatsDague, 0.4f, 0.8f, s_executionArmeLegere, s_penetrationDague)},
                      {{Caracteristique::Adresse, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::EpeeADeuxMains, "Greatsword", CategorieObjet::Arme, 0, {}, {},
                       armeDeMelee(TypeDArme::EpeeADeuxMains, 2, s_degatsEpeeADeuxMains, 1.2f, 2.8f, s_executionArmeLourde)},
                      {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::Sceptre, "Scepter", CategorieObjet::Arme, 0, {}, {}, armeDeTir(TypeDArme::Sceptre, 1, s_degatsSceptre)},
                      {{Caracteristique::Volonte, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::Baton, "Staff", CategorieObjet::Arme, 0, {}, {}, armeDeTir(TypeDArme::Baton, 2, s_degatsBaton)},
                      {{Caracteristique::Savoir, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::Arc, "Bow", CategorieObjet::Arme, 0, {}, {}, armeDeTir(TypeDArme::Arc, 2, s_degatsArc)},
                      {{Caracteristique::Adresse, s_penteSeuilPrincipale}}),
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
    static const DescriptionDAttaque s_mainsNues = {.degats = plageFixe(degatsPhysiques(s_degatsMainsNues)), .rechargeMin = s_rechargeTirMin,
                                                    .rechargeMax = s_rechargeTirMax, .tempsExecution = s_executionArmeLegere,
                                                    .portee = s_porteeDeContact, .nature = NatureDAttaque::Contact,
                                                    .penetration = 0};
    return s_mainsNues;
}

bool objetValide(const Objet& objet) {
    if (objet.niveau < 1 || objet.qualite < 0 || objet.qualite > s_qualiteMaximale) {
        return false;
    }
    const CategorieObjet categorie = definitionObjet(objet.modele).categorie;
    const auto affixeValide = [&](const std::optional<Affixe>& affixe, PositionDAffixe position) {
        return !affixe.has_value() || (affixe->valeur > 0 && definitionAffixe(affixe->id).position == position &&
                                       affixePermis(affixe->id, categorie));
    };
    if (!affixeValide(objet.prefixe, PositionDAffixe::Prefixe) || !affixeValide(objet.suffixe, PositionDAffixe::Suffixe)) {
        return false;
    }
    switch (objet.rarete) {
        case Rarete::Commun: return !objet.prefixe.has_value() && !objet.suffixe.has_value();
        case Rarete::Magique: return objet.prefixe.has_value() != objet.suffixe.has_value();
        case Rarete::Rare: return objet.prefixe.has_value() && objet.suffixe.has_value();
    }
    return false;
}

std::string nomDeLObjet(const Objet& objet) {
    std::string nom = definitionObjet(objet.modele).nom;
    if (objet.prefixe.has_value()) {
        nom = definitionAffixe(objet.prefixe->id).nom + " " + nom;
    }
    if (objet.suffixe.has_value()) {
        nom += " " + definitionAffixe(objet.suffixe->id).nom;
    }
    return nom;
}

std::vector<SeuilDeCaracteristique> seuilsDeLObjet(const Objet& objet) {
    std::vector<SeuilDeCaracteristique> seuils;
    for (const ExigenceDeCaracteristique& exigence : definitionObjet(objet.modele).exigences) {
        seuils.push_back({exigence.caracteristique, s_seuilAuNiveau1 + exigence.pente * (objet.niveau - 1)});
    }
    return seuils;
}

bool seuilsAtteints(const Objet& objet, const Caracteristiques& caracteristiques) {
    for (const SeuilDeCaracteristique& seuil : seuilsDeLObjet(objet)) {
        if (caracteristiques.valeur(seuil.caracteristique) < seuil.valeur) {
            return false;
        }
    }
    return true;
}

bool estADeuxMains(IdObjet id) {
    const std::optional<DonneesDArme>& arme = definitionObjet(id).arme;
    return arme.has_value() && arme->mains == 2;
}
