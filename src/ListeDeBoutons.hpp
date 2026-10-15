// ./src/ListeDeBoutons.hpp
#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

// Une colonne de boutons rectangulaires de même taille, centrée sur un point
// de la vue de l'interface. Ne garde aucune position : la disposition est
// recalculée à chaque appel à partir du `centre` reçu, qui change avec la
// résolution de la fenêtre.
class ListeDeBoutons {
public:
    explicit ListeDeBoutons(std::vector<std::string> libelles);

    // Rectangle du bouton `indice` (0 = le premier, en haut), en coordonnées
    // de la vue de l'interface ; `indice` doit être valide.
    [[nodiscard]] sf::FloatRect rectangleDuBouton(std::size_t indice, sf::Vector2f centre) const;

    // Un bouton inactif est grisé et n'est jamais sous le curseur : ni survol,
    // ni clic. Tous les boutons sont actifs à la construction.
    void definirActif(std::size_t indice, bool actif);

    // Indice du bouton actif qui contient `point`, vide si aucun.
    [[nodiscard]] std::optional<std::size_t> boutonSous(sf::Vector2f point, sf::Vector2f centre) const;

    // Dessine les boutons ; `survole` (le résultat de `boutonSous` sous le
    // curseur) est mis en surbrillance.
    void dessiner(sf::RenderTarget& rendu, const sf::Font& police, sf::Vector2f centre, std::optional<std::size_t> survole) const;

private:
    std::vector<std::string> m_libelles;
    std::vector<bool> m_actifs;
};
