// ./tests/test_Escouade_deplacement.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Escouade.hpp"
#include "TileMap.hpp"

TEST_CASE("Le centre initial est au centre du carre de depart") {
    const Escouade escouade({50.0f, 50.0f});

    verifierPointProche(escouade.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("Un seul tic avance le centre de vitesse * dt") {
    // Terrain dégagé, destination à plusieurs cases : le premier point visé
    // est donc loin, bien au-delà de la distance parcourue en un seul tic
    // (180 px/s * 0.1 s = 18 px), quelle que soit sa direction exacte.
    const std::vector<std::string> plan = {
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f centreInitial = escouade.getCentre();
    escouade.allerVers({300.0f, 300.0f}, carte);
    escouade.mettreAJour(0.1f, carte, {});

    const float distanceParcourue = (escouade.getCentre() - centreInitial).length();
    REQUIRE_THAT(distanceParcourue, Catch::Matchers::WithinAbs(18.0f, 0.5f));
}

TEST_CASE("En terrain degage, l'escouade finit centree sur le point exact demande") {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    // Point choisi hors de tout centre de case (les centres de case sont à
    // 20, 60, 100, 140... pour une taille de case de 40).
    const sf::Vector2f destination{137.0f, 251.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 10.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("Avec un mur entre le depart et la destination, l'escouade le contourne et l'atteint") {
    // Mur sur la rangée 3, colonnes 1 à 3 ; ouverture par la colonne 0 (et
    // par les colonnes 4 à 6). Départ en case (1, 1), destination en case
    // (1, 5) : directement sous le départ, mais le mur bloque le trajet
    // direct en colonne 1.
    const std::vector<std::string> plan = {
        ".......",
        ".......",
        ".......",
        ".###...",
        ".......",
        ".......",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f destination = carte.centreDeCase({1, 5});
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 10.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("Sans ordre, l'escouade ne bouge pas") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    for (int i = 0; i < 60; ++i) {
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    verifierPointProche(escouade.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("A l'arret, un ordre vers un mur ne fait pas bouger l'escouade") {
    // Mur en case (2, 2) ; départ en case (1, 1), comme pour tous les
    // autres tests.
    const std::vector<std::string> plan = {
        "....",
        "....",
        "..#.",
        "....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f centreInitial = escouade.getCentre();

    escouade.allerVers(carte.centreDeCase({2, 2}), carte);
    simuler(escouade, carte, 1.0f);

    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("Un ordre vers un mur pendant un deplacement n'interrompt pas le trajet en cours") {
    // Mur isolé en case (0, 0), loin du trajet entre le départ (1, 1) et
    // la première destination (case (5, 5)) : il ne sert qu'à fournir un
    // second ordre invalide, sans jamais bloquer le chemin réel.
    const std::vector<std::string> plan = {
        "#.....",
        "......",
        "......",
        "......",
        "......",
        "......",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f destination{220.0f, 220.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 0.3f); // en route, loin d'avoir fini

    escouade.allerVers(carte.centreDeCase({0, 0}), carte); // ordre invalide, ignoré
    simuler(escouade, carte, 10.0f); // largement assez pour finir le trajet d'origine

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("Un second ordre valide pendant un deplacement remplace la destination") {
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
    Escouade escouade({50.0f, 50.0f});

    escouade.allerVers({280.0f, 100.0f}, carte);
    simuler(escouade, carte, 0.3f); // en route vers la première destination

    const sf::Vector2f secondeDestination{100.0f, 280.0f};
    escouade.allerVers(secondeDestination, carte);
    simuler(escouade, carte, 10.0f); // largement assez pour atteindre la seconde

    verifierPointProche(escouade.getCentre(), secondeDestination);
}

TEST_CASE("Le lissage rend un trajet diagonal plus rapide qu'un escalier") {
    // Terrain dégagé, destination 240 px plus loin en x et en y. Sans
    // lissage, le A* produirait un escalier dont la longueur totale vaut
    // la distance de Manhattan (240 + 240 = 480 px, soit 480 / 180 ≈
    // 2,67 s). Avec lissage, le trajet devient la ligne droite directe,
    // de longueur euclidienne (240 * racine(2) ≈ 339,4 px, soit ≈ 1,89 s).
    // Une simulation de 2 s est donc suffisante pour la ligne droite, mais
    // insuffisante pour l'escalier : si l'escouade est bien arrivée après
    // seulement 2 s, c'est que le chemin a été lissé.
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f destination{300.0f, 300.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 2.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("estVivante est vraie a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante reste vraie apres la mort d'un seul personnage") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    escouade.recevoirDegats(1, point, 100);

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante devient fausse une fois les 4 personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, point, 100);
    }

    REQUIRE_FALSE(escouade.estVivante());
}

TEST_CASE("cerclesDesVivants renvoie 4 cercles a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.cerclesDesVivants().size() == 4);
}

TEST_CASE("cerclesDesVivants n'inclut plus le cercle d'un personnage mort") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> futurMort = escouade.proieDuMonstre(1, point);
    REQUIRE(futurMort.has_value());

    escouade.recevoirDegats(1, point, 100);
    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();

    REQUIRE(cercles.size() == 3);
    for (const Cercle& cercle : cercles) {
        REQUIRE_FALSE(cercle.centre == futurMort->centre);
    }
}

TEST_CASE("cerclesDesVivants est vide une fois tous les personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, point, 100);
    }

    REQUIRE(escouade.cerclesDesVivants().empty());
}

TEST_CASE("Les obstacles bloquent les personnages mais pas l'ancre") {
    const std::vector<std::string> plan = {
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f}); // centre (60, 60), personnages en (35|85, 35|85)
    const std::vector<Cercle> obstacles = {{{200.0f, 60.0f}, 30.0f}};

    escouade.allerVers({400.0f, 60.0f}, carte);
    for (int i = 0; i < 300; ++i) {
        escouade.mettreAJour(1.0f / 60.0f, carte, obstacles);
    }

    // L'ancre a traversé l'obstacle (bord droit en x = 230)...
    REQUIRE(escouade.getCentre().x > 230.0f);

    // ... mais aucun personnage n'y est entré.
    for (const Cercle& cercle : escouade.cerclesDesVivants()) {
        const float ecartCentres = (cercle.centre - obstacles[0].centre).length();
        REQUIRE(ecartCentres >= cercle.rayon + obstacles[0].rayon - 0.001f);
    }
}

TEST_CASE("En terrain degage, les places de formation ne sont pas modifiees") {
    const TileMap carte({"##########",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "##########"},
                        40.0f);
    Escouade escouade({150.0f, 110.0f});
    escouade.mettreAJour(0.016f, carte, {});

    const sf::Vector2f centre = escouade.getCentre();
    const std::vector<sf::Vector2f> attendues = {centre + sf::Vector2f(25.0f, -25.0f),
                                                 centre + sf::Vector2f(25.0f, 25.0f),
                                                 centre + sf::Vector2f(-25.0f, -25.0f),
                                                 centre + sf::Vector2f(-25.0f, 25.0f)};
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 4);
    for (std::size_t i = 0; i < places.size(); ++i) {
        REQUIRE_THAT(places[i].x, Catch::Matchers::WithinAbs(attendues[i].x, 0.01f));
        REQUIRE_THAT(places[i].y, Catch::Matchers::WithinAbs(attendues[i].y, 0.01f));
    }
}

TEST_CASE("Ancre collee a un mur, aucune place de formation n'est dans un mur") {
    const TileMap carte({"##########",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "##########"},
                        40.0f);
    Escouade escouade({40.0f, 80.0f}); // centre (50, 90) : le bord gauche de l'ancre touche le mur
    escouade.mettreAJour(0.016f, carte, {});

    const sf::Vector2f taille{20.0f, 20.0f};
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 4);
    for (const sf::Vector2f& place : places) {
        REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect(place - taille / 2.0f, taille)));
    }
    // Les places de gauche, qui tombaient dans le mur, se sont rapprochées de l'ancre.
    REQUIRE(places[2].x > escouade.getCentre().x - 25.0f);
}

// Rotation de la formation. Aucun accesseur ne donne l'orientation : on la
// déduit de la place de formation du personnage 1, dont le décalage vaut
// (25, -25) à l'orientation 0, c'est-à-dire un angle de -45°.

namespace {

// Orientation de la formation en degrés, entre -180° et 180°, déduite de la
// place du personnage 1 calculée par le dernier mettreAJour.
float orientationDeLaFormation(const Escouade& escouade) {
    const sf::Vector2f decalage = escouade.getPlacesDeFormation()[0] - escouade.getCentre();
    return (decalage.angle() + sf::degrees(45.0f)).wrapSigned().asDegrees();
}

}  // namespace

TEST_CASE("Rotation : la formation finit tournee vers la direction de marche") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({100.0f, 100.0f});  // centre (110, 110), orientation 0
    escouade.allerVers({110.0f, 300.0f}, carte);  // vers le bas : 90°

    simuler(escouade, carte, 2.0f);

    REQUIRE_THAT(orientationDeLaFormation(escouade), Catch::Matchers::WithinAbs(90.0f, 0.5f));
}

TEST_CASE("Rotation : la formation tourne par le plus court cote") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({200.0f, 200.0f});  // centre (210, 210), orientation 0
    escouade.allerVers({210.0f, 100.0f}, carte);  // vers le haut : -90° (270°, pas +270°)

    simuler(escouade, carte, 0.1f);
    REQUIRE(orientationDeLaFormation(escouade) < 0.0f);

    simuler(escouade, carte, 1.0f);
    REQUIRE_THAT(orientationDeLaFormation(escouade), Catch::Matchers::WithinAbs(-90.0f, 0.5f));
}

TEST_CASE("Rotation : la vitesse est bornee a 360 degres par seconde") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({100.0f, 100.0f});
    escouade.allerVers({110.0f, 300.0f}, carte);  // 90° à parcourir

    simuler(escouade, carte, 0.1f);

    const float orientation = orientationDeLaFormation(escouade);
    REQUIRE(orientation > 0.0f);     // elle a commencé à tourner
    REQUIRE(orientation <= 36.01f);  // mais pas plus de 36° en 0,1 s
}

TEST_CASE("Rotation : elle se termine meme quand l'ancre s'est arretee") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({100.0f, 100.0f});  // centre (110, 110)
    escouade.allerVers({110.0f, 120.0f}, carte);  // 10 px : l'ancre s'arrête en quelques tics, la rotation de 90° en demande une quinzaine

    simuler(escouade, carte, 0.1f);
    verifierPointProche(escouade.getCentre(), {110.0f, 120.0f});
    REQUIRE(orientationDeLaFormation(escouade) < 89.0f);  // l'ancre est arrêtée, la rotation n'est pas finie

    simuler(escouade, carte, 1.0f);
    REQUIRE_THAT(orientationDeLaFormation(escouade), Catch::Matchers::WithinAbs(90.0f, 0.5f));
}
