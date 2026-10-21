// ./src/interface/DispositionInventaire.hpp
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <variant>

#include "objets/Emplacement.hpp"

// Où se dessine chaque élément de l'écran d'inventaire, en coordonnées de la vue d'interface
// (`vueDInterface` : 800 px de haut, largeur selon le format). Fonctions pures, aucune position
// stockée : tout est recalculé depuis la taille de la vue. Le dessin, le survol et les tests
// appellent les mêmes fonctions.
//
// En haut, une colonne par personnage, dans l'ordre des places (de 1 à `s_nombreDePlaces`) : une
// bande d'en-tête (la classe), une ligne d'armure, puis les 10 emplacements en deux colonnes de 5
// (Tête, Torse, Jambes, Gants, Bottes à gauche ; Main droite, Main gauche, Collier, Anneau 1,
// Anneau 2 à droite : l'ordre de `Emplacement`). En bas, le sac : `s_casesDuSac` cases carrées,
// `s_colonnesDuSac` par rangée, dans l'ordre des indices. Tout tient dans une vue de
// 1200 × 800 au moins, sans chevauchement.

inline constexpr std::size_t s_nombreDePlaces = 3;
inline constexpr std::size_t s_colonnesDuSac = 10;
inline constexpr std::size_t s_casesDuSac = 40;  // la capacité du sac de l'escouade

// La bande d'en-tête de la colonne de cette place (le nom de la classe). `assert` : place de 1 à
// `s_nombreDePlaces`.
[[nodiscard]] sf::FloatRect rectangleDeLEnTete(std::size_t place, sf::Vector2f tailleDeVue);

// La ligne sous l'en-tête (« Armure : N », « (mort) »). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDeLArmure(std::size_t place, sf::Vector2f tailleDeVue);

// Le rectangle de cet emplacement pour le personnage de cette place. Même précondition.
[[nodiscard]] sf::FloatRect rectangleDeLEmplacement(std::size_t place, Emplacement emplacement, sf::Vector2f tailleDeVue);

// Le rectangle de cette case du sac. `assert(indice < s_casesDuSac)`.
[[nodiscard]] sf::FloatRect rectangleDeLaCaseDuSac(std::size_t indice, sf::Vector2f tailleDeVue);

// Ce que vise un point : l'emplacement d'un personnage, ou une case du sac.
struct ElementEmplacement {
    std::size_t place;
    Emplacement emplacement;

    bool operator==(const ElementEmplacement&) const = default;
};
struct ElementCase {
    std::size_t indice;

    bool operator==(const ElementCase&) const = default;
};
using ElementInventaire = std::variant<ElementEmplacement, ElementCase>;

// L'élément dont le rectangle contient ce point ; vide dans un écart ou hors des éléments.
[[nodiscard]] std::optional<ElementInventaire> elementSous(sf::Vector2f point, sf::Vector2f tailleDeVue);

// Le rectangle d'une infobulle de cette taille pour un curseur en `curseur` : sous et à droite du
// curseur, ramenée de l'autre côté du curseur si elle sortirait de la vue, puis gardée
// entièrement dans la vue. `tailleInfobulle` doit tenir dans `tailleDeVue`.
[[nodiscard]] sf::FloatRect placerInfobulle(sf::Vector2f curseur, sf::Vector2f tailleInfobulle, sf::Vector2f tailleDeVue);
