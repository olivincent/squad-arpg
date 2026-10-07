// ./src/Attaque.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

class Attaque {
public:
    Attaque(float portee, int degats, float tempsRecharge);
    void mettreAJour(float dt);
    bool estAPortee(sf::Vector2f origine, sf::Vector2f cible) const;
    int frapper(sf::Vector2f origine, sf::Vector2f cible);

private:
    float m_portee;         // en pixels
    int m_degats;
    float m_tempsRecharge;  // en secondes
    float m_recharge;       // en secondes, avant le prochain coup possible
};
