// ./src/interface/DispositionInventaire.hpp
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <variant>

#include "combat/Caracteristiques.hpp"
#include "escouade/TailleDeLEscouade.hpp"
#include "objets/Emplacement.hpp"
#include "combat/Place.hpp"
#include "objets/PositionDObjet.hpp"

// Où se dessine chaque élément de l'écran d'inventaire, en coordonnées de la vue d'interface
// (`vueDInterface` : 800 px de haut, largeur selon le format). Fonctions pures, aucune position
// stockée : tout est recalculé depuis la taille de la vue. Le dessin, le survol et les tests
// appellent les mêmes fonctions.
//
// En haut, une colonne par personnage, dans l'ordre des places (de 1 à `s_tailleDeLEscouade`) : une
// bande d'en-tête (la classe), une ligne d'armure, puis les 10 emplacements en deux colonnes de 5
// (Tête, Torse, Jambes, Gants, Bottes à gauche ; Main droite, Main gauche, Collier, Anneau 1,
// Anneau 2 à droite : l'ordre de `Emplacement`), puis le bloc des caractéristiques et des points de vie, puis
// le bloc des statistiques de combat. En bas, le sac : `s_casesDuSac` cases carrées,
// `s_colonnesDuSac` par rangée (deux rangées), dans l'ordre des indices. Tout tient dans une vue de
// 1200 × 800 au moins, sans chevauchement (les boutons des points libres sont dans les lignes du bloc des caractéristiques).

// Les places de 1 à `s_tailleDeLEscouade`, dans l'ordre : les colonnes.
inline constexpr std::array<Place, s_tailleDeLEscouade> s_places = [] {
    std::array<Place, s_tailleDeLEscouade> places{};
    for (std::size_t i = 0; i < places.size(); ++i) {
        places[i] = Place::depuisIndice(i);
    }
    return places;
}();
inline constexpr std::size_t s_colonnesDuSac = 20;
inline constexpr std::size_t s_lignesDeCaracteristiques = s_nombreDeCaracteristiques + 1;  // du bloc des caractéristiques : une par caractéristique, puis les points de vie
inline constexpr std::size_t s_lignesDeStatistiques = 3;       // du bloc des statistiques de combat
inline constexpr std::size_t s_cellulesDeStatistiques = 2 * s_lignesDeStatistiques;  // en deux colonnes
inline constexpr std::size_t s_casesDuSac = 40;  // la capacité du sac de l'escouade

// Le panneau de l'inventaire : de la taille de son contenu (la même quelle que soit la vue), centré dans la vue, qui laisse
// voir le jeu tout autour. Ce qui est lâché en dehors de lui, hors de la « fenêtre d'inventaire », est jeté au sol.
[[nodiscard]] sf::FloatRect rectangleDuPanneau(sf::Vector2f tailleDeVue);

// La bande d'en-tête de la colonne de cette place (le nom de la classe). `assert` : place de 1 à
// `s_tailleDeLEscouade`.
[[nodiscard]] sf::FloatRect rectangleDeLEnTete(Place place, sf::Vector2f tailleDeVue);

// La ligne sous l'en-tête (« Armure : N », « (mort) »). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDeLArmure(Place place, sf::Vector2f tailleDeVue);

// Le rectangle de cet emplacement pour le personnage de cette place. Même précondition.
[[nodiscard]] sf::FloatRect rectangleDeLEmplacement(Place place, Emplacement emplacement, sf::Vector2f tailleDeVue);

// Le bloc sous les emplacements : les cinq caractéristiques et les points de vie, en une colonne de
// `s_lignesDeCaracteristiques` lignes pleine largeur (la place des boutons des points libres à droite). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDesCaracteristiques(Place place, sf::Vector2f tailleDeVue);

// La ligne du bloc : l'indice 0 à 4 est la caractéristique de ce rang dans l'enum (`s_caracteristiques`), l'indice 5 les
// points de vie, de haut en bas. `assert(indice <= s_nombreDeCaracteristiques)`.
[[nodiscard]] sf::FloatRect rectangleDeLaCellule(Place place, std::size_t indice, sf::Vector2f tailleDeVue);

