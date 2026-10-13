// ./src/BarreDeSkills.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "Skill.hpp"

// Lettre affichée sur l'icône d'un skill.
char lettreDuSkill(TypeSkill type);

// Secondes entières affichées pour un temps de recharge restant (arrondi vers le haut).
int secondesAffichees(float tempsRestant);

// Sommets (pour un TriangleFan) de la zone sombre d'une icône carrée de côté
// `cote` centrée sur `centre` : du centre à l'« aiguille », puis le long des
// bords du carré jusqu'à midi. `fractionSombre` va de 0 (aucun sommet) à 1.
std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre);
