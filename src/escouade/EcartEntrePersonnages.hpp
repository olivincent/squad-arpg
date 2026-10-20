// ./src/escouade/EcartEntrePersonnages.hpp
#pragma once

// Distance minimale voulue entre deux centres de personnages (≈ (20 + 4) × √2 : 4 px visibles même en
// diagonale) ; aussi entre deux places, ou une place et un personnage immobile. En-tête à part :
// la formation, la séparation et les places d'attaque en ont besoin sans dépendre l'une de l'autre.
inline constexpr float s_ecartEntrePersonnages = 34.0f;
