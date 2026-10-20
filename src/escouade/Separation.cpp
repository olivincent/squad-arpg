// ./src/escouade/Separation.cpp
#include "escouade/Separation.hpp"
#include <cstddef>

std::vector<sf::Vector2f> pousseesDeSeparation(const std::vector<MembreASeparer>& membres, float ecartMinimal,
                                               float vitesseMax, float dt) {
    std::vector<sf::Vector2f> poussees(membres.size(), sf::Vector2f(0.0f, 0.0f));

    for (std::size_t i = 0; i < membres.size(); ++i) {
        for (std::size_t j = i + 1; j < membres.size(); ++j) {
            const MembreASeparer& a = membres[i];
            const MembreASeparer& b = membres[j];
            if (a.etat == EtatDeSeparation::Absent || b.etat == EtatDeSeparation::Absent) {
                continue;
            }
            const sf::Vector2f ecart = b.centre - a.centre;
            const float distance = ecart.length();
            const float chevauchement = ecartMinimal - distance;
            if (chevauchement <= 0.0f) {
                continue;
            }
            // Centres confondus : repli fixe sur l'axe des x, le plus petit indice vers la gauche.
            const sf::Vector2f direction = distance > 0.0f ? ecart / distance : sf::Vector2f(1.0f, 0.0f);
            const bool aBouge = a.etat == EtatDeSeparation::Libre;
            const bool bBouge = b.etat == EtatDeSeparation::Libre;
            if (!aBouge && !bBouge) {
                continue;
            }
            const float partA = bBouge ? 0.5f : 1.0f;  // si l'autre est immobile, on prend tout
            const float partB = aBouge ? 0.5f : 1.0f;
            if (aBouge) {
                poussees[i] -= direction * (chevauchement * partA);
            }
            if (bBouge) {
                poussees[j] += direction * (chevauchement * partB);
            }
        }
    }

    const float pousseeMax = vitesseMax * dt;
    for (sf::Vector2f& poussee : poussees) {
        const float longueur = poussee.length();
        if (longueur > pousseeMax) {
            poussee = poussee * (pousseeMax / longueur);
        }
    }
    return poussees;
}
