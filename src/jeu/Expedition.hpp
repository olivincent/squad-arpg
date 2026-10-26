// ./src/jeu/Expedition.hpp
#pragma once
#include <vector>
#include "base/Aleatoire.hpp"
#include "combat/Caracteristiques.hpp"
#include "escouade/Composition.hpp"
#include "objets/Equipement.hpp"
#include "objets/Inventaire.hpp"

// Un personnage tel que l'expédition le garde d'un étage à l'autre.
struct MembreDExpedition {
    // Sa classe : couleur, skills, rayon de vision et rang, qui ne changent pas ; `caracteristiques` y est sa base, qui
    // grandira (expérience, points libres). Son `equipementDeDepart` est vide : le kit de départ est devenu `equipement`.
    DefinitionPersonnage definition;
    // Ce qu'il porte, objets complets (rareté, affixes).
    Equipement equipement;
};

// Ce qui survit d'un étage à l'autre, et rien d'autre : la profondeur, la graine, les personnages (dans l'ordre des places) et
// le sac commun. Aucune position, aucun combat, aucun monstre ni objet au sol : tout cela appartient à l'étage (`Partie`), qui
// se reconstruit en entier à chaque descente. Une valeur : la copier donne une autre expédition. Elle grandira (expérience,
// points libres) et sera, un jour, ce qu'on sauvegarde.
struct Expedition {  // NOLINT(cppcoreguidelines-pro-type-member-init): la graine n'a pas de valeur par défaut exprès, un oubli est signalé par le compilateur
    int profondeur = 1;                    // l'étage où l'on est : 1 au départ
    Generateur::result_type graine;        // celle de l'expédition ; chaque étage en dérive la sienne (`graineDEtage`)
    std::vector<MembreDExpedition> membres;
    Inventaire sac;
};

// Une expédition qui commence : profondeur 1, chaque personnage dans l'état de sa définition (équipement de départ porté,
// caractéristiques de sa classe), le sac de départ (`sacDeDepart`). `assert` : la composition n'est pas vide.
[[nodiscard]] Expedition nouvelleExpedition(std::vector<DefinitionPersonnage> composition, Generateur::result_type graine);

// Les personnages à construire pour l'étage où l'expédition est : leur définition, avec les caractéristiques et l'équipement
// qu'elle garde. Un personnage construit ainsi est à pleine vie, vivant, à sa place de formation : la vie ne se garde pas.
[[nodiscard]] std::vector<DefinitionPersonnage> compositionDeLExpedition(const Expedition& expedition);

// La graine de l'étage de cette profondeur : une fonction pure de la graine de l'expédition et de la profondeur. La même
// expédition donne donc les mêmes cartes, quels que soient les combats, et chaque profondeur a la sienne.
[[nodiscard]] Generateur::result_type graineDEtage(Generateur::result_type graineDExpedition, int profondeur);
