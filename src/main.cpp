#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Suqad-ARPG");
    sf::RectangleShape carre({100, 100});
    carre.setFillColor(sf::Color::Green);
 
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        window.clear();
        window.draw(carre);
        window.display();
    }
    return 0;
}