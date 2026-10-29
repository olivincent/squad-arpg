// ./src/interface/EntreeDInterface.cpp
#include "interface/EntreeDInterface.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include "interface/Contexte.hpp"
#include "interface/TailleDeVue.hpp"

std::optional<PointDInterface> clicGaucheDInterface(const sf::Event& evenement, const Contexte& contexte) {
    const auto* data = evenement.getIf<sf::Event::MouseButtonPressed>();
    if (data == nullptr || data->button != sf::Mouse::Button::Left) {
        return std::nullopt;
    }
    const sf::View vue = vueDInterface(contexte.fenetre.getSize());
    return PointDInterface{contexte.fenetre.mapPixelToCoords(data->position, vue), vue.getCenter()};
}

std::optional<PointDInterface> relachementGaucheDInterface(const sf::Event& evenement, const Contexte& contexte) {
    const auto* data = evenement.getIf<sf::Event::MouseButtonReleased>();
    if (data == nullptr || data->button != sf::Mouse::Button::Left) {
        return std::nullopt;
    }
    const sf::View vue = vueDInterface(contexte.fenetre.getSize());
    return PointDInterface{contexte.fenetre.mapPixelToCoords(data->position, vue), vue.getCenter()};
}

std::optional<PointDInterface> deplacementDeLaSourisDInterface(const sf::Event& evenement, const Contexte& contexte) {
    const auto* data = evenement.getIf<sf::Event::MouseMoved>();
    if (data == nullptr) {
        return std::nullopt;
    }
    const sf::View vue = vueDInterface(contexte.fenetre.getSize());
    return PointDInterface{contexte.fenetre.mapPixelToCoords(data->position, vue), vue.getCenter()};
}

PointDInterface preparerLInterface(const Contexte& contexte) {
    sf::RenderWindow& fenetre = contexte.fenetre;
    const sf::View vue = vueDInterface(fenetre.getSize());
    fenetre.setView(vue);
    return PointDInterface{fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre), vue), vue.getCenter()};
}
