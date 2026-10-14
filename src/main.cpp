// ./src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>
#include <iostream>
#include <optional>
#include <random>
#include "Aleatoire.hpp"
#include "BarreDeSkills.hpp"
#include "Controleur.hpp"
#include "EcranDeDefaite.hpp"
#include "EtatSkill.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"

namespace {

// Tire une graine et l'affiche dans la console, pour pouvoir rejouer une
// partie à l'identique.
Generateur::result_type tirerGraine(std::random_device& tirage) {
    const Generateur::result_type graine = tirage();
    std::cout << "Graine : " << graine << '\n';
    return graine;
}

}  // namespace

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Squad-ARPG");
    window.setFramerateLimit(60);
    // Une touche maintenue ne produit qu'un seul événement d'appui.
    window.setKeyRepeatEnabled(false);
    // La police doit vivre plus longtemps que tout sf::Text qui l'utilise.
    sf::Font police;
    if (!police.openFromFile("assets/fonts/DejaVuSans.ttf")) {
        std::cerr << "Impossible de charger la police assets/fonts/DejaVuSans.ttf "
                     "(lancer le jeu depuis le dossier de main.exe)\n";
        return 1;
    }
    std::random_device tirageDeGraine;
    Partie partie(creerNiveau(), tirerGraine(tirageDeGraine));
    Controleur controleur;

    sf::Clock horloge;
    sf::View camera = window.getDefaultView();

    while (window.isOpen()) {
        const float dt = horloge.restart().asSeconds();
        // L'image précédente a fini sur la vue de l'interface : les clics
        // doivent être convertis avec la caméra du monde.
        window.setView(camera);
        controleur.debutImage(partie);
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (data->button == sf::Mouse::Button::Left) {
                    controleur.appuiGauche(partie, window.mapPixelToCoords(data->position));
                } else if (data->button == sf::Mouse::Button::Right) {
                    controleur.appuiDroit();
                }
            }
            if (const auto* data = event->getIf<sf::Event::KeyPressed>()) {
                // Scancode : la touche physique « 1 » de la rangée du haut,
                // quelle que soit la disposition du clavier (AZERTY : « & »).
                // Touche N = personnage à la place N de la formation.
                if (data->scancode == sf::Keyboard::Scan::Num1) {
                    controleur.appuiTouche(partie, 1);
                } else if (data->scancode == sf::Keyboard::Scan::Num2) {
                    controleur.appuiTouche(partie, 2);
                } else if (data->scancode == sf::Keyboard::Scan::Num3) {
                    controleur.appuiTouche(partie, 3);
                } else if (data->scancode == sf::Keyboard::Scan::Num4) {
                    controleur.appuiTouche(partie, 4);
                } else if (data->scancode == sf::Keyboard::Scan::Escape) {
                    controleur.appuiEchap();
                } else if (data->scancode == sf::Keyboard::Scan::R && partie.estPerdue()) {
                    // Réaffectation : la Partie garde son adresse, et le
                    // Controleur repart sans visée ni déplacement continu.
                    partie = Partie(creerNiveau(), tirerGraine(tirageDeGraine));
                    controleur = Controleur{};
                }
            }
        }

        const bool boutonTenu = window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        controleur.mettreAJourDeplacement(partie, boutonTenu, window.mapPixelToCoords(sf::Mouse::getPosition(window)));
        partie.mettreAJour(dt);

        camera.setCenter(partie.getEscouade().getCentre());
        window.setView(camera);
        window.clear();
        partie.dessiner(window);
        controleur.dessinerApercu(partie, window, window.mapPixelToCoords(sf::Mouse::getPosition(window)));

        // Interface fixe à l'écran : vue par défaut, dessinée en dernier.
        window.setView(window.getDefaultView());
        std::array<std::optional<EtatSkill>, 4> etatsDesSkills;
        for (std::size_t i = 0; i < etatsDesSkills.size(); ++i) {
            etatsDesSkills[i] = partie.getEscouade().etatDuSkill(i + 1);
        }
        dessinerBarreDeSkills(window, police, etatsDesSkills, controleur.getPlaceEnVisee());
        if (partie.estPerdue()) {
            dessinerEcranDeDefaite(window, police);
        }
        window.display();
    }
    return 0;
}
