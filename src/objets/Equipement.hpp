// ./src/objets/Equipement.hpp
#pragma once
#include <array>
#include <optional>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Un objet que porte un personnage dès sa création.
struct ObjetDeDepart {
    Emplacement emplacement;
    IdObjet objet;

    bool operator==(const ObjetDeDepart&) const = default;
};

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

    // Somme des bonus de statistiques de combat des objets portés (esquive, capacités…), recalculée à chaque appel.
    [[nodiscard]] StatistiquesDeCombat bonusDeCombatTotal() const;

private:
    std::array<std::optional<Objet>, s_nombreEmplacements> m_objets;
};
