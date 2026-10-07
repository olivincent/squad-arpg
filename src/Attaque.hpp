// ./src/Attaque.hpp
#pragma once
#include "Cercle.hpp"

class Attaque {
public:
    Attaque(float portee, int degats, float tempsRecharge);
    void mettreAJour(float dt);
    bool estAPortee(Cercle origine, Cercle cible) const;
    int frapper(Cercle origine, Cercle cible);

private:
    float m_portee;         // en pixels, écart bord à bord
    int m_degats;
    float m_tempsRecharge;  // en secondes
    float m_recharge;       // en secondes, avant le prochain coup possible
};
