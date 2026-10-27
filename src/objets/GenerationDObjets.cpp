// ./src/objets/GenerationDObjets.cpp
#include "objets/GenerationDObjets.hpp"
#include "base/Aleatoire.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <vector>
#include "objets/CatalogueAffixes.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
// Les poids des raretés au niveau 1, puis ce que chaque niveau au-delà du premier y change : le commun recule (jusqu'à son
// plancher), le magique et le rare avancent (valeurs provisoires, « à tester »).
constexpr int s_poidsCommun = 70;
constexpr int s_poidsMagique = 25;
constexpr int s_poidsRare = 5;
constexpr int s_perteDeCommunParNiveau = 5;
constexpr int s_plancherDeCommun = 20;
constexpr int s_gainDeMagiqueParNiveau = 3;
constexpr int s_gainDeRareParNiveau = 2;

// Un élément au hasard, uniformément.
template <typename T>
const T& tirerParmi(Generateur& generateur, const std::vector<T>& elements) {
    assert(!elements.empty());
    return elements[static_cast<std::size_t>(tirerEntre(generateur, 0, static_cast<int>(elements.size()) - 1))];
}
}  // namespace

int poidsDeRarete(Rarete rarete, int niveau) {
    assert(niveau >= 1);
    const int niveauxAuDela = niveau - 1;
    switch (rarete) {
        case Rarete::Commun: return std::max(s_plancherDeCommun, s_poidsCommun - s_perteDeCommunParNiveau * niveauxAuDela);
        case Rarete::Magique: return s_poidsMagique + s_gainDeMagiqueParNiveau * niveauxAuDela;
        case Rarete::Rare: return s_poidsRare + s_gainDeRareParNiveau * niveauxAuDela;
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
// Une rareté au hasard, selon les poids de ce niveau.
Rarete tirerRarete(Generateur& generateur, int niveau) {
    int poidsTotal = 0;
    for (const Rarete rarete : s_raretes) {
        poidsTotal += poidsDeRarete(rarete, niveau);
    }
    int reste = tirerEntre(generateur, 0, poidsTotal - 1);
    for (const Rarete rarete : s_raretes) {
        if (reste < poidsDeRarete(rarete, niveau)) {
            return rarete;
        }
        reste -= poidsDeRarete(rarete, niveau);
    }
    assert(false);  // le tirage est dans [0, poidsTotal)
    return Rarete::Commun;
}

// Un modèle au hasard, uniformément parmi ceux qui peuvent porter cette rareté et qui existent déjà à ce niveau (leur niveau
// d'apparition).
IdObjet tirerModele(Generateur& generateur, Rarete rarete, int niveau) {
    std::vector<IdObjet> possibles;
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (definition.niveauDApparition <= niveau && peutPorterLaRarete(definition.id, rarete)) {
            possibles.push_back(definition.id);
        }
    }
    return tirerParmi(generateur, possibles);
}

// Cet affixe, avec une valeur au hasard dans sa plage à ce niveau.
Affixe avecUneValeur(Generateur& generateur, IdAffixe id, int niveau) {
    const PlageDeValeurs plage = plageAuNiveau(id, niveau);
    return {id, tirerEntre(generateur, plage.min, plage.max)};
}
}  // namespace

Objet genererObjet(Generateur& generateur, int niveau) {
    assert(niveau >= 1);
    const Rarete rarete = tirerRarete(generateur, niveau);
    Objet objet{tirerModele(generateur, rarete, niveau), rarete};
    // La qualité de tout objet, arme ou non : l'ordre des tirages ne change plus quand d'autres objets en dépendront.
    objet.niveau = niveau;
    objet.qualite = tirerEntre(generateur, 0, s_qualiteMaximale);
    const CategorieObjet categorie = definitionObjet(objet.modele).categorie;
    const std::vector<IdAffixe> prefixes = affixesPermisPour(categorie, PositionDAffixe::Prefixe);
    const std::vector<IdAffixe> suffixes = affixesPermisPour(categorie, PositionDAffixe::Suffixe);

    switch (rarete) {
        case Rarete::Commun: break;
        case Rarete::Magique: {
            std::vector<IdAffixe> tous = prefixes;
            tous.insert(tous.end(), suffixes.begin(), suffixes.end());
            const Affixe affixe = avecUneValeur(generateur, tirerParmi(generateur, tous), niveau);
            (definitionAffixe(affixe.id).position == PositionDAffixe::Prefixe ? objet.prefixe : objet.suffixe) = affixe;
            break;
        }
        case Rarete::Rare:
            objet.prefixe = avecUneValeur(generateur, tirerParmi(generateur, prefixes), niveau);
            objet.suffixe = avecUneValeur(generateur, tirerParmi(generateur, suffixes), niveau);
            break;
    }
    assert(objetValide(objet));
    return objet;
}

std::vector<Objet> genererButin(Generateur& generateur, int chancePourCent, int nombre, int niveau) {
    assert(chancePourCent >= 0 && chancePourCent <= 100);
    if (chancePourCent <= 0 || (chancePourCent < 100 && tirerEntre(generateur, 1, 100) > chancePourCent)) {
        return {};
    }
    std::vector<Objet> butin;
    butin.reserve(static_cast<std::size_t>(nombre));
    for (int i = 0; i < nombre; ++i) {
        butin.push_back(genererObjet(generateur, niveau));
    }
    return butin;
}
