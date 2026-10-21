// ./src/combat/EvenementDeCombat.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <variant>
#include "combat/IdSkill.hpp"

// Ce qui s'est passé pendant une image, en données : le noyau (Partie) les produit, l'affichage
// les lit. Une file d'événements, pas de callback : Partie ne connaît pas ceux qui la lisent.

// Le camp de la cible : l'escouade (idCible = place, de 1 à N) ou les monstres (idCible = identifiant).
enum class Camp { Escouade, Monstres };

// Le détail propre à chaque sorte d'événement : le variant dit le type, et chaque sorte ne porte que
// les champs qui ont un sens pour elle.
// `montant` : ce qui a réellement été appliqué (armure comprise, soin plafonné), jamais 0.
// `opportunite` : coup gratuit.
struct Degats {
    int montant;
    bool opportunite;
};
struct Soin {
    int montant;
};
// Un skill de la barre part (fin de son activation), au-dessus de son lanceur ; jamais l'attaque de base.
struct SkillLance {
    IdSkill skill;
};
using DetailEvenement = std::variant<Degats, Soin, SkillLance>;

// `position` : COPIE du centre de la cible à ce moment ; aucune référence vers l'entité, qui peut
// disparaître dans la même image (monstre tué par ce coup).
struct EvenementDeCombat {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    Camp campCible;
    int idCible;
    sf::Vector2f position;
    DetailEvenement detail;
};
