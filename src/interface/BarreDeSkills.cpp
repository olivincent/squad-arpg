// ./src/interface/BarreDeSkills.cpp
#include "interface/BarreDeSkills.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <array>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>

#include "interface/BarreDeRaccourcis.hpp"
#include "escouade/Escouade.hpp"

namespace {

constexpr unsigned int s_tailleSigle = 20;
constexpr unsigned int s_tailleNomDeTouche = 11;
constexpr unsigned int s_tailleSecondes = 20;
constexpr unsigned int s_taillePlace = 11;
constexpr float s_hauteurBande = 14.0f;

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

// Point du bord du carré dans la direction d'un angle mesuré depuis midi,
// en sens horaire, en degrés (y vers le bas).
sf::Vector2f pointDuBord(sf::Vector2f centre, float demiCote, float angleDegres) {
    const float angle = angleDegres * std::numbers::pi_v<float> / 180.0f;
    const sf::Vector2f direction{std::sin(angle), -std::cos(angle)};
    const float facteur = demiCote / std::max(std::abs(direction.x), std::abs(direction.y));
    return centre + direction * facteur;
}

}  // namespace

// Pas de `default` : un IdSkill oublié est signalé par le compilateur.
std::string sigleDuSkill(IdSkill id) {
    switch (id) {
        case IdSkill::FrappePuissante:
            return "FP";
        case IdSkill::Provocation:
            return "PR";
        case IdSkill::Soin:
            return "SO";
        case IdSkill::Explosion:
            return "EX";
        case IdSkill::FrappeSacree:
            return "FS";
        case IdSkill::ImpositionDesMains:
            return "IM";
        case IdSkill::TirPercant:
            return "TP";
        case IdSkill::PluieDeFleches:
            return "PF";
    }
    return "??";
}

int secondesAffichees(float tempsRestant) {
    return static_cast<int>(std::ceil(tempsRestant));
}

std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre) {
    const float fraction = std::clamp(fractionSombre, 0.0f, 1.0f);
    if (fraction <= 0.0f) {
        return {};
    }

    const float demiCote = cote / 2.0f;
    // La zone sombre va de l'aiguille jusqu'à midi (360°).
    const float angleAiguille = (1.0f - fraction) * 360.0f;

    std::vector<sf::Vector2f> points = {centre, pointDuBord(centre, demiCote, angleAiguille)};
    const std::array<float, 4> anglesDesCoins = {45.0f, 135.0f, 225.0f, 315.0f};
    for (const float angleCoin : anglesDesCoins) {
        if (angleCoin > angleAiguille) {
            points.push_back(pointDuBord(centre, demiCote, angleCoin));
        }
    }
    points.push_back({centre.x, centre.y - demiCote});
    return points;
}

sf::Color couleurDuContour(bool enVisee, bool enActivation) {
    if (enVisee) {
        return sf::Color::Yellow;
    }
    return enActivation ? sf::Color::White : sf::Color(140, 140, 140);
}

std::array<IconeDeRaccourci, s_emplacementsPrincipaux> iconesDeLaBarre(
    const BarreDeRaccourcis& barre, const Escouade& escouade, const TouchesDesRaccourcis& touches,
    std::optional<AdresseSkill> enVisee) {
    std::array<IconeDeRaccourci, s_emplacementsPrincipaux> icones;
    for (std::size_t i = 0; i < icones.size(); ++i) {
        icones[i].touche = sf::Keyboard::getDescription(touches[i]);
        const std::optional<AdresseSkill> adresse = barre.adresse(i);
        if (!adresse.has_value()) {
            continue;
        }
        icones[i].etat = escouade.etatDuSkill(*adresse);
        icones[i].couleurDuLanceur = escouade.couleurDuPersonnage(adresse->place).value_or(sf::Color::White);
        icones[i].place = adresse->place;
        icones[i].enVisee = adresse == enVisee;
    }
    return icones;
}

void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police,
                           const std::array<IconeDeRaccourci, s_emplacementsPrincipaux>& icones) {
    const sf::Vector2f tailleVue = rendu.getView().getSize();

    for (std::size_t i = 0; i < icones.size(); ++i) {
        const IconeDeRaccourci& icone = icones[i];
        const sf::FloatRect rectangle = rectangleDeLEmplacement(i, tailleVue);
        const sf::Vector2f coin = rectangle.position;
        const float cote = rectangle.size.x;
        const sf::Vector2f centre = coin + rectangle.size / 2.0f;

        sf::RectangleShape fond(rectangle.size);
        fond.setPosition(coin);
        const bool enActivation = icone.etat.has_value() && icone.etat->enActivation;
        fond.setFillColor(icone.etat.has_value() ? sf::Color(50, 50, 50) : sf::Color(28, 28, 28));
        fond.setOutlineColor(couleurDuContour(icone.enVisee, enActivation));
        fond.setOutlineThickness(icone.enVisee || enActivation ? 3.0f : 1.0f);
        rendu.draw(fond);

        // Lanceur mort, arme exigée absente ou emplacement vide : texte atténué, ni quartier ni chiffre.
        const bool lanceurVivant = icone.etat.has_value() && icone.etat->lanceurVivant;
        const bool utilisable = lanceurVivant && !icone.etat->armeAbsente;
        const sf::Color couleurTexte = utilisable ? sf::Color::White : sf::Color(90, 90, 90);
        sf::Text nomDeLaTouche(police, icone.touche, s_tailleNomDeTouche);
        nomDeLaTouche.setFillColor(couleurTexte);
        nomDeLaTouche.setPosition(coin + sf::Vector2f{4.0f, 1.0f});
        rendu.draw(nomDeLaTouche);

        if (!icone.etat.has_value()) {
            continue;
        }
        dessinerTexteCentre(rendu, police, sigleDuSkill(icone.etat->id), s_tailleSigle, centre, couleurTexte);

        // Bande de la couleur du personnage lanceur, au bas de l'icône, avec sa place.
        sf::RectangleShape bande({cote, s_hauteurBande});
        bande.setPosition({coin.x, coin.y + cote - s_hauteurBande});
        bande.setFillColor(lanceurVivant ? icone.couleurDuLanceur : sf::Color(90, 90, 90));
        rendu.draw(bande);
        sf::Text numeroDePlace(police, std::to_string(icone.place), s_taillePlace);
        numeroDePlace.setFillColor(sf::Color::White);
        numeroDePlace.setOutlineColor(sf::Color::Black);
        numeroDePlace.setOutlineThickness(1.0f);
        const sf::FloatRect zoneDuNumero = numeroDePlace.getLocalBounds();
        numeroDePlace.setOrigin(zoneDuNumero.position + zoneDuNumero.size / 2.0f);
        numeroDePlace.setPosition(bande.getPosition() + bande.getSize() / 2.0f);
        rendu.draw(numeroDePlace);
        if (!utilisable) {
            continue;
        }

        // Zone sombre : la fraction de recharge restante, de l'aiguille à midi.
        sf::VertexArray quartier(sf::PrimitiveType::TriangleFan);
        for (const sf::Vector2f point : pointsDuQuartier(centre, cote, 1.0f - icone.etat->progression)) {
            quartier.append({point, sf::Color(0, 0, 0, 170)});
        }
        rendu.draw(quartier);

        if (icone.etat->tempsRestant > 0.0f) {
            dessinerTexteCentre(rendu, police, std::to_string(secondesAffichees(icone.etat->tempsRestant)), s_tailleSecondes, centre, sf::Color::White);
        }
    }
}
