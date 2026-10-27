// ./src/objets/CatalogueObjets.hpp
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/CatalogueAffixes.hpp"
#include "objets/DonneesDArme.hpp"
#include "objets/DonneesDeBouclier.hpp"
#include "objets/ObjetsActifs.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Le modèle d'un objet : ce qui est commun à toutes ses instances.
struct DefinitionObjet {
    IdObjet id;
    std::string nom;
    CategorieObjet categorie;
    int armure;
    Caracteristiques bonus = {};  // ajouté aux caractéristiques de qui le porte (vide : aucun bonus)
    StatistiquesDeCombat bonusDeCombat = {};  // ajouté aux statistiques de combat de qui le porte (esquive, capacités…)
    std::optional<DonneesDArme> arme = {};    // seulement pour une arme
    std::optional<DonneesDeBouclier> bouclier = {};  // seulement pour un bouclier : son blocage
    std::vector<SeuilDeCaracteristique> seuils = {};  // vide : aucun seuil (cuir, bijoux)
};

// Tous les modèles, dans l'ordre de `IdObjet` : l'unique source de leurs valeurs.
[[nodiscard]] const std::vector<DefinitionObjet>& catalogueObjets();

// Le modèle de cet identifiant.
[[nodiscard]] const DefinitionObjet& definitionObjet(IdObjet id);

// L'objet respecte-t-il ses invariants ? Une fonction de validation plutôt qu'une fonction de construction avec
// `assert` : `Objet` est un agrégat (on l'écrit en toutes lettres, comme le fait le tirage de `genererObjet`), donc aucune
// construction n'est obligée de passer par une fonction ; la validation, elle, se vérifie à chaque frontière
// (`Equipement::poser`, le sac, les tests de données) et se teste sans `assert`. Un objet est valide quand :
// - son niveau est d'au moins 1 et sa qualité va de 0 à `s_qualiteMaximale` ;
// - chaque affixe est à sa position (un préfixe dans `prefixe`, un suffixe dans `suffixe`), permis pour la catégorie de
//   son modèle (`affixePermis`), et de valeur strictement positive (le calcul des objets actifs suppose des bonus
//   positifs ou nuls) ;
// - le nombre d'affixes correspond à la rareté : commun, aucun ; magique, exactement un (préfixe ou suffixe) ; rare, un
//   préfixe et un suffixe.
[[nodiscard]] bool objetValide(const Objet& objet);

// Le nom affiché de cet objet : « [préfixe] modèle [suffixe] » (« Blazing Mace », « Keen Bow of the Fox »). L'anglais
// n'accorde pas : aucun genre ni nombre à gérer sur les modèles.
[[nodiscard]] std::string nomDeLObjet(const Objet& objet);

// Ces caractéristiques atteignent-elles tous les seuils de cet objet ? Toujours vrai sans seuil.
[[nodiscard]] bool seuilsAtteints(IdObjet id, const Caracteristiques& caracteristiques);

// Cet objet prend-il les deux mains (une arme à deux mains) ?
[[nodiscard]] bool estADeuxMains(IdObjet id);

// L'attaque de base de qui ne porte aucune arme : les mains nues, qui ne sont pas un objet.
[[nodiscard]] const DescriptionDAttaque& mainsNues();
