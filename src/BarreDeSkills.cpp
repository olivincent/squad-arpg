// ./src/BarreDeSkills.cpp
#include "BarreDeSkills.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>

namespace {

constexpr float s_coteIcone = 56.0f;
constexpr float s_espacement = 8.0f;
constexpr float s_margeBas = 16.0f;

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

char lettreDuSkill(TypeSkill type) {
    switch (type) {
        case TypeSkill::Degats:
            return 'D';
        case TypeSkill::Provocation:
            return 'P';
        case TypeSkill::Soin:
            return 'S';
        case TypeSkill::Zone:
            return 'Z';
    }
    return '?';
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

void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police,
                           const std::array<std::optional<EtatSkill>, 4>& etats,
                           std::optional<std::size_t> placeEnVisee) {
    const sf::Vector2f tailleVue = rendu.getDefaultView().getSize();
    const float largeurBarre = 4.0f * s_coteIcone + 3.0f * s_espacement;
    const float gauche = (tailleVue.x - largeurBarre) / 2.0f;
    const float haut = tailleVue.y - s_margeBas - s_coteIcone;

    for (std::size_t i = 0; i < etats.size(); ++i) {
        const sf::Vector2f coin{gauche + static_cast<float>(i) * (s_coteIcone + s_espacement), haut};

        sf::RectangleShape fond({s_coteIcone, s_coteIcone});
        fond.setPosition(coin);
        fond.setFillColor(sf::Color(50, 50, 50));
        const bool enVisee = placeEnVisee == i + 1;
        const bool enActivation = etats[i].has_value() && etats[i]->enActivation;
        fond.setOutlineColor(couleurDuContour(enVisee, enActivation));
        fond.setOutlineThickness(enVisee || enActivation ? 3.0f : 1.0f);
        rendu.draw(fond);

        if (!etats[i].has_value()) {
            continue;
        }
        const sf::Vector2f centre = coin + sf::Vector2f{s_coteIcone, s_coteIcone} / 2.0f;
        // Lanceur mort : lettre et numéro atténués, ni quartier ni chiffre.
        const sf::Color couleurTexte = etats[i]->lanceurVivant ? sf::Color::White : sf::Color(90, 90, 90);
        dessinerTexteCentre(rendu, police, std::string(1, lettreDuSkill(etats[i]->type)), 30, centre, couleurTexte);
        dessinerTexteCentre(rendu, police, std::to_string(i + 1), 12, coin + sf::Vector2f{9.0f, 11.0f}, couleurTexte);
        if (!etats[i]->lanceurVivant) {
            continue;
        }

        // Zone sombre : la fraction de recharge restante, de l'aiguille à midi.
        sf::VertexArray quartier(sf::PrimitiveType::TriangleFan);
        for (const sf::Vector2f point : pointsDuQuartier(centre, s_coteIcone, 1.0f - etats[i]->progression)) {
            quartier.append({point, sf::Color(0, 0, 0, 170)});
        }
        rendu.draw(quartier);

        if (etats[i]->tempsRestant > 0.0f) {
            dessinerTexteCentre(rendu, police, std::to_string(secondesAffichees(etats[i]->tempsRestant)), 24, centre, sf::Color::White);
        }
    }
}
