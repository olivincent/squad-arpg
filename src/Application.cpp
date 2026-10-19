// ./src/Application.cpp
#include "Application.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>

#include "Cible.hpp"
#include "EcranMenu.hpp"

namespace {

constexpr unsigned s_imagesParSeconde = 60;
constexpr const char* s_fichierReglages = "reglages.ini";

// Pas de temps maximal d'une image, en secondes (trois images à 60 par seconde). Après un gel
// (fenêtre déplacée par sa barre de titre, point d'arrêt, mise en veille), l'horloge rend
// plusieurs secondes d'un coup. Un pas aussi grand ferait traverser les murs (le glissement ne
// teste que la position d'arrivée : à 240 px/s, un personnage saute une case de 40 px dès
// 0,25 s) et ferait sauter les minuteries (activations, recharges, provocations). Borné, le jeu
// ralentit pendant le gel au lieu de sauter. Le seul endroit où dt est borné.
constexpr float s_dtMax = 1.0f / 20.0f;

// Fichier absent ou illisible : réglages par défaut (lireReglages ne lève
// jamais d'exception et corrige les valeurs invalides).
Reglages chargerReglages() {
    std::ifstream fichier(s_fichierReglages);
    if (!fichier) {
        return Reglages{};
    }
    return lireReglages(fichier);
}

// Échec d'écriture : simple message, le jeu continue avec ses réglages en mémoire.
void sauvegarderReglages(const Reglages& reglages) {
    std::ofstream fichier(s_fichierReglages);
    if (!fichier) {
        std::cerr << "Impossible d'écrire " << s_fichierReglages << '\n';
        return;
    }
    ecrireReglages(fichier, reglages);
}

}  // namespace

Application::Application() : m_reglages(chargerReglages()) {
    ouvrirFenetre();
    if (!m_police.openFromFile("assets/fonts/DejaVuSans.ttf")) {
        throw std::runtime_error(
            "Impossible de charger la police assets/fonts/DejaVuSans.ttf "
            "(lancer le jeu depuis le dossier de main.exe)");
    }
    m_pile.push_back(std::make_unique<EcranMenu>());
}

void Application::ouvrirFenetre() {
    if (m_reglages.mode == ModeAffichage::PleinEcran) {
        // Toujours la résolution du bureau ; celle des réglages ne sert qu'en fenêtré.
        m_fenetre.create(sf::VideoMode::getDesktopMode(), "Squad-ARPG", sf::State::Fullscreen);
    } else {
        m_fenetre.create(sf::VideoMode({m_reglages.largeur, m_reglages.hauteur}), "Squad-ARPG",
                         sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed);
    }
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
            [this](const AppliquerReglages& appliquer) {
                m_reglages = appliquer.reglages;
                ouvrirFenetre();
                sauvegarderReglages(m_reglages);
            },
        },
        transition);
}

int Application::executer() {
    sf::Clock horloge;
    while (m_fenetre.isOpen() && !m_pile.empty()) {
        const float dt = std::min(horloge.restart().asSeconds(), s_dtMax);
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
