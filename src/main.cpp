// ./src/main.cpp
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Suqad-ARPG");
    sf::RectangleShape carre({20, 20});
    carre.setFillColor(sf::Color::Green);
    sf::Font font; 
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        return 1;
    }
    sf::Text text(font);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                std::cout << "x: " << data->position.x <<", y: " << data->position.y << std::endl;
                carre.setPosition(sf::Vector2f(data->position));
                text.setString(std::to_string(data->position.x) + " , " + std::to_string(data->position.y));
                text.setPosition(sf::Vector2f(data->position.x - 60.0f, data->position.y + 40.0f));
            }
        }
        window.clear();
        window.draw(carre);
        window.draw(text);
        window.display();
    }
    return 0;
}