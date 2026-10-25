// ./src/interface/AffichageObjetsAuSol.hpp
#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <vector>
#include "jeu/ObjetAuSol.hpp"

// Les objets posés au sol, à l'écran. À appeler avec la vue du monde.

// Un petit losange par objet, dans la couleur de sa rareté (`couleurDeLaRarete`, comme l'inventaire), cerné de noir. À
// dessiner après la carte et avant les monstres et l'escouade ; le voile du brouillard, dessiné après, cache ceux d'une case
// inconnue et assombrit ceux d'une case explorée, comme le terrain.
void dessinerObjetsAuSol(sf::RenderTarget& rendu, const std::vector<ObjetAuSol>& objets);

// Le nom composé de cet objet (« Keen Bow of the Fox », `nomDeLObjet`) au-dessus de son losange, dans la couleur de sa
// rareté, avec un contour noir. L'infobulle complète reste dans l'inventaire.
void dessinerNomDObjetAuSol(sf::RenderTarget& rendu, const sf::Font& police, const ObjetAuSol& objet);
