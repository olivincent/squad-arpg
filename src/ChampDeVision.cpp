// ./src/ChampDeVision.cpp
#include "ChampDeVision.hpp"
#include <array>
#include "TileMap.hpp"

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

// Le balayage d'un quadrant.
class Balayage {
public:
    Balayage(const TileMap& carte, sf::Vector2i origine, Quadrant quadrant, int rayon, std::vector<sf::Vector2i>& visibles)
        : m_carte(carte), m_origine(origine), m_quadrant(quadrant), m_rayon(rayon), m_visibles(visibles) {}

    void balayer(Rangee rangee) {
        if (rangee.profondeur > m_rayon) {
            return;
        }
        CasePrecedente precedente = CasePrecedente::Aucune;
        const int colonneFin = derniereColonne(rangee);
        for (int colonne = premiereColonne(rangee); colonne <= colonneFin; ++colonne) {
            const sf::Vector2i coordCase = coordonnees(rangee.profondeur, colonne);
            const bool mur = estBloquante(m_carte.getTile(coordCase.x, coordCase.y));
            if (mur || centreDansLIntervalle(rangee, colonne)) {
                reveler(rangee.profondeur, colonne);
            }
            if (precedente == CasePrecedente::Mur && !mur) {
                // Le sol qui suit un mur : la lumière recommence au bord du mur.
                rangee.debut = penteDuBord(rangee.profondeur, colonne);
            }
            if (precedente == CasePrecedente::Sol && mur) {
                // Le mur qui suit du sol : la lumière d'avant le mur continue plus loin.
                balayer({rangee.profondeur + 1, rangee.debut, penteDuBord(rangee.profondeur, colonne)});
            }
            precedente = mur ? CasePrecedente::Mur : CasePrecedente::Sol;
        }
        if (precedente == CasePrecedente::Sol) {
            balayer({rangee.profondeur + 1, rangee.debut, rangee.fin});
        }
    }

private:
    // Case de la carte pour (profondeur, colonne) dans ce quadrant.
    sf::Vector2i coordonnees(int profondeur, int colonne) const {
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

    // Distance au centre de l'origine comparée au carré, en entiers : pas de racine.
    void reveler(int profondeur, int colonne) {
        const sf::Vector2i coordCase = coordonnees(profondeur, colonne);
        if (profondeur * profondeur + colonne * colonne <= m_rayon * m_rayon && dansLaCarte(m_carte, coordCase)) {
            m_visibles.push_back(coordCase);
        }
    }

    const TileMap& m_carte;
    sf::Vector2i m_origine;
    Quadrant m_quadrant;
    int m_rayon;
    std::vector<sf::Vector2i>& m_visibles;
};

}  // namespace

std::vector<sf::Vector2i> champDeVision(const TileMap& carte, sf::Vector2i origine, int rayon) {
    std::vector<sf::Vector2i> visibles;
    if (dansLaCarte(carte, origine)) {
        visibles.push_back(origine);
    }
    const std::array<Quadrant, 4> quadrants = {Quadrant::Nord, Quadrant::Est, Quadrant::Sud, Quadrant::Ouest};
    for (const Quadrant quadrant : quadrants) {
        // Au départ, toute la largeur du quadrant est éclairée : pentes de -1 à 1.
        Balayage(carte, origine, quadrant, rayon, visibles).balayer({1, {-1, 1}, {1, 1}});
    }
    return visibles;
}
