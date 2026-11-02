// ./src/interface/AffichageEffets.hpp
#pragma once
#include <string>

#include "combat/EffetActif.hpp"

// Les textes de l'interface pour les effets actifs (étape 111), en anglais (`docs/glossaire.md`). Le noyau ne connaît que les nombres.

// Le titre du bloc de l'inventaire : « Active effects ».
[[nodiscard]] std::string titreDesEffetsActifs();

// Le nom d'un état de contrôle infligé (étape 113), tel que l'écran le dit : « Stunned », « Slowed », « Hampered », « Disrupted ». Le
// texte flottant à la pose et la ligne de l'inventaire le partagent.
[[nodiscard]] std::string nomDeLEtatInflige(EtatInflige etat);

// Le nom de la statistique qu'un modificateur change : « Armor ».
[[nodiscard]] std::string nomDeLaStatistique(StatistiqueModifiable statistique);

// La ligne d'un effet actif dans l'inventaire : son nom (celui du skill qui l'a posé ; pour des dégâts dans le temps, celui de leur
// élément : « Bleeding », « Burning »), « ×N » quand il a plus d'une pile, la valeur de l'effet entier (les piles comprises) et la durée
// restante en secondes entières (arrondie vers le haut : un effet encore actif n'affiche jamais « 0 s »), par exemple
// « Blessing: +20 % Armor (10 s) », « Power Strike: −25 % Armor (8 s) », « Bleeding ×2: 14 per s (4 s) » ou « Renew: 9 per s (6 s) ».
// Un contrôle porte le nom de son état et, pour le ralentissement et l'entrave, son pourcentage : « Stunned (2 s) »,
// « Slowed: −30 % (3 s) », « Hampered: −20 % (3 s) ».
// Le signe moins est le signe typographique (U+2212), le × aussi (U+00D7).
[[nodiscard]] std::string ligneDEffetActif(const EffetActif& effet);
