// ./src/TextesFlottants.cpp
#include "TextesFlottants.hpp"
#include <SFML/Graphics/Text.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
constexpr float s_departAuDessus = 20.0f;  // px au-dessus du centre de la cible, à la naissance
constexpr float s_montee = 30.0f;          // px gagnés sur toute la durée de vie
constexpr float s_ecartEmpilement = 14.0f; // px entre deux textes de la même cible dans la même image
constexpr float s_finDeLaOpacitePleine = 0.5f;  // fraction de la vie pendant laquelle le texte est opaque
constexpr unsigned int s_tailleNormale = 16;
constexpr unsigned int s_tailleCoupGratuit = 20;
constexpr float s_epaisseurContour = 1.0f;
const sf::Color s_couleurDegatsSurUnMonstre = sf::Color::White;
const sf::Color s_couleurDegatsSurUnPersonnage(255, 60, 60);
const sf::Color s_couleurSoin(80, 230, 80);
const sf::Color s_couleurCoupGratuit(255, 150, 0);
}  // namespace

float hauteurDuTexte(float age) {
    const float fraction = std::clamp(age / TextesFlottants::s_dureeDeVie, 0.0f, 1.0f);
    return s_departAuDessus + s_montee * fraction;
}

float opaciteDuTexte(float age) {
    const float fraction = age / TextesFlottants::s_dureeDeVie;
    if (fraction <= s_finDeLaOpacitePleine) {
        return 1.0f;
    }
    return std::clamp((1.0f - fraction) / (1.0f - s_finDeLaOpacitePleine), 0.0f, 1.0f);
}

sf::Vector2f positionDuTexte(const TexteFlottant& texte) {
    const float decalage = hauteurDuTexte(texte.age) + s_ecartEmpilement * static_cast<float>(texte.rang);
    return {texte.evenement.position.x, texte.evenement.position.y - decalage};
}

std::string libelleDuTexte(const EvenementDeCombat& evenement) {
    const std::string montant = std::to_string(evenement.montant);
    if (evenement.type == TypeEvenement::Soin) {
        return "+" + montant;
    }
    return evenement.opportunite ? montant + "!" : montant;
}

sf::Color couleurDuTexte(const EvenementDeCombat& evenement) {
    if (evenement.opportunite) {
        return s_couleurCoupGratuit;
    }
    if (evenement.type == TypeEvenement::Soin) {
        return s_couleurSoin;
    }
    return evenement.campCible == Camp::Monstres ? s_couleurDegatsSurUnMonstre : s_couleurDegatsSurUnPersonnage;
}

unsigned int tailleDuTexte(const EvenementDeCombat& evenement) {
    return evenement.opportunite ? s_tailleCoupGratuit : s_tailleNormale;
}

void TextesFlottants::ajouter(const EvenementDeCombat& evenement) {
    const auto memeCibleMemeImage = [&](const TexteFlottant& texte) {
        return texte.image == m_image && texte.evenement.campCible == evenement.campCible &&
               texte.evenement.idCible == evenement.idCible;
    };
    const int rang = static_cast<int>(std::ranges::count_if(m_textes, memeCibleMemeImage));
    m_textes.push_back({evenement, 0.0f, rang, m_image});
}

void TextesFlottants::mettreAJour(float dt) {
    for (TexteFlottant& texte : m_textes) {
        texte.age += dt;
    }
    std::erase_if(m_textes, [](const TexteFlottant& texte) { return texte.age >= s_dureeDeVie; });
    ++m_image;
}

const std::vector<TexteFlottant>& TextesFlottants::actifs() const {
    return m_textes;
}

void TextesFlottants::dessiner(sf::RenderTarget& rendu, const sf::Font& police) const {
    for (const TexteFlottant& texte : m_textes) {
        const auto alpha = static_cast<std::uint8_t>(std::lround(255.0f * opaciteDuTexte(texte.age)));
        sf::Color remplissage = couleurDuTexte(texte.evenement);
        remplissage.a = alpha;
        sf::Color contour = sf::Color::Black;
        contour.a = alpha;

        sf::Text objet(police, libelleDuTexte(texte.evenement), tailleDuTexte(texte.evenement));
        objet.setFillColor(remplissage);
        objet.setOutlineColor(contour);
        objet.setOutlineThickness(s_epaisseurContour);
        const sf::FloatRect zone = objet.getLocalBounds();
        objet.setOrigin(zone.position + zone.size / 2.0f);
        objet.setPosition(positionDuTexte(texte));
        rendu.draw(objet);
    }
}
