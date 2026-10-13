// ./src/BarreDeSkills.hpp
#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <vector>
#include "EtatSkill.hpp"
#include "Skill.hpp"

// Lettre affichée sur l'icône d'un skill.
char lettreDuSkill(TypeSkill type);

// Secondes entières affichées pour un temps de recharge restant (arrondi vers le haut).
int secondesAffichees(float tempsRestant);

// Sommets (pour un TriangleFan) de la zone sombre d'une icône carrée de côté
// `cote` centrée sur `centre` : du centre à l'« aiguille », puis le long des
// bords du carré jusqu'à midi. `fractionSombre` va de 0 (aucun sommet) à 1.
std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre);

// Dessine la barre des 4 skills en bas de la cible, en coordonnées de la
// fenêtre : à appeler avec la vue par défaut. `etats[i]` est la place i + 1 ;
// vide = pas de skill (cadre vide). `placeEnVisee` : place dont le skill est
// en cours de visée (contour jaune), vide sinon.
void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police,
                           const std::array<std::optional<EtatSkill>, 4>& etats,
                           std::optional<std::size_t> placeEnVisee);
