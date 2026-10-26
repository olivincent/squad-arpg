// ./src/jeu/ObjetAuSol.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include "objets/Objet.hpp"

// Un objet posé au sol, dans le monde. Possédé par `ObjetsAuSol` (donc par la `Partie`), qui lui donne un identifiant jamais réutilisé : tout ce
// qui désigne un objet au sol (survol, clic, ramassage) passe par lui, jamais par un indice du conteneur, qui change
// quand on retire un élément.
struct ObjetAuSol {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int id;
    Objet objet;
    sf::Vector2f position;  // le centre de sa case, en pixels du monde

    bool operator==(const ObjetAuSol&) const = default;
};

// Le rayon (en pixels du monde) de la zone où le curseur désigne un objet au sol : un peu plus que son dessin (un losange de
// 8 px de rayon, `interface/AffichageObjetsAuSol`), pour ne pas exiger la précision au pixel.
inline constexpr float s_rayonDeSurvolDObjet = 14.0f;

// La distance (centre à centre, en pixels du monde) de l'ancre de l'escouade à un objet visé en deçà de laquelle il est ramassé
// (« à tester »).
inline constexpr float s_distanceDeRamassage = 30.0f;
