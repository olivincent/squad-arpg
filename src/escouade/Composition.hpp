// ./src/escouade/Composition.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/Skill.hpp"
#include "combat/SkillConnu.hpp"
#include "objets/Equipement.hpp"

// Rang d'un personnage dans la formation : devant ou derrière l'ancre.
enum class Rang { Avant, Arriere };

// Tout ce qu'il faut pour construire un personnage de l'escouade. L'escouade ne
// choisit aucune valeur : elle reçoit sa composition.
struct DefinitionPersonnage {
    sf::Color couleur;
    std::vector<SkillConnu> skills = {};
    std::vector<ObjetDeDepart> equipementDeDepart = {};  // son armure, son attaque de base (son arme) et sa zone de contrôle en viennent
    float rayonVision;
    Caracteristiques caracteristiques = {};  // la base de sa classe ; l'équipement s'y ajoute (ses points de vie en viennent : `pointsDeVieMax`)
    Rang rang = Rang::Avant;
};

// Décalage de chaque personnage par rapport à l'ancre, pour une escouade orientée vers +x,
// dans l'ordre de `rangs`. Les Avant sont sur x = +25, les Arriere sur x = -25 ; dans un
// rang, n personnages sont espacés de 50 px en y et centrés (y = (k - (n - 1) / 2) × 50,
// k dans l'ordre d'apparition).
[[nodiscard]] std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs);
