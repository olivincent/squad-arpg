// ./src/Eclaircir.cpp
#include "Eclaircir.hpp"
#include <cstdint>

namespace {
std::uint8_t melangerAuBlanc(std::uint8_t composante, float fraction) {
    const float valeur = static_cast<float>(composante) + (255.0f - static_cast<float>(composante)) * fraction;
    return static_cast<std::uint8_t>(valeur + 0.5f);
}

std::uint8_t melangerAuNoir(std::uint8_t composante, float fraction) {
    const float valeur = static_cast<float>(composante) * (1.0f - fraction);
    return static_cast<std::uint8_t>(valeur + 0.5f);
}
}  // namespace

sf::Color eclaircir(sf::Color couleur, float fraction) {
    return sf::Color(melangerAuBlanc(couleur.r, fraction), melangerAuBlanc(couleur.g, fraction),
                     melangerAuBlanc(couleur.b, fraction), couleur.a);
}

sf::Color assombrir(sf::Color couleur, float fraction) {
    return sf::Color(melangerAuNoir(couleur.r, fraction), melangerAuNoir(couleur.g, fraction),
                     melangerAuNoir(couleur.b, fraction), couleur.a);
}