// Les boutons des points libres. Les rectangles sont toujours les mêmes : que le bouton soit visible ou actif, c'est à
// l'écran d'en décider (`EcranInventaire`). « + » et « − » sont à droite de la ligne de la caractéristique ; « Apply » et
// « Reset » à droite de la ligne des points de vie. `assert` : place de 1 à `s_tailleDeLEscouade`.
[[nodiscard]] sf::FloatRect rectangleDuBoutonPlus(Place place, Caracteristique caracteristique, sf::Vector2f tailleDeVue);
[[nodiscard]] sf::FloatRect rectangleDuBoutonMoins(Place place, Caracteristique caracteristique, sf::Vector2f tailleDeVue);
[[nodiscard]] sf::FloatRect rectangleDuBoutonAppliquer(Place place, sf::Vector2f tailleDeVue);
[[nodiscard]] sf::FloatRect rectangleDuBoutonReinitialiser(Place place, sf::Vector2f tailleDeVue);

// Un bouton des points libres : le « + » d'une caractéristique d'un personnage, son « − », « Apply » ou « Reset » de
// ce personnage (sa place).
struct BoutonPlus {
    Place place;
    Caracteristique caracteristique;

    bool operator==(const BoutonPlus&) const = default;
};
struct BoutonMoins {
    Place place;
    Caracteristique caracteristique;

    bool operator==(const BoutonMoins&) const = default;
};
struct BoutonAppliquer {
    Place place;

    bool operator==(const BoutonAppliquer&) const = default;
};
struct BoutonReinitialiser {
    Place place;

    bool operator==(const BoutonReinitialiser&) const = default;
};
using BoutonDePoints = std::variant<BoutonPlus, BoutonMoins, BoutonAppliquer, BoutonReinitialiser>;

// Le bouton dont le rectangle contient ce point ; vide ailleurs. Il ne dit pas si le bouton est visible : ils le sont ou
// non selon l'état de l'écran.
[[nodiscard]] std::optional<BoutonDePoints> boutonSous(sf::Vector2f point, sf::Vector2f tailleDeVue);

// La caractéristique d'un personnage dont la ligne contient ce point (boutons compris), pour son infobulle.
struct CaracteristiqueSurvolee {
    Place place;
    Caracteristique caracteristique;

    bool operator==(const CaracteristiqueSurvolee&) const = default;
};
// Vide si le point n'est sur la ligne d'aucune caractéristique.
[[nodiscard]] std::optional<CaracteristiqueSurvolee> caracteristiqueSous(sf::Vector2f point, sf::Vector2f tailleDeVue);

// Le bloc sous celui des caractéristiques : les statistiques de combat, en deux colonnes de
// `s_lignesDeStatistiques` lignes (`rectangleDeLaStatistique`). Même précondition.
[[nodiscard]] sf::FloatRect rectangleDesStatistiques(Place place, sf::Vector2f tailleDeVue);

// La cellule d'une ligne du bloc des statistiques, remplies colonne par colonne.
// `assert(indice < s_cellulesDeStatistiques)`.
[[nodiscard]] sf::FloatRect rectangleDeLaStatistique(Place place, std::size_t indice, sf::Vector2f tailleDeVue);

// Le rectangle de cette case du sac. `assert(indice < s_casesDuSac)`.
[[nodiscard]] sf::FloatRect rectangleDeLaCaseDuSac(std::size_t indice, sf::Vector2f tailleDeVue);

// La position (emplacement d'un personnage ou case du sac) dont le rectangle contient ce point ; vide dans
// un écart ou hors des éléments.
[[nodiscard]] std::optional<PositionDObjet> elementSous(sf::Vector2f point, sf::Vector2f tailleDeVue);

// Le rectangle d'une infobulle de cette taille pour un curseur en `curseur` : sous et à droite du
// curseur, ramenée de l'autre côté du curseur si elle sortirait de la vue, puis gardée
// entièrement dans la vue. `tailleInfobulle` doit tenir dans `tailleDeVue`.
[[nodiscard]] sf::FloatRect placerInfobulle(sf::Vector2f curseur, sf::Vector2f tailleInfobulle, sf::Vector2f tailleDeVue);
