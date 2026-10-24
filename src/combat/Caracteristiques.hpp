// ./src/combat/Caracteristiques.hpp
#pragma once
#include <array>
#include <cstddef>

// Les cinq caractéristiques, communes aux personnages et aux objets (`docs/classes-et-combat.md`,
// section 5). L'ordre de l'enum est l'index dans `Caracteristiques`.
enum class Caracteristique { Puissance, Endurance, Adresse, Savoir, Volonte };

inline constexpr std::size_t s_nombreDeCaracteristiques = 5;

// Toutes les caractéristiques, dans l'ordre de l'enum, pour les parcourir.
inline constexpr std::array<Caracteristique, s_nombreDeCaracteristiques> s_caracteristiques = {
    Caracteristique::Puissance, Caracteristique::Endurance, Caracteristique::Adresse, Caracteristique::Savoir,
    Caracteristique::Volonte};

// Une valeur par caractéristique : la base d'une classe, le bonus d'un objet (peut être négatif), la
// somme des deux. Type valeur : un tableau indexé plutôt que cinq champs nommés, parce que les bonus,
// les coefficients et l'affichage parcourent les cinq de la même façon.
class Caracteristiques {
public:
    // Toutes à 0.
    constexpr Caracteristiques() = default;
    // Dans l'ordre de l'enum : Puissance, Endurance, Adresse, Savoir, Volonté.
    explicit constexpr Caracteristiques(const std::array<int, s_nombreDeCaracteristiques>& valeurs)
        : m_valeurs(valeurs) {}

    // Un bonus d'une seule caractéristique, les autres à 0.
    [[nodiscard]] static constexpr Caracteristiques seule(Caracteristique caracteristique, int valeur) {
        Caracteristiques resultat;
        resultat.m_valeurs[static_cast<std::size_t>(caracteristique)] = valeur;
        return resultat;
    }

    [[nodiscard]] constexpr int valeur(Caracteristique caracteristique) const {
        return m_valeurs[static_cast<std::size_t>(caracteristique)];
    }

    constexpr Caracteristiques& operator+=(const Caracteristiques& autre) {
        for (std::size_t i = 0; i < s_nombreDeCaracteristiques; ++i) {
            m_valeurs[i] += autre.m_valeurs[i];
        }
        return *this;
    }

    [[nodiscard]] friend constexpr Caracteristiques operator+(Caracteristiques a, const Caracteristiques& b) {
        a += b;
        return a;
    }

    [[nodiscard]] constexpr bool operator==(const Caracteristiques&) const = default;

private:
    std::array<int, s_nombreDeCaracteristiques> m_valeurs{};
};
