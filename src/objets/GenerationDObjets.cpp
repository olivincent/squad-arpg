// ./src/objets/GenerationDObjets.cpp
#include "objets/GenerationDObjets.hpp"
#include "objets/CatalogueAffixes.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
// Les poids des raretés (valeurs provisoires, « à tester »).
constexpr int s_poidsCommun = 70;
constexpr int s_poidsMagique = 25;
constexpr int s_poidsRare = 5;
}  // namespace

int poidsDeRarete(Rarete rarete) {
    switch (rarete) {
        case Rarete::Commun: return s_poidsCommun;
        case Rarete::Magique: return s_poidsMagique;
        case Rarete::Rare: return s_poidsRare;
    }
    return 0;
}

bool peutPorterLaRarete(IdObjet modele, Rarete rarete) {
    const CategorieObjet categorie = definitionObjet(modele).categorie;
    const bool aUnPrefixe = !affixesPermisPour(categorie, PositionDAffixe::Prefixe).empty();
    const bool aUnSuffixe = !affixesPermisPour(categorie, PositionDAffixe::Suffixe).empty();
    switch (rarete) {
        case Rarete::Commun: return true;
        case Rarete::Magique: return aUnPrefixe || aUnSuffixe;
        case Rarete::Rare: return aUnPrefixe && aUnSuffixe;
    }
    return false;
}
