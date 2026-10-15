// ./src/Application.cpp
#include "Application.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <optional>
#include <stdexcept>
#include <utility>

#include "Cible.hpp"
#include "EcranMenu.hpp"

namespace {

constexpr unsigned s_imagesParSeconde = 60;

}  // namespace

Application::Application() {
    ouvrirFenetre();
    if (!m_police.openFromFile("assets/fonts/DejaVuSans.ttf")) {
        throw std::runtime_error(
            "Impossible de charger la police assets/fonts/DejaVuSans.ttf "
            "(lancer le jeu depuis le dossier de main.exe)");
    }
    m_pile.push_back(std::make_unique<EcranMenu>());
}

void Application::ouvrirFenetre() {
    m_fenetre.create(sf::VideoMode({m_reglages.largeur, m_reglages.hauteur}), "Squad-ARPG", sf::Style::Titlebar | sf::Style::Close);
    m_fenetre.setFramerateLimit(s_imagesParSeconde);
    // Une touche maintenue ne produit qu'un seul événement d'appui.
    m_fenetre.setKeyRepeatEnabled(false);
}

Contexte Application::contexte() {
    return Contexte{m_fenetre, m_police, m_reglages};
}

void Application::appliquer(Transition transition) {
    std::visit(
        Surcharges{
            [](Aucune) {},
            [this](Empiler& empiler) { m_pile.push_back(std::move(empiler.ecran)); },
            [this](Depiler) { m_pile.pop_back(); },
            [this](Quitter) { m_fenetre.close(); },
        },
        transition);
}

int Application::executer() {
    sf::Clock horloge;
    while (m_fenetre.isOpen() && !m_pile.empty()) {
        const float dt = horloge.restart().asSeconds();
        while (const std::optional evenement = m_fenetre.pollEvent()) {
            if (evenement->is<sf::Event::Closed>()) {
                // La croix quitte, quel que soit l'écran.
                m_fenetre.close();
                break;
            }
            appliquer(m_pile.back()->traiterEvenement(*evenement, contexte()));
            if (!m_fenetre.isOpen() || m_pile.empty()) {
                break;
            }
        }
        if (!m_fenetre.isOpen() || m_pile.empty()) {
            break;
        }
        m_pile.back()->mettreAJour(dt, contexte());
        m_pile.back()->dessiner(contexte());
        m_fenetre.display();
    }
    return 0;
}
