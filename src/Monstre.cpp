// ./src/Monstre.cpp
#include "Monstre.hpp"

Monstre::Monstre(sf::Vector2f centre)
    : m_centre(centre),
      m_taille(30.0f, 30.0f),
      m_pointsDeVie(100)
{
}
