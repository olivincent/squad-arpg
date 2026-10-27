// ./src/combat/ModeDeCible.hpp
#pragma once

// COMMENT un skill vise, indépendamment de ce qu'il fait : c'est lui, et non le type d'effet, qui décide du choix de la
// cible, du déclenchement, de la mise en attente et de la visée à la souris.
// - `Soi` : aucune cible, le skill part du lanceur (la provocation) ;
// - `Monstre` : un monstre, celui du verrou ou celui que le lanceur frapperait en autodéfense ;
// - `Allie` : un personnage de l'escouade, le plus blessé ;
// - `Point` : un point du monde que le joueur désigne (visée à la souris).
enum class ModeDeCible { Soi, Monstre, Allie, Point };
