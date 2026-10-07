// ./src/Deplacement.cpp
#include "Deplacement.hpp"
#include "TileMap.hpp"

bool deplacementBloque(const Cercle& avant, const Cercle& apres, const std::vector<Cercle>& obstacles) {
    for (const Cercle& obstacle : obstacles) {
        const float distanceApres = (apres.centre - obstacle.centre).length();
        const float distanceAvant = (avant.centre - obstacle.centre).length();

        if (distanceApres < apres.rayon + obstacle.rayon && distanceApres < distanceAvant) {
            return true;
        }
    }
    return false;
}

namespace {

// Cercle de collision d'un rectangle : centré dessus, de rayon la demi-largeur
// (cohérent avec getCercle() de Personnage et de Monstre, qui sont des carrés).
Cercle cercleDe(sf::Vector2f position, sf::Vector2f taille) {
    return Cercle{position + taille / 2.0f, taille.x / 2.0f};
}

}

sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement,
                                    const std::vector<Cercle>& obstacles) {
    sf::Vector2f position = rect.position;

    const sf::Vector2f essaiX = position + sf::Vector2f(deplacement.x, 0.0f);
    if (!carte.chevaucheUnMur(sf::FloatRect(essaiX, rect.size)) &&
        !deplacementBloque(cercleDe(position, rect.size), cercleDe(essaiX, rect.size), obstacles)) {
        position = essaiX;
    }

    const sf::Vector2f essaiY = position + sf::Vector2f(0.0f, deplacement.y);
    if (!carte.chevaucheUnMur(sf::FloatRect(essaiY, rect.size)) &&
        !deplacementBloque(cercleDe(position, rect.size), cercleDe(essaiY, rect.size), obstacles)) {
        position = essaiY;
    }

    return position;
}
