// ./src/interface/EcranMenu.cpp
#include "interface/EcranMenu.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <memory>
#include <optional>
#include <utility>

#include "interface/Contexte.hpp"
#include "interface/EcranChoixEscouade.hpp"
#include "interface/EcranOptions.hpp"
#include "interface/EntreeDInterface.hpp"

namespace {

constexpr std::size_t s_boutonCommencer = 0;
constexpr std::size_t s_boutonOptions = 1;
constexpr std::size_t s_boutonQuitter = 2;

}  // namespace

EcranMenu::EcranMenu(std::vector<Resolution> resolutions)
    : m_resolutions(std::move(resolutions)), m_boutons({"Commencer partie", "Options", "Quitter"}) {}

Transition EcranMenu::cliquer(sf::Vector2f point, sf::Vector2f centre, const Reglages& reglages) const {
    const std::optional<std::size_t> bouton = m_boutons.boutonSous(point, centre);
    if (bouton == s_boutonCommencer) {
        return Empiler{std::make_unique<EcranChoixEscouade>()};
    }
    if (bouton == s_boutonOptions) {
        return Empiler{std::make_unique<EcranOptions>(reglages, m_resolutions)};
    }
    if (bouton == s_boutonQuitter) {
        return Quitter{};
    }
    return Aucune{};
}

Transition EcranMenu::traiterEvenement(const sf::Event& evenement, const Contexte& contexte) {
    const std::optional<PointDInterface> clic = clicGaucheDInterface(evenement, contexte);
    if (!clic.has_value()) {
        return Aucune{};
    }
    return cliquer(clic->point, clic->centre, contexte.reglages);
}

void EcranMenu::mettreAJour(float, const Contexte&) {}

void EcranMenu::dessiner(const Contexte& contexte) const {
    const PointDInterface curseur = preparerLInterface(contexte);
    m_boutons.dessiner(contexte.fenetre, contexte.police, curseur.centre, m_boutons.boutonSous(curseur.point, curseur.centre));
}
