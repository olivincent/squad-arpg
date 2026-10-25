// ./src/interface/AffichageObjets.cpp
#include "interface/AffichageObjets.hpp"

#include <cmath>

#include "interface/AffichageCaracteristiques.hpp"
#include "interface/AffichageStatistiques.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
// Des secondes : « 3 » pour 3,0, « 0,3 » pour 0,3 (une décimale au plus, virgule française).
std::string secondes(float valeur) {
    const float arrondi = std::round(valeur * 10.0f) / 10.0f;
    if (arrondi == std::floor(arrondi)) {
        return std::to_string(static_cast<int>(arrondi));
    }
    std::string texte = std::to_string(arrondi);  // « 0.300000 »
    texte.erase(texte.find('.') + 2);
    texte[texte.find('.')] = ',';
    return texte;
}

// Les lignes d'une arme : l'attaque de base qu'elle donne et, à deux mains, « Deux mains ».
void ajouterLesLignesDUneArme(std::vector<std::string>& lignes, const DonneesDArme& arme) {
    const DescriptionDAttaque& attaque = arme.attaque;
    lignes.push_back("Dégâts : " + std::to_string(attaque.degats));
    lignes.push_back("Recharge : " + (attaque.rechargeMin == attaque.rechargeMax
                                          ? secondes(attaque.rechargeMin)
                                          : secondes(attaque.rechargeMin) + " à " + secondes(attaque.rechargeMax)) + " s");
    lignes.push_back("Portée : " + std::to_string(static_cast<int>(std::lround(attaque.portee))) + " px");
    // Le `switch` n'a pas de `default` : une nature ajoutée sans texte fait avertir le compilateur.
    switch (attaque.nature) {
        case NatureDAttaque::Contact: lignes.push_back("Contact"); break;
        case NatureDAttaque::Projectile: lignes.push_back("Projectile"); break;
        case NatureDAttaque::Zone: lignes.push_back("Zone"); break;
    }
    if (arme.mains == 2) {
        lignes.push_back("Deux mains");
    }
}
}  // namespace

// Les trois `switch` n'ont pas de `default` : un emplacement, une catégorie ou un objet ajouté
// sans texte fait avertir le compilateur.

std::string nomDeLEmplacement(Emplacement emplacement) {
    switch (emplacement) {
        case Emplacement::Tete: return "Tête";
        case Emplacement::Torse: return "Torse";
        case Emplacement::Jambes: return "Jambes";
        case Emplacement::Gants: return "Gants";
        case Emplacement::Bottes: return "Bottes";
        case Emplacement::MainDroite: return "Main droite";
        case Emplacement::MainGauche: return "Main gauche";
        case Emplacement::Collier: return "Collier";
        case Emplacement::Anneau1: return "Anneau 1";
        case Emplacement::Anneau2: return "Anneau 2";
    }
    return "";
}

std::string nomDeLaCategorie(CategorieObjet categorie) {
    switch (categorie) {
        case CategorieObjet::Casque: return "Casque";
        case CategorieObjet::Armure: return "Armure";
        case CategorieObjet::Jambieres: return "Jambières";
        case CategorieObjet::Gants: return "Gants";
        case CategorieObjet::Bottes: return "Bottes";
        case CategorieObjet::Bouclier: return "Bouclier";
        case CategorieObjet::Collier: return "Collier";
        case CategorieObjet::Anneau: return "Anneau";
        case CategorieObjet::Arme: return "Arme";
    }
    return "";
}

std::string sigleDeLObjet(IdObjet id) {
    switch (id) {
        case IdObjet::CasqueDeFer: return "CF";
        case IdObjet::CuirasseDeMailles: return "CM";
        case IdObjet::JambieresDeFer: return "JF";
        case IdObjet::BouclierRond: return "BR";
        case IdObjet::GantsDeCuir: return "GC";
        case IdObjet::BottesDeCuir: return "BC";
        case IdObjet::CasqueDeCuir: return "CC";
        case IdObjet::TuniqueMatelassee: return "TM";
        case IdObjet::CollierDeProtection: return "CP";
        case IdObjet::AnneauDeProtection: return "AP";
        case IdObjet::AnneauDePuissance: return "AU";
        case IdObjet::CollierDEndurance: return "CE";
        case IdObjet::EpeeLongue: return "EL";
        case IdObjet::Masse: return "MA";
        case IdObjet::Dague: return "DA";
        case IdObjet::EpeeADeuxMains: return "E2";
        case IdObjet::Sceptre: return "SC";
        case IdObjet::Baton: return "BT";
        case IdObjet::Arc: return "AR";
    }
    return "";
}

std::string ligneDeSeuil(const SeuilDeCaracteristique& seuil) {
    return "Exige " + nomDeLaCaracteristique(seuil.caracteristique) + " " + std::to_string(seuil.valeur);
}

std::string ligneDObjetInactif() {
    return "Inactif : seuil non atteint";
}

std::vector<std::string> lignesDeLInfobulle(IdObjet id) {
    const DefinitionObjet& definition = definitionObjet(id);
    std::vector<std::string> lignes = {definition.nom, nomDeLaCategorie(definition.categorie)};
    for (const SeuilDeCaracteristique& seuil : definition.seuils) {
        lignes.push_back(ligneDeSeuil(seuil));
    }
    if (definition.armure != 0) {
        lignes.push_back("Armure : +" + std::to_string(definition.armure));
    }
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (const int bonus = definition.bonus.valeur(caracteristique); bonus != 0) {
            lignes.push_back(nomDeLaCaracteristique(caracteristique) + " : " + (bonus > 0 ? "+" : "") + std::to_string(bonus));
        }
    }
    for (const std::string& ligne : lignesDeBonusDeCombat(definition.bonusDeCombat)) {
        lignes.push_back(ligne);
    }
    if (definition.bouclier.has_value()) {
        lignes.push_back("Blocage : " + std::to_string(definition.bouclier->chanceDeBlocage) + " %, " +
                         std::to_string(definition.bouclier->montantBloque) + " points");
    }
    if (definition.arme.has_value()) {
        ajouterLesLignesDUneArme(lignes, *definition.arme);
    }
    return lignes;
}
