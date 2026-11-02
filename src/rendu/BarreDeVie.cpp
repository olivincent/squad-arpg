// ./src/rendu/BarreDeVie.cpp
#include "rendu/BarreDeVie.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>

namespace {
constexpr float s_ecartBarreDeVie = 6.0f;  // entre le haut du carré et le bas de la barre
const sf::Color s_couleurDeLAegis(200, 205, 215);  // argenté, comme le texte « Absorbed » de l'Aegis
const sf::Color s_couleurDuWard(80, 140, 255);     // bleu, comme celui du Ward
}  // namespace

sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille) {
    return {centre.x - taille.x / 2.0f, centre.y - taille.y / 2.0f - s_ecartBarreDeVie - s_hauteurBarreDeVie};
}

void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio) {
    sf::RectangleShape fond({largeur, s_hauteurBarreDeVie});
    fond.setPosition(position);
    fond.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fond);

    sf::RectangleShape vie({largeur * ratio, s_hauteurBarreDeVie});
    vie.setPosition(position);
    vie.setFillColor(sf::Color::Green);
    rendu.draw(vie);
}

std::pair<SegmentDeBarre, SegmentDeBarre> segmentsDAbsorption(float ratioDeVie, float ratioAegis, float ratioWard) {
    const float aegis = std::clamp(ratioAegis, 0.0f, 1.0f);
    const float ward = std::clamp(ratioWard, 0.0f, 1.0f - aegis);
    const float debut = std::min(std::clamp(ratioDeVie, 0.0f, 1.0f), 1.0f - aegis - ward);
    return {{debut, debut + aegis}, {debut + aegis, debut + aegis + ward}};
}

void dessinerAbsorption(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratioDeVie, float ratioAegis, float ratioWard) {
    const auto [aegis, ward] = segmentsDAbsorption(ratioDeVie, ratioAegis, ratioWard);
    const auto dessiner = [&](SegmentDeBarre segment, sf::Color couleur) {
        if (segment.fin <= segment.debut) {
            return;
        }
        sf::RectangleShape forme({largeur * (segment.fin - segment.debut), s_hauteurBarreDeVie});
        forme.setPosition({position.x + largeur * segment.debut, position.y});
        forme.setFillColor(couleur);
        rendu.draw(forme);
    };
    dessiner(aegis, s_couleurDeLAegis);
    dessiner(ward, s_couleurDuWard);
}
