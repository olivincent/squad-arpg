// ./src/objets/DeplacementDObjets.hpp
#pragma once
#include <vector>
#include "objets/Equipement.hpp"
#include "objets/Inventaire.hpp"
#include "objets/PositionDObjet.hpp"

// Tous les objets de l'escouade, en valeurs : le sac et l'équipement de chaque place (indice = place - 1). Un
// déplacement se fait sur une copie de cet état, qu'on garde ou qu'on jette : c'est ce qui le rend transactionnel
// (tout se fait ou rien ne bouge) et qui permet de répondre « est-ce permis ? » sans rien changer.
struct EtatDesObjets {
    Inventaire sac;
    std::vector<Equipement> equipements;
};

// Déplace l'objet de `de` vers `vers` sur cet état, ou rend faux sans rien changer. Seule source de la règle des
// déplacements : l'écran (surbrillance des destinations) et l'action y passent par `deplacementPossible` et
// `Escouade::deplacerObjet`, donc ne peuvent pas diverger. La règle :
// - la règle de base des positions (`deplacementPermis` : source occupée, catégorie compatible avec l'emplacement
//   d'arrivée, et de départ si c'est un échange) ;
// - les deux mains : poser une arme à deux mains alors que la main gauche est occupée range l'objet de la main
//   gauche dans la première case libre du sac ; poser un bouclier alors que la main droite tient une arme à deux mains
//   range l'arme. Pas de case libre : le déplacement est refusé. Les deux extrémités d'un échange entre deux
//   personnages sont traitées pareil.
// `assert` : places valides, cases du sac valides.
[[nodiscard]] bool deplacerLesObjets(EtatDesObjets& etat, const PositionDObjet& de, const PositionDObjet& vers);

// Le déplacement serait-il permis ? Le même calcul que `deplacerLesObjets`, sur une copie.
[[nodiscard]] bool deplacementPossible(const EtatDesObjets& etat, const PositionDObjet& de, const PositionDObjet& vers);
