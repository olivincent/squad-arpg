// ./src/interface/AffichageObjetsAuSol.cpp
#include "interface/AffichageObjetsAuSol.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include "interface/AffichageObjets.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
constexpr float s_rayonDuLosange = 8.0f;  // en pixels du monde : un losange est un cercle à quatre côtés
constexpr float s_epaisseurDuContourDuLosange = 1.5f;
constexpr unsigned int s_tailleDuNom = 14;
constexpr float s_epaisseurDuContourDuNom = 2.0f;
constexpr float s_hauteurDuNom = 20.0f;  // en pixels : du centre du losange au centre du nom
}  // namespace

void dessinerObjetsAuSol(sf::RenderTarget& rendu, const std::vector<ObjetAuSol>& objets) {
    for (const ObjetAuSol& objet : objets) {
        sf::CircleShape losange(s_rayonDuLosange, 4);
        losange.setOrigin({s_rayonDuLosange, s_rayonDuLosange});
        losange.setPosition(objet.position);
        losange.setFillColor(couleurDeLaRarete(objet.objet.rarete));
        losange.setOutlineColor(sf::Color::Black);
        losange.setOutlineThickness(s_epaisseurDuContourDuLosange);
        rendu.draw(losange);
    }
}

void dessinerNomDObjetAuSol(sf::RenderTarget& rendu, const sf::Font& police, const ObjetAuSol& objet) {
    const std::string nom = nomDeLObjet(objet.objet);
    sf::Text texte(police, sf::String::fromUtf8(nom.begin(), nom.end()), s_tailleDuNom);
    texte.setFillColor(couleurDeLaRarete(objet.objet.rarete));
    texte.setOutlineColor(sf::Color::Black);
    texte.setOutlineThickness(s_epaisseurDuContourDuNom);
    const sf::FloatRect zone = texte.getLocalBounds();
    texte.setOrigin(zone.position + zone.size / 2.0f);
    texte.setPosition(objet.position - sf::Vector2f(0.0f, s_hauteurDuNom));
    rendu.draw(texte);
}
