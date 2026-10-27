// ./src/combat/TypeDArme.hpp
#pragma once

// Le type d'une arme (`objets/DonneesDArme`) : ce qu'un skill peut exiger pour être lancé (`DefinitionDeSkill`),
// sans restriction par classe (modèle de *Titan Quest*). Les mains nues ne sont pas une arme : elles n'ont pas de type.
// Treize gabarits (étape 102) : huit armes de contact (cinq à une main, la lance comprise, et trois à deux mains) et cinq
// armes de tir (la baguette et le sceptre à une main, le bâton, l'arc et l'arbalète à deux mains).
enum class TypeDArme {
    Dague,
    EpeeAUneMain,
    HacheAUneMain,
    MasseAUneMain,
    Lance,
    EpeeADeuxMains,
    HacheADeuxMains,
    MasseADeuxMains,
    Baguette,
    Sceptre,
    Baton,
    Arc,
    Arbalete
};
