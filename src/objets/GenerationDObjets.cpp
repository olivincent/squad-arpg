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

// Les poids des familles de butin : les mêmes à toute profondeur (valeurs provisoires, « à tester »).
constexpr int s_poidsArme = 35;
constexpr int s_poidsArmure = 35;
constexpr int s_poidsMainGauche = 15;
constexpr int s_poidsBijou = 15;

// Un élément au hasard, uniformément.
template <typename T>
const T& tirerParmi(Generateur& generateur, const std::vector<T>& elements) {
    assert(!elements.empty());
    return elements[static_cast<std::size_t>(tirerEntre(generateur, 0, static_cast<int>(elements.size()) - 1))];
}

// Un élément au hasard, selon son poids (un entier positif ; le total doit être positif). Un seul tirage.
template <typename Conteneur, typename Poids>
typename Conteneur::value_type tirerSelonLesPoids(Generateur& generateur, const Conteneur& elements, Poids poids) {
    int poidsTotal = 0;
    for (const auto& element : elements) {
        poidsTotal += poids(element);
    }
    assert(poidsTotal > 0);
    int reste = tirerEntre(generateur, 0, poidsTotal - 1);
    for (const auto& element : elements) {
        if (reste < poids(element)) {
            return element;
        }
        reste -= poids(element);
    }
    assert(false);  // le tirage est dans [0, poidsTotal)
    return elements.front();
}
}  // namespace

FamilleDeButin familleDe(CategorieObjet categorie) {
    switch (categorie) {
        case CategorieObjet::Arme: return FamilleDeButin::Arme;
        case CategorieObjet::Casque:
        case CategorieObjet::Armure:
        case CategorieObjet::Jambieres:
        case CategorieObjet::Gants:
        case CategorieObjet::Bottes: return FamilleDeButin::Armure;
        case CategorieObjet::Bouclier:
        case CategorieObjet::Relique: return FamilleDeButin::MainGauche;
        case CategorieObjet::Collier:
        case CategorieObjet::Anneau: return FamilleDeButin::Bijou;
    }
    return FamilleDeButin::Arme;
}

int poidsDeFamille(FamilleDeButin famille) {
    switch (famille) {
        case FamilleDeButin::Arme: return s_poidsArme;
        case FamilleDeButin::Armure: return s_poidsArmure;
        case FamilleDeButin::MainGauche: return s_poidsMainGauche;
        case FamilleDeButin::Bijou: return s_poidsBijou;
    }
    return 0;
}

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
    return tirerSelonLesPoids(generateur, s_raretes, [niveau](Rarete rarete) { return poidsDeRarete(rarete, niveau); });
}

bool estDeLaFamille(IdObjet modele, FamilleDeButin famille) {
    return familleDe(definitionObjet(modele).categorie) == famille;
}

// Un modèle au hasard, en deux temps : une famille de butin, selon ses poids, parmi celles qui ont au moins un modèle possible ;
// puis un modèle uniformément dans cette famille. Possible : il peut porter cette rareté et il existe déjà à ce niveau (son
// niveau d'apparition).
IdObjet tirerModele(Generateur& generateur, Rarete rarete, int niveau) {
    std::vector<IdObjet> possibles;
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (definition.niveauDApparition <= niveau && peutPorterLaRarete(definition.id, rarete)) {
            possibles.push_back(definition.id);
        }
    }
    std::vector<FamilleDeButin> familles;
    for (const FamilleDeButin famille : s_famillesDeButin) {
        if (std::ranges::any_of(possibles, [famille](IdObjet modele) { return estDeLaFamille(modele, famille); })) {
            familles.push_back(famille);
        }
    }
    const FamilleDeButin famille = tirerSelonLesPoids(generateur, familles, poidsDeFamille);
    std::erase_if(possibles, [famille](IdObjet modele) { return !estDeLaFamille(modele, famille); });
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
    // Le poids d'un affixe permis : l'affinité du gabarit (1 sans affinité).
    const auto poids = [modele = objet.modele](IdAffixe id) { return affiniteDe(modele, id); };

    switch (rarete) {
        case Rarete::Commun: break;
        case Rarete::Magique: {
            std::vector<IdAffixe> tous = prefixes;
            tous.insert(tous.end(), suffixes.begin(), suffixes.end());
            const Affixe affixe = avecUneValeur(generateur, tirerSelonLesPoids(generateur, tous, poids), niveau);
            (definitionAffixe(affixe.id).position == PositionDAffixe::Prefixe ? objet.prefixe : objet.suffixe) = affixe;
            break;
        }
        case Rarete::Rare:
            objet.prefixe = avecUneValeur(generateur, tirerSelonLesPoids(generateur, prefixes, poids), niveau);
            objet.suffixe = avecUneValeur(generateur, tirerSelonLesPoids(generateur, suffixes, poids), niveau);
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
