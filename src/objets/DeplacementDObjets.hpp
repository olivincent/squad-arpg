// ./src/objets/DeplacementDObjets.hpp
#pragma once
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Equipement.hpp"
#include "objets/Inventaire.hpp"
#include "objets/PositionDObjet.hpp"

// Tous les objets de l'escouade, en valeurs : le sac et l'équipement de chaque place (indice = place - 1). Un
// déplacement se fait sur une copie de cet état, qu'on garde ou qu'on jette : c'est ce qui le rend transactionnel
// (tout se fait ou rien ne bouge) et qui permet de répondre « est-ce permis ? » sans rien changer.
struct EtatDesObjets {
    Inventaire sac;
    std::vector<Equipement> equipements;
    std::vector<Caracteristiques> bases;  // les caractéristiques de base de chaque place (celles de la classe), pour les seuils
};

// Déplace l'objet de `de` vers `vers` sur cet état, ou rend faux sans rien changer. Seule source de la règle des
// déplacements : l'écran (surbrillance des destinations) et l'action y passent par `deplacementPossible` et
// `Escouade::deplacerObjet`, donc ne peuvent pas diverger. La règle :
// - la règle de base des positions (`deplacementPermis` : source occupée, catégorie compatible avec l'emplacement
//   d'arrivée, et de départ si c'est un échange) ;
// - les deux mains : poser une arme à deux mains alors que la main gauche est occupée range l'objet de la main
//   gauche dans la première case libre du sac ; poser un bouclier alors que la main droite tient une arme à deux mains
//   range l'arme. Pas de case libre : le déplacement est refusé. Les deux extrémités d'un échange entre deux
//   personnages sont traitées pareil ;
// - les seuils (étape 92) : un objet posé sur un personnage exige que celui-ci atteigne ses seuils avec ses
//   caractéristiques effectives **sans compter l'objet posé** et en tenant compte de ce que le déplacement retire
//   (l'objet échangé, ce qui est rangé dans le sac). Vers le sac, aucun seuil.
// `assert` : places valides, cases du sac valides.
[[nodiscard]] bool deplacerLesObjets(EtatDesObjets& etat, const PositionDObjet& de, const PositionDObjet& vers);

// Les seuils que le personnage qui recevrait l'objet de `de` à la position `vers` n'atteindrait pas (étape 92.3), avec
// les mêmes caractéristiques que `deplacerLesObjets` (sans compter l'objet posé, après ce que le déplacement retire).
// Vide vers une case du sac, et quand le déplacement est refusé pour une autre raison (catégorie, sac plein). Sert à
// montrer en rouge ce qui manque pendant un glissement.
[[nodiscard]] std::vector<SeuilDeCaracteristique> seuilsManques(const EtatDesObjets& etat, const PositionDObjet& de,
                                                                const PositionDObjet& vers);

// Le déplacement serait-il permis ? Le même calcul que `deplacerLesObjets`, sur une copie.
[[nodiscard]] bool deplacementPossible(const EtatDesObjets& etat, const PositionDObjet& de, const PositionDObjet& vers);
