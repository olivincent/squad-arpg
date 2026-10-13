// ./src/Chemin.cpp
#include "Chemin.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <cstdlib>
#include <queue>

namespace {

// Un nœud de la file de priorité : la case, le coût réellement parcouru
// depuis le départ (g) et le coût total estimé (f = g + heuristique),
// utilisé pour l'ordre de sortie de la file.
struct Noeud {
    sf::Vector2i position;
    int coutParcouru;
    int coutTotal;
};

// std::priority_queue sort toujours le PLUS GRAND élément en premier. On
// veut l'inverse (le plus petit coût total), donc le comparateur inverse
// l'ordre : il renvoie vrai quand `a` doit être considéré "plus petit"
// que `b` au sens de la file, c'est-à-dire quand son coût total est plus
// grand (donc moins prioritaire).
struct ComparateurNoeud {
    bool operator()(const Noeud& a, const Noeud& b) const {
        return a.coutTotal > b.coutTotal;
    }
};

// Distance de Manhattan : nombre de pas minimum entre deux cases en se
// déplaçant uniquement horizontalement ou verticalement, sans tenir
// compte des murs. Toujours inférieure ou égale au coût réel du chemin,
// ce qui garantit que l'A* trouve bien le plus court chemin.
int heuristique(sf::Vector2i a, sf::Vector2i b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

// Nombre de divisions du segment dans la recherche de replierVersOrigine :
// le point trouvé est précis à 1/2^6 de la longueur du segment.
constexpr int s_iterationsRepli = 6;

} // namespace

std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee) {
    if (carte.getTile(depart.x, depart.y) == Tile::Wall || carte.getTile(arrivee.x, arrivee.y) == Tile::Wall) {
        return {};
    }
    if (depart == arrivee) {
        return {depart};
    }

    const int largeur = carte.getLargeur();
    const int hauteur = carte.getHauteur();
    const auto indice = [largeur](sf::Vector2i p) { return p.y * largeur + p.x; };

    // Coût parcouru depuis le départ pour chaque case ; -1 = pas encore
    // atteinte. Case de départ et d'arrivée : voir validation plus haut.
    std::vector<int> cout(largeur * hauteur, -1);
    std::vector<sf::Vector2i> parent(largeur * hauteur);

    std::priority_queue<Noeud, std::vector<Noeud>, ComparateurNoeud> aExplorer;

    cout[indice(depart)] = 0;
    aExplorer.push({depart, 0, heuristique(depart, arrivee)});

    const sf::Vector2i directions[] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

    while (!aExplorer.empty()) {
        const Noeud actuel = aExplorer.top();
        aExplorer.pop();

        // Entrée périmée : une meilleure a déjà été trouvée pour cette
        // case depuis que celle-ci a été mise dans la file (la file ne
        // sait pas retirer ou mettre à jour une entrée existante, donc on
        // en pousse une nouvelle et on ignore les anciennes à la sortie).
        if (actuel.coutParcouru > cout[indice(actuel.position)]) {
            continue;
        }

        if (actuel.position == arrivee) {
            break;
        }

        for (const sf::Vector2i& direction : directions) {
            const sf::Vector2i voisin = actuel.position + direction;

            if (carte.getTile(voisin.x, voisin.y) == Tile::Wall) {
                continue;
            }

            const int nouveauCout = actuel.coutParcouru + 1;
            const int indiceVoisin = indice(voisin);

            if (cout[indiceVoisin] == -1 || nouveauCout < cout[indiceVoisin]) {
                cout[indiceVoisin] = nouveauCout;
                parent[indiceVoisin] = actuel.position;
                aExplorer.push({voisin, nouveauCout, nouveauCout + heuristique(voisin, arrivee)});
            }
        }
    }

    if (cout[indice(arrivee)] == -1) {
        return {};
    }

    std::vector<sf::Vector2i> chemin;
    sf::Vector2i courant = arrivee;
    while (courant != depart) {
        chemin.push_back(courant);
        courant = parent[indice(courant)];
    }
    chemin.push_back(depart);

    std::reverse(chemin.begin(), chemin.end());
    return chemin;
}

std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart,
                                        const std::vector<sf::Vector2f>& points, sf::Vector2f taille) {
    std::vector<sf::Vector2f> resultat;
    sf::Vector2f ancre = depart;
    std::size_t indice = 0;

    while (indice < points.size()) {
        const sf::FloatRect rectAncre(ancre - taille / 2.0f, taille);

        // Cherche, en partant de la fin, le point le plus lointain
        // atteignable en ligne droite depuis l'ancre. S'arrête au premier
        // trouvé (donc le plus lointain, puisqu'on part de la fin).
        std::size_t loin = indice;
        for (std::size_t i = points.size() - 1; i > indice; --i) {
            if (carte.trajetLibre(rectAncre, points[i] - ancre)) {
                loin = i;
                break;
            }
        }

        // Aucun point au-delà de `indice` n'est atteignable : on garde le
        // prochain point tel quel, la collision restant le filet de
        // sécurité dans Mobile::mettreAJour.
        resultat.push_back(points[loin]);
        ancre = points[loin];
        indice = loin + 1;
    }

    return resultat;
}

sf::Vector2f replierVersOrigine(const TileMap& carte, sf::Vector2f origine, sf::Vector2f cible,
                                sf::Vector2f taille) {
    const sf::FloatRect rectOrigine(origine - taille / 2.0f, taille);
    const sf::Vector2f segment = cible - origine;

    if (carte.trajetLibre(rectOrigine, segment)) {
        return cible;
    }

    // Dichotomie sur la fraction du segment : `libre` est une fraction dont
    // le trajet est libre (0 = l'origine elle-même), `bloque` une fraction
    // dont le trajet ne l'est pas. Le trajet vers un point plus proche est
    // un début du trajet vers un point plus lointain, donc si un point est
    // atteignable, tous les points plus proches le sont aussi.
    float libre = 0.0f;
    float bloque = 1.0f;
    for (int i = 0; i < s_iterationsRepli; ++i) {
        const float milieu = (libre + bloque) / 2.0f;
        if (carte.trajetLibre(rectOrigine, segment * milieu)) {
            libre = milieu;
        } else {
            bloque = milieu;
        }
    }

    return origine + segment * libre;
}
