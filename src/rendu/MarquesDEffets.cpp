// ./src/rendu/MarquesDEffets.cpp
#include "rendu/MarquesDEffets.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <array>
#include <cassert>
#include <string>

namespace {
constexpr float s_ecartSousLeCarre = 3.0f;  // entre le bas du carré de l'entité et la rangée de marques
constexpr int s_colonnesDUnChiffre = 3;
constexpr int s_lignesDUnChiffre = 5;
constexpr float s_ecartEntreChiffres = 1.0f;  // entre deux chiffres d'un même nombre de piles

// Les cinq lignes de chaque chiffre, un bit par colonne (le bit de poids fort est la colonne de gauche).
constexpr std::array<std::array<unsigned int, s_lignesDUnChiffre>, 10> s_chiffres = {{
    {0b111, 0b101, 0b101, 0b101, 0b111},  // 0
    {0b010, 0b110, 0b010, 0b010, 0b111},  // 1
    {0b111, 0b001, 0b111, 0b100, 0b111},  // 2
    {0b111, 0b001, 0b111, 0b001, 0b111},  // 3
    {0b101, 0b101, 0b111, 0b001, 0b001},  // 4
    {0b111, 0b100, 0b111, 0b001, 0b111},  // 5
    {0b111, 0b100, 0b111, 0b101, 0b111},  // 6
    {0b111, 0b001, 0b001, 0b010, 0b010},  // 7
    {0b111, 0b101, 0b111, 0b101, 0b111},  // 8
    {0b111, 0b101, 0b111, 0b001, 0b111},  // 9
}};

const sf::Color s_couleurDesBonus(60, 200, 60);
const sf::Color s_couleurDesMalus(220, 50, 50);
const sf::Color s_couleurDesControles(240, 215, 40);  // jaune : ni le vert d'un bonus ni le rouge d'un malus

void dessinerUnRectangle(sf::RenderTarget& rendu, sf::Vector2f position, float cote, sf::Color couleur) {
    sf::RectangleShape forme({cote, cote});
    forme.setPosition(position);
    forme.setFillColor(couleur);
    rendu.draw(forme);
}

// Le nombre de piles en chiffres à partir de `debut` ; rend l'abscisse qui suit le dernier chiffre.
float dessinerLeNombre(sf::RenderTarget& rendu, sf::Vector2f debut, int nombre, sf::Color couleur) {
    float x = debut.x;
    for (const char caractere : std::to_string(nombre)) {
        for (const sf::Vector2i point : pointsDuChiffre(caractere - '0')) {
            dessinerUnRectangle(rendu, {x + static_cast<float>(point.x) * s_cotePointDeChiffre, debut.y + static_cast<float>(point.y) * s_cotePointDeChiffre},
                                s_cotePointDeChiffre, couleur);
        }
        x += static_cast<float>(s_colonnesDUnChiffre) * s_cotePointDeChiffre + s_ecartEntreChiffres;
    }
    return x - s_ecartEntreChiffres;
}

// Une rangée de marques, une par élément de `piles`, à partir de `debut` ; rend l'abscisse qui suit la dernière.
float dessinerUneRangee(sf::RenderTarget& rendu, sf::Vector2f debut, const std::vector<int>& piles, sf::Color couleur) {
    float x = debut.x;
    for (const int pile : piles) {
        dessinerUnRectangle(rendu, {x, debut.y}, s_coteDUneMarque, couleur);
        x += s_coteDUneMarque + s_ecartEntreMarques;
        if (pile > 1) {
            x = dessinerLeNombre(rendu, {x, debut.y}, pile, couleur) + s_ecartEntreMarques;
        }
    }
    return x;
}
}  // namespace

sf::Vector2f positionMarquesDEffets(sf::Vector2f centre, sf::Vector2f taille) {
    return {centre.x - taille.x / 2.0f, centre.y + taille.y / 2.0f + s_ecartSousLeCarre};
}

std::vector<sf::Vector2i> pointsDuChiffre(int chiffre) {
    assert(chiffre >= 0 && chiffre <= 9);
    std::vector<sf::Vector2i> points;
    for (int ligne = 0; ligne < s_lignesDUnChiffre; ++ligne) {
        for (int colonne = 0; colonne < s_colonnesDUnChiffre; ++colonne) {
            const unsigned int bit = 1u << static_cast<unsigned int>(s_colonnesDUnChiffre - 1 - colonne);
            if ((s_chiffres[static_cast<std::size_t>(chiffre)][static_cast<std::size_t>(ligne)] & bit) != 0u) {
                points.push_back({colonne, ligne});
            }
        }
    }
    return points;
}

void dessinerMarquesDEffets(sf::RenderTarget& rendu, sf::Vector2f position, const std::vector<int>& pilesDesBonus,
                            const std::vector<int>& pilesDesMalus, int controles) {
    const float suiteDesBonus = dessinerUneRangee(rendu, position, pilesDesBonus, s_couleurDesBonus);
    const float suiteDesMalus = dessinerUneRangee(rendu, {suiteDesBonus, position.y}, pilesDesMalus, s_couleurDesMalus);
    dessinerUneRangee(rendu, {suiteDesMalus, position.y}, std::vector<int>(static_cast<std::size_t>(controles), 1), s_couleurDesControles);
}
