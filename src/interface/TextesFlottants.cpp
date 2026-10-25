// ./src/interface/TextesFlottants.cpp
#include "interface/TextesFlottants.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <variant>
#include "combat/Cible.hpp"
#include "combat/Element.hpp"
#include "interface/AffichageSkills.hpp"

namespace {
constexpr float s_departAuDessus = 20.0f;  // px au-dessus du centre de la cible, à la naissance
constexpr float s_montee = 30.0f;          // px gagnés sur toute la durée de vie
constexpr float s_ecartEmpilement = 14.0f; // px entre deux textes de la même cible dans la même image
constexpr float s_finDeLaOpacitePleine = 0.5f;  // fraction de la vie pendant laquelle le texte est opaque
constexpr unsigned int s_tailleNormale = 16;
constexpr unsigned int s_tailleCoupGratuit = 20;
constexpr unsigned int s_tailleCritique = 18;  // plus gros qu'un coup normal, moins qu'un coup gratuit
constexpr unsigned int s_tailleNomDeSkill = 14;
constexpr unsigned int s_tailleProvoque = 14;
constexpr unsigned int s_tailleEsquive = 14;
constexpr unsigned int s_tailleBloque = 14;
constexpr float s_epaisseurContour = 1.0f;
const sf::Color s_couleurDegatsSurUnMonstre = sf::Color::White;
const sf::Color s_couleurDegatsSurUnPersonnage(255, 60, 60);
const sf::Color s_couleurSoin(80, 230, 80);
const sf::Color s_couleurCoupGratuit(255, 150, 0);
const sf::Color s_couleurCritique(255, 235, 0);     // jaune vif, plus clair que le jaune doré des noms de skills
const std::string s_suiteDuCritique = " Critique !";
// La couleur du nombre d'un coup selon l'élément de sa plus grosse portion ; le physique n'en a pas (la couleur de
// la cible, comme avant). Distinctes du blanc, du rouge, de l'orange du coup gratuit et du jaune du critique
// (`test_TextesFlottants`) : le feu est un orange rougeâtre, le sacré un or pâle.
const ParElement<std::optional<sf::Color>> s_couleurDeLElement = [] {
    ParElement<std::optional<sf::Color>> couleurs;
    couleurs[Element::Feu] = sf::Color(255, 95, 10);
    couleurs[Element::Sacre] = sf::Color(225, 185, 75);
    return couleurs;
}();
const sf::Color s_couleurNomDeSkill(255, 210, 60);  // jaune doré
const sf::Color s_couleurProvoque(255, 100, 40);    // rouge orangé
const sf::Color s_couleurEsquive(215, 215, 215);    // gris clair
const sf::Color s_couleurBloque(150, 150, 150);     // gris
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
    return std::visit(Surcharges{[](const DegatsSubis& degats) {
                                     const std::string montant = std::to_string(degats.montant);
                                     const std::string coupGratuit = degats.opportunite ? montant + "!" : montant;
                                     return degats.critique ? coupGratuit + s_suiteDuCritique : coupGratuit;
                                 },
                                 [](const Soin& soin) {
                                     const std::string montant = "+" + std::to_string(soin.montant);
                                     return soin.critique ? montant + s_suiteDuCritique : montant;
                                 },
                                 [](const SkillLance& lance) { return nomDuSkill(lance.skill); },
                                 [](const Provoque&) { return std::string("Provoqué"); },
                                 [](const Esquive&) { return std::string("Esquivé"); },
                                 [](const Bloque& bloque) {
                                     // Un blocage partiel dit combien il a absorbé (« Bloqué −40 », signe moins U+2212).
                                     return bloque.total ? std::string("Bloqué") : "Bloqué \u2212" + std::to_string(bloque.montant);
                                 }},
                      evenement.detail);
}

sf::Color couleurDuTexte(const EvenementDeCombat& evenement) {
    return std::visit(Surcharges{[&](const DegatsSubis& degats) {
                                     if (degats.critique) {
                                         return s_couleurCritique;  // l'emporte sur le coup gratuit
                                     }
                                     if (degats.opportunite) {
                                         return s_couleurCoupGratuit;
                                     }
                                     if (const std::optional<sf::Color>& couleur = s_couleurDeLElement[degats.element]) {
                                         return *couleur;
                                     }
                                     return evenement.campCible == Camp::Monstres ? s_couleurDegatsSurUnMonstre
                                                                                  : s_couleurDegatsSurUnPersonnage;
                                 },
                                 [](const Soin&) { return s_couleurSoin; },
                                 [](const SkillLance&) { return s_couleurNomDeSkill; },
                                 [](const Provoque&) { return s_couleurProvoque; },
                                 [](const Esquive&) { return s_couleurEsquive; },
                                 [](const Bloque&) { return s_couleurBloque; }},
                      evenement.detail);
}

unsigned int tailleDuTexte(const EvenementDeCombat& evenement) {
    return std::visit(Surcharges{[](const DegatsSubis& degats) {
                                     if (degats.critique) {
                                         return s_tailleCritique;
                                     }
                                     return degats.opportunite ? s_tailleCoupGratuit : s_tailleNormale;
                                 },
                                 [](const Soin& soin) { return soin.critique ? s_tailleCritique : s_tailleNormale; },
                                 [](const SkillLance&) { return s_tailleNomDeSkill; },
                                 [](const Provoque&) { return s_tailleProvoque; },
                                 [](const Esquive&) { return s_tailleEsquive; },
                                 [](const Bloque&) { return s_tailleBloque; }},
                      evenement.detail);
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

        // Les libellés sont en UTF-8 (noms de skills accentués).
        const std::string libelle = libelleDuTexte(texte.evenement);
        sf::Text objet(police, sf::String::fromUtf8(libelle.begin(), libelle.end()), tailleDuTexte(texte.evenement));
        objet.setFillColor(remplissage);
        objet.setOutlineColor(contour);
        objet.setOutlineThickness(s_epaisseurContour);
        const sf::FloatRect zone = objet.getLocalBounds();
        objet.setOrigin(zone.position + zone.size / 2.0f);
        objet.setPosition(positionDuTexte(texte));
        rendu.draw(objet);
    }
}
