// ./src/EcranMenu.cpp
#include "EcranMenu.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <memory>

#include "Contexte.hpp"
#include "EcranJeu.hpp"
#include "EcranOptions.hpp"
#include "TailleDeVue.hpp"

namespace {

constexpr std::size_t s_boutonCommencer = 0;
constexpr std::size_t s_boutonOptions = 1;
constexpr std::size_t s_boutonQuitter = 2;

}  // namespace

EcranMenu::EcranMenu() : m_boutons({"Commencer partie", "Options", "Quitter"}) {}

Transition EcranMenu::cliquer(sf::Vector2f point, sf::Vector2f centre, const Reglages& reglages) const {
    const std::optional<std::size_t> bouton = m_boutons.boutonSous(point, centre);
    if (bouton == s_boutonCommencer) {
        return Empiler{std::make_unique<EcranJeu>()};
    }
    if (bouton == s_boutonOptions) {
        return Empiler{std::make_unique<EcranOptions>(reglages, resolutionsDisponibles(sf::VideoMode::getDesktopMode().size))};
    }
    if (bouton == s_boutonQuitter) {
        return Quitter{};
    }
    return Aucune{};
}

Transition EcranMenu::traiterEvenement(const sf::Event& evenement, const Contexte& contexte) {
    const auto* data = evenement.getIf<sf::Event::MouseButtonPressed>();
    if (data == nullptr || data->button != sf::Mouse::Button::Left) {
        return Aucune{};
    }
    const sf::View vue = vueDInterface(contexte.fenetre.getSize());
    return cliquer(contexte.fenetre.mapPixelToCoords(data->position, vue), vue.getCenter(), contexte.reglages);
}

void EcranMenu::mettreAJour(float, const Contexte&) {}

void EcranMenu::dessiner(const Contexte& contexte) const {
    sf::RenderWindow& fenetre = contexte.fenetre;
    const sf::View vue = vueDInterface(fenetre.getSize());
    fenetre.setView(vue);
    fenetre.clear();
    const sf::Vector2f curseur = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre), vue);
    m_boutons.dessiner(fenetre, contexte.police, vue.getCenter(), m_boutons.boutonSous(curseur, vue.getCenter()));
}
