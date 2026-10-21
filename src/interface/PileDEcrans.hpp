// ./src/interface/PileDEcrans.hpp
#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "interface/Ecran.hpp"

// La pile d'écrans de l'`Application`, du plus bas au plus haut (le dernier est l'écran du dessus).
using PileDEcrans = std::vector<std::unique_ptr<Ecran>>;

// L'indice du premier écran à dessiner : le plus haut écran qui ne laisse pas voir dessous
// (`Ecran::laisseVoirDessous`), ou l'écran du dessus lui-même. On dessine ensuite, de bas en
// haut, jusqu'à l'écran du dessus. `pile` ne doit pas être vide.
[[nodiscard]] std::size_t premierEcranADessiner(const PileDEcrans& pile);
