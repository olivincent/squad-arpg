// ./src/interface/EmplacementsPrincipaux.hpp
#pragma once
#include <cstddef>

// Nombre d'emplacements de la barre principale (4 colonnes de 3 : une colonne par personnage).
// En-tête à part : les réglages (la table des touches) en ont besoin sans dépendre du modèle
// de la barre (BarreDeRaccourcis).
inline constexpr std::size_t s_emplacementsPrincipaux = 12;
