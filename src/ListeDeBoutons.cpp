// ./src/ListeDeBoutons.cpp
#include "ListeDeBoutons.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <utility>

namespace {

constexpr float s_largeurBouton = 320.0f;
constexpr float s_hauteurBouton = 60.0f;
constexpr float s_espaceEntreBoutons = 20.0f;
constexpr float s_epaisseurContour = 2.0f;
constexpr unsigned int s_tailleLibelle = 28;
const sf::Color s_couleurFond(50, 50, 50);
const sf::Color s_couleurSurvol(95, 95, 95);

}  // namespace

ListeDeBoutons::ListeDeBoutons(std::vector<std::string> libelles) : m_libelles(std::move(libelles)) {}

sf::FloatRect ListeDeBoutons::rectangleDuBouton(std::size_t indice, sf::Vector2f centre) const {
    const float nombre = static_cast<float>(m_libelles.size());
    const float hauteurTotale = nombre * s_hauteurBouton + (nombre - 1.0f) * s_espaceEntreBoutons;
    const float haut = centre.y - hauteurTotale / 2.0f + static_cast<float>(indice) * (s_hauteurBouton + s_espaceEntreBoutons);
    return sf::FloatRect({centre.x - s_largeurBouton / 2.0f, haut}, {s_largeurBouton, s_hauteurBouton});
}

std::optional<std::size_t> ListeDeBoutons::boutonSous(sf::Vector2f point, sf::Vector2f centre) const {
    for (std::size_t i = 0; i < m_libelles.size(); ++i) {
        if (rectangleDuBouton(i, centre).contains(point)) {
            return i;
        }
    }
    return std::nullopt;
}

void ListeDeBoutons::dessiner(sf::RenderTarget& rendu, const sf::Font& police, sf::Vector2f centre,
                              std::optional<std::size_t> survole) const {
    for (std::size_t i = 0; i < m_libelles.size(); ++i) {
        const sf::FloatRect zone = rectangleDuBouton(i, centre);

        sf::RectangleShape fond(zone.size);
        fond.setPosition(zone.position);
        fond.setFillColor(survole == i ? s_couleurSurvol : s_couleurFond);
        fond.setOutlineColor(sf::Color::White);
        fond.setOutlineThickness(s_epaisseurContour);
        rendu.draw(fond);

        sf::Text texte(police, m_libelles[i], s_tailleLibelle);
        const sf::FloatRect limites = texte.getLocalBounds();
        texte.setOrigin(limites.position + limites.size / 2.0f);
        texte.setPosition(zone.position + zone.size / 2.0f);
        rendu.draw(texte);
    }
}
