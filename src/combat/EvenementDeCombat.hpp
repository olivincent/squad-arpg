// ./src/combat/EvenementDeCombat.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <variant>
#include <vector>
#include "combat/IdSkill.hpp"
#include "combat/ResolutionDuCoup.hpp"

// Ce qui s'est passé pendant une image, en données : le noyau (Partie) les produit, l'affichage
// les lit. Une file d'événements, pas de callback : Partie ne connaît pas ceux qui la lisent.

// Le camp de la cible : l'escouade (idCible = place, de 1 à N) ou les monstres (idCible = identifiant).
enum class Camp { Escouade, Monstres };

// Le détail propre à chaque sorte d'événement : le variant dit le type, et chaque sorte ne porte que
// les champs qui ont un sens pour elle.
// `montant` : ce qui a réellement été appliqué (armure comprise, soin plafonné), jamais 0.
// `opportunite` : coup gratuit. `critique` : le coup ou le soin a été un critique (le montant est déjà multiplié).
struct DegatsSubis {
    int montant;
    bool opportunite;
    bool critique;
};
struct Soin {
    int montant;
    bool critique;
};
// Un skill de la barre part (fin de son activation), au-dessus de son lanceur ; jamais l'attaque de base.
struct SkillLance {
    IdSkill skill;
};
// Un monstre vient d'être provoqué (de nouveau, s'il l'était déjà), au-dessus du monstre.
struct Provoque {};
// Un coup a été esquivé, au-dessus de la cible : l'attaque a eu lieu, elle n'a rien fait.
struct Esquive {};
// Un blocage a absorbé `montant` points (jamais 0) du coup. `total` : il a tout absorbé, aucun `DegatsSubis` ne suit ;
// sinon un `DegatsSubis` pour le reste a été signalé juste avant (même cible, même image).
struct Bloque {
    int montant;
    bool total;
};
using DetailEvenement = std::variant<DegatsSubis, Soin, SkillLance, Provoque, Esquive, Bloque>;

// `position` : COPIE du centre de la cible à ce moment ; aucune référence vers l'entité, qui peut
// disparaître dans la même image (monstre tué par ce coup).
struct EvenementDeCombat {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    Camp campCible;
    int idCible;
    sf::Vector2f position;
    DetailEvenement detail;
};

// Ce qu'un coup donne, en détails d'événements, dans l'ordre où ils s'empilent à l'écran (même cible, même
// image : le premier en bas) : un coup esquivé donne `Esquive` seul ; sinon les `DegatsSubis` (rien s'ils sont nuls,
// donc rien après un blocage total ni pour un mort), puis `Bloque` s'il y a eu un blocage, au-dessus des dégâts.
// `opportunite` : coup gratuit. Un résultat « rien » (mort, monstre invulnérable, montant nul) donne une liste vide.
[[nodiscard]] std::vector<DetailEvenement> detailsDuCoup(const ResultatDuCoup& resultat, bool opportunite);
