// ./src/monde/CompositionDeGroupe.cpp
#include "monde/CompositionDeGroupe.hpp"

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr int s_poidsDeLaNuee = 25;
constexpr int s_poidsDeLInfanterie = 25;
constexpr int s_poidsDuMelange = 20;
constexpr int s_poidsDesTireurs = 15;
constexpr int s_poidsDeLEscouade = 15;
constexpr int s_nueeMin = 5;
constexpr int s_nueeMax = 7;
constexpr int s_infanterieMin = 1;
constexpr int s_infanterieMax = 3;
constexpr int s_melangeInfanterieMin = 1;
constexpr int s_melangeInfanterieMax = 2;
constexpr int s_melangeLourdes = 1;
constexpr int s_tireursInfanterieMin = 1;
constexpr int s_tireursInfanterieMax = 2;
constexpr int s_tireursMin = 1;
constexpr int s_tireursMax = 2;
constexpr int s_escouadeInfanterieMin = 1;
constexpr int s_escouadeInfanterieMax = 2;
constexpr int s_escouadeTireurs = 1;
constexpr int s_escouadeSoigneurs = 1;
}  // namespace

std::vector<CompositionDeGroupe> compositionsOrdinaires() {
    return {
        {.effectifs = {{CategorieDeMonstre::Nuee, s_nueeMin, s_nueeMax}}, .poids = s_poidsDeLaNuee, .serre = true},
        {.effectifs = {{CategorieDeMonstre::Infanterie, s_infanterieMin, s_infanterieMax}},
         .poids = s_poidsDeLInfanterie},
        {.effectifs = {{CategorieDeMonstre::Infanterie, s_melangeInfanterieMin, s_melangeInfanterieMax},
                       {CategorieDeMonstre::InfanterieLourde, s_melangeLourdes, s_melangeLourdes}},
         .poids = s_poidsDuMelange},
        {.effectifs = {{CategorieDeMonstre::Infanterie, s_tireursInfanterieMin, s_tireursInfanterieMax},
                       {CategorieDeMonstre::Tireur, s_tireursMin, s_tireursMax}},
         .poids = s_poidsDesTireurs},
        {.effectifs = {{CategorieDeMonstre::Infanterie, s_escouadeInfanterieMin, s_escouadeInfanterieMax},
                       {CategorieDeMonstre::Tireur, s_escouadeTireurs, s_escouadeTireurs},
                       {CategorieDeMonstre::Soigneur, s_escouadeSoigneurs, s_escouadeSoigneurs}},
         .poids = s_poidsDeLEscouade},
    };
}

CompositionDeGroupe compositionDeLaCreature() {
    return {.effectifs = {{CategorieDeMonstre::CreatureMonstrueuse, 1, 1}}, .poids = 0};
}
