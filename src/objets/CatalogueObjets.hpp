// ./src/objets/CatalogueObjets.hpp
#pragma once
#include <string>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Le modèle d'un objet : ce qui est commun à toutes ses instances.
struct DefinitionObjet {
    IdObjet id;
    std::string nom;
    CategorieObjet categorie;
    int armure;
    Caracteristiques bonus = {};  // ajouté aux caractéristiques de qui le porte (vide : aucun bonus)
    StatistiquesDeCombat bonusDeCombat = {};  // ajouté aux statistiques de combat de qui le porte (esquive, capacités…)
};

// Tous les modèles, dans l'ordre de `IdObjet` : l'unique source de leurs valeurs.
[[nodiscard]] const std::vector<DefinitionObjet>& catalogueObjets();

// Le modèle de cet identifiant.
[[nodiscard]] const DefinitionObjet& definitionObjet(IdObjet id);

// Le contenu du sac au début d'une partie.
[[nodiscard]] std::vector<Objet> sacDeDepart();
