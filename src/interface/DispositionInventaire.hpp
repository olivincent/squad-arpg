// ./src/interface/DispositionInventaire.hpp
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>

#include "objets/Emplacement.hpp"
#include "objets/PositionDObjet.hpp"

// Où se dessine chaque élément de l'écran d'inventaire, en coordonnées de la vue d'interface
// (`vueDInterface` : 800 px de haut, largeur selon le format). Fonctions pures, aucune position
// stockée : tout est recalculé depuis la taille de la vue. Le dessin, le survol et les tests
// appellent les mêmes fonctions.
//
// En haut, une colonne par personnage, dans l'ordre des places (de 1 à `s_nombreDePlaces`) : une
// bande d'en-tête (la classe), une ligne d'armure, puis les 10 emplacements en deux colonnes de 5
// (Tête, Torse, Jambes, Gants, Bottes à gauche ; Main droite, Main gauche, Collier, Anneau 1,
// Anneau 2 à droite : l'ordre de `Emplacement`), puis le bloc des caractéristiques et des points de vie, puis
// le bloc des statistiques de combat. En bas, le sac : `s_casesDuSac` cases carrées,
// `s_colonnesDuSac` par rangée (deux rangées), dans l'ordre des indices. Tout tient dans une vue de
// 1200 × 800 au moins, sans chevauchement.

inline constexpr std::size_t s_nombreDePlaces = 3;
inline constexpr std::size_t s_colonnesDuSac = 20;
inline constexpr std::size_t s_lignesDeCaracteristiques = 3;  // du bloc des caractéristiques
inline constexpr std::size_t s_lignesDeStatistiques = 3;       // du bloc des statistiques de combat
inline constexpr std::size_t s_cellulesDeStatistiques = 2 * s_lignesDeStatistiques;  // en deux colonnes
inline constexpr std::size_t s_casesDuSac = 40;  // la capacité du sac de l'escouade

// Le panneau de l'inventaire : de la taille de son contenu (la même quelle que soit la vue), centré dans la vue, qui laisse
// voir le jeu tout autour. Ce qui est lâché en dehors de lui, hors de la « fenêtre d'inventaire », est jeté au sol.
[[nodiscard]] sf::FloatRect rectangleDuPanneau(sf::Vector2f tailleDeVue);

// La bande d'en-tête de la colonne de cette place (le nom de la classe). `assert` : place de 1 à
// `s_nombreDePlaces`.
[[nodiscard]] sf::FloatRect rectangleDeLEnTete(std::size_t place, sf::Vector2f tailleDeVue);

// La ligne sous l'en-tête (« Armure : N », « (mort) »). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDeLArmure(std::size_t place, sf::Vector2f tailleDeVue);

// Le rectangle de cet emplacement pour le personnage de cette place. Même précondition.
[[nodiscard]] sf::FloatRect rectangleDeLEmplacement(std::size_t place, Emplacement emplacement, sf::Vector2f tailleDeVue);

// Le bloc sous les emplacements : les cinq caractéristiques et les points de vie, en deux colonnes de
// `s_lignesDeCaracteristiques` lignes (`rectangleDeLaCaracteristique`). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDesCaracteristiques(std::size_t place, sf::Vector2f tailleDeVue);

// La cellule d'une ligne du bloc : l'indice 0 à 4 est la caractéristique de ce rang dans l'enum
// (`s_caracteristiques`), l'indice 5 les points de vie ; remplies colonne par colonne (Puissance,
// Endurance, Adresse, puis Savoir, Volonté, points de vie). `assert(indice <= s_nombreDeCaracteristiques)`.
[[nodiscard]] sf::FloatRect rectangleDeLaCellule(std::size_t place, std::size_t indice, sf::Vector2f tailleDeVue);

// Le bloc sous celui des caractéristiques : les statistiques de combat, en deux colonnes de
// `s_lignesDeStatistiques` lignes (`rectangleDeLaStatistique`). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDesStatistiques(std::size_t place, sf::Vector2f tailleDeVue);

// La cellule d'une ligne du bloc des statistiques, remplies colonne par colonne.
// `assert(indice < s_cellulesDeStatistiques)`.
[[nodiscard]] sf::FloatRect rectangleDeLaStatistique(std::size_t place, std::size_t indice, sf::Vector2f tailleDeVue);

// Le rectangle de cette case du sac. `assert(indice < s_casesDuSac)`.
[[nodiscard]] sf::FloatRect rectangleDeLaCaseDuSac(std::size_t indice, sf::Vector2f tailleDeVue);

// La position (emplacement d'un personnage ou case du sac) dont le rectangle contient ce point ; vide dans
// un écart ou hors des éléments.
[[nodiscard]] std::optional<PositionDObjet> elementSous(sf::Vector2f point, sf::Vector2f tailleDeVue);

// Le rectangle d'une infobulle de cette taille pour un curseur en `curseur` : sous et à droite du
// curseur, ramenée de l'autre côté du curseur si elle sortirait de la vue, puis gardée
// entièrement dans la vue. `tailleInfobulle` doit tenir dans `tailleDeVue`.
[[nodiscard]] sf::FloatRect placerInfobulle(sf::Vector2f curseur, sf::Vector2f tailleInfobulle, sf::Vector2f tailleDeVue);
