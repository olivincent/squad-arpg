// ./src/monde/TileMap.cpp
#include "monde/TileMap.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace {
const sf::Color s_couleurSol(60, 45, 35);
const sf::Color s_couleurPorte(139, 90, 43);
const sf::Color s_couleurContourCase(20, 20, 20);
constexpr float s_epaisseurContourCase = 1.0f;
constexpr float s_epaisseurCadrePorte = 4.0f;
constexpr int s_nombreDeMarches = 4;  // les bandes qui font l'escalier, de la plus sombre (en haut) à la plus claire

// Les marches d'un escalier : des bandes horizontales de plus en plus claires dans la case dont `coin` est le coin haut gauche.
void dessinerLesMarches(sf::RenderTarget& rendu, sf::Vector2f coin, float cote) {
    const float hauteur = cote / static_cast<float>(s_nombreDeMarches);
    sf::RectangleShape marche({cote, hauteur});
    for (int i = 0; i < s_nombreDeMarches; ++i) {
        const auto nuance = static_cast<std::uint8_t>(90 + 35 * i);
        marche.setPosition({coin.x, coin.y + static_cast<float>(i) * hauteur});
        marche.setFillColor(sf::Color(nuance, nuance, static_cast<std::uint8_t>(nuance - 25)));
        marche.setOutlineColor(s_couleurContourCase);
        marche.setOutlineThickness(-s_epaisseurContourCase);
        rendu.draw(marche);
    }
}
}

bool estBloquante(Tile tile) {
    return tile == Tile::Wall || tile == Tile::PorteFermee;
}

TileMap::TileMap(const std::vector<std::string>& plan, float tailleCase)
    : m_largeur(static_cast<int>(plan.at(0).size())),
      m_hauteur(static_cast<int>(plan.size())),
      m_tailleCase(tailleCase),
      m_cases(m_largeur * m_hauteur, Tile::Floor),
      m_escalier()
{
    for (int y = 0; y < m_hauteur; ++y) {
        const std::string& ligne = plan[y];
        if (static_cast<int>(ligne.size()) != m_largeur) {
            throw std::invalid_argument("TileMap : la ligne " + std::to_string(y) + " n'a pas la bonne longueur");
        }
        for (int x = 0; x < m_largeur; ++x) {
            if (ligne[x] == s_caractereDeMur) {
                m_cases[indice(x, y)] = Tile::Wall;
            } else if (ligne[x] == s_caractereDEscalier) {
                assert(!m_escalier.has_value());  // un seul escalier par plan
                m_cases[indice(x, y)] = Tile::Escalier;
                m_escalier = sf::Vector2i{x, y};
            }
        }
    }
}

int TileMap::indice(int x, int y) const {
    return y * m_largeur + x;
}

Tile TileMap::getTile(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_largeur || y >= m_hauteur) {
        return Tile::Wall;
    }
    return m_cases[indice(x, y)];
}

int TileMap::getLargeur() const {
    return m_largeur;
}

int TileMap::getHauteur() const {
    return m_hauteur;
}

float TileMap::getTailleCase() const {
    return m_tailleCase;
}

std::optional<sf::Vector2i> TileMap::caseDeLEscalier() const {
    return m_escalier;
}

sf::Vector2i TileMap::caseContenant(sf::Vector2f point) const {
    return {static_cast<int>(std::floor(point.x / m_tailleCase)), static_cast<int>(std::floor(point.y / m_tailleCase))};
}

sf::Vector2f TileMap::centreDeCase(sf::Vector2i coordCase) const {
    return {(static_cast<float>(coordCase.x) + 0.5f) * m_tailleCase, (static_cast<float>(coordCase.y) + 0.5f) * m_tailleCase};
}

void TileMap::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme({m_tailleCase, m_tailleCase});

    for (int y = 0; y < m_hauteur; ++y) {
        for (int x = 0; x < m_largeur; ++x) {
            forme.setPosition({static_cast<float>(x) * m_tailleCase, static_cast<float>(y) * m_tailleCase});
            const Tile tile = m_cases[indice(x, y)];
            forme.setOutlineThickness(tile == Tile::PorteOuverte ? -s_epaisseurCadrePorte : -s_epaisseurContourCase);
            if (tile == Tile::Wall) {
                forme.setFillColor(sf::Color(128, 128, 128));
                forme.setOutlineColor(s_couleurContourCase);
            } else if (tile == Tile::PorteFermee) {
                forme.setFillColor(s_couleurPorte);
                forme.setOutlineColor(s_couleurContourCase);
            } else if (tile == Tile::PorteOuverte) {
                forme.setFillColor(s_couleurSol);
                forme.setOutlineColor(s_couleurPorte);
            } else {
                forme.setFillColor(s_couleurSol);
                forme.setOutlineColor(s_couleurContourCase);
            }
            rendu.draw(forme);
            if (tile == Tile::Escalier) {
                dessinerLesMarches(rendu, forme.getPosition(), m_tailleCase);
            }
        }
    }
}

