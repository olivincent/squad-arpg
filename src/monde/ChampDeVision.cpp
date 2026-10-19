// ./src/monde/ChampDeVision.cpp
#include "monde/ChampDeVision.hpp"
#include <array>
#include <cstdlib>
#include <optional>
#include "monde/TileMap.hpp"

// Principe (« symmetric shadowcasting » d'Albert Ford) : on regarde le monde par
// quatre quadrants (nord, est, sud, ouest). Dans un quadrant, on balaie des
// RANGÉES de cases, de plus en plus loin de l'origine : la rangée de profondeur 1 est
// collée à l'origine, celle de profondeur 2 est derrière, etc. Dans une rangée, la
// colonne 0 est dans l'axe de l'origine, les colonnes négatives d'un côté, les positives
// de l'autre.
//
// La lumière qui traverse une rangée est un intervalle de PENTES [début, fin] (la pente
// est colonne / profondeur : -1 = diagonale d'un côté, 0 = tout droit, 1 = diagonale de
// l'autre). Au départ, tout le quadrant est éclairé : [-1, 1]. Dans une rangée :
//   - on parcourt les cases dont la colonne tombe dans l'intervalle ;
//   - une case bloquante est révélée (on voit sa face) ;
//   - une case libre n'est révélée que si son CENTRE est dans l'intervalle : c'est ce
//     critère, et lui seul, qui rend le résultat symétrique ;
//   - un mur qui suit du sol coupe l'intervalle : la lumière qui passait avant le mur
//     continue dans une rangée suivante, dont la pente de fin est celle du bord du mur ;
//   - un sol qui suit un mur rétrécit le début de l'intervalle à partir du bord du mur ;
//   - si la rangée se termine sur du sol, la lumière passe tout entière à la suivante.
//
// Les pentes sont des fractions d'entiers, comparées par produits croisés : aucun
// flottant, donc aucun arrondi qui casserait la symétrie.
//
// Ce balayage n'existe qu'en UN exemplaire (la classe Balayage) : il appelle un visiteur
// pour chaque case révélée. champDeVision collecte les cases jusqu'à un rayon ;
// estVisibleDepuis cherche une seule case et arrête le balayage dès qu'il la trouve.

namespace {

// Pente = numerateur / denominateur, avec denominateur > 0.
struct Pente {
    int numerateur;
    int denominateur;
};

struct Rangee {
    int profondeur;
    Pente debut;
    Pente fin;
};

// Division entière arrondie vers le bas (le `/` du C++ arrondit vers zéro), diviseur > 0.
int diviserParDefaut(int dividende, int diviseur) {
    int quotient = dividende / diviseur;
    if (dividende % diviseur != 0 && dividende < 0) {
        --quotient;
    }
    return quotient;
}

// Division entière arrondie vers le haut, diviseur > 0.
int diviserParExces(int dividende, int diviseur) {
    return -diviserParDefaut(-dividende, diviseur);
}

// Pente du bord de la case (profondeur, colonne) du côté des colonnes négatives : le
// milieu entre ses colonnes voisines, vu de l'origine.
Pente penteDuBord(int profondeur, int colonne) {
    return {2 * colonne - 1, 2 * profondeur};
}

// Première colonne de la rangée : profondeur × début, arrondi à l'entier le plus proche
// (moitié vers le haut).
int premiereColonne(const Rangee& rangee) {
    return diviserParDefaut(2 * rangee.profondeur * rangee.debut.numerateur + rangee.debut.denominateur,
                            2 * rangee.debut.denominateur);
}

// Dernière colonne de la rangée : profondeur × fin, arrondi à l'entier le plus proche
// (moitié vers le bas).
int derniereColonne(const Rangee& rangee) {
    return diviserParExces(2 * rangee.profondeur * rangee.fin.numerateur - rangee.fin.denominateur,
                           2 * rangee.fin.denominateur);
}

// Le centre de la colonne est-il dans l'intervalle de pentes de la rangée ?
bool centreDansLIntervalle(const Rangee& rangee, int colonne) {
    return colonne * rangee.debut.denominateur >= rangee.profondeur * rangee.debut.numerateur
        && colonne * rangee.fin.denominateur <= rangee.profondeur * rangee.fin.numerateur;
}

enum class Quadrant { Nord, Est, Sud, Ouest };

enum class CasePrecedente { Aucune, Mur, Sol };

bool dansLaCarte(const TileMap& carte, sf::Vector2i coordCase) {
    return coordCase.x >= 0 && coordCase.y >= 0 && coordCase.x < carte.getLargeur() && coordCase.y < carte.getHauteur();
}

// (profondeur, colonne) d'une case dans un quadrant.
struct Position {
    int profondeur;
    int colonne;
};

// Position de la case située à `decalage` de l'origine, dans ce quadrant ; vide si elle
// n'est pas dans le quadrant (profondeur >= 1 et |colonne| <= profondeur, la diagonale
// appartient donc à deux quadrants). Inverse de Balayage::coordonnees.
std::optional<Position> positionDansLeQuadrant(Quadrant quadrant, sf::Vector2i decalage) {
    Position position{0, 0};
    switch (quadrant) {
        case Quadrant::Nord:
            position = {-decalage.y, decalage.x};
            break;
        case Quadrant::Sud:
            position = {decalage.y, decalage.x};
            break;
        case Quadrant::Est:
            position = {decalage.x, decalage.y};
            break;
        case Quadrant::Ouest:
            position = {-decalage.x, decalage.y};
            break;
    }
    if (position.profondeur < 1 || std::abs(position.colonne) > position.profondeur) {
        return std::nullopt;
    }
    return position;
}

// Le balayage d'un quadrant, jusqu'à la profondeur `profondeurMax`. `visiteur(profondeur,
// colonne, case)` est appelé pour chaque case révélée DANS LA CARTE ; s'il renvoie vrai,
// le balayage s'arrête aussitôt. C'est ici, et seulement ici, que se décide si une case
// est révélée.
template <typename Visiteur>
class Balayage {
public:
    Balayage(const TileMap& carte, sf::Vector2i origine, Quadrant quadrant, int profondeurMax, Visiteur visiteur)
        : m_carte(carte), m_origine(origine), m_quadrant(quadrant), m_profondeurMax(profondeurMax), m_visiteur(visiteur) {}

