// ./src/ChampDeVision.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

class TileMap;

// Cases qu'un observateur placé dans la case `origine` voit, jusqu'à `rayon` cases
// (distance entre les centres des cases <= rayon). Shadowcasting symétrique : pour deux
// cases non bloquantes A et B, B est dans le champ de A si et seulement si A est dans
// celui de B. Une case bloquante (mur, porte fermée) qui reçoit la lumière est révélée et
// l'arrête : on voit sa face, jamais ce qui est derrière. Une case hors de la carte est un
// mur, mais n'est jamais renvoyée. L'origine est révélée (si elle est dans la carte). Les
// doublons sont permis : les quadrants partagent leurs axes.
[[nodiscard]] std::vector<sf::Vector2i> champDeVision(const TileMap& carte, sf::Vector2i origine, int rayon);

// La case `cible` serait-elle révélée par `champDeVision` depuis `origine` avec un rayon
// assez grand pour l'atteindre ? Même critère, mais le balayage s'arrête dès que la cible
// est révélée et ne regarde que les quadrants qui la contiennent. Pas de limite de rayon
// (la portée se vérifie ailleurs). `origine == cible` : vrai. Une cible hors de la carte
// n'est jamais révélée.
[[nodiscard]] bool estVisibleDepuis(const TileMap& carte, sf::Vector2i origine, sf::Vector2i cible);

// Le point `a` voit-il le point `b` (pixels du monde) ? La vue se décide en cases : même
// réponse pour tous les points d'une même case, deux points dans la même case se voient
// toujours. Faux si la case de `a` ou celle de `b` est bloquante (mur, porte fermée, hors
// de la carte) : on ne vise pas un point dans un mur et on ne voit pas depuis un mur.
// Sinon `estVisibleDepuis` : symétrique, `ligneDeVue(a, b) == ligneDeVue(b, a)`.
[[nodiscard]] bool ligneDeVue(const TileMap& carte, sf::Vector2f a, sf::Vector2f b);
