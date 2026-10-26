// ./tests/monde/test_TileMap.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "monde/Chemin.hpp"
#include "monde/TileMap.hpp"

TEST_CASE("getTile distingue une case de sol d'une case de mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.getTile(0, 0) == Tile::Floor);
    REQUIRE(carte.getTile(1, 1) == Tile::Wall);
}

TEST_CASE("Les dimensions de la carte sont deduites du plan") {
    // Bord en sol (pas en mur) : sinon une case de bord et une case hors
    // carte renverraient toutes les deux Wall, et ce test ne prouverait
    // rien sur les dimensions réelles de la carte.
    const std::vector<std::string> plan = {
        "....",
        "....",
        "....",
        "....",
    };
    TileMap carte(plan, 40.0f);

    // Dernière case valide : (largeur - 1, hauteur - 1) = (3, 3)
    REQUIRE(carte.getTile(3, 3) == Tile::Floor);
    // Juste au-delà en X et en Y : hors carte, donc mur
    REQUIRE(carte.getTile(4, 3) == Tile::Wall);
    REQUIRE(carte.getTile(3, 4) == Tile::Wall);
}

TEST_CASE("getLargeur et getHauteur renvoient les dimensions du plan") {
    // Plan non carré : si largeur et hauteur étaient inversées par erreur
    // dans l'implémentation, ce test échouerait, contrairement à un plan
    // carré où l'erreur serait invisible.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.getLargeur() == 5);
    REQUIRE(carte.getHauteur() == 3);
}

TEST_CASE("caseContenant renvoie la case d'un point au milieu de celle-ci") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    // Milieu de la case (1, 1) : x = 1*40 + 20 = 60, y pareil.
    REQUIRE(carte.caseContenant({60.0f, 60.0f}) == sf::Vector2i(1, 1));
}

TEST_CASE("caseContenant renvoie la case de droite sur la limite exacte entre deux cases") {
    // x = 40 est à la fois le bord droit de la case 0 et le bord gauche de
    // la case 1 : comme chevaucheUnMur (std::floor), la limite appartient
    // à la case de droite.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.caseContenant({40.0f, 0.0f}) == sf::Vector2i(1, 0));
}

TEST_CASE("caseContenant renvoie une case negative pour un point avant la carte") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.caseContenant({-10.0f, -10.0f}) == sf::Vector2i(-1, -1));
}

TEST_CASE("centreDeCase renvoie le centre de la case (0, 0)") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.centreDeCase({0, 0}) == sf::Vector2f(20.0f, 20.0f));
}

TEST_CASE("centreDeCase renvoie le centre d'une autre case") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.centreDeCase({2, 1}) == sf::Vector2f(100.0f, 60.0f));
}

TEST_CASE("caseContenant et centreDeCase sont des conversions inverses") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::Vector2i coordCase{3, 2};
    REQUIRE(carte.caseContenant(carte.centreDeCase(coordCase)) == coordCase);
}

TEST_CASE("Une coordonnee negative est hors carte, donc un mur") {
    const std::vector<std::string> plan = {
        "....",
        "....",
        "....",
        "....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.getTile(-1, -1) == Tile::Wall);
}

TEST_CASE("Un plan vide leve std::out_of_range") {
    const std::vector<std::string> plan = {};

    REQUIRE_THROWS_AS(TileMap(plan, 40.0f), std::out_of_range);
}

TEST_CASE("Des lignes de longueurs differentes levent std::invalid_argument") {
    const std::vector<std::string> plan = {
        "....",
        "...",
        "....",
    };

    REQUIRE_THROWS_AS(TileMap(plan, 40.0f), std::invalid_argument);
}

TEST_CASE("chevaucheUnMur renvoie faux pour un rectangle entierement sur du sol") {
    // Mur unique en (2, 2), donc en pixels x dans [80, 120), y dans [80, 120)
    // pour une taille de case de 40.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {40.0f, 40.0f});
    REQUIRE_FALSE(carte.chevaucheUnMur(rect));
}

TEST_CASE("chevaucheUnMur renvoie vrai pour un rectangle qui chevauche un mur") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({90.0f, 90.0f}, {20.0f, 20.0f});
    REQUIRE(carte.chevaucheUnMur(rect));
}

TEST_CASE("Un rectangle qui touche exactement le bord d'un mur ne le chevauche pas") {
    // Bord droit du rectangle en x = 80, exactement sur le bord gauche du
    // mur (qui commence à x = 80) : convention ceil - 1, un contact sans
    // chevauchement ne doit pas compter comme une collision.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({40.0f, 80.0f}, {40.0f, 40.0f});
    REQUIRE_FALSE(carte.chevaucheUnMur(rect));
}

TEST_CASE("Le meme rectangle decale de 1 px vers le mur le chevauche") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({41.0f, 80.0f}, {40.0f, 40.0f});
    REQUIRE(carte.chevaucheUnMur(rect));
}

