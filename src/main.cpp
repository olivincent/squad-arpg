// ./src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <vector>
#include "TileMap.hpp"
#include "Escouade.hpp"
#include "Monstre.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Squad-ARPG");
    window.setFramerateLimit(60);
    const std::vector<std::string> plan = {
        "###########################################################",
        "#.........#..................###########..................#",
        "#.........#..................###########..................#",
        "#.........#...............................................#",
        "#.........#...............................................#",
        "#.........#.......####....................................#",
        "#.........#.......####.......###########..................#",
        "#.........#..................###########..................#",
        "#............................####################...#######",
        "#............................#............................#",
        "#............................#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "#.........#.......####.......#............................#",
        "#.........#.......####.......#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "#.........#..................#............................#",
        "##############################################...##########",
        "##############################################...##########",
        "#............................######.......................#",
        "#............................######.......................#",
        "#............................#............................#",
        "#............................#............................#",
        "#.................####.......#............................#",
        "#.................####.......######.......................#",
        "#............................######.......................#",
        "#............................####################...#######",
        "#............................#............................#",
        "#............................#............................#",
        "###########..................#............................#",
        "#.........#..................#............................#",
        "#.........#.......####.................###................#",
        "#.........#.......####.................###................#",
        "#......................................###.........########",
        "#............................#............................#",
        "#............................#............................#",
        "#.........#..................#............................#",
        "###########################################################",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({100.0f, 100.0f});
    Monstre monstre({300.0f, 100.0f});
    bool cibleVerrouillee = false;

    sf::Clock horloge;
    sf::View camera = window.getDefaultView();

    while (window.isOpen()) {
        const float dt = horloge.restart().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (data->button == sf::Mouse::Button::Left) {
                    const sf::Vector2f clicMonde = window.mapPixelToCoords(data->position);
                    if (monstre.estVivant() && monstre.contient(clicMonde)) {
                        cibleVerrouillee = true;
                    }
                }
            }
        }

        if (cibleVerrouillee) {
            monstre.recevoirDegats(escouade.attaquer(monstre.getCentre(), carte));
        }

        if (!window.hasFocus() || !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || !monstre.estVivant()) {
            cibleVerrouillee = false;
        }

        if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !cibleVerrouillee) {
            const sf::Vector2f curseurMonde = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            escouade.allerVers(curseurMonde, carte);
        }

        // MISE À JOUR
        escouade.mettreAJour(dt, carte);

        camera.setCenter(escouade.getCentre());
        window.setView(camera);
        window.clear();
        carte.dessiner(window);
        if (monstre.estVivant()) {
            monstre.dessiner(window);
        }
        escouade.dessiner(window);
        window.display();
    }
    return 0;
}