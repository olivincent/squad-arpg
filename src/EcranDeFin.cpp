// ./src/EcranDeFin.cpp
#include "EcranDeFin.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

namespace {

constexpr unsigned char s_opaciteVoile = 170;
constexpr unsigned int s_tailleTitre = 64;
constexpr unsigned int s_tailleConsigne = 24;
constexpr float s_ecartTitreConsigne = 60.0f;

// Écrit `texte` centré sur `centre`.
void dessinerTexteCentre(sf::RenderTarget& rendu, const sf::Font& police, const std::string& texte,
                         unsigned int taille, sf::Vector2f centre, sf::Color couleur) {
    sf::Text objet(police, texte, taille);
    objet.setFillColor(couleur);
    const sf::FloatRect zone = objet.getLocalBounds();
    objet.setOrigin(zone.position + zone.size / 2.0f);
    objet.setPosition(centre);
    rendu.draw(objet);
}

}  // namespace

void dessinerEcranDeFin(sf::RenderTarget& rendu, const sf::Font& police, const std::string& titre) {
    const sf::Vector2f tailleVue = rendu.getView().getSize();

    sf::RectangleShape voile(tailleVue);
    voile.setFillColor(sf::Color(0, 0, 0, s_opaciteVoile));
    rendu.draw(voile);

    const sf::Vector2f centre = tailleVue / 2.0f;
    dessinerTexteCentre(rendu, police, titre, s_tailleTitre, centre, sf::Color::White);
    dessinerTexteCentre(rendu, police, "Appuyez sur R pour revenir au menu", s_tailleConsigne,
                        centre + sf::Vector2f{0.0f, s_ecartTitreConsigne}, sf::Color::White);
}
