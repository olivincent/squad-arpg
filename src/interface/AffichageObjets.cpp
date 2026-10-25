// ./src/interface/AffichageObjets.cpp
#include "interface/AffichageObjets.hpp"

#include "interface/AffichageCaracteristiques.hpp"
#include "interface/AffichageStatistiques.hpp"
#include "objets/CatalogueObjets.hpp"

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

std::vector<std::string> lignesDeLInfobulle(IdObjet id) {
    const DefinitionObjet& definition = definitionObjet(id);
    std::vector<std::string> lignes = {definition.nom, nomDeLaCategorie(definition.categorie)};
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
    return lignes;
}
