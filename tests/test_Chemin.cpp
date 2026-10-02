// ./tests/test_Chemin.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdlib>
#include <string>
#include <vector>

#include "Chemin.hpp"
#include "TileMap.hpp"

// Vérifie les propriétés qu'un chemin valide doit respecter, sans exiger
// une liste exacte de cases (plusieurs plus courts chemins peuvent
// exister) : commence au départ, finit à l'arrivée, chaque case est du
// sol, deux cases consécutives sont voisines (distance de Manhattan de
// 1), et la longueur correspond au plus court chemin attendu.
static void verifierCheminValide(const std::vector<sf::Vector2i>& chemin, const TileMap& carte,
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

TEST_CASE("Le chemin entre une case de départ et d'arrivée identiques est cette seule case") {
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

TEST_CASE("Le chemin est vide si le départ est un mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {1, 1}, {0, 0}).empty());
}

TEST_CASE("Le chemin est vide si l'arrivée est un mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {1, 1}).empty());
}

TEST_CASE("Le chemin est vide si le départ est hors carte") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {-1, 0}, {0, 0}).empty());
}

TEST_CASE("Le chemin est vide si l'arrivée est hors carte") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {3, 0}).empty());
}

TEST_CASE("Le chemin est vide quand départ et arrivée sont la même case murée") {
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

TEST_CASE("Le chemin contourne un mur placé entre le départ et l'arrivée") {
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

TEST_CASE("Le chemin est vide si l'arrivée est enfermée par des murs") {
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

TEST_CASE("En terrain dégagé, lisserChemin ne garde que le dernier point") {
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

TEST_CASE("lisserChemin réduit un chemin en escalier qui contourne un mur") {
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
