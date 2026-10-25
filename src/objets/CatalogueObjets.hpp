// ./src/objets/CatalogueObjets.hpp
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/DonneesDArme.hpp"
#include "objets/DonneesDeBouclier.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Une valeur minimale de caractéristique que le porteur doit avoir pour équiper un objet (étape 92).
struct SeuilDeCaracteristique {
    Caracteristique caracteristique;
    int valeur;

    bool operator==(const SeuilDeCaracteristique&) const = default;
};

// Le modèle d'un objet : ce qui est commun à toutes ses instances.
struct DefinitionObjet {
    IdObjet id;
    std::string nom;
    CategorieObjet categorie;
    int armure;
    Caracteristiques bonus = {};  // ajouté aux caractéristiques de qui le porte (vide : aucun bonus)
    StatistiquesDeCombat bonusDeCombat = {};  // ajouté aux statistiques de combat de qui le porte (esquive, capacités…)
    std::optional<DonneesDArme> arme = {};    // seulement pour une arme
    std::optional<DonneesDeBouclier> bouclier = {};  // seulement pour un bouclier : son blocage
    std::vector<SeuilDeCaracteristique> seuils = {};  // vide : aucun seuil (cuir, bijoux)
};

// Tous les modèles, dans l'ordre de `IdObjet` : l'unique source de leurs valeurs.
[[nodiscard]] const std::vector<DefinitionObjet>& catalogueObjets();

// Le modèle de cet identifiant.
[[nodiscard]] const DefinitionObjet& definitionObjet(IdObjet id);

// Ces caractéristiques atteignent-elles tous les seuils de cet objet ? Toujours vrai sans seuil.
[[nodiscard]] bool seuilsAtteints(IdObjet id, const Caracteristiques& caracteristiques);

// Cet objet prend-il les deux mains (une arme à deux mains) ?
[[nodiscard]] bool estADeuxMains(IdObjet id);

// L'attaque de base de qui ne porte aucune arme : les mains nues, qui ne sont pas un objet.
[[nodiscard]] const DescriptionDAttaque& mainsNues();

// Le contenu du sac au début d'une partie.
[[nodiscard]] std::vector<Objet> sacDeDepart();
