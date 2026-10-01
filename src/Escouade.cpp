// ./src/Escouade.cpp
#include "Escouade.hpp"

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_position(positionDepart),
      m_taille(20.0f, 20.0f),
      m_cible(positionDepart),
      m_vitesse(180.0f)
{
}