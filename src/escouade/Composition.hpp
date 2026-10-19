// ./src/escouade/Composition.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "combat/Skill.hpp"
#include "combat/SkillConnu.hpp"

// Rang d'un personnage dans la formation : devant ou derrière l'ancre.
enum class Rang { Avant, Arriere };

// Tout ce qu'il faut pour construire un personnage de l'escouade. L'escouade ne
// choisit aucune valeur : elle reçoit sa composition.
struct DefinitionPersonnage {
    sf::Color couleur;
    Skill attaqueDeBase;
    std::vector<SkillConnu> skills;
    int armure;
    float rayonVision;
    float zoneDeControle;  // en pixels, bord à bord ; 0 = n'engage personne
    Rang rang;
};

// Décalage de chaque personnage par rapport à l'ancre, pour une escouade orientée vers +x,
// dans l'ordre de `rangs`. Les Avant sont sur x = +25, les Arriere sur x = -25 ; dans un
// rang, n personnages sont espacés de 50 px en y et centrés (y = (k - (n - 1) / 2) × 50,
// k dans l'ordre d'apparition).
[[nodiscard]] std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs);
