// ./src/interface/AffichageObjets.cpp
#include "interface/AffichageObjets.hpp"

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
    }
    return "";
}
