// ./src/objets/Emplacement.hpp
#pragma once
#include <cstddef>

// Où un personnage porte un objet. Dix emplacements, un objet au plus par emplacement.
enum class Emplacement { Tete, Torse, Jambes, Gants, Bottes, MainDroite, MainGauche, Collier, Anneau1, Anneau2 };

inline constexpr std::size_t s_nombreEmplacements = 10;

// Ce qu'est un objet. Distinct de l'emplacement : deux emplacements (les anneaux) reçoivent la
// même catégorie. Pas encore de catégorie d'arme : la main droite reste vide.
enum class CategorieObjet { Casque, Armure, Jambieres, Gants, Bottes, Bouclier, Collier, Anneau };

// Un objet de cette catégorie peut-il être porté à cet emplacement ? Seule source de la règle.
[[nodiscard]] bool peutAller(CategorieObjet categorie, Emplacement emplacement);
