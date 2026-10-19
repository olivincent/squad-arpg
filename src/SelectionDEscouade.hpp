// ./src/SelectionDEscouade.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "Specialite.hpp"

// Les classes choisies pour l'escouade, dans l'ordre de sélection (c'est l'ordre des places 1 à 3).
// Une même classe peut être choisie plusieurs fois. Logique pure : ni écran ni SFML graphique.
class SelectionDEscouade {
public:
    static constexpr std::size_t s_taille = 3;  // nombre de classes d'une escouade

    // Ajoute à la fin s'il reste de la place ; sans effet sinon.
    void ajouter(IdSpecialite id);

    // Retire la classe de cette case ; les suivantes remontent. `assert(indice < liste().size())`.
    void retirer(std::size_t indice);

    [[nodiscard]] bool estComplete() const;
    [[nodiscard]] const std::vector<IdSpecialite>& liste() const;

private:
    std::vector<IdSpecialite> m_specialites;
};

// Rectangle de la case de sélection `indice` (0 à `s_taille` - 1) : une colonne de cases de
// 520 × 60 px, 20 px d'écart, centrée sur `centre`, la case 0 en haut. `assert(indice < s_taille)` ;
// `[[nodiscard]]`. Le dessin et le clic s'en servent tous les deux.
[[nodiscard]] sf::FloatRect rectangleDeLaCase(std::size_t indice, sf::Vector2f centre);

// Case de sélection qui contient le point ; vide dans un écart ou hors des cases. `[[nodiscard]]`.
[[nodiscard]] std::optional<std::size_t> caseSous(sf::Vector2f point, sf::Vector2f centre);
