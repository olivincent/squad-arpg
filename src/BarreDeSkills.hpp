// ./src/BarreDeSkills.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <vector>
#include "BarreDeRaccourcis.hpp"
#include "EtatSkill.hpp"
#include "TypeSkill.hpp"

// Lettre affichée sur l'icône d'un skill.
[[nodiscard]] char lettreDuSkill(TypeSkill type);

// Secondes entières affichées pour un temps de recharge restant (arrondi vers le haut).
[[nodiscard]] int secondesAffichees(float tempsRestant);

// Sommets (pour un TriangleFan) de la zone sombre d'une icône carrée de côté
// `cote` centrée sur `centre` : du centre à l'« aiguille », puis le long des
// bords du carré jusqu'à midi. `fractionSombre` va de 0 (aucun sommet) à 1.
[[nodiscard]] std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre);

// Couleur du contour d'une icône : jaune en visée, blanc pendant l'activation
// du skill, gris sinon. La visée l'emporte.
[[nodiscard]] sf::Color couleurDuContour(bool enVisee, bool enActivation);

// Ce que l'icône d'un emplacement montre (calculé par l'écran de jeu, à chaque image).
struct IconeDeRaccourci {
    std::optional<EtatSkill> etat;  // vide = emplacement vide (cadre sombre, seulement le nom de la touche)
    sf::String touche;              // nom de la touche, affiché dans un coin
    sf::Color couleurDuLanceur;     // bande au bas de l'icône : la couleur du personnage
    bool enVisee = false;           // contour jaune : le skill de cet emplacement est en cours de visée
};

// Dessine les icônes des emplacements de la barre principale, à la place que donne
// `rectangleDeLEmplacement` (le même calcul que le clic), en coordonnées de la vue active : à
// appeler avec la vue de l'interface (`vueDInterface`). Une icône dont le skill est en
// activation a un contour blanc ; en visée, jaune (la visée l'emporte).
void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police,
                           const std::array<IconeDeRaccourci, s_emplacementsPrincipaux>& icones);
