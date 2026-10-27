// ./src/objets/GenerationDObjets.hpp
#pragma once
#include <array>
#include <vector>
#include "base/Aleatoire.hpp"
#include "objets/Affixe.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// La chance relative de cette rareté d'être tirée pour un objet de ce niveau (le niveau d'un objet est la profondeur où il est
// tiré). Au niveau 1 : commun 70, magique 25, rare 5. Pour chaque niveau au-delà du premier : commun −5 (jusqu'à un plancher
// de 20), magique +3, rare +2 ; au niveau 5, 50 / 37 / 13 (valeurs provisoires, « à tester »). Toujours positifs. Des poids,
// pas des pour cent : seul leur rapport compte, comme pour les compositions de groupe. `assert` : `niveau >= 1`.
[[nodiscard]] int poidsDeRarete(Rarete rarete, int niveau);

// Les quatre familles de butin : le premier tirage du modèle d'un objet. La famille se déduit de la catégorie (`familleDe`),
// elle n'est stockée nulle part.
enum class FamilleDeButin { Arme, Armure, MainGauche, Bijou };
inline constexpr std::array s_famillesDeButin = {FamilleDeButin::Arme, FamilleDeButin::Armure, FamilleDeButin::MainGauche,
                                                 FamilleDeButin::Bijou};

// La famille d'une catégorie : armes ; casque, armure, jambières, gants, bottes ; bouclier et relique (la main gauche) ; collier
// et anneau (les bijoux). Seule source de la règle.
[[nodiscard]] FamilleDeButin familleDe(CategorieObjet categorie);

// La chance relative de cette famille d'être tirée : Arme 35, Armure 35, Main gauche 15, Bijou 15 (valeurs provisoires, « à
// tester »). Les mêmes à toute profondeur : débloquer des gabarits (l'armure intermédiaire, la lourde) ne change pas la part
// des familles. Des poids, pas des pour cent : seul leur rapport compte.
[[nodiscard]] int poidsDeFamille(FamilleDeButin famille);

// Un objet de ce modèle peut-il avoir cette rareté ? Commun : toujours ; magique : s'il peut porter au moins un affixe ;
// rare : s'il peut porter au moins un préfixe et un suffixe (les bijoux n'ont aucun préfixe : ils ne sont jamais rares).
// L'unique endroit de cette règle ; le tirage ne choisit parmi les modèles que ceux qui la satisfont.
[[nodiscard]] bool peutPorterLaRarete(IdObjet modele, Rarete rarete);

// Tire un objet au hasard, toujours valide (`objetValide`) :
// 1. la rareté, selon `poidsDeRarete` à ce niveau ;
// 2. le modèle, en deux temps : la famille de butin, selon `poidsDeFamille`, parmi celles qui ont au moins un modèle possible
//    (qui peut porter cette rareté, `peutPorterLaRarete`, et dont le niveau d'apparition est atteint) ; puis un modèle
//    uniformément parmi les modèles possibles de cette famille ;
// 3. la qualité, un entier uniforme de 0 à `s_qualiteMaximale`, pour TOUT objet (arme ou non : l'ordre des tirages ne dépend
//    pas du modèle) ;
// 4. magique : un affixe uniformément parmi TOUS ceux que la catégorie permet, préfixes et suffixes confondus, placé selon
//    sa position ; rare : un préfixe et un suffixe, chacun uniformément parmi ceux que la catégorie permet ;
// 5. chaque valeur : un entier uniforme dans la plage de l'affixe à ce niveau (`plageAuNiveau`, la même pour un objet magique
//    et pour un objet rare).
// `niveau` : le niveau de l'objet, la profondeur où il est tiré (`assert` : au moins 1) ; l'objet le retient.
// Fonction sans état : le seul hasard est le `Generateur` reçu, que la `Partie` possède. La suite d'objets d'une graine
// dépend de la bibliothèque standard (`std::uniform_int_distribution` n'est pas défini par la norme, contrairement à
// `std::mt19937`) : sans conséquence tant que rien n'est sauvegardé ni partagé (`docs/objets.md`).
[[nodiscard]] Objet genererObjet(Generateur& generateur, int niveau);

// Le butin d'une mort : d'abord la chance (en pour cent), puis, si le butin tombe, `nombre` objets tirés par `genererObjet`, de ce
// niveau.
// Un seul tirage pour la chance ; aucun quand elle vaut 0 (jamais de butin) ou 100 (toujours), comme le moteur du coup
// ne tire pas pour une chance nulle. Sans butin, le résultat est vide et le `Generateur` n'a pas avancé.
[[nodiscard]] std::vector<Objet> genererButin(Generateur& generateur, int chancePourCent, int nombre, int niveau);
