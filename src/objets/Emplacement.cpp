// ./src/objets/Emplacement.cpp
#include "objets/Emplacement.hpp"

bool peutAller(CategorieObjet categorie, Emplacement emplacement) {
    switch (categorie) {
        case CategorieObjet::Casque:    return emplacement == Emplacement::Tete;
        case CategorieObjet::Armure:    return emplacement == Emplacement::Torse;
        case CategorieObjet::Jambieres: return emplacement == Emplacement::Jambes;
        case CategorieObjet::Gants:     return emplacement == Emplacement::Gants;
        case CategorieObjet::Bottes:    return emplacement == Emplacement::Bottes;
        case CategorieObjet::Bouclier:
        case CategorieObjet::Relique:   return emplacement == Emplacement::MainGauche;  // une relique prend la place d'un bouclier
        case CategorieObjet::Arme:      return emplacement == Emplacement::MainDroite;
        case CategorieObjet::Collier:   return emplacement == Emplacement::Collier;
        case CategorieObjet::Anneau:
            return emplacement == Emplacement::Anneau1 || emplacement == Emplacement::Anneau2;
    }
    return false;
}
