// ./src/interface/AffichageObjets.hpp
#pragma once

#include <string>
#include <vector>

#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Les textes de l'interface pour les objets, en UTF-8 (à passer à `sf::String::fromUtf8`).
// Le noyau (`objets/`) ne connaît que les identifiants ; les mots à l'écran sont ici.

// « Tête », « Torse », « Main droite »…
[[nodiscard]] std::string nomDeLEmplacement(Emplacement emplacement);

// « Casque », « Armure », « Anneau »…
[[nodiscard]] std::string nomDeLaCategorie(CategorieObjet categorie);

// Deux lettres, pour une case du sac (« CF » pour le casque de fer).
[[nodiscard]] std::string sigleDeLObjet(IdObjet id);

// Les lignes de l'infobulle d'un objet : son nom, sa catégorie, son armure (« Armure : +10 », absente si
// elle est nulle) et une ligne par bonus de caractéristique (« Puissance : +3 »).
[[nodiscard]] std::vector<std::string> lignesDeLInfobulle(IdObjet id);
