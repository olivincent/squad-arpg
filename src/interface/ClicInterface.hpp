// ./src/interface/ClicInterface.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>

// Les barres de l'interface sur lesquelles on peut cliquer (la barre d'auras s'ajoutera ici).
enum class Barre { Raccourcis };

// Un clic tombé sur un élément de l'interface : quelle barre, quel emplacement.
struct ClicInterface {
    Barre barre;
    std::size_t emplacement;

    bool operator==(const ClicInterface&) const = default;
};

// Règle « l'interface d'abord » : ce que ce point (en coordonnées de la vue d'interface, de taille
// `tailleVue`) touche dans l'interface ; vide s'il tombe sur le monde. Un clic qui touche
// l'interface est consommé : il n'atteint jamais le monde.
[[nodiscard]] std::optional<ClicInterface> clicSurInterface(sf::Vector2f point, sf::Vector2f tailleVue);