    void balayer(Rangee rangee) {
        if (m_arrete || rangee.profondeur > m_profondeurMax) {
            return;
        }
        CasePrecedente precedente = CasePrecedente::Aucune;
        const int colonneFin = derniereColonne(rangee);
        for (int colonne = premiereColonne(rangee); colonne <= colonneFin; ++colonne) {
            const sf::Vector2i coordCase = coordonnees(rangee.profondeur, colonne);
            const bool mur = estBloquante(m_carte.getTile(coordCase.x, coordCase.y));
            if (mur || centreDansLIntervalle(rangee, colonne)) {
                reveler(rangee.profondeur, colonne);
                if (m_arrete) {
                    return;
                }
            }
            if (precedente == CasePrecedente::Mur && !mur) {
                // Le sol qui suit un mur : la lumière recommence au bord du mur.
                rangee.debut = penteDuBord(rangee.profondeur, colonne);
            }
            if (precedente == CasePrecedente::Sol && mur) {
                // Le mur qui suit du sol : la lumière d'avant le mur continue plus loin.
                balayer({rangee.profondeur + 1, rangee.debut, penteDuBord(rangee.profondeur, colonne)});
                if (m_arrete) {
                    return;
                }
            }
            precedente = mur ? CasePrecedente::Mur : CasePrecedente::Sol;
        }
        if (precedente == CasePrecedente::Sol) {
            balayer({rangee.profondeur + 1, rangee.debut, rangee.fin});
        }
    }

private:
    // Case de la carte pour (profondeur, colonne) dans ce quadrant.
    [[nodiscard]] sf::Vector2i coordonnees(int profondeur, int colonne) const {
        switch (m_quadrant) {
            case Quadrant::Nord:
                return {m_origine.x + colonne, m_origine.y - profondeur};
            case Quadrant::Sud:
                return {m_origine.x + colonne, m_origine.y + profondeur};
            case Quadrant::Est:
                return {m_origine.x + profondeur, m_origine.y + colonne};
            case Quadrant::Ouest:
                break;
        }
        return {m_origine.x - profondeur, m_origine.y + colonne};
    }

    void reveler(int profondeur, int colonne) {
        const sf::Vector2i coordCase = coordonnees(profondeur, colonne);
        if (dansLaCarte(m_carte, coordCase)) {
            m_arrete = m_visiteur(profondeur, colonne, coordCase);
        }
    }

    const TileMap& m_carte;
    sf::Vector2i m_origine;
    Quadrant m_quadrant;
    int m_profondeurMax;
    Visiteur m_visiteur;
    bool m_arrete = false;
};

// Au départ, toute la largeur du quadrant est éclairée : pentes de -1 à 1.
constexpr Rangee s_premiereRangee{1, {-1, 1}, {1, 1}};

constexpr std::array<Quadrant, 4> s_quadrants = {Quadrant::Nord, Quadrant::Est, Quadrant::Sud, Quadrant::Ouest};

}  // namespace

std::vector<sf::Vector2i> champDeVision(const TileMap& carte, sf::Vector2i origine, int rayon) {
    std::vector<sf::Vector2i> visibles;
    if (dansLaCarte(carte, origine)) {
        visibles.push_back(origine);
    }
    // Distance au centre de l'origine comparée au carré, en entiers : pas de racine.
    const auto collecter = [&](int profondeur, int colonne, sf::Vector2i coordCase) {
        if (profondeur * profondeur + colonne * colonne <= rayon * rayon) {
            visibles.push_back(coordCase);
        }
        return false;
    };
    for (const Quadrant quadrant : s_quadrants) {
        Balayage(carte, origine, quadrant, rayon, collecter).balayer(s_premiereRangee);
    }
    return visibles;
}

bool estVisibleDepuis(const TileMap& carte, sf::Vector2i origine, sf::Vector2i cible) {
    if (origine == cible) {
        return true;
    }
    bool trouvee = false;
    const auto chercher = [&](int, int, sf::Vector2i coordCase) {
        trouvee = (coordCase == cible);
        return trouvee;
    };
    for (const Quadrant quadrant : s_quadrants) {
        // Un quadrant qui ne contient pas la cible ne peut pas la révéler ; la cible d'une
        // diagonale est dans deux quadrants, il suffit qu'un des deux la révèle.
        const std::optional<Position> position = positionDansLeQuadrant(quadrant, cible - origine);
        if (!position) {
            continue;
        }
        Balayage(carte, origine, quadrant, position->profondeur, chercher).balayer(s_premiereRangee);
        if (trouvee) {
            return true;
        }
    }
    return false;
}

bool ligneDeVue(const TileMap& carte, sf::Vector2f a, sf::Vector2f b) {
    const sf::Vector2i caseA = carte.caseContenant(a);
    const sf::Vector2i caseB = carte.caseContenant(b);
    if (estBloquante(carte.getTile(caseA.x, caseA.y)) || estBloquante(carte.getTile(caseB.x, caseB.y))) {
        return false;
    }
    return estVisibleDepuis(carte, caseA, caseB);
}
