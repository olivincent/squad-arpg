// ./src/escouade/Intention.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <variant>
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"

// Ce qu'un personnage veut faire à cette image, décidé sans carte ni place d'attaque. Escouade le
// traduit ensuite en un point à atteindre (la résolution).
// Il reste où il est ; sa place d'attaque est gardée si elle est autour de ce monstre (vide : oubliée).
struct ResterSurPlace {
    std::optional<int> placeAutourDe;
    bool operator==(const ResterSurPlace&) const = default;
};
// Il marche vers ce point ; sa place d'attaque est oubliée.
struct AllerAuPoint {
    sf::Vector2f point;
    bool operator==(const AllerAuPoint&) const = default;
};
// Il approche ce monstre jusqu'à une place d'attaque autour de lui, avec la portée (bord à bord) de
// l'action avec laquelle il l'approche. Le cercle est celui que la situation a reçu.
struct ApprocherMonstre {
    int idMonstre;
    Cercle cercle;
    float portee;
    bool operator==(const ApprocherMonstre&) const = default;
};
using Intention = std::variant<ResterSurPlace, AllerAuPoint, ApprocherMonstre>;

// Tout ce que choisirIntention sait d'un personnage, calculé avant par Escouade. Un monstre nommé
// porte son cercle, pris à sa source (voir Escouade::situationDe).
struct SituationDuPersonnage {
    bool vivant;
    bool enExecution;                         // règle 0 : un coup ou un skill en exécution
    std::optional<int> monstreDeLaPlace;      // règle 0 : cible de sa place d'attaque retenue
    std::optional<Intention> skillEnAttente;  // règle 1 : il lance le skill en attente, qui le fait marcher
    std::optional<EntiteVue> degagement;      // règle 2 : monstre dont il se dégage (une place libre l'attend)
    std::optional<EntiteVue> verrou;          // règle 3 : monstre verrouillé, s'il est vu
    bool verrouAtteint;                       // règle 3 : à portée de son attaque de base et en vue
    bool ancreArretee;                        // règle 4
    std::optional<EntiteVue> agresseur;       // règle 4 : agresseur retenu dans sa laisse
    bool monstreAPortee;                      // règle 4 : un monstre ciblable à portée de son attaque de base et en vue
    sf::Vector2f placeDeFormation;            // règle 5
    float porteeAttaqueDeBase;                // portée des approches des règles 2 à 4
};

// Les règles par priorité, la première qui s'applique gagne :
// 0. une action en exécution : il reste sur place (sa place d'attaque est gardée, quelle qu'elle soit) ;
// 1. le lanceur d'un skill en attente suit l'intention de ce skill ;
// 2. il se dégage : il approche le monstre, sans s'arrêter à portée ;
// 3. vivant, avec un verrou : il reste sur place s'il l'atteint, sinon il l'approche ;
// 4. vivant, ancre à l'arrêt, avec un agresseur dans sa laisse : il reste sur place s'il a déjà un
//    monstre à portée, sinon il approche l'agresseur (l'ordre du joueur gagne : ancre en marche, rien) ;
// 5. sinon, sa place de formation.
// Pure : aucun calcul de géométrie, les cercles sont transmis tels quels. Le point de branchement des
// futurs gambits.
[[nodiscard]] Intention choisirIntention(const SituationDuPersonnage& situation);

// Le monstre de l'intention : celui qu'il approche, ou celui autour duquel il garde sa place ; vide pour
// un point. Il n'est pas à contourner, et c'est autour de lui que la place d'attaque est gardée.
[[nodiscard]] std::optional<int> monstreDeLIntention(const Intention& intention);
