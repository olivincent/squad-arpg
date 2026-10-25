// ./src/monde/CompositionDeGroupe.cpp
#include "monde/CompositionDeGroupe.hpp"

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr int s_poidsDeLaNuee = 30;
constexpr int s_poidsDeLInfanterie = 40;
constexpr int s_poidsDuMelange = 30;
constexpr int s_nueeMin = 5;
constexpr int s_nueeMax = 7;
constexpr int s_infanterieMin = 1;
constexpr int s_infanterieMax = 3;
constexpr int s_melangeInfanterieMin = 1;
constexpr int s_melangeInfanterieMax = 2;
constexpr int s_melangeLourdes = 1;
}  // namespace

std::vector<CompositionDeGroupe> compositionsOrdinaires() {
    return {
        {.effectifs = {{CategorieDeMonstre::Nuee, s_nueeMin, s_nueeMax}}, .poids = s_poidsDeLaNuee, .serre = true},
        {.effectifs = {{CategorieDeMonstre::Infanterie, s_infanterieMin, s_infanterieMax}},
         .poids = s_poidsDeLInfanterie},
        {.effectifs = {{CategorieDeMonstre::Infanterie, s_melangeInfanterieMin, s_melangeInfanterieMax},
                       {CategorieDeMonstre::InfanterieLourde, s_melangeLourdes, s_melangeLourdes}},
         .poids = s_poidsDuMelange},
    };
}

CompositionDeGroupe compositionDeLaCreature() {
    return {.effectifs = {{CategorieDeMonstre::CreatureMonstrueuse, 1, 1}}, .poids = 0};
}