TEST_CASE("Un rectangle hors carte en coordonnees negatives chevauche toujours un mur") {
    // Bord en sol : seule la sortie de carte doit déclencher la collision,
    // pas un mur du plan.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({-10.0f, -10.0f}, {20.0f, 20.0f});
    REQUIRE(carte.chevaucheUnMur(rect));
}

TEST_CASE("trajetLibre renvoie vrai pour un trajet horizontal degage") {
    // Mur unique en (2, 2) : x dans [80, 120), y dans [80, 120). Le
    // rectangle reste sur la rangée y: [0, 20), loin de la rangée du mur.
    // Déplacement de 160 px : arrivée x: [160, 180), à l'intérieur de la
    // carte (5 cases de 40 px = 200 px de large).
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    REQUIRE(carte.trajetLibre(rect, {160.0f, 0.0f}));
}

TEST_CASE("trajetLibre renvoie faux pour un trajet horizontal qui traverse un mur") {
    // Même mur ; rectangle sur y: [90, 110), entièrement dans la bande du
    // mur (y: [80, 120)). Le trajet horizontal x: [0, 180) traverse donc
    // la colonne du mur (x: [80, 120)), sans sortir de la carte.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 90.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {160.0f, 0.0f}));
}

TEST_CASE("Un trajet vertical qui touche exactement le mur est libre") {
    // Rectangle en x: [90, 110), dans la colonne du mur (x: [80, 120)).
    // Départ y: [40, 60), déplacement vertical de 20 px : arrivée
    // y: [60, 80), donc enveloppe y: [40, 80) — touche le bord du mur
    // (y = 80) sans le chevaucher, même convention que chevaucheUnMur.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({90.0f, 40.0f}, {20.0f, 20.0f});
    REQUIRE(carte.trajetLibre(rect, {0.0f, 20.0f}));
}

TEST_CASE("Le meme trajet vertical decale de 1 px vers le mur n'est plus libre") {
    // Déplacement de 21 px au lieu de 20 : l'enveloppe devient
    // y: [40, 81), qui chevauche le mur (y: [80, 120)) sur 1 px.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({90.0f, 40.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {0.0f, 21.0f}));
}

TEST_CASE("Un deplacement nul est libre si le rectangle n'est pas deja dans un mur") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    REQUIRE(carte.trajetLibre(rect, {0.0f, 0.0f}));
}

TEST_CASE("trajetLibre renvoie faux pour un trajet horizontal qui sort de la carte") {
    // Carte de 5 cases (0 à 4) de 40 px : 200 px de large. Rectangle sur
    // la dernière case (x: [180, 200)), déplacement de 40 px vers la
    // droite : l'enveloppe x: [180, 240) dépasse la carte, et hors carte
    // = mur.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({180.0f, 180.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {40.0f, 0.0f}));
}

TEST_CASE("trajetLibre renvoie vrai pour un trajet oblique degage") {
    // Mur en (2, 2) : x et y dans [80, 120). Le rectangle balaie les
    // colonnes 0 et 1 (x : [0, 60)), loin de la colonne 2 du mur.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    REQUIRE(carte.trajetLibre(rect, {40.0f, 40.0f}));
}

TEST_CASE("trajetLibre renvoie faux pour un mur en plein milieu, dx et dy positifs") {
    // Départ (60, 60), arrivée (100, 100) : à mi-trajet (t = 0.5), le
    // rectangle est en (80, 80), exactement dans la case du mur.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({60.0f, 60.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {40.0f, 40.0f}));
}

TEST_CASE("trajetLibre renvoie faux pour un mur en plein milieu, dx positif et dy negatif") {
    // Même trajet que le précédent, reflété verticalement par rapport au
    // centre du mur (y = 100) : départ (60, 140), arrivée (100, 100).
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({60.0f, 140.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {40.0f, -40.0f}));
}

TEST_CASE("trajetLibre renvoie faux pour un mur en plein milieu, dx negatif et dy positif") {
    // Reflété horizontalement par rapport au centre du mur (x = 100) :
    // départ (140, 60), arrivée (100, 100).
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({140.0f, 60.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {-40.0f, 40.0f}));
}

TEST_CASE("trajetLibre renvoie faux pour un mur en plein milieu, dx et dy negatifs") {
    // Reflété par rapport au centre du mur sur les deux axes : départ
    // (140, 140), arrivée (100, 100).
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({140.0f, 140.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {-40.0f, -40.0f}));
}

TEST_CASE("Un coin du rectangle accroche le mur meme si la ligne des centres passe a cote") {
    // Départ (90, 40), arrivée (150, 100). Le centre du rectangle va de
    // (100, 50) à (160, 110) : pour que le centre soit dans la case du
    // mur (x et y dans [80, 120)), il faudrait x < 120 (t < 1/3) ET
    // y > 80 (t > 1/2), deux conditions incompatibles — le centre ne
    // traverse donc jamais le mur. Pourtant, tant que le rectangle
    // chevauche la colonne du mur (t de 0 à 0,5), son bord bas descend
    // jusqu'à y = 90, soit 10 px à l'intérieur de la case du mur : son
    // coin bas-droit accroche le coin du mur que son centre a évité.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({90.0f, 40.0f}, {20.0f, 20.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {60.0f, 60.0f}));
}

