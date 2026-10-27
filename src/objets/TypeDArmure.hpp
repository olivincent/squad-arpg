// ./src/objets/TypeDArmure.hpp
#pragma once

// Le type d'une pièce d'armure (torse, jambes, tête, gants, bottes) : du plus léger au plus lourd. Il fixe, dans le gabarit, la
// valeur d'armure de base, les seuils, le malus d'esquive et le niveau d'apparition de la pièce (`DefinitionObjet`).
enum class TypeDArmure { Tissu, Leger, Intermediaire, Lourd };
