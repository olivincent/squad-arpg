// ./src/objets/GenerationDObjets.hpp
#pragma once
#include "objets/Affixe.hpp"
#include "objets/Objet.hpp"

// La chance relative de cette rareté d'être tirée (commun 70, magique 25, rare 5 : valeurs provisoires, « à tester »).
// Des poids, pas des pour cent : seul leur rapport compte, comme pour les compositions de groupe.
[[nodiscard]] int poidsDeRarete(Rarete rarete);

// Un objet de ce modèle peut-il avoir cette rareté ? Commun : toujours ; magique : s'il peut porter au moins un affixe ;
// rare : s'il peut porter au moins un préfixe et un suffixe (les bijoux n'ont aucun préfixe : ils ne sont jamais rares).
// L'unique endroit de cette règle ; le tirage ne choisit parmi les modèles que ceux qui la satisfont.
[[nodiscard]] bool peutPorterLaRarete(IdObjet modele, Rarete rarete);
