// ./src/interface/AffichageEffets.hpp
#pragma once
#include <string>

#include "combat/EffetActif.hpp"

// Les textes de l'interface pour les effets actifs (étape 111), en anglais (`docs/glossaire.md`). Le noyau ne connaît que les nombres.

// Le titre du bloc de l'inventaire : « Active effects ».
[[nodiscard]] std::string titreDesEffetsActifs();

// Le nom de la statistique qu'un modificateur change : « Armor ».
[[nodiscard]] std::string nomDeLaStatistique(StatistiqueModifiable statistique);

// La ligne d'un effet actif dans l'inventaire : le nom du skill qui l'a posé, la valeur et la durée restante en secondes entières
// (arrondie vers le haut : un effet encore actif n'affiche jamais « 0 s »), par exemple « Blessing: +20 % Armor (10 s) » ou
// « Power Strike: −25 % Armor (8 s) ». Le signe moins est le signe typographique (U+2212).
[[nodiscard]] std::string ligneDEffetActif(const EffetActif& effet);
