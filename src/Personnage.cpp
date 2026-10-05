// ./src/Personnage.cpp
#include "Personnage.hpp"

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur)
    : m_centre(centre),
      m_taille(20.0f, 20.0f),
      m_couleur(couleur)
{
}
