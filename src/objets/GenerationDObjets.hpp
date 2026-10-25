// ./src/objets/GenerationDObjets.hpp
#pragma once
#include <vector>
#include "base/Aleatoire.hpp"
#include "objets/Affixe.hpp"
#include "objets/Objet.hpp"

// La chance relative de cette rareté d'être tirée (commun 70, magique 25, rare 5 : valeurs provisoires, « à tester »).
// Des poids, pas des pour cent : seul leur rapport compte, comme pour les compositions de groupe.
[[nodiscard]] int poidsDeRarete(Rarete rarete);

// Un objet de ce modèle peut-il avoir cette rareté ? Commun : toujours ; magique : s'il peut porter au moins un affixe ;
// rare : s'il peut porter au moins un préfixe et un suffixe (les bijoux n'ont aucun préfixe : ils ne sont jamais rares).
// L'unique endroit de cette règle ; le tirage ne choisit parmi les modèles que ceux qui la satisfont.
[[nodiscard]] bool peutPorterLaRarete(IdObjet modele, Rarete rarete);

// Tire un objet au hasard, toujours valide (`objetValide`) :
// 1. la rareté, selon `poidsDeRarete` ;
// 2. le modèle, uniformément parmi ceux qui peuvent porter cette rareté (`peutPorterLaRarete`) ;
// 3. magique : un affixe uniformément parmi TOUS ceux que la catégorie permet, préfixes et suffixes confondus, placé selon
//    sa position ; rare : un préfixe et un suffixe, chacun uniformément parmi ceux que la catégorie permet ;
// 4. chaque valeur : un entier uniforme dans la plage de l'affixe (la même pour un objet magique et pour un objet rare).
// Fonction sans état : le seul hasard est le `Generateur` reçu, que la `Partie` possède. La suite d'objets d'une graine
// dépend de la bibliothèque standard (`std::uniform_int_distribution` n'est pas défini par la norme, contrairement à
// `std::mt19937`) : sans conséquence tant que rien n'est sauvegardé ni partagé (`docs/objets.md`).
[[nodiscard]] Objet genererObjet(Generateur& generateur);

// Le butin d'une mort : d'abord la chance (en pour cent), puis, si le butin tombe, `nombre` objets tirés par `genererObjet`.
// Un seul tirage pour la chance ; aucun quand elle vaut 0 (jamais de butin) ou 100 (toujours), comme le moteur du coup
// ne tire pas pour une chance nulle. Sans butin, le résultat est vide et le `Generateur` n'a pas avancé.
[[nodiscard]] std::vector<Objet> genererButin(Generateur& generateur, int chancePourCent, int nombre);
