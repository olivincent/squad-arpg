// ./src/interface/AffichageObjets.hpp
#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#include <vector>

#include "objets/Affixe.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Les textes de l'interface pour les objets, en UTF-8 (à passer à `sf::String::fromUtf8`).
// Le noyau (`objets/`) ne connaît que les identifiants ; les mots à l'écran sont ici.

// « Head », « Chest », « Main Hand »…
[[nodiscard]] std::string nomDeLEmplacement(Emplacement emplacement);

// « Helmet », « Body Armor », « Ring »…
[[nodiscard]] std::string nomDeLaCategorie(CategorieObjet categorie);

// Deux lettres, pour une case du sac (« IH » pour le casque de fer, Iron Helm).
[[nodiscard]] std::string sigleDeLObjet(IdObjet id);

// Une ligne de seuil : « Requires Might 11 ». Les lignes d'infobulle d'un objet à seuils la contiennent ; l'écran
// colore en rouge celles que le personnage sous le curseur n'atteint pas pendant un glissement.
[[nodiscard]] std::string ligneDeSeuil(const SeuilDeCaracteristique& seuil);

// La ligne que porte l'infobulle d'un objet porté dont le seuil n'est plus atteint : « Inactive: requirement not met ».
[[nodiscard]] std::string ligneDObjetInactif();

// La couleur du texte d'un objet selon sa rareté : blanc (commun), jaune (magique), vert (rare). Lisibles sur les fonds de
// l'inventaire (le plus clair est gris à 70 sur 255) ; le cadre rouge d'un objet inactif reste prioritaire.
[[nodiscard]] sf::Color couleurDeLaRarete(Rarete rarete);

// La ligne d'infobulle d'un affixe, avec sa valeur : « Fire Damage: +6 », « Holy Damage: +4 », « Penetration: +15 % »,
// « Armor: +3 », « Dexterity: +2 », « Dodge vs Melee: +3 % », « Dodge vs Projectiles: +4 % », « Block: +5 % ».
[[nodiscard]] std::string ligneDAffixe(const Affixe& affixe);

// Les lignes de l'infobulle d'un objet, dans cet ordre :
// - son nom composé (« Keen Bow of the Fox », `nomDeLObjet`) et sa catégorie ;
// - les propriétés de son modèle : son armure (« Armor: +10 », absente si elle est nulle), une ligne par bonus de
//   caractéristique (« Might: +3 »), une par bonus de combat (« Dodge vs Melee: +5 % » : `lignesDeBonusDeCombat`), pour un
//   bouclier son blocage (« Block: 30 %, 12 points ») et pour une arme son attaque (« Damage: 15 », « Cooldown: 0.3 to
//   0.7 s », « Range: 20 px », « Melee » ou « Projectile », « Penetration: 30 % » si l'arme perce l'armure, et
//   « Two-handed » à deux mains) ;
// - une ligne par affixe (`ligneDAffixe`), préfixe puis suffixe ;
// - les seuils du modèle (« Requires Might 11 »).
// La mention « inactif » n'en fait pas partie : elle dépend de l'endroit où l'objet est porté (`ligneDObjetInactif`).
[[nodiscard]] std::vector<std::string> lignesDeLInfobulle(const Objet& objet);
