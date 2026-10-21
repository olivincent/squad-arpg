// ./src/interface/EcranInventaire.cpp
#include "interface/EcranInventaire.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>

#include "interface/Contexte.hpp"
#include "interface/EntreeDInterface.hpp"

namespace {

constexpr unsigned char s_opaciteVoile = 170;
constexpr float s_margePanneau = 40.0f;  // entre le panneau et le bord de la vue
constexpr float s_epaisseurContour = 2.0f;
constexpr unsigned int s_tailleTitre = 36;
constexpr float s_hauteurTitre = 20.0f;  // du haut du panneau
const sf::Color s_couleurPanneau(40, 40, 40);

}  // namespace

Transition EcranInventaire::appuyer(sf::Keyboard::Scancode touche) const {
    if (touche == sf::Keyboard::Scan::I || touche == sf::Keyboard::Scan::Escape) {
        return Depiler{};
    }
    return Aucune{};
}

Transition EcranInventaire::traiterEvenement(const sf::Event& evenement, const Contexte&) {
    if (const auto* data = evenement.getIf<sf::Event::KeyPressed>()) {
        return appuyer(data->scancode);
    }
    return Aucune{};
}

void EcranInventaire::mettreAJour(float, const Contexte&) {}

bool EcranInventaire::laisseVoirDessous() const {
    return true;
}

void EcranInventaire::dessiner(const Contexte& contexte) const {
    const PointDInterface vue = preparerLInterface(contexte);
    sf::RenderWindow& fenetre = contexte.fenetre;
    const sf::Vector2f tailleVue = vue.centre * 2.0f;

    sf::RectangleShape voile(tailleVue);
    voile.setFillColor(sf::Color(0, 0, 0, s_opaciteVoile));
    fenetre.draw(voile);

    sf::RectangleShape panneau(tailleVue - sf::Vector2f{2.0f * s_margePanneau, 2.0f * s_margePanneau});
    panneau.setPosition({s_margePanneau, s_margePanneau});
    panneau.setFillColor(s_couleurPanneau);
    panneau.setOutlineColor(sf::Color::White);
    panneau.setOutlineThickness(s_epaisseurContour);
    fenetre.draw(panneau);

    const std::string titre = "Inventaire";
    sf::Text texte(contexte.police, sf::String::fromUtf8(titre.begin(), titre.end()), s_tailleTitre);
    const sf::FloatRect limites = texte.getLocalBounds();
    texte.setOrigin({limites.position.x + limites.size.x / 2.0f, limites.position.y});
    texte.setPosition({vue.centre.x, s_margePanneau + s_hauteurTitre});
    fenetre.draw(texte);
}
