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

    // La somme des cinq valeurs.
    [[nodiscard]] constexpr int somme() const {
        int total = 0;
        for (const int valeur : m_valeurs) {
            total += valeur;
        }
        return total;
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

// Combien un skill profite de chaque caractéristique pour son montant (dégâts ou soin) : 0 = pas du tout,
// 1 = à plein. Même forme que `Caracteristiques`, en flottants. Tout à 0 par défaut.
class Coefficients {
public:
    constexpr Coefficients() = default;
    // Dans l'ordre de l'enum : Puissance, Endurance, Adresse, Savoir, Volonté.
    explicit constexpr Coefficients(const std::array<float, s_nombreDeCaracteristiques>& valeurs)
        : m_valeurs(valeurs) {}

    // Un seul coefficient, les autres à 0.
    [[nodiscard]] static constexpr Coefficients seul(Caracteristique caracteristique, float valeur) {
        Coefficients resultat;
        resultat.m_valeurs[static_cast<std::size_t>(caracteristique)] = valeur;
        return resultat;
    }

    [[nodiscard]] constexpr float valeur(Caracteristique caracteristique) const {
        return m_valeurs[static_cast<std::size_t>(caracteristique)];
    }

    [[nodiscard]] constexpr bool operator==(const Coefficients&) const = default;

private:
    std::array<float, s_nombreDeCaracteristiques> m_valeurs{};
};

// Valeur de référence (à la *Pillars of Eternity*) : un effet de caractéristique se calcule par rapport à
// 10, qui ne donne ni bonus ni malus. Provisoire, à tester.
inline constexpr int s_valeurDeReference = 10;

// Part du montant que gagne (ou perd, sous la référence) chaque point de caractéristique, multipliée par
// le coefficient du skill. Provisoire, à tester.
inline constexpr float s_bonusParPoint = 0.03f;

// Points de vie de base d'un personnage, les mêmes pour toutes les classes : l'Endurance fait seule la différence.
// Provisoire, à tester.
inline constexpr int s_pointsDeVieDeBase = 240;

// Points de vie maximum que donne chaque point d'Endurance. Provisoire, à tester.
inline constexpr int s_pvParPointDEndurance = 6;

// Points de vie maximum que donne chaque niveau au-delà du premier. Provisoire, à tester.
inline constexpr int s_pvParNiveau = 8;

// Points de vie maximum d'un personnage : `s_pointsDeVieDeBase` plus `s_pvParPointDEndurance` par point
// d'Endurance (effective : base de classe plus croissance plus équipement) plus `s_pvParNiveau` par niveau au-delà du premier.
// Préconditions : niveau >= 1, le résultat est > 0.
[[nodiscard]] int pointsDeVieMax(const Caracteristiques& caracteristiques, int niveau);

// Le montant (dégâts ou soin) d'un skill de personnage, une fois ses caractéristiques prises en compte :
//   montant × (1 + Σ coefficient × `s_bonusParPoint` × (caractéristique − `s_valeurDeReference`)),
// arrondi au plus proche, jamais négatif. Seul endroit qui calcule ce montant : tout ce qui inflige un
// dégât ou un soin de personnage le lit. Précondition : montant >= 0.
[[nodiscard]] int montantEffectif(int montant, const Coefficients& coefficients, const Caracteristiques& caracteristiques);
