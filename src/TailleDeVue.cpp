// ./src/TailleDeVue.cpp
#include "TailleDeVue.hpp"

#include <cassert>

sf::Vector2f tailleDeVue(sf::Vector2u tailleFenetre, float hauteur) {
    assert(tailleFenetre.y > 0);
    const float largeurFenetre = static_cast<float>(tailleFenetre.x);
    const float hauteurFenetre = static_cast<float>(tailleFenetre.y);
    return {hauteur * largeurFenetre / hauteurFenetre, hauteur};
}

sf::View vueDInterface(sf::Vector2u tailleFenetre) {
    const sf::Vector2f taille = tailleDeVue(tailleFenetre, s_hauteurDeVue);
    return sf::View(taille / 2.0f, taille);
}
