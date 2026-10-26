// ./src/monstres/DefinitionDeMonstre.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <array>
#include "combat/Degats.hpp"
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
    DefinitionDeSkill attaque; // portée bord à bord, recharge, exécution (sa puissance est 0 : les dégâts sont dans `degats`)
    Degats degats;             // les portions du coup, par élément, avant les défenses de la cible
    StatistiquesDeCombat statistiques;  // un monstre n'a pas de caractéristiques : il reçoit ses statistiques
    int chanceDeButin;         // en pour cent : la chance que sa mort laisse du butin (un seul tirage)
    int nombreDObjets;         // le nombre d'objets que laisse un butin
    int experience;            // ce que sa mort rapporte à l'escouade, quel que soit le tueur (« à tester »)
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

// Cette définition renforcée par la profondeur de l'étage (`profondeur` : 1 au premier étage, où elle est rendue identique).
// Pour chaque étage au-delà du premier : les points de vie maximum +25 %, les dégâts de l'attaque +15 %, les capacités
// offensive et défensive +10 ; chaque valeur est arrondie au plus proche, et le pourcentage des dégâts s'applique à **chaque
// portion** du coup (physique, feu, sacré), une portion nulle restant nulle. Le niveau passe par les nombres, sans formule d'écart
// de niveau. L'armure, l'esquive, le blocage, les résistances, la vitesse, la taille, la portée et la recharge de l'attaque et le
// butin ne changent pas. L'expérience que rapporte le monstre est majorée de 25 % par étage au-delà du premier, comme ses
// points de vie. Une fonction pure ; `assert` : profondeur d'au moins 1.
[[nodiscard]] DefinitionDeMonstre renforcer(const DefinitionDeMonstre& definition, int profondeur);
