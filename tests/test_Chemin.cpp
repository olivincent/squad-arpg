// ./tests/test_Chemin.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdlib>
#include <string>
#include <vector>

#include "Chemin.hpp"
#include "TileMap.hpp"

namespace {

// Vérifie les propriétés qu'un chemin valide doit respecter, sans exiger
// une liste exacte de cases (plusieurs plus courts chemins peuvent
// exister) : commence au départ, finit à l'arrivée, chaque case est du
// sol, deux cases consécutives sont voisines (distance de Manhattan de
// 1), et la longueur correspond au plus court chemin attendu.
void verifierCheminValide(const std::vector<sf::Vector2i>& chemin, const TileMap& carte,
                                  sf::Vector2i depart, sf::Vector2i arrivee, std::size_t longueurAttendue) {
    REQUIRE(chemin.size() == longueurAttendue);
    REQUIRE(chemin.front() == depart);
    REQUIRE(chemin.back() == arrivee);

    for (std::size_t i = 0; i < chemin.size(); ++i) {
        REQUIRE(carte.getTile(chemin[i].x, chemin[i].y) == Tile::Floor);
        if (i > 0) {
            const sf::Vector2i ecart = chemin[i] - chemin[i - 1];
            REQUIRE(std::abs(ecart.x) + std::abs(ecart.y) == 1);
        }
    }
}

}  // namespace

TEST_CASE("Le chemin entre une case de depart et d'arrivee identiques est cette seule case") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    const std::vector<sf::Vector2i> chemin = trouverChemin(carte, {1, 1}, {1, 1});

    REQUIRE(chemin.size() == 1);
    REQUIRE(chemin[0] == sf::Vector2i(1, 1));
}

TEST_CASE("Le chemin est vide si le depart est un mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {1, 1}, {0, 0}).empty());
}

TEST_CASE("Le chemin est vide si l'arrivee est un mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {1, 1}).empty());
}

TEST_CASE("Le chemin est vide si le depart est hors carte") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {-1, 0}, {0, 0}).empty());
}

TEST_CASE("Le chemin est vide si l'arrivee est hors carte") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {3, 0}).empty());
}

TEST_CASE("Le chemin est vide quand depart et arrivee sont la meme case muree") {
    // Ce cas doit l'emporter sur le raccourci « départ == arrivée » : être
    // sur la même case ne suffit pas si cette case est un mur.
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {1, 1}, {1, 1}).empty());
}

TEST_CASE("Le chemin en ligne droite sans obstacle a la longueur de Manhattan") {
    const std::vector<std::string> plan = {
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::Vector2i depart{0, 0};
    const sf::Vector2i arrivee{4, 0};
    const std::vector<sf::Vector2i> chemin = trouverChemin(carte, depart, arrivee);

    verifierCheminValide(chemin, carte, depart, arrivee, 5);
}

TEST_CASE("Le chemin contourne un mur place entre le depart et l'arrivee") {
    // Mur sur toute la largeur de la rangée du milieu, sauf les bords :
    // il faut remonter d'une rangée pour passer, puis redescendre.
    const std::vector<std::string> plan = {
        ".....",
        ".###.",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::Vector2i depart{0, 1};
    const sf::Vector2i arrivee{4, 1};
    const std::vector<sf::Vector2i> chemin = trouverChemin(carte, depart, arrivee);

    // 4 pas horizontaux + 1 pas pour sortir de la rangée + 1 pas pour y
    // revenir = 6 pas, donc 7 cases.
    verifierCheminValide(chemin, carte, depart, arrivee, 7);
}

TEST_CASE("Le chemin est vide si l'arrivee est enfermee par des murs") {
    // La case (2, 2) est entourée de murs sur ses 4 côtés : aucun chemin
    // ne peut l'atteindre depuis l'extérieur.
    const std::vector<std::string> plan = {
        ".....",
        ".###.",
        ".#.#.",
        ".###.",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {2, 2}).empty());
}

TEST_CASE("lisserChemin renvoie une liste vide pour une liste vide") {
    const std::vector<std::string> plan = {
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const std::vector<sf::Vector2f> resultat = lisserChemin(carte, {20.0f, 20.0f}, {}, {20.0f, 20.0f});

    REQUIRE(resultat.empty());
}

TEST_CASE("lisserChemin avec un seul point le renvoie tel quel") {
    const std::vector<std::string> plan = {
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const std::vector<sf::Vector2f> points = {{150.0f, 20.0f}};
    const std::vector<sf::Vector2f> resultat = lisserChemin(carte, {20.0f, 20.0f}, points, {20.0f, 20.0f});

    REQUIRE(resultat.size() == 1);
    REQUIRE(resultat[0] == points[0]);
}

TEST_CASE("En terrain degage, lisserChemin ne garde que le dernier point") {
    // Carte 8x8 entièrement en sol : la ligne droite du départ au dernier
    // point ne peut chevaucher aucun mur, donc tous les points
    // intermédiaires sont superflus.
    const std::vector<std::string> plan = {
        "........",
        "........",
        "........",
        "........",
        "........",
        "........",
        "........",
        "........",
    };
    TileMap carte(plan, 40.0f);

    const std::vector<sf::Vector2f> points = {{60.0f, 20.0f}, {140.0f, 60.0f}, {220.0f, 100.0f}, {300.0f, 140.0f}};
    const std::vector<sf::Vector2f> resultat = lisserChemin(carte, {20.0f, 20.0f}, points, {20.0f, 20.0f});

    REQUIRE(resultat.size() == 1);
    REQUIRE(resultat[0] == points.back());
}

TEST_CASE("lisserChemin reduit un chemin en escalier qui contourne un mur") {
    // Mur sur la rangée 1, colonnes 2 à 4 ; ouverture par la rangée 0 (et
    // la rangée 2). Départ en case (0, 1), arrivée en case (6, 1) :
    // directement à droite du départ, mais le mur bloque le trajet
    // direct. Le chemin du A* (en escalier par la rangée 0) est converti
    // en centres de case, comme le ferait Escouade::allerVers, puis lissé.
    const std::vector<std::string> plan = {
        ".......",
        "..###..",
        ".......",
    };
    TileMap carte(plan, 40.0f);

    const sf::Vector2i caseDepart{0, 1};
    const sf::Vector2i caseArrivee{6, 1};
    const std::vector<sf::Vector2i> chemin = trouverChemin(carte, caseDepart, caseArrivee);
    REQUIRE(chemin.size() == 9); // 6 pas horizontaux + 2 pas pour sortir et revenir sur la rangée 1

    std::vector<sf::Vector2f> points;
    for (std::size_t i = 1; i < chemin.size(); ++i) {
        points.push_back(carte.centreDeCase(chemin[i]));
    }

    const sf::Vector2f depart = carte.centreDeCase(caseDepart);
    const sf::Vector2f taille{20.0f, 20.0f};
    const std::vector<sf::Vector2f> resultat = lisserChemin(carte, depart, points, taille);

    REQUIRE(resultat.size() < points.size());
    REQUIRE(resultat.back() == points.back());

    sf::Vector2f ancre = depart;
    for (const sf::Vector2f& point : resultat) {
        const sf::FloatRect rect(ancre - taille / 2.0f, taille);
        REQUIRE(carte.trajetLibre(rect, point - ancre));
        ancre = point;
    }
}

namespace {
// Salle de 7 x 6 cases de 40 px, avec un pilier en case (3, 3).
TileMap salleAvecPilier() {
    return TileMap({"#######",
                    "#.....#",
                    "#.....#",
                    "#..#..#",
                    "#.....#",
                    "#######"},
                   40.0f);
}
} // namespace

TEST_CASE("replierVersOrigine renvoie la cible telle quelle si le trajet est libre") {
    const TileMap carte = salleAvecPilier();
    const sf::Vector2f origine{60.0f, 60.0f};
    const sf::Vector2f cible{180.0f, 60.0f};

    REQUIRE(replierVersOrigine(carte, origine, cible, {20.0f, 20.0f}) == cible);
}

TEST_CASE("replierVersOrigine ramene une cible situee dans un mur sur un point libre du segment") {
    const TileMap carte = salleAvecPilier();
    const sf::Vector2f origine{60.0f, 60.0f};
    const sf::Vector2f cible{260.0f, 60.0f}; // dans le mur de la case (6, 1)
    const sf::Vector2f taille{20.0f, 20.0f};

    const sf::Vector2f resultat = replierVersOrigine(carte, origine, cible, taille);

    REQUIRE(resultat.y == 60.0f);
    REQUIRE(resultat.x >= origine.x);
    REQUIRE(resultat.x < cible.x);
    REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect(resultat - taille / 2.0f, taille)));
}

TEST_CASE("replierVersOrigine ramene une cible cachee derriere un coin de pilier") {
    const TileMap carte = salleAvecPilier();
    const sf::Vector2f origine{60.0f, 60.0f};
    const sf::Vector2f cible{220.0f, 220.0f}; // case (5, 5), derrière le pilier en diagonale
    const sf::Vector2f taille{20.0f, 20.0f};

    const sf::Vector2f resultat = replierVersOrigine(carte, origine, cible, taille);

    REQUIRE(resultat != cible);
    // Le résultat est sur le segment origine-cible (colinéaire, entre les deux).
    const sf::Vector2f segment = cible - origine;
    const sf::Vector2f versResultat = resultat - origine;
    REQUIRE(segment.cross(versResultat) == 0.0f);
    REQUIRE(versResultat.length() < segment.length());
    // Le trajet de l'origine au résultat est libre.
    const sf::FloatRect rectOrigine(origine - taille / 2.0f, taille);
    REQUIRE(carte.trajetLibre(rectOrigine, versResultat));
}

TEST_CASE("replierVersOrigine avec origine et cible identiques renvoie l'origine") {
    const TileMap carte = salleAvecPilier();
    const sf::Vector2f origine{60.0f, 60.0f};

    REQUIRE(replierVersOrigine(carte, origine, origine, {20.0f, 20.0f}) == origine);
}
