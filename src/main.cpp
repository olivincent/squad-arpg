// ./src/main.cpp
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <string>
#include <vector>
#include "TileMap.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Squad-ARPG");
    window.setFramerateLimit(60);
    const std::vector<std::string> plan = {
        "##############################",
        "#.........#..................#",
        "#.........#..................#",
        "#.........#..................#",
        "#.........#..................#",
        "#.........#.......####.......#",
        "#.........#.......####.......#",
        "#.........#..................#",
        "#............................#",
        "#............................#",
        "#............................#",
        "#.........#..................#",
        "#.........#..................#",
        "#.........#.......####.......#",
        "#.........#.......####.......#",
        "#.........#..................#",
        "#.........#..................#",
        "#.........#..................#",
        "#.........#..................#",
        "##############################",
    };
    TileMap carte(plan, 40.0f);

    sf::RectangleShape carre({20, 20});
    carre.setFillColor(sf::Color::Green);
    carre.setPosition({100.0f, 100.0f});
    
    sf::Font font; 
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        return 1;
    }

    sf::Text text(font);

    sf::Vector2f cible = carre.getPosition();
    const float pixelsParSeconde = 180.0f;
    sf::Clock horloge;

    while (window.isOpen()) {
        const float dt = horloge.restart().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                std::cout << "x: " << data->position.x <<", y: " << data->position.y << std::endl;
                cible = sf::Vector2f(data->position);
                text.setString(std::to_string(data->position.x) + " , " + std::to_string(data->position.y));
                text.setPosition(sf::Vector2f(data->position.x - 60.0f, data->position.y + 40.0f));
            }
        }

        // MISE À JOUR
        const sf::Vector2f direction = cible - carre.getPosition();
        const float distance = direction.length();
        const float pasDuTic = pixelsParSeconde * dt;
        sf::Vector2f prochainePosition = carre.getPosition();

        if (distance > pasDuTic ) {
            prochainePosition += (direction.normalized() * pasDuTic);
        }else if (distance > 0) {
            prochainePosition = cible;
        }

        const sf::Vector2f deplacement = prochainePosition - carre.getPosition();

        const sf::Vector2f essaiX = carre.getPosition() + sf::Vector2f(deplacement.x, 0.0f);
        if (!carte.chevaucheUnMur(sf::FloatRect(essaiX, carre.getSize()))) {
            carre.setPosition(essaiX);
        }

        const sf::Vector2f essaiY = carre.getPosition() + sf::Vector2f(0.0f, deplacement.y);
        if (!carte.chevaucheUnMur(sf::FloatRect(essaiY, carre.getSize()))) {
            carre.setPosition(essaiY);
        }

        window.clear();
        carte.dessiner(window);
        window.draw(carre);
        window.draw(text);
        window.display();
    }
    return 0;
}