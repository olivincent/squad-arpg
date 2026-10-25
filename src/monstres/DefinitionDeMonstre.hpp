// ./src/monstres/DefinitionDeMonstre.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <array>
#include "combat/Skill.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "monde/CategorieDeMonstre.hpp"

// Les valeurs d'équilibrage d'un monstre, nommées à la construction (initialisation désignée) : un monstre
// ne choisit rien, il reçoit sa définition. Une définition par catégorie (`catalogueDeMonstres`).
struct DefinitionDeMonstre {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    float cote;                // du carré, en pixels ; son cercle de collision en est inscrit (rayon = cote / 2)
    sf::Color couleur;         // celle de sa catégorie, pour le reconnaître à l'écran
    float vitesse;             // en pixels par seconde
    int pointsDeVieMax;
    int armure;
    float rayonDetection;      // en pixels
    float rayonTerritoire;     // distance maximale au poste au-delà de laquelle le monstre abandonne, en pixels
    float zoneDeControle;      // en pixels, bord à bord : tout monstre engage ce qui est à cette distance
    float regenerationRetour;  // fraction des points de vie maximum rendue par seconde pendant le retour au poste
    DefinitionDeSkill attaque; // portée bord à bord, dégâts avant l'armure de la cible, recharge, exécution
    StatistiquesDeCombat statistiques;  // un monstre n'a pas de caractéristiques : il reçoit ses statistiques
};

// Une définition par catégorie, dans l'ordre de `CategorieDeMonstre`.
using CatalogueDeMonstres = std::array<DefinitionDeMonstre, s_nombreDeCategoriesDeMonstre>;

// Les définitions de toutes les catégories : l'unique source de leurs valeurs.
[[nodiscard]] const CatalogueDeMonstres& catalogueDeMonstres();

// La définition de cette catégorie, dans ce catalogue.
[[nodiscard]] const DefinitionDeMonstre& definitionDeMonstre(const CatalogueDeMonstres& catalogue,
                                                             CategorieDeMonstre categorie);

// La définition de cette catégorie, dans le catalogue du jeu.
[[nodiscard]] const DefinitionDeMonstre& definitionDeMonstre(CategorieDeMonstre categorie);
