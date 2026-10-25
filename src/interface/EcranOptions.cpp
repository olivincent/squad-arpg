// ./src/interface/EcranOptions.cpp
#include "interface/EcranOptions.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <utility>

#include "interface/Contexte.hpp"
#include "interface/EntreeDInterface.hpp"

namespace {

constexpr std::size_t s_ligneMode = 0;
constexpr std::size_t s_ligneResolution = 1;
constexpr std::size_t s_ligneRetour = 2;

// Le tiers gauche (ou droit) de la ligne de résolution sert de flèche.
constexpr float s_fractionFleche = 1.0f / 3.0f;

}  // namespace

EcranOptions::EcranOptions(const Reglages& reglages, std::vector<Resolution> resolutions)
    : m_reglages(reglages),
      m_resolutions(std::move(resolutions)),
      m_indiceResolution(0),
      m_boutons(creerBoutons()) {
    const auto position = std::find(m_resolutions.begin(), m_resolutions.end(), Resolution{reglages.largeur, reglages.hauteur});
    if (position != m_resolutions.end()) {
        m_indiceResolution = static_cast<std::size_t>(position - m_resolutions.begin());
    }
}

ListeDeBoutons EcranOptions::creerBoutons() const {
    const bool pleinEcran = m_reglages.mode == ModeAffichage::PleinEcran;
    ListeDeBoutons boutons({
        pleinEcran ? "Mode: Fullscreen" : "Mode: Windowed",
        "Resolution: < " + std::to_string(m_reglages.largeur) + " × " + std::to_string(m_reglages.hauteur) + " >",
        "Back",
    });
    boutons.definirActif(s_ligneResolution, !pleinEcran && !m_resolutions.empty());
    return boutons;
}

Transition EcranOptions::changerResolution(int decalage) {
    if ((decalage < 0 && m_indiceResolution == 0) || (decalage > 0 && m_indiceResolution + 1 >= m_resolutions.size())) {
        return Aucune{};
    }
    m_indiceResolution = decalage < 0 ? m_indiceResolution - 1 : m_indiceResolution + 1;
    m_reglages.largeur = m_resolutions[m_indiceResolution].largeur;
    m_reglages.hauteur = m_resolutions[m_indiceResolution].hauteur;
    m_boutons = creerBoutons();
    return AppliquerReglages{m_reglages};
}

Transition EcranOptions::cliquer(sf::Vector2f point, sf::Vector2f centre) {
    const std::optional<std::size_t> ligne = m_boutons.boutonSous(point, centre);
    if (ligne == s_ligneMode) {
        m_reglages.mode = m_reglages.mode == ModeAffichage::Fenetre ? ModeAffichage::PleinEcran : ModeAffichage::Fenetre;
        m_boutons = creerBoutons();
        return AppliquerReglages{m_reglages};
    }
    if (ligne == s_ligneResolution) {
        const sf::FloatRect zone = m_boutons.rectangleDuBouton(s_ligneResolution, centre);
        if (point.x < zone.position.x + zone.size.x * s_fractionFleche) {
            return changerResolution(-1);
        }
        if (point.x > zone.position.x + zone.size.x * (1.0f - s_fractionFleche)) {
            return changerResolution(1);
        }
        return Aucune{};
    }
    if (ligne == s_ligneRetour) {
        return Depiler{};
    }
    return Aucune{};
}

Transition EcranOptions::traiterEvenement(const sf::Event& evenement, const Contexte& contexte) {
    const std::optional<PointDInterface> clic = clicGaucheDInterface(evenement, contexte);
    if (!clic.has_value()) {
        return Aucune{};
    }
    return cliquer(clic->point, clic->centre);
}

void EcranOptions::mettreAJour(float, const Contexte&) {}

void EcranOptions::dessiner(const Contexte& contexte) const {
    const PointDInterface curseur = preparerLInterface(contexte);
    m_boutons.dessiner(contexte.fenetre, contexte.police, curseur.centre, m_boutons.boutonSous(curseur.point, curseur.centre));
}
