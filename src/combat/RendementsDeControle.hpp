// ./src/combat/RendementsDeControle.hpp
#pragma once
#include <array>
#include <cstddef>
#include "combat/EffetActif.hpp"

// Le rendement décroissant du contrôle (étape 113), compté par état et par entité subie, dans les deux camps. Les applications
// successives d'un même état comptent : la première vaut 100 % de sa durée, la deuxième 50 %, la troisième 25 %, puis l'entité est
// IMMUNE à cet état. Le compteur d'un état revient à zéro `s_remiseAZeroDuRendement` secondes après la dernière application de cet
// état qui a joué ; une application bloquée par l'immunité ne pose rien et ne repousse pas cette échéance (sinon un joueur qui insiste
// resterait bloqué pour toujours).
//
// Le compteur vit avec les effets actifs de l'entité (`EffetsActifs` le possède) : il s'écoule avec eux, et disparaît avec eux (mort,
// descente d'un étage, monstre qui rentre à son poste).

// Le temps sans application après lequel le compteur d'un état revient à zéro, en secondes. Provisoire, à tester.
inline constexpr float s_remiseAZeroDuRendement = 15.0f;

// Le facteur de la première, de la deuxième et de la troisième application ; la suivante vaut 0 (l'immunité). Provisoire, à tester.
inline constexpr std::array<float, 3> s_rendementsDesApplications = {1.0f, 0.5f, 0.25f};

class RendementsDeControle {
public:
    // Compte une application de cet état et rend le facteur qui s'applique à sa durée : 1, 0,5, 0,25, puis 0. Un facteur de 0 est
    // l'immunité : l'application est bloquée, elle ne compte pas et ne repousse pas la remise à zéro.
    [[nodiscard]] float appliquer(EtatInflige etat);
    // Le facteur que rendrait la prochaine application, sans la compter.
    [[nodiscard]] float facteurSuivant(EtatInflige etat) const;
    // Combien d'applications qui ont joué sont comptées pour cet état.
    [[nodiscard]] int applications(EtatInflige etat) const;
    // Fait couler le temps : le compteur d'un état revient à zéro `s_remiseAZeroDuRendement` secondes après sa dernière application.
    void mettreAJour(float dt);
    // Remet tous les compteurs à zéro (la mort de l'entité).
    void vider();

private:
    struct Compteur {
        int applications = 0;
        float depuisLaDerniere = 0.0f;  // en secondes, depuis la dernière application qui a joué
    };
    std::array<Compteur, static_cast<std::size_t>(EtatInflige::Nombre)> m_compteurs = {};
};