void TileMap::placerPorte(sf::Vector2i coordCase) {
    assert(getTile(coordCase.x, coordCase.y) == Tile::Floor);
    m_cases[indice(coordCase.x, coordCase.y)] = Tile::PorteFermee;
}

void TileMap::ouvrirPorte(sf::Vector2i coordCase) {
    assert(getTile(coordCase.x, coordCase.y) == Tile::PorteFermee);
    m_cases[indice(coordCase.x, coordCase.y)] = Tile::PorteOuverte;
}

bool TileMap::chevaucheUnMur(const sf::FloatRect& rect) const {
    const int xMin = static_cast<int>(std::floor(rect.position.x / m_tailleCase));
    const int yMin = static_cast<int>(std::floor(rect.position.y / m_tailleCase));
    const int xMax = static_cast<int>(std::ceil((rect.position.x + rect.size.x) / m_tailleCase)) - 1;
    const int yMax = static_cast<int>(std::ceil((rect.position.y + rect.size.y) / m_tailleCase)) - 1;

    for (int y = yMin; y <= yMax; ++y) {
        for (int x = xMin; x <= xMax; ++x) {
            if (estBloquante(getTile(x, y))) {
                return true;
            }
        }
    }
    return false;
}

bool TileMap::trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const {
    if (deplacement.x == 0.0f || deplacement.y == 0.0f) {
        // Déplacement horizontal, vertical ou nul : la zone balayée est le
        // rectangle qui englobe la position de départ et celle d'arrivée.
        const sf::Vector2f position(std::min(rect.position.x, rect.position.x + deplacement.x),
                                     std::min(rect.position.y, rect.position.y + deplacement.y));
        const sf::Vector2f taille(rect.size.x + std::abs(deplacement.x), rect.size.y + std::abs(deplacement.y));

        return !chevaucheUnMur(sf::FloatRect(position, taille));
    }

    // Cas oblique : balayage par colonnes de cases. Pour chaque colonne
    // couverte par le mouvement, on calcule la fenêtre de temps [tEntree,
    // tSortie] (t = 0 au départ, t = 1 à l'arrivée) durant laquelle le
    // rectangle en mouvement chevauche horizontalement cette colonne, puis
    // la plage de hauteur balayée par le rectangle pendant cette fenêtre.
    const float xMin = std::min(rect.position.x, rect.position.x + deplacement.x);
    const float xMax = std::max(rect.position.x + rect.size.x, rect.position.x + rect.size.x + deplacement.x);
    const int colonneGauche = static_cast<int>(std::floor(xMin / m_tailleCase));
    const int colonneDroite = static_cast<int>(std::ceil(xMax / m_tailleCase)) - 1;

    for (int colonne = colonneGauche; colonne <= colonneDroite; ++colonne) {
        const float xGauche = static_cast<float>(colonne) * m_tailleCase;
        const float xDroite = xGauche + m_tailleCase;

        // t1 : instant où le bord arrière du rectangle atteint xGauche.
        // t2 : instant où son bord avant atteint xDroite. Selon le sens du
        // déplacement (signe de deplacement.x), l'un ou l'autre peut être
        // l'entrée ou la sortie : min/max rend le calcul valable dans les
        // deux sens, sans avoir à distinguer les cas.
        const float t1 = (xGauche - (rect.position.x + rect.size.x)) / deplacement.x;
        const float t2 = (xDroite - rect.position.x) / deplacement.x;
        const float tEntree = std::clamp(std::min(t1, t2), 0.0f, 1.0f);
        const float tSortie = std::clamp(std::max(t1, t2), 0.0f, 1.0f);

        if (tEntree >= tSortie) {
            continue; // colonne seulement touchée, jamais réellement chevauchée
        }

        const float haut = rect.position.y + std::min(tEntree * deplacement.y, tSortie * deplacement.y);
        const float bas = rect.position.y + rect.size.y + std::max(tEntree * deplacement.y, tSortie * deplacement.y);
        const int ligneHaut = static_cast<int>(std::floor(haut / m_tailleCase));
        const int ligneBas = static_cast<int>(std::ceil(bas / m_tailleCase)) - 1;

        for (int ligne = ligneHaut; ligne <= ligneBas; ++ligne) {
            if (estBloquante(getTile(colonne, ligne))) {
                return false;
            }
        }
    }

    return true;
}
