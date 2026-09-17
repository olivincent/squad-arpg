#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Suqad-ARPG");
    sf::RectangleShape carre({100, 100});
    carre.setFillColor(sf::Color::Green);
 
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* data = event->getIf<sf::Event::MouseButtonPressed>()) {
                std::cout << "x: " << data->position.x <<", y: " << data->position.y << std::endl;
            }
        }
        window.clear();
        window.draw(carre);
        window.display();
    }
    return 0;
}