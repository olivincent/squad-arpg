// ./tests/test_ChampDeVision.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <random>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "Aleatoire.hpp"
#include "ChampDeVision.hpp"
#include "GenerateurDonjon.hpp"
#include "Niveau.hpp"
#include "TileMap.hpp"

namespace {

constexpr float s_tailleCase = 40.0f;

using Champ = std::vector<sf::Vector2i>;

bool contient(const Champ& champ, int x, int y) {
    return std::ranges::find(champ, sf::Vector2i{x, y}) != champ.end();
}

// Carte entièrement dégagée de largeur × hauteur cases.
TileMap salleVide(int largeur, int hauteur) {
    return TileMap(std::vector<std::string>(static_cast<std::size_t>(hauteur), std::string(static_cast<std::size_t>(largeur), '.')), s_tailleCase);
}

// Pour toute paire de cases non bloquantes A et B à distance <= rayon : B est dans le
// champ de A si et seulement si A est dans celui de B. Renvoie le nombre de paires
// vérifiées, dont `cachees` sont des paires qui ne se voient pas (un test qui ne
// vérifierait que des paires visibles n'éprouverait rien).
struct BilanSymetrie {
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> asymetriques;
    int paires = 0;
    int cachees = 0;
};

BilanSymetrie verifierSymetrie(const TileMap& carte, int rayon) {
    const int largeur = carte.getLargeur();
    const int hauteur = carte.getHauteur();
    const std::size_t nombre = static_cast<std::size_t>(largeur * hauteur);

    // voit[a][b] : la case b est dans le champ de la case a (indice = y * largeur + x).
    std::vector<std::vector<bool>> voit(nombre, std::vector<bool>(nombre, false));
    for (int y = 0; y < hauteur; ++y) {
        for (int x = 0; x < largeur; ++x) {
            if (estBloquante(carte.getTile(x, y))) {
                continue;
            }
            for (const sf::Vector2i vue : champDeVision(carte, {x, y}, rayon)) {
                voit[static_cast<std::size_t>(y * largeur + x)][static_cast<std::size_t>(vue.y * largeur + vue.x)] = true;
            }
        }
    }

    BilanSymetrie bilan;
    for (int a = 0; a < largeur * hauteur; ++a) {
        const sf::Vector2i caseA{a % largeur, a / largeur};
        if (estBloquante(carte.getTile(caseA.x, caseA.y))) {
            continue;
        }
        for (int b = a + 1; b < largeur * hauteur; ++b) {
            const sf::Vector2i caseB{b % largeur, b / largeur};
            const int dx = caseB.x - caseA.x;
            const int dy = caseB.y - caseA.y;
            if (estBloquante(carte.getTile(caseB.x, caseB.y)) || dx * dx + dy * dy > rayon * rayon) {
                continue;
            }
            ++bilan.paires;
            const bool ab = voit[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)];
            const bool ba = voit[static_cast<std::size_t>(b)][static_cast<std::size_t>(a)];
            if (!ab) {
                ++bilan.cachees;
            }
            if (ab != ba) {
                bilan.asymetriques.emplace_back(caseA, caseB);
            }
        }
    }
    return bilan;
}

// Compare estVisibleDepuis à champDeVision et à lui-même (symétrie), pour toute case A non
// bloquante et toute case B de la carte à distance euclidienne <= rayon (B quelconque pour
// l'équivalence, non bloquante pour la symétrie). `cachees` compte les paires qui ne se
// voient pas : un test qui ne vérifierait que des paires visibles n'éprouverait rien.
struct BilanEquivalence {
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> divergences;
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> asymetriques;
    int paires = 0;
    int cachees = 0;
};

BilanEquivalence verifierEquivalence(const TileMap& carte, int rayon) {
    BilanEquivalence bilan;
    for (int ya = 0; ya < carte.getHauteur(); ++ya) {
        for (int xa = 0; xa < carte.getLargeur(); ++xa) {
            const sf::Vector2i caseA{xa, ya};
            if (estBloquante(carte.getTile(xa, ya))) {
                continue;
            }
            const Champ champ = champDeVision(carte, caseA, rayon);
            for (int yb = std::max(0, ya - rayon); yb <= std::min(carte.getHauteur() - 1, ya + rayon); ++yb) {
                for (int xb = std::max(0, xa - rayon); xb <= std::min(carte.getLargeur() - 1, xa + rayon); ++xb) {
                    const int dx = xb - xa;
                    const int dy = yb - ya;
                    if (dx * dx + dy * dy > rayon * rayon) {
                        continue;
                    }
                    const sf::Vector2i caseB{xb, yb};
                    const bool vue = estVisibleDepuis(carte, caseA, caseB);
                    ++bilan.paires;
                    if (!vue) {
                        ++bilan.cachees;
                    }
                    if (vue != contient(champ, xb, yb)) {
                        bilan.divergences.emplace_back(caseA, caseB);
                    }
                    if (!estBloquante(carte.getTile(xb, yb)) && vue != estVisibleDepuis(carte, caseB, caseA)) {
                        bilan.asymetriques.emplace_back(caseA, caseB);
                    }
                }
            }
        }
    }
    return bilan;
}

// Plan à piliers et à coins, bordé de murs (13 × 13).
const std::vector<std::string> s_planDePiliers = {
    "#############",
    "#...........#",
    "#..#....#...#",
    "#...........#",
    "#.....#.....#",
    "#...##......#",
    "#...........#",
    "#.#.....#.#.#",
    "#...........#",
    "#....##.....#",
    "#...........#",
    "#.......#...#",
    "#############",
};

// Niveau généré par la graine, ses portes fermées une sur deux (les autres ouvertes).
TileMap carteGeneree(Generateur::result_type graine) {
    Generateur generateur(graine);
    const Niveau niveau = genererNiveau(generateur, ParametresDonjon{});
    TileMap carte(niveau.plan, niveau.tailleCase);
    for (std::size_t i = 0; i < niveau.portes.size(); ++i) {
        for (const sf::Vector2i coordCase : niveau.portes[i].cases) {
            carte.placerPorte(coordCase);
            if (i % 2 == 0) {
                carte.ouvrirPorte(coordCase);
            }
        }
    }
    return carte;
}

}  // namespace

TEST_CASE("champDeVision d'une salle vide : les cases a distance <= rayon, pas au-dela") {
    const TileMap carte = salleVide(11, 11);
    const int rayon = 4;
    const Champ champ = champDeVision(carte, {5, 5}, rayon);

    for (int y = 0; y < 11; ++y) {
        for (int x = 0; x < 11; ++x) {
            const int dx = x - 5;
            const int dy = y - 5;
            INFO("case (" << x << ", " << y << ")");
            REQUIRE(contient(champ, x, y) == (dx * dx + dy * dy <= rayon * rayon));
        }
    }
}

TEST_CASE("champDeVision de rayon 0 ne renvoie que l'origine") {
    const TileMap carte = salleVide(5, 5);

    const Champ champ = champDeVision(carte, {2, 2}, 0);

    REQUIRE(champ.size() == 1);
    REQUIRE(contient(champ, 2, 2));
}

TEST_CASE("champDeVision revele un mur mais pas la case derriere lui") {
    std::vector<std::string> plan(5, std::string(9, '.'));
    plan[2][4] = '#';
    const TileMap carte(plan, s_tailleCase);

    const Champ champ = champDeVision(carte, {1, 2}, 7);

    REQUIRE(contient(champ, 3, 2));
    REQUIRE(contient(champ, 4, 2));
    REQUIRE_FALSE(contient(champ, 5, 2));
    REQUIRE_FALSE(contient(champ, 6, 2));
    REQUIRE_FALSE(contient(champ, 7, 2));
}

TEST_CASE("champDeVision : l'ombre d'un pilier cache l'arriere, pas les cotes") {
    std::vector<std::string> plan(9, std::string(9, '.'));
    plan[4][4] = '#';
    const TileMap carte(plan, s_tailleCase);

    const Champ champ = champDeVision(carte, {2, 4}, 6);

    REQUIRE(contient(champ, 4, 4));  // le pilier lui-même
    REQUIRE_FALSE(contient(champ, 5, 4));
    REQUIRE_FALSE(contient(champ, 6, 4));
    REQUIRE_FALSE(contient(champ, 7, 4));
    REQUIRE_FALSE(contient(champ, 8, 4));
    REQUIRE(contient(champ, 5, 3));  // sur les côtés de l'ombre
    REQUIRE(contient(champ, 5, 5));
    REQUIRE(contient(champ, 6, 2));
    REQUIRE(contient(champ, 6, 6));
}

TEST_CASE("champDeVision : une porte fermee est revelee, la salle derriere est cachee") {
    std::vector<std::string> plan = {
        "#########",
        "#...#...#",
        "#.......#",
        "#...#...#",
        "#########",
    };
    TileMap carte(plan, s_tailleCase);
    carte.placerPorte({4, 2});

    const Champ champ = champDeVision(carte, {2, 2}, 6);

    REQUIRE(contient(champ, 4, 2));  // la porte
    REQUIRE_FALSE(contient(champ, 5, 2));
    REQUIRE_FALSE(contient(champ, 6, 2));
    REQUIRE_FALSE(contient(champ, 7, 1));
    REQUIRE(contient(champ, 3, 2));  // la première salle reste visible
}

TEST_CASE("champDeVision : une porte ouverte laisse voir la salle derriere") {
    std::vector<std::string> plan = {
        "#########",
        "#...#...#",
        "#.......#",
        "#...#...#",
        "#########",
    };
    TileMap carte(plan, s_tailleCase);
    carte.placerPorte({4, 2});
    carte.ouvrirPorte({4, 2});

    const Champ champ = champDeVision(carte, {2, 2}, 6);

    REQUIRE(contient(champ, 4, 2));
    REQUIRE(contient(champ, 5, 2));
    REQUIRE(contient(champ, 6, 2));
    REQUIRE(contient(champ, 7, 2));
}

TEST_CASE("champDeVision depuis un bord ou un coin de la carte ne renvoie rien hors carte") {
    const TileMap carte = salleVide(5, 5);

    for (const sf::Vector2i origine : {sf::Vector2i{0, 0}, sf::Vector2i{4, 4}, sf::Vector2i{0, 2}, sf::Vector2i{2, 4}}) {
        const Champ champ = champDeVision(carte, origine, 20);  // rayon plus grand que la carte

        REQUIRE(contient(champ, origine.x, origine.y));
        for (const sf::Vector2i coordCase : champ) {
            REQUIRE(coordCase.x >= 0);
            REQUIRE(coordCase.y >= 0);
            REQUIRE(coordCase.x < 5);
            REQUIRE(coordCase.y < 5);
        }
        // Salle vide : toute la carte est dans le champ.
        for (int y = 0; y < 5; ++y) {
            for (int x = 0; x < 5; ++x) {
                REQUIRE(contient(champ, x, y));
            }
        }
    }
}

TEST_CASE("champDeVision est symetrique sur un plan a piliers et a coins") {
    const TileMap carte(s_planDePiliers, s_tailleCase);

    const BilanSymetrie bilan = verifierSymetrie(carte, 6);

    REQUIRE(bilan.paires > 0);
    REQUIRE(bilan.cachees > 0);  // le plan cache bien des paires : le test éprouve quelque chose
    if (!bilan.asymetriques.empty()) {
        INFO("première paire asymétrique : (" << bilan.asymetriques.front().first.x << ", " << bilan.asymetriques.front().first.y << ") et ("
                                              << bilan.asymetriques.front().second.x << ", " << bilan.asymetriques.front().second.y << ")");
    }
    REQUIRE(bilan.asymetriques.empty());
}

TEST_CASE("champDeVision est symetrique sur des cartes generees") {
    for (const Generateur::result_type graine : {Generateur::result_type{1}, Generateur::result_type{2}, Generateur::result_type{3}}) {
        const TileMap carte = carteGeneree(graine);

        const BilanSymetrie bilan = verifierSymetrie(carte, 8);

        INFO("graine " << graine);
        REQUIRE(bilan.paires > 0);
        REQUIRE(bilan.cachees > 0);
        REQUIRE(bilan.asymetriques.empty());
    }
}

TEST_CASE("estVisibleDepuis : une case se voit elle-meme, meme dans un mur") {
    const TileMap carte(s_planDePiliers, s_tailleCase);

    REQUIRE(estVisibleDepuis(carte, {1, 1}, {1, 1}));
    REQUIRE(estVisibleDepuis(carte, {3, 2}, {3, 2}));  // un pilier
}

TEST_CASE("estVisibleDepuis : une cible sur un axe ou une diagonale, en terrain degage") {
    const TileMap carte = salleVide(11, 11);

    for (const sf::Vector2i cible : {sf::Vector2i{5, 1}, sf::Vector2i{9, 5}, sf::Vector2i{5, 9}, sf::Vector2i{1, 5}, sf::Vector2i{8, 8},
                                      sf::Vector2i{2, 2}, sf::Vector2i{8, 2}, sf::Vector2i{2, 8}, sf::Vector2i{6, 5}}) {
        INFO("cible (" << cible.x << ", " << cible.y << ")");
        REQUIRE(estVisibleDepuis(carte, {5, 5}, cible));
    }
}

TEST_CASE("estVisibleDepuis : un mur cache la cible derriere lui, sur un axe comme sur une diagonale") {
    std::vector<std::string> plan(11, std::string(11, '.'));
    plan[3][5] = '#';  // au nord de l'origine (5, 5)
    plan[6][6] = '#';  // au sud-est
    const TileMap carte(plan, s_tailleCase);

    REQUIRE_FALSE(estVisibleDepuis(carte, {5, 5}, {5, 1}));  // axe, derrière le mur
    REQUIRE(estVisibleDepuis(carte, {5, 5}, {5, 3}));        // le mur lui-même est vu
    REQUIRE_FALSE(estVisibleDepuis(carte, {5, 5}, {8, 8}));  // diagonale, dans les deux quadrants
    REQUIRE(estVisibleDepuis(carte, {5, 5}, {5, 9}));        // l'autre axe est libre
}

TEST_CASE("estVisibleDepuis : une porte fermee bloque, une porte ouverte laisse passer") {
    const std::vector<std::string> plan = {
        "#########",
        "#...#...#",
        "#.......#",
        "#...#...#",
        "#########",
    };
    TileMap carte(plan, s_tailleCase);
    carte.placerPorte({4, 2});

    REQUIRE(estVisibleDepuis(carte, {2, 2}, {4, 2}));  // la porte elle-même
    REQUIRE_FALSE(estVisibleDepuis(carte, {2, 2}, {6, 2}));
    REQUIRE_FALSE(estVisibleDepuis(carte, {6, 2}, {2, 2}));

    carte.ouvrirPorte({4, 2});

    REQUIRE(estVisibleDepuis(carte, {2, 2}, {6, 2}));
    REQUIRE(estVisibleDepuis(carte, {6, 2}, {2, 2}));
}

TEST_CASE("estVisibleDepuis : une cible hors de la carte n'est jamais visible") {
    const TileMap carte = salleVide(5, 5);

    REQUIRE_FALSE(estVisibleDepuis(carte, {2, 2}, {7, 2}));
    REQUIRE_FALSE(estVisibleDepuis(carte, {2, 2}, {-1, -1}));
}

TEST_CASE("estVisibleDepuis equivaut a champDeVision et est symetrique sur un plan a piliers") {
    const TileMap carte(s_planDePiliers, s_tailleCase);

    const BilanEquivalence bilan = verifierEquivalence(carte, 10);

    REQUIRE(bilan.paires > 0);
    REQUIRE(bilan.cachees > 0);
    if (!bilan.divergences.empty()) {
        INFO("première divergence : (" << bilan.divergences.front().first.x << ", " << bilan.divergences.front().first.y << ") vers ("
                                       << bilan.divergences.front().second.x << ", " << bilan.divergences.front().second.y << ")");
    }
    REQUIRE(bilan.divergences.empty());
    REQUIRE(bilan.asymetriques.empty());
}

TEST_CASE("estVisibleDepuis equivaut a champDeVision et est symetrique sur des cartes generees") {
    for (const Generateur::result_type graine : {Generateur::result_type{1}, Generateur::result_type{2}, Generateur::result_type{3}}) {
        const TileMap carte = carteGeneree(graine);

        const BilanEquivalence bilan = verifierEquivalence(carte, 10);

        INFO("graine " << graine);
        REQUIRE(bilan.paires > 0);
        REQUIRE(bilan.cachees > 0);
        REQUIRE(bilan.divergences.empty());
        REQUIRE(bilan.asymetriques.empty());
    }
}

TEST_CASE("ligneDeVue libre : faux si un des points est dans un mur, une porte fermee ou hors de la carte") {
    std::vector<std::string> plan = {
        ".....",
        "..#..",
        ".....",
    };
    TileMap carte(plan, s_tailleCase);
    carte.placerPorte({0, 0});

    const sf::Vector2f sol = carte.centreDeCase({4, 2});
    REQUIRE_FALSE(ligneDeVue(carte, sol, carte.centreDeCase({2, 1})));  // arrivée dans un mur
    REQUIRE_FALSE(ligneDeVue(carte, carte.centreDeCase({2, 1}), sol));  // départ dans un mur
    REQUIRE_FALSE(ligneDeVue(carte, sol, carte.centreDeCase({0, 0})));  // porte fermée
    REQUIRE_FALSE(ligneDeVue(carte, carte.centreDeCase({0, 0}), sol));
    REQUIRE_FALSE(ligneDeVue(carte, sol, {-10.0f, 20.0f}));  // hors de la carte
    REQUIRE_FALSE(ligneDeVue(carte, {1000.0f, 20.0f}, sol));
    REQUIRE(ligneDeVue(carte, sol, carte.centreDeCase({4, 0})));  // contrôle : le sol se voit
}

TEST_CASE("ligneDeVue libre : deux points dans la meme case se voient toujours") {
    const TileMap carte(s_planDePiliers, s_tailleCase);

    REQUIRE(ligneDeVue(carte, {41.0f, 41.0f}, {79.0f, 79.0f}));
    REQUIRE(ligneDeVue(carte, {50.0f, 50.0f}, {50.0f, 50.0f}));
}

TEST_CASE("ligneDeVue libre : terrain degage") {
    const TileMap carte = salleVide(5, 3);

    REQUIRE(ligneDeVue(carte, {10.0f, 10.0f}, {190.0f, 110.0f}));
}

TEST_CASE("ligneDeVue libre : un mur entre deux salles bloque la vue") {
    const std::vector<std::string> plan = {
        "#########",
        "#...#...#",
        "#...#...#",
        "#...#...#",
        "#########",
    };
    TileMap carte(plan, s_tailleCase);
    const sf::Vector2f gauche = carte.centreDeCase({2, 2});
    const sf::Vector2f droite = carte.centreDeCase({6, 2});

    REQUIRE_FALSE(ligneDeVue(carte, gauche, droite));
    REQUIRE_FALSE(ligneDeVue(carte, droite, gauche));
}

TEST_CASE("ligneDeVue libre : une porte fermee bloque, ouverte elle laisse voir") {
    const std::vector<std::string> plan = {
        "#########",
        "#...#...#",
        "#.......#",
        "#...#...#",
        "#########",
    };
    TileMap carte(plan, s_tailleCase);
    carte.placerPorte({4, 2});
    const sf::Vector2f gauche = carte.centreDeCase({2, 2});
    const sf::Vector2f droite = carte.centreDeCase({6, 2});

    REQUIRE_FALSE(ligneDeVue(carte, gauche, droite));

    carte.ouvrirPorte({4, 2});

    REQUIRE(ligneDeVue(carte, gauche, droite));
    REQUIRE(ligneDeVue(carte, droite, gauche));
}

TEST_CASE("ligneDeVue libre est symetrique pour des points pris au hasard sur une carte generee") {
    const TileMap carte = carteGeneree(1);
    std::vector<sf::Vector2i> sols;
    for (int y = 0; y < carte.getHauteur(); ++y) {
        for (int x = 0; x < carte.getLargeur(); ++x) {
            if (!estBloquante(carte.getTile(x, y))) {
                sols.push_back({x, y});
            }
        }
    }
    Generateur generateur(42);
    std::uniform_int_distribution<std::size_t> choixDeCase(0, sols.size() - 1);
    std::uniform_real_distribution<float> dansLaCase(0.0f, s_tailleCase);
    const auto pointAuHasard = [&] {
        const sf::Vector2i coordCase = sols[choixDeCase(generateur)];
        return sf::Vector2f{static_cast<float>(coordCase.x) * s_tailleCase + dansLaCase(generateur),
                            static_cast<float>(coordCase.y) * s_tailleCase + dansLaCase(generateur)};
    };

    int vues = 0;
    int cachees = 0;
    for (int i = 0; i < 2000; ++i) {
        const sf::Vector2f a = pointAuHasard();
        const sf::Vector2f b = pointAuHasard();
        const bool ab = ligneDeVue(carte, a, b);
        REQUIRE(ab == ligneDeVue(carte, b, a));
        (ab ? vues : cachees) += 1;
    }
    REQUIRE(vues > 0);
    REQUIRE(cachees > 0);
}

TEST_CASE("estVisibleDepuis : deux cases en diagonale entre deux murs qui se touchent se voient") {
    // Comportement de l'algorithme de Ford, conservé tel quel à l'étape 73 : la lumière passe
    // par le coin commun des deux murs (l'ancien segment centre à centre était bloqué).
    const std::vector<std::string> plan = {
        "....",
        "..#.",
        ".#..",
        "....",
    };
    const TileMap carte(plan, s_tailleCase);

    REQUIRE(estVisibleDepuis(carte, {1, 1}, {2, 2}));
    REQUIRE(estVisibleDepuis(carte, {2, 2}, {1, 1}));
    REQUIRE(ligneDeVue(carte, carte.centreDeCase({1, 1}), carte.centreDeCase({2, 2})));
}
