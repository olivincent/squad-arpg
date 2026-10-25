// ./src/monde/CompositionDeGroupe.hpp
#pragma once
#include <vector>
#include "monde/CategorieDeMonstre.hpp"

// Combien de monstres d'une catégorie un groupe contient : un nombre tiré dans [min, max], bornes incluses.
struct EffectifDeCategorie {
    CategorieDeMonstre categorie;
    int min;
    int max;
};

// La recette d'un groupe de monstres, en données : le générateur n'en connaît pas d'autre. Un groupe occupe une
// salle (ses monstres vont ensemble : ils s'alertent mutuellement).
struct CompositionDeGroupe {
    std::vector<EffectifDeCategorie> effectifs;  // dans l'ordre de placement
    int poids;                                   // chance relative d'être tirée parmi les compositions ordinaires
    bool serre = false;                          // placé en grappe : les monstres sont près du premier, case contre case
};

// Les compositions tirées au hasard pour peupler une salle : nuée (5 à 7 nuées), infanterie (1 à 3), mélange
// (1 à 2 infanteries et 1 infanterie lourde), de poids 30, 40 et 30. Valeurs provisoires, à tester.
[[nodiscard]] std::vector<CompositionDeGroupe> compositionsOrdinaires();

// La composition de la salle la plus éloignée du départ : une seule créature monstrueuse. Son poids n'est pas utilisé
// (elle n'est jamais tirée).
[[nodiscard]] CompositionDeGroupe compositionDeLaCreature();
