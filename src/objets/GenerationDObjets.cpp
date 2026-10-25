// ./src/objets/GenerationDObjets.cpp
#include "objets/GenerationDObjets.hpp"
#include "base/Aleatoire.hpp"
#include <cassert>
#include <cstddef>
#include <random>
#include <vector>
#include "objets/CatalogueAffixes.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
// Les poids des raretés (valeurs provisoires, « à tester »).
constexpr int s_poidsCommun = 70;
constexpr int s_poidsMagique = 25;
constexpr int s_poidsRare = 5;

// Un élément au hasard, uniformément.
template <typename T>
const T& tirerParmi(Generateur& generateur, const std::vector<T>& elements) {
    assert(!elements.empty());
    return elements[static_cast<std::size_t>(tirerEntre(generateur, 0, static_cast<int>(elements.size()) - 1))];
}
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

namespace {
// Une rareté au hasard, selon les poids.
Rarete tirerRarete(Generateur& generateur) {
    int poidsTotal = 0;
    for (const Rarete rarete : s_raretes) {
        poidsTotal += poidsDeRarete(rarete);
    }
    int reste = tirerEntre(generateur, 0, poidsTotal - 1);
    for (const Rarete rarete : s_raretes) {
        if (reste < poidsDeRarete(rarete)) {
            return rarete;
        }
        reste -= poidsDeRarete(rarete);
    }
    assert(false);  // le tirage est dans [0, poidsTotal)
    return Rarete::Commun;
}

// Un modèle au hasard, uniformément parmi ceux qui peuvent porter cette rareté.
IdObjet tirerModele(Generateur& generateur, Rarete rarete) {
    std::vector<IdObjet> possibles;
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (peutPorterLaRarete(definition.id, rarete)) {
            possibles.push_back(definition.id);
        }
    }
    return tirerParmi(generateur, possibles);
}

// Cet affixe, avec une valeur au hasard dans sa plage.
Affixe avecUneValeur(Generateur& generateur, IdAffixe id) {
    const PlageDeValeurs& plage = definitionAffixe(id).valeurs;
    return {id, tirerEntre(generateur, plage.min, plage.max)};
}
}  // namespace

Objet genererObjet(Generateur& generateur) {
    const Rarete rarete = tirerRarete(generateur);
    Objet objet{tirerModele(generateur, rarete), rarete};
    const CategorieObjet categorie = definitionObjet(objet.modele).categorie;
    const std::vector<IdAffixe> prefixes = affixesPermisPour(categorie, PositionDAffixe::Prefixe);
    const std::vector<IdAffixe> suffixes = affixesPermisPour(categorie, PositionDAffixe::Suffixe);

    switch (rarete) {
        case Rarete::Commun: break;
        case Rarete::Magique: {
            std::vector<IdAffixe> tous = prefixes;
            tous.insert(tous.end(), suffixes.begin(), suffixes.end());
            const Affixe affixe = avecUneValeur(generateur, tirerParmi(generateur, tous));
            (definitionAffixe(affixe.id).position == PositionDAffixe::Prefixe ? objet.prefixe : objet.suffixe) = affixe;
            break;
        }
        case Rarete::Rare:
            objet.prefixe = avecUneValeur(generateur, tirerParmi(generateur, prefixes));
            objet.suffixe = avecUneValeur(generateur, tirerParmi(generateur, suffixes));
            break;
    }
    assert(objetValide(objet));
    return objet;
}
