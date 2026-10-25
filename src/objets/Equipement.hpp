// ./src/objets/Equipement.hpp
#pragma once
#include <array>
#include <optional>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/DescriptionDAttaque.hpp"
#include "combat/TypeDArme.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Un objet que porte un personnage dès sa création.
struct ObjetDeDepart {
    Emplacement emplacement;
    IdObjet objet;

    bool operator==(const ObjetDeDepart&) const = default;
};

// Les caractéristiques effectives d'un porteur de ces caractéristiques de base et de cet équipement : la base plus les
// bonus des objets portés. Seul endroit qui les calcule (`Personnage` comme les déplacements d'objets s'en servent).
class Equipement;
[[nodiscard]] Caracteristiques caracteristiquesEffectives(const Caracteristiques& base, const Equipement& equipement);

// Les objets portés par un personnage : au plus un par emplacement. Possédé par valeur par
// chaque `Personnage`. Les objets passent d'un endroit à l'autre par valeur.
class Equipement {
public:
    // Équipé des objets de départ. `assert` : chaque objet peut aller à son emplacement
    // (`peutAller`) et deux entrées n'ont pas le même emplacement.
    explicit Equipement(const std::vector<ObjetDeDepart>& objetsDeDepart = {});

    // L'objet porté à cet emplacement, s'il y en a un.
    [[nodiscard]] const std::optional<Objet>& a(Emplacement emplacement) const;

    // Porte `objet` à cet emplacement et rend celui qui y était. `assert` : l'objet peut aller à
    // cet emplacement (`peutAller`) ; c'est à l'appelant de le vérifier.
    [[nodiscard]] std::optional<Objet> poser(Emplacement emplacement, Objet objet);

    // Enlève l'objet de cet emplacement et le rend (vide si l'emplacement l'était).
    [[nodiscard]] std::optional<Objet> retirer(Emplacement emplacement);

    // Somme de l'armure des objets portés, recalculée à chaque appel.
    [[nodiscard]] int armureTotale() const;

    // Somme des bonus de caractéristiques des objets portés, recalculée à chaque appel.
    [[nodiscard]] Caracteristiques bonusTotal() const;

    // Somme des bonus de statistiques de combat des objets portés (esquive, capacités…) et du blocage de leurs boucliers
    // (chance, montant), recalculée à chaque appel.
    [[nodiscard]] StatistiquesDeCombat bonusDeCombatTotal() const;

    // L'attaque de base que donne cet équipement : celle de l'arme de la main droite, sinon les mains nues
    // (`mainsNues`). Une valeur dérivée, recalculée à chaque appel, jamais stockée.
    [[nodiscard]] DescriptionDAttaque attaqueDeBase() const;

    // Le type de l'arme de la main droite ; vide sans arme (les mains nues n'ont pas de type).
    [[nodiscard]] std::optional<TypeDArme> typeDArme() const;

    [[nodiscard]] bool operator==(const Equipement&) const = default;

private:
    std::array<std::optional<Objet>, s_nombreEmplacements> m_objets;
};