TEST_CASE("Un trajet voisin decale de la meme distance passe juste a cote du mur") {
    // Même direction (60, 60) que le test précédent, départ décalé de
    // (15, -5) : assez pour que le rectangle ne touche plus jamais la
    // case du mur à aucun instant du trajet.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({105.0f, 35.0f}, {20.0f, 20.0f});
    REQUIRE(carte.trajetLibre(rect, {60.0f, 60.0f}));
}

TEST_CASE("trajetLibre renvoie faux quand l'arrivee est dans un mur") {
    // Départ (40, 40), loin du mur ; arrivée (110, 110) : le rectangle y
    // couvre (110,120)x(110,120), entièrement dans la case du mur
    // (x et y dans [80, 120)).
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({40.0f, 40.0f}, {10.0f, 10.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {70.0f, 70.0f}));
}

TEST_CASE("trajetLibre renvoie faux quand le depart est dans un mur") {
    // Départ (85, 85) : déjà entièrement dans la case du mur.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({85.0f, 85.0f}, {10.0f, 10.0f});
    REQUIRE_FALSE(carte.trajetLibre(rect, {40.0f, 40.0f}));
}

TEST_CASE("placerPorte : une porte fermee est bloquante, une porte ouverte non") {
    TileMap carte({"...", "...", "..."}, 40.0f);
    carte.placerPorte({1, 1});
    REQUIRE(carte.getTile(1, 1) == Tile::PorteFermee);
    REQUIRE(estBloquante(Tile::PorteFermee));
    carte.ouvrirPorte({1, 1});
    REQUIRE(carte.getTile(1, 1) == Tile::PorteOuverte);
    REQUIRE_FALSE(estBloquante(Tile::PorteOuverte));
    REQUIRE_FALSE(estBloquante(Tile::Floor));
    REQUIRE(estBloquante(Tile::Wall));
}

TEST_CASE("porte fermee : bloque chevaucheUnMur et trajetLibre") {
    TileMap carte({".....", ".....", "....."}, 40.0f);
    carte.placerPorte({2, 1});
    const sf::FloatRect surLaPorte({85.0f, 45.0f}, {10.0f, 10.0f});
    REQUIRE(carte.chevaucheUnMur(surLaPorte));
    REQUIRE_FALSE(carte.trajetLibre(sf::FloatRect({25.0f, 55.0f}, {10.0f, 10.0f}), {100.0f, 0.0f}));
    REQUIRE_FALSE(carte.trajetLibre(sf::FloatRect({25.0f, 45.0f}, {10.0f, 10.0f}), {100.0f, 20.0f}));
}

TEST_CASE("porte ouverte : ne bloque pas le mouvement") {
    TileMap carte({".....", ".....", "....."}, 40.0f);
    carte.placerPorte({2, 1});
    carte.ouvrirPorte({2, 1});
    REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect({85.0f, 45.0f}, {10.0f, 10.0f})));
    REQUIRE(carte.trajetLibre(sf::FloatRect({25.0f, 55.0f}, {10.0f, 10.0f}), {100.0f, 0.0f}));
}

TEST_CASE("porte fermee : trouverChemin la traite comme un mur") {
    TileMap carte({"#####", "#...#", "#####"}, 40.0f);
    carte.placerPorte({2, 1});
    REQUIRE(trouverChemin(carte, {1, 1}, {3, 1}).empty());
    carte.ouvrirPorte({2, 1});
    REQUIRE(trouverChemin(carte, {1, 1}, {3, 1}).size() == 3);
}

TEST_CASE("l'escalier : le caractere '>' du plan est une case Escalier, que la carte retrouve") {
    const std::vector<std::string> plan = {
        "....",
        ".>#.",
        "....",
    };
    const TileMap carte(plan, 40.0f);

    REQUIRE(carte.getTile(1, 1) == Tile::Escalier);
    REQUIRE(carte.caseDeLEscalier() == sf::Vector2i(1, 1));
    REQUIRE(carte.getTile(0, 1) == Tile::Floor);
    REQUIRE(carte.getTile(2, 1) == Tile::Wall);
}

TEST_CASE("l'escalier : un plan sans '>' n'a pas d'escalier") {
    const TileMap carte({"...", "...", "..."}, 40.0f);

    REQUIRE_FALSE(carte.caseDeLEscalier().has_value());
}

TEST_CASE("l'escalier ne bloque pas : on le traverse, la vue aussi") {
    const TileMap carte({"...", ".>.", "..."}, 40.0f);

    REQUIRE_FALSE(estBloquante(Tile::Escalier));
    REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect({40.0f, 40.0f}, {40.0f, 40.0f})));
    REQUIRE(carte.trajetLibre(sf::FloatRect({5.0f, 45.0f}, {20.0f, 20.0f}), {60.0f, 0.0f}));
}
