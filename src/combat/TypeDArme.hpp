// ./src/combat/TypeDArme.hpp
#pragma once

// Le type d'une arme (`objets/DonneesDArme`) : ce qu'un skill peut exiger pour être lancé (`DefinitionDeSkill`),
// sans restriction par classe (modèle de *Titan Quest*). Les mains nues ne sont pas une arme : elles n'ont pas de type.
enum class TypeDArme { EpeeLongue, Masse, Dague, EpeeADeuxMains, Sceptre, Baton, Arc };
