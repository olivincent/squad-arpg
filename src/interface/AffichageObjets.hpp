// ./src/interface/AffichageObjets.hpp
#pragma once

#include <string>
#include <vector>

#include "objets/CatalogueObjets.hpp"
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

// Une ligne de seuil : « Exige Puissance 11 ». Les lignes d'infobulle d'un objet à seuils la contiennent ; l'écran
// colore en rouge celles que le personnage sous le curseur n'atteint pas pendant un glissement.
[[nodiscard]] std::string ligneDeSeuil(const SeuilDeCaracteristique& seuil);

// La ligne que porte l'infobulle d'un objet porté dont le seuil n'est plus atteint : « Inactif : seuil non atteint ».
[[nodiscard]] std::string ligneDObjetInactif();

// Les lignes de l'infobulle d'un objet : son nom, sa catégorie, ses seuils (« Exige Puissance 11 »), son armure (« Armure : +10 », absente si
// elle est nulle), une ligne par bonus de caractéristique (« Puissance : +3 »), une par bonus de combat
// (« Esquive contre la mêlée : +5 % » : `lignesDeBonusDeCombat`), pour un bouclier son blocage
// (« Blocage : 30 %, 12 points ») et pour une arme son attaque (« Dégâts : 15 », « Recharge : 0,3 à 0,7 s »,
// « Portée : 20 px », « Contact » ou « Projectile », et « Deux mains » à deux mains).
[[nodiscard]] std::vector<std::string> lignesDeLInfobulle(IdObjet id);
