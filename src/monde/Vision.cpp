// ./src/monde/Vision.cpp
#include "monde/Vision.hpp"
#include <algorithm>
#include <cassert>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <cmath>
#include "monde/ChampDeVision.hpp"
#include "monde/TileMap.hpp"

namespace {
const sf::Color s_voileInconnue(0, 0, 0, 255);
const sf::Color s_voileExploree(0, 0, 0, 150);
}  // namespace

Vision::Vision(const TileMap& carte)
    : m_largeur(carte.getLargeur()),
      m_hauteur(carte.getHauteur()),
      m_tailleCase(carte.getTailleCase()),
      m_cases(static_cast<std::size_t>(m_largeur * m_hauteur), EtatCase::Inconnue) {}

void Vision::mettreAJour(const TileMap& carte, const std::vector<Cercle>& sources) {
    assert(carte.getLargeur() == m_largeur && carte.getHauteur() == m_hauteur);

    std::ranges::replace(m_cases, EtatCase::Visible, EtatCase::Exploree);

    for (const Cercle& source : sources) {
        const int rayonEnCases = static_cast<int>(std::floor(source.rayon / m_tailleCase));
        for (const sf::Vector2i coordCase : champDeVision(carte, carte.caseContenant(source.centre), rayonEnCases)) {
            m_cases[static_cast<std::size_t>(coordCase.y * m_largeur + coordCase.x)] = EtatCase::Visible;
        }
    }
}

EtatCase Vision::etat(sf::Vector2i coordCase) const {
    if (coordCase.x < 0 || coordCase.y < 0 || coordCase.x >= m_largeur || coordCase.y >= m_hauteur) {
        return EtatCase::Inconnue;
    }
    return m_cases[static_cast<std::size_t>(coordCase.y * m_largeur + coordCase.x)];
}

bool Vision::estVisible(sf::Vector2f point) const {
    const sf::Vector2i coordCase(static_cast<int>(std::floor(point.x / m_tailleCase)), static_cast<int>(std::floor(point.y / m_tailleCase)));
    return etat(coordCase) == EtatCase::Visible;
}

// Un seul appel de dessin : toutes les cases voilées sont des triangles d'un même tableau de
// sommets, reconstruit à chaque appel (la vision change presque à chaque image).
void Vision::dessiner(sf::RenderTarget& rendu) const {
    sf::VertexArray sommets(sf::PrimitiveType::Triangles);
    for (int y = 0; y < m_hauteur; ++y) {
        for (int x = 0; x < m_largeur; ++x) {
            const EtatCase etatDeLaCase = m_cases[static_cast<std::size_t>(y * m_largeur + x)];
            if (etatDeLaCase == EtatCase::Visible) {
                continue;
            }
            const sf::Color couleur = etatDeLaCase == EtatCase::Inconnue ? s_voileInconnue : s_voileExploree;
            const sf::Vector2f hautGauche(static_cast<float>(x) * m_tailleCase, static_cast<float>(y) * m_tailleCase);
            const sf::Vector2f hautDroit = hautGauche + sf::Vector2f(m_tailleCase, 0.0f);
            const sf::Vector2f basGauche = hautGauche + sf::Vector2f(0.0f, m_tailleCase);
            const sf::Vector2f basDroit = hautGauche + sf::Vector2f(m_tailleCase, m_tailleCase);
            // Deux triangles par case.
            for (const sf::Vector2f coin : {hautGauche, hautDroit, basGauche, hautDroit, basDroit, basGauche}) {
                sommets.append(sf::Vertex{coin, couleur});
            }
        }
    }
    rendu.draw(sommets);
}
