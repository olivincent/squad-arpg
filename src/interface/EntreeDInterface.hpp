// ./src/interface/EntreeDInterface.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <optional>

namespace sf {
class Event;
}  // namespace sf

struct Contexte;

// Un point (clic ou curseur) en coordonnées de la vue de l'interface, avec le centre de cette
// vue : les écrans de menu centrent leurs boutons sur `centre`.
struct PointDInterface {
    sf::Vector2f point;
    sf::Vector2f centre;
};

// Le point d'un clic gauche de la souris, en coordonnées de la vue de l'interface ; vide pour
// tout autre événement (autre bouton, autre type).
[[nodiscard]] std::optional<PointDInterface> clicGaucheDInterface(const sf::Event& evenement, const Contexte& contexte);

// Le point du relâchement du bouton gauche, en coordonnées de la vue de l'interface ; vide pour
// tout autre événement. Pour le glisser-déposer.
[[nodiscard]] std::optional<PointDInterface> relachementGaucheDInterface(const sf::Event& evenement, const Contexte& contexte);

// Pose la vue de l'interface sur la fenêtre et renvoie la position du curseur dans cette vue.
// Début du `dessiner` d'un écran de menu ; ne vide pas la fenêtre (c'est l'`Application`).
[[nodiscard]] PointDInterface preparerLInterface(const Contexte& contexte);
