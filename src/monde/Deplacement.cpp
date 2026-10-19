// ./src/monde/Deplacement.cpp
#include "monde/Deplacement.hpp"
#include "monde/TileMap.hpp"
#include <optional>

bool deplacementBloque(Cercle avant, Cercle apres, const std::vector<Cercle>& obstacles) {
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

// Cercle obstacle le plus proche du mobile parmi ceux qui refuseraient cet essai ; vide si aucun.
std::optional<Cercle> obstacleRefusant(Cercle avant, Cercle apres, const std::vector<Cercle>& obstacles) {
    std::optional<Cercle> plusProche;
    float distanceMin = 0.0f;
    for (const Cercle& obstacle : obstacles) {
        if (!deplacementBloque(avant, apres, {obstacle})) {
            continue;
        }
        const float distance = (avant.centre - obstacle.centre).length();
        if (!plusProche.has_value() || distance < distanceMin) {
            plusProche = obstacle;
            distanceMin = distance;
        }
    }
    return plusProche;
}

// Déplacement de même longueur que `deplacement`, le long de la tangente de `obstacle` au point
// où se trouve le mobile (perpendiculaire à la droite des centres). Côté : celui où la projection
// de `deplacement` est positive ; projection nulle (poussée droit contre le cercle) : à gauche
// du déplacement voulu (y vers le bas, gauche = (y, -x)). Vide si les centres sont confondus.
std::optional<sf::Vector2f> deplacementTangent(sf::Vector2f deplacement, Cercle mobile, Cercle obstacle) {
    const sf::Vector2f ecart = mobile.centre - obstacle.centre;
    if (ecart.lengthSquared() == 0.0f) {
        return std::nullopt;
    }
    const sf::Vector2f tangente = sf::Vector2f(-ecart.y, ecart.x).normalized();
    float projection = deplacement.dot(tangente);
    if (projection == 0.0f) {
        projection = sf::Vector2f(deplacement.y, -deplacement.x).dot(tangente);
    }
    return tangente * (projection >= 0.0f ? deplacement.length() : -deplacement.length());
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

    // Ni X ni Y n'ont passé. Contre un mur, il n'y a rien de plus à faire. Contre un cercle,
    // l'axe par axe ne suffit pas : la surface du cercle n'est parallèle à aucun axe, et un
    // mobile calé contre lui (poussé droit dessus, ou presque) ne peut avancer sur aucun des
    // deux sans se rapprocher. On essaie donc un déplacement permis de plus, le long de la
    // tangente du cercle le plus proche parmi ceux qui ont refusé. La règle de blocage ne change pas.
    if (position == rect.position && deplacement != sf::Vector2f(0.0f, 0.0f)) {
        const Cercle avant = cercleDe(position, rect.size);
        const std::optional<Cercle> a = obstacleRefusant(avant, cercleDe(position + sf::Vector2f(deplacement.x, 0.0f), rect.size), obstacles);
        const std::optional<Cercle> b = obstacleRefusant(avant, cercleDe(position + sf::Vector2f(0.0f, deplacement.y), rect.size), obstacles);
        std::optional<Cercle> bloquant = a;
        if (b.has_value() && (!a.has_value() || (avant.centre - b->centre).length() < (avant.centre - a->centre).length())) {
            bloquant = b;
        }
        if (bloquant.has_value()) {
            if (const std::optional<sf::Vector2f> tangent = deplacementTangent(deplacement, avant, *bloquant)) {
                const sf::Vector2f essai = position + *tangent;
                if (!carte.chevaucheUnMur(sf::FloatRect(essai, rect.size)) &&
                    !deplacementBloque(avant, cercleDe(essai, rect.size), obstacles)) {
                    position = essai;
                }
            }
        }
    }

    return position;
}
