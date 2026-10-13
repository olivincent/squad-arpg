// ./src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>
#include <iostream>
#include <optional>
#include "BarreDeSkills.hpp"
#include "Cercle.hpp"
#include "EtatSkill.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"

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
    Partie partie(creerNiveau());
    bool deplacementContinu = false;
    // Mode visée : état d'interface. Vide = mode normal, sinon la place du
    // lanceur du skill de zone en cours de visée.
    std::optional<std::size_t> placeEnVisee;

    sf::Clock horloge;
    sf::View camera = window.getDefaultView();

    while (window.isOpen()) {
        const float dt = horloge.restart().asSeconds();
        // L'image précédente a fini sur la vue de l'interface : les clics
        // doivent être convertis avec la caméra du monde.
        window.setView(camera);
        std::optional<std::size_t> placeSkillDemandee;
        std::optional<sf::Vector2f> pointDeLancer;
        // Lanceur mort ou escouade vaincue : on sort de la visée.
        if (placeEnVisee.has_value() && !partie.getEscouade().skillEstPret(*placeEnVisee)) {
            placeEnVisee.reset();
        }
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (placeEnVisee.has_value()) {
                    // En visée, le clic gauche lance le sort (même sur un
                    // monstre) et le clic droit annule ; rien d'autre.
                    if (data->button == sf::Mouse::Button::Left) {
                        pointDeLancer = window.mapPixelToCoords(data->position);
                    } else if (data->button == sf::Mouse::Button::Right) {
                        placeEnVisee.reset();
                    }
                } else if (data->button == sf::Mouse::Button::Left) {
                    const ResultatClic resultat = partie.cliquer(window.mapPixelToCoords(data->position));
                    if (resultat == ResultatClic::Monstre) {
                        deplacementContinu = false;
                    } else if (resultat == ResultatClic::Sol) {
                        deplacementContinu = true;
                    }
                }
            }
            if (const auto* data = event->getIf<sf::Event::KeyPressed>()) {
                // Scancode : la touche physique « 1 » de la rangée du haut,
                // quelle que soit la disposition du clavier (AZERTY : « & »).
                // Touche N = personnage à la place N de la formation.
                if (data->scancode == sf::Keyboard::Scan::Num1) {
                    placeSkillDemandee = 1;
                } else if (data->scancode == sf::Keyboard::Scan::Num2) {
                    placeSkillDemandee = 2;
                } else if (data->scancode == sf::Keyboard::Scan::Num3) {
                    placeSkillDemandee = 3;
                } else if (data->scancode == sf::Keyboard::Scan::Escape) {
                    placeEnVisee.reset();
                } else if (data->scancode == sf::Keyboard::Scan::Num4) {
                    if (placeEnVisee.has_value()) {
                        placeEnVisee.reset();
                    } else if (partie.getEscouade().estVivante() && partie.getEscouade().skillEstPret(4)) {
                        placeEnVisee = 4;
                        // Le déplacement continu est suspendu : il ne
                        // reprendra qu'à un nouvel appui sur le sol.
                        deplacementContinu = false;
                    }
                }
            }
        }

        // Lancer du skill de zone : sur l'événement d'appui, jamais sur
        // l'état du bouton. Réussi ou refusé, on sort de la visée.
        if (placeEnVisee.has_value() && pointDeLancer.has_value()) {
            partie.lancerSkillDeZone(*placeEnVisee, *pointDeLancer);
            placeEnVisee.reset();
        }
        if (placeSkillDemandee.has_value()) {
            partie.demanderSkill(*placeSkillDemandee);
        }
        if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && deplacementContinu) {
            partie.ordonnerDeplacement(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
        }
        partie.mettreAJour(dt);

        camera.setCenter(partie.getEscouade().getCentre());
        window.setView(camera);
        window.clear();
        partie.dessiner(window);
        if (placeEnVisee.has_value()) {
            // Cercle de prévisualisation au rayon exact de la zone, sous le curseur.
            const float rayon = partie.getEscouade().rayonZoneDuSkill(*placeEnVisee);
            sf::CircleShape apercu(rayon);
            apercu.setOrigin({rayon, rayon});
            apercu.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
            apercu.setFillColor(sf::Color::Transparent);
            apercu.setOutlineColor(sf::Color::Yellow);
            apercu.setOutlineThickness(2.0f);
            window.draw(apercu);

            // Cercle de portée de lancement autour du lanceur.
            const Cercle portee = partie.getEscouade().cercleDePorteeDuSkill(*placeEnVisee);
            sf::CircleShape contourPortee(portee.rayon);
            contourPortee.setOrigin({portee.rayon, portee.rayon});
            contourPortee.setPosition(portee.centre);
            contourPortee.setFillColor(sf::Color::Transparent);
            contourPortee.setOutlineColor(sf::Color(255, 255, 255, 160));
            contourPortee.setOutlineThickness(2.0f);
            window.draw(contourPortee);
        }

        // Interface fixe à l'écran : vue par défaut, dessinée en dernier.
        window.setView(window.getDefaultView());
        std::array<std::optional<EtatSkill>, 4> etatsDesSkills;
        for (std::size_t i = 0; i < etatsDesSkills.size(); ++i) {
            etatsDesSkills[i] = partie.getEscouade().etatDuSkill(i + 1);
        }
        dessinerBarreDeSkills(window, police, etatsDesSkills, placeEnVisee);
        window.display();
    }
    return 0;
}
