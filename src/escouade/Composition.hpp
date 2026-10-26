// ./src/escouade/Composition.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/Progression.hpp"
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
    // Sa recette d'équipement : des noms de modèles, lue une seule fois, par `nouvelleExpedition`, qui la convertit en
    // `Equipement`. Ni `Personnage` ni `Escouade` ne la lisent : ils reçoivent l'`Equipement` (`MembreDEscouade`).
    std::vector<ObjetDeDepart> equipementDeDepart = {};
    float rayonVision;
    Caracteristiques caracteristiques = {};  // la base de sa classe, au niveau 1 ; la croissance de ses niveaux, puis l'équipement s'y ajoutent
    // Les deux caractéristiques qui grandissent avec le niveau, en alternance (`croissanceAuNiveau`) ; par défaut Puissance et
    // Endurance, celles du guerrier.
    PaireDeCroissance croissance = {Caracteristique::Puissance, Caracteristique::Endurance};
    Rang rang = Rang::Avant;
};

// Un personnage à construire : sa définition, ce qu'il porte (objets complets : rareté, affixes) et ses points dépensés.
// L'équipement a une seule
// source, l'expédition ; son armure, son attaque de base (son arme) et sa zone de contrôle en viennent.
struct MembreDEscouade {
    DefinitionPersonnage definition;
    Equipement equipement;
    // Les points libres qu'il a déjà dépensés, par caractéristique (`combat/Progression`) : ils s'ajoutent à sa base.
    Caracteristiques pointsDepenses;
};

// Décalage de chaque personnage par rapport à l'ancre, pour une escouade orientée vers +x,
// dans l'ordre de `rangs`. Les Avant sont sur x = +25, les Arriere sur x = -25 ; dans un
// rang, n personnages sont espacés de 50 px en y et centrés (y = (k - (n - 1) / 2) × 50,
// k dans l'ordre d'apparition).
[[nodiscard]] std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs);
