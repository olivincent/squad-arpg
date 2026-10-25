// ./src/interface/AffichageObjets.cpp
#include "interface/AffichageObjets.hpp"

#include <cmath>

#include "interface/AffichageCaracteristiques.hpp"
#include "interface/AffichageStatistiques.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
// Des secondes : « 3 » pour 3.0, « 0.3 » pour 0.3 (une décimale au plus, point décimal).
std::string secondes(float valeur) {
    const float arrondi = std::round(valeur * 10.0f) / 10.0f;
    if (arrondi == std::floor(arrondi)) {
        return std::to_string(static_cast<int>(arrondi));
    }
    std::string texte = std::to_string(arrondi);  // « 0.300000 »
    texte.erase(texte.find('.') + 2);
    return texte;
}

// Les lignes d'une arme : l'attaque de base qu'elle donne et, à deux mains, « Two-handed ».
void ajouterLesLignesDUneArme(std::vector<std::string>& lignes, const DonneesDArme& arme) {
    const DescriptionDAttaque& attaque = arme.attaque;
    lignes.push_back("Damage: " + std::to_string(attaque.degats.total()));
    lignes.push_back("Cooldown: " + (attaque.rechargeMin == attaque.rechargeMax
                                          ? secondes(attaque.rechargeMin)
                                          : secondes(attaque.rechargeMin) + " to " + secondes(attaque.rechargeMax)) + " s");
    lignes.push_back("Range: " + std::to_string(static_cast<int>(std::lround(attaque.portee))) + " px");
    // Le `switch` n'a pas de `default` : une nature ajoutée sans texte fait avertir le compilateur.
    switch (attaque.nature) {
        case NatureDAttaque::Contact: lignes.push_back("Melee"); break;
        case NatureDAttaque::Projectile: lignes.push_back("Projectile"); break;
        case NatureDAttaque::Zone: lignes.push_back("Area"); break;
    }
    if (attaque.penetration > 0) {
        lignes.push_back("Penetration: " + std::to_string(attaque.penetration) + " %");
    }
    if (arme.mains == 2) {
        lignes.push_back("Two-handed");
    }
}
}  // namespace

// Les trois `switch` n'ont pas de `default` : un emplacement, une catégorie ou un objet ajouté
// sans texte fait avertir le compilateur.

std::string nomDeLEmplacement(Emplacement emplacement) {
    switch (emplacement) {
        case Emplacement::Tete: return "Head";
        case Emplacement::Torse: return "Chest";
        case Emplacement::Jambes: return "Legs";
        case Emplacement::Gants: return "Hands";
        case Emplacement::Bottes: return "Feet";
        case Emplacement::MainDroite: return "Main Hand";
        case Emplacement::MainGauche: return "Off Hand";
        case Emplacement::Collier: return "Necklace";
        case Emplacement::Anneau1: return "Ring 1";
        case Emplacement::Anneau2: return "Ring 2";
    }
    return "";
}

std::string nomDeLaCategorie(CategorieObjet categorie) {
    switch (categorie) {
        case CategorieObjet::Casque: return "Helmet";
        case CategorieObjet::Armure: return "Body Armor";
        case CategorieObjet::Jambieres: return "Leggings";
        case CategorieObjet::Gants: return "Gloves";
        case CategorieObjet::Bottes: return "Boots";
        case CategorieObjet::Bouclier: return "Shield";
        case CategorieObjet::Collier: return "Necklace";
        case CategorieObjet::Anneau: return "Ring";
        case CategorieObjet::Arme: return "Weapon";
    }
    return "";
}

std::string sigleDeLObjet(IdObjet id) {
    switch (id) {
        case IdObjet::CasqueDeFer: return "IH";
        case IdObjet::CuirasseDeMailles: return "CM";
        case IdObjet::JambieresDeFer: return "IG";
        case IdObjet::BouclierRond: return "RS";
        case IdObjet::GantsDeCuir: return "LG";
        case IdObjet::BottesDeCuir: return "LB";
        case IdObjet::CasqueDeCuir: return "LC";
        case IdObjet::TuniqueMatelassee: return "PT";
        case IdObjet::CollierDeProtection: return "NP";
        case IdObjet::AnneauDeProtection: return "RP";
        case IdObjet::AnneauDePuissance: return "RM";
        case IdObjet::CollierDEndurance: return "NC";
        case IdObjet::EpeeLongue: return "LS";
        case IdObjet::Masse: return "MA";
        case IdObjet::Dague: return "DA";
        case IdObjet::EpeeADeuxMains: return "GS";
        case IdObjet::Sceptre: return "SC";
        case IdObjet::Baton: return "ST";
        case IdObjet::Arc: return "BO";
    }
    return "";
}

std::string ligneDeSeuil(const SeuilDeCaracteristique& seuil) {
    return "Requires " + nomDeLaCaracteristique(seuil.caracteristique) + " " + std::to_string(seuil.valeur);
}

std::string ligneDObjetInactif() {
    return "Inactive: requirement not met";
}

std::vector<std::string> lignesDeLInfobulle(IdObjet id) {
    const DefinitionObjet& definition = definitionObjet(id);
    std::vector<std::string> lignes = {definition.nom, nomDeLaCategorie(definition.categorie)};
    for (const SeuilDeCaracteristique& seuil : definition.seuils) {
        lignes.push_back(ligneDeSeuil(seuil));
    }
    if (definition.armure != 0) {
        lignes.push_back("Armor: +" + std::to_string(definition.armure));
    }
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (const int bonus = definition.bonus.valeur(caracteristique); bonus != 0) {
            lignes.push_back(nomDeLaCaracteristique(caracteristique) + ": " + (bonus > 0 ? "+" : "") + std::to_string(bonus));
        }
    }
    for (const std::string& ligne : lignesDeBonusDeCombat(definition.bonusDeCombat)) {
        lignes.push_back(ligne);
    }
    if (definition.bouclier.has_value()) {
        lignes.push_back("Block: " + std::to_string(definition.bouclier->chanceDeBlocage) + " %, " +
                         std::to_string(definition.bouclier->montantBloque) + " points");
    }
    if (definition.arme.has_value()) {
        ajouterLesLignesDUneArme(lignes, *definition.arme);
    }
    return lignes;
}
