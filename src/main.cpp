// ./src/main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <optional>
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
    int prochainId = 1;
    std::vector<Monstre> monstres = {
        Monstre({300.0f, 100.0f}, prochainId++),
        Monstre({100.0f, 300.0f}, prochainId++),
        Monstre({300.0f, 500.0f}, prochainId++),
    };
    std::optional<int> idCible;

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
                    const auto it = std::find_if(monstres.begin(), monstres.end(), [&](const Monstre& m) {
                        return m.contient(clicMonde);
                    });
                    if (it != monstres.end()) {
                        idCible = it->getId();
                    }
                }
            }
        }

        if (idCible.has_value()) {
            const auto it = std::find_if(monstres.begin(), monstres.end(), [&](const Monstre& m) {
                return m.getId() == idCible.value();
            });
            if (it == monstres.end()) {
                idCible.reset();
            } else {
                it->recevoirDegats(escouade.attaquer(it->getCentre(), carte));
                if (!it->estVivant()) {
                    idCible.reset();
                }
            }
        }

        if (!window.hasFocus() || !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            idCible.reset();
        }

        std::erase_if(monstres, [](const Monstre& m) { return !m.estVivant(); });

        if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !idCible.has_value()) {
            const sf::Vector2f curseurMonde = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            escouade.allerVers(curseurMonde, carte);
        }

        // MISE À JOUR
        escouade.mettreAJour(dt, carte);

        camera.setCenter(escouade.getCentre());
        window.setView(camera);
        window.clear();
        carte.dessiner(window);
        for (const Monstre& m : monstres) {
            m.dessiner(window);
        }
        escouade.dessiner(window);
        window.display();
    }
    return 0;
}