// ./src/ListeDeBoutons.cpp
#include "ListeDeBoutons.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <utility>

namespace {

constexpr float s_largeurBouton = 520.0f;
constexpr float s_hauteurBouton = 60.0f;
constexpr float s_espaceEntreBoutons = 20.0f;
constexpr float s_epaisseurContour = 2.0f;
constexpr unsigned int s_tailleLibelle = 28;
const sf::Color s_couleurFond(50, 50, 50);
const sf::Color s_couleurSurvol(95, 95, 95);
const sf::Color s_couleurFondInactif(30, 30, 30);
const sf::Color s_couleurInactif(110, 110, 110);

}  // namespace

ListeDeBoutons::ListeDeBoutons(std::vector<std::string> libelles)
    : m_libelles(std::move(libelles)), m_actifs(m_libelles.size(), true) {}

void ListeDeBoutons::definirActif(std::size_t indice, bool actif) {
    m_actifs[indice] = actif;
}

sf::FloatRect ListeDeBoutons::rectangleDuBouton(std::size_t indice, sf::Vector2f centre) const {
    const float nombre = static_cast<float>(m_libelles.size());
    const float hauteurTotale = nombre * s_hauteurBouton + (nombre - 1.0f) * s_espaceEntreBoutons;
    const float haut = centre.y - hauteurTotale / 2.0f + static_cast<float>(indice) * (s_hauteurBouton + s_espaceEntreBoutons);
    return sf::FloatRect({centre.x - s_largeurBouton / 2.0f, haut}, {s_largeurBouton, s_hauteurBouton});
}

std::optional<std::size_t> ListeDeBoutons::boutonSous(sf::Vector2f point, sf::Vector2f centre) const {
    for (std::size_t i = 0; i < m_libelles.size(); ++i) {
        if (m_actifs[i] && rectangleDuBouton(i, centre).contains(point)) {
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
        const bool actif = m_actifs[i];
        fond.setFillColor(!actif ? s_couleurFondInactif : (survole == i ? s_couleurSurvol : s_couleurFond));
        fond.setOutlineColor(actif ? sf::Color::White : s_couleurInactif);
        fond.setOutlineThickness(s_epaisseurContour);
        rendu.draw(fond);

        // Les libellés sont écrits en UTF-8 (accents, « × ») : sf::String les
        // convertirait comme de l'ANSI sans fromUtf8.
        sf::Text texte(police, sf::String::fromUtf8(m_libelles[i].begin(), m_libelles[i].end()), s_tailleLibelle);
        texte.setFillColor(actif ? sf::Color::White : s_couleurInactif);
        const sf::FloatRect limites = texte.getLocalBounds();
        texte.setOrigin(limites.position + limites.size / 2.0f);
        texte.setPosition(zone.position + zone.size / 2.0f);
        rendu.draw(texte);
    }
}
