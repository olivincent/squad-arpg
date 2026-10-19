// ./tests/test_Escouade_deplacement.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "Cercle.hpp"
#include "Escouade.hpp"
#include "TileMap.hpp"

TEST_CASE("Le centre initial est au centre du carre de depart") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});
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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    const sf::Vector2f destination{300.0f, 300.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 2.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("estVivante est vraie a la creation") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante reste vraie apres la mort d'un seul personnage") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    escouade.recevoirDegats(1, point, 200);

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante devient fausse une fois les 3 personnages morts") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 3; ++i) {
        escouade.recevoirDegats(1, point, 200);
    }

    REQUIRE_FALSE(escouade.estVivante());
}

TEST_CASE("cerclesDesVivants renvoie 3 cercles a la creation") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.cerclesDesVivants().size() == 3);
}

TEST_CASE("cerclesDesVivants n'inclut plus le cercle d'un personnage mort") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<EntiteVue> futurMort = escouade.proieDuMonstre(1, point);
    REQUIRE(futurMort.has_value());

    escouade.recevoirDegats(1, point, 200);
    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();

    REQUIRE(cercles.size() == 2);
    for (const Cercle& cercle : cercles) {
        REQUIRE_FALSE(cercle.centre == futurMort->cercle.centre);
    }
}

TEST_CASE("cerclesDesVivants est vide une fois tous les personnages morts") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 3; ++i) {
        escouade.recevoirDegats(1, point, 200);
    }

    REQUIRE(escouade.cerclesDesVivants().empty());
}

TEST_CASE("cerclesDeVision renvoie le centre et le rayon de vision de chacun des 3 personnages") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::vector<Cercle> vision = escouade.cerclesDeVision();
    const std::vector<Cercle> corps = escouade.cerclesDesVivants();

    REQUIRE(vision.size() == 3);
    REQUIRE(corps.size() == 3);
    for (std::size_t i = 0; i < vision.size(); ++i) {
        REQUIRE(vision[i].centre == corps[i].centre);
        REQUIRE(vision[i].rayon == 320.0f);
    }
}

TEST_CASE("cerclesDeVision n'inclut pas un personnage mort") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};
    const std::optional<EntiteVue> futurMort = escouade.proieDuMonstre(1, point);
    REQUIRE(futurMort.has_value());

    escouade.recevoirDegats(1, point, 200);
    const std::vector<Cercle> vision = escouade.cerclesDeVision();

    REQUIRE(vision.size() == 2);
    for (const Cercle& cercle : vision) {
        REQUIRE_FALSE(cercle.centre == futurMort->cercle.centre);
    }
}

TEST_CASE("cerclesDeVision est vide une fois tous les personnages morts") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 3; ++i) {
        escouade.recevoirDegats(1, point, 200);
    }

    REQUIRE(escouade.cerclesDeVision().empty());
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
    Escouade escouade = uneEscouade({50.0f, 50.0f}); // centre (60, 60), guerrier en (85, 60), prêtre en (35, 35), mage en (35, 85)
    const std::vector<EntiteVue> obstacles = {{1, {{200.0f, 60.0f}, 30.0f}}};

    escouade.allerVers({400.0f, 60.0f}, carte);
    for (int i = 0; i < 300; ++i) {
        escouade.mettreAJour(1.0f / 60.0f, carte, obstacles);
    }

    // L'ancre a traversé l'obstacle (bord droit en x = 230)...
    REQUIRE(escouade.getCentre().x > 230.0f);

    // ... mais aucun personnage n'y est entré.
    for (const Cercle& cercle : escouade.cerclesDesVivants()) {
        const float ecartCentres = (cercle.centre - obstacles[0].cercle.centre).length();
        REQUIRE(ecartCentres >= cercle.rayon + obstacles[0].cercle.rayon - 0.001f);
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
    Escouade escouade = uneEscouade({150.0f, 110.0f});
    escouade.mettreAJour(0.016f, carte, {});

    const sf::Vector2f centre = escouade.getCentre();
    const std::vector<sf::Vector2f> attendues = {centre + sf::Vector2f(25.0f, 0.0f),
                                                 centre + sf::Vector2f(-25.0f, -25.0f),
                                                 centre + sf::Vector2f(-25.0f, 25.0f)};
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 3);
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
    Escouade escouade = uneEscouade({40.0f, 80.0f}); // centre (50, 90) : le bord gauche de l'ancre touche le mur
    escouade.mettreAJour(0.016f, carte, {});

    const sf::Vector2f taille{20.0f, 20.0f};
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 3);
    for (const sf::Vector2f& place : places) {
        REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect(place - taille / 2.0f, taille)));
    }
    // Les places de gauche, qui tombaient dans le mur, se sont rapprochées de l'ancre.
    REQUIRE(places[1].x > escouade.getCentre().x - 25.0f);
}

// Rotation de la formation. Aucun accesseur ne donne l'orientation : on la
// déduit de la place de formation du personnage 1 (le guerrier), dont le décalage
// vaut (25, 0) à l'orientation 0, c'est-à-dire un angle de 0°.

namespace {

// Orientation de la formation en degrés, entre -180° et 180°, déduite de la
// place du personnage 1 calculée par le dernier mettreAJour.
float orientationDeLaFormation(const Escouade& escouade) {
    const sf::Vector2f decalage = escouade.getPlacesDeFormation()[0] - escouade.getCentre();
    return decalage.angle().wrapSigned().asDegrees();
}

}  // namespace

TEST_CASE("Rotation : la formation finit tournee vers la direction de marche") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({100.0f, 100.0f});  // centre (110, 110), orientation 0
    escouade.allerVers({110.0f, 300.0f}, carte);  // vers le bas : 90°

    simuler(escouade, carte, 2.0f);

    REQUIRE_THAT(orientationDeLaFormation(escouade), Catch::Matchers::WithinAbs(90.0f, 0.5f));
}

TEST_CASE("Rotation : la formation tourne par le plus court cote") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({200.0f, 200.0f});  // centre (210, 210), orientation 0
    escouade.allerVers({210.0f, 100.0f}, carte);  // vers le haut : -90° (270°, pas +270°)

    simuler(escouade, carte, 0.1f);
    REQUIRE(orientationDeLaFormation(escouade) < 0.0f);

    simuler(escouade, carte, 1.0f);
    REQUIRE_THAT(orientationDeLaFormation(escouade), Catch::Matchers::WithinAbs(-90.0f, 0.5f));
}

TEST_CASE("Rotation : la vitesse est bornee a 360 degres par seconde") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({100.0f, 100.0f});
    escouade.allerVers({110.0f, 300.0f}, carte);  // 90° à parcourir

    simuler(escouade, carte, 0.1f);

    const float orientation = orientationDeLaFormation(escouade);
    REQUIRE(orientation > 0.0f);     // elle a commencé à tourner
    REQUIRE(orientation <= 36.01f);  // mais pas plus de 36° en 0,1 s
}

TEST_CASE("Rotation : elle se termine meme quand l'ancre s'est arretee") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({100.0f, 100.0f});  // centre (110, 110)
    escouade.allerVers({110.0f, 120.0f}, carte);  // 10 px : l'ancre s'arrête en quelques tics, la rotation de 90° en demande une quinzaine

    simuler(escouade, carte, 0.1f);
    verifierPointProche(escouade.getCentre(), {110.0f, 120.0f});
    REQUIRE(orientationDeLaFormation(escouade) < 89.0f);  // l'ancre est arrêtée, la rotation n'est pas finie

    simuler(escouade, carte, 1.0f);
    REQUIRE_THAT(orientationDeLaFormation(escouade), Catch::Matchers::WithinAbs(90.0f, 0.5f));
}

namespace {

// Plus petit écart entre les centres de deux personnages vivants.
float ecartMinimalEntrePersonnages(const Escouade& escouade) {
    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    float minimum = 1.0e9f;
    for (std::size_t i = 0; i < cercles.size(); ++i) {
        for (std::size_t j = i + 1; j < cercles.size(); ++j) {
            minimum = std::min(minimum, (cercles[i].centre - cercles[j].centre).length());
        }
    }
    return minimum;
}

// Écart voulu entre deux centres de personnages (s_ecartEntrePersonnages d'Escouade).
constexpr float s_ecartSansChevauchement = 34.0f;

}  // namespace

TEST_CASE("Separation : des personnages trop proches s'ecartent en moins de 1 s") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({500.0f, 500.0f});  // place 3 en (485, 535)
    const TileMap& carte = carteOuverte();
    // La place 3 reste clouee sur place (activation jamais terminee, car pas d'attaquer) :
    // les autres, qui suivent leur place sur l'arc du demi-tour, la croisent.
    REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, {485.0f, 560.0f}, {}, carte, generateur).empty());

    escouade.allerVers({360.0f, 570.0f}, carte);
    bool chevauchement = false;
    int ticsDeSuite = 0;
    int plusLong = 0;
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        escouade.mettreAJour(s_dureeTic, carte, {});
        if (ecartMinimalEntrePersonnages(escouade) < s_ecartSansChevauchement - 0.5f) {
            chevauchement = true;
            plusLong = std::max(plusLong, ++ticsDeSuite);
        } else {
            ticsDeSuite = 0;
        }
    }

    REQUIRE(chevauchement);  // sinon le test ne prouve rien
    REQUIRE(plusLong < nombreDeTics(1.0f));  // le croisement d'un personnage cloué dure ~0,5 s à 34 px
}

TEST_CASE("Separation : apres un demi-tour, les trois personnages rejoignent leurs places") {
    Escouade escouade = uneEscouade({500.0f, 500.0f});
    const TileMap& carte = carteOuverte();

    escouade.allerVers({400.0f, 510.0f}, carte);
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        escouade.mettreAJour(s_dureeTic, carte, {});
    }

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    REQUIRE(cercles.size() == 3);
    for (std::size_t i = 0; i < cercles.size(); ++i) {
        verifierPointProche(cercles[i].centre, escouade.getPlacesDeFormation()[i], 2.0f);
    }
    REQUIRE(ecartMinimalEntrePersonnages(escouade) >= s_ecartSansChevauchement);
}

TEST_CASE("Separation : un mort ne pousse pas et n'est pas pousse") {
    Escouade escouade = uneEscouade({500.0f, 500.0f});  // centre (510, 510), personnage de la place 3 en (485, 535)
    const TileMap& carte = carteOuverte();
    escouade.recevoirDegats(1, {-1000.0f, 1000.0f}, 200);  // tue la place 3 (la plus proche de ce point, trop loin pour devenir un agresseur gênant)
    REQUIRE(escouade.cerclesDesVivants().size() == 2);

    // Demi-tour de 50 px vers la gauche : la place finale de la place 2 est
    // exactement celle ou gît le mort. Un mort qui poussait l'en écarterait.
    escouade.allerVers({460.0f, 510.0f}, carte);
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        escouade.mettreAJour(s_dureeTic, carte, {});
    }

    const std::vector<Cercle> vivants = escouade.cerclesDesVivants();
    REQUIRE(vivants.size() == 2);
    verifierPointProche(vivants[1].centre, {485.0f, 535.0f}, 0.05f);
}

TEST_CASE("Separation : un personnage en activation n'est pas pousse") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({500.0f, 500.0f});  // place 3 en (485, 535)
    const TileMap& carte = carteOuverte();
    REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, {485.0f, 560.0f}, {}, carte, generateur).empty());  // activation en cours
    REQUIRE(escouade.etatDuSkill(s_adresseExplosion)->enActivation);

    // Demi-tour : la place 2 vient se poser sur celle de la place 3. Sans
    // attaquer(), l'activation ne se termine jamais : la place 3 reste en exécution.
    escouade.allerVers({460.0f, 510.0f}, carte);
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        escouade.mettreAJour(s_dureeTic, carte, {});
    }

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    verifierPointProche(cercles[2].centre, {485.0f, 535.0f}, 0.01f);
}

TEST_CASE("Separation : la poussee ne fait pas entrer dans un mur") {
    const TileMap carte = carteDegageeEntouree();  // murs sur la bordure de 40 px
    Escouade escouade = uneEscouade({300.0f, 300.0f});

    // Allers-retours collés au mur de gauche : demi-tours et croisements.
    for (const sf::Vector2f destination : {sf::Vector2f(600.0f, 300.0f), sf::Vector2f(60.0f, 300.0f),
                                           sf::Vector2f(600.0f, 300.0f), sf::Vector2f(60.0f, 300.0f)}) {
        escouade.allerVers(destination, carte);
        for (int i = 0; i < nombreDeTics(2.5f); ++i) {
            escouade.mettreAJour(s_dureeTic, carte, {});
            for (const Cercle& cercle : escouade.cerclesDesVivants()) {
                const sf::FloatRect rectangle(cercle.centre - sf::Vector2f(10.0f, 10.0f), {20.0f, 20.0f});
                REQUIRE_FALSE(carte.chevaucheUnMur(rectangle));
            }
        }
    }
}

TEST_CASE("Ancre collee a un mur, les trois places de formation sont distinctes et les personnages ne se chevauchent pas") {
    const TileMap carte({"##########",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "##########"},
                        40.0f);
    Escouade escouade = uneEscouade({40.0f, 80.0f});  // centre (50, 90) : le bord gauche de l'ancre touche le mur
    escouade.mettreAJour(s_dureeTic, carte, {});

    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 3);
    for (std::size_t i = 0; i < places.size(); ++i) {
        for (std::size_t j = i + 1; j < places.size(); ++j) {
            REQUIRE((places[i] - places[j]).length() >= 34.0f - 0.001f);
        }
        REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect(places[i] - sf::Vector2f(10.0f, 10.0f), {20.0f, 20.0f})));
    }

    for (int i = 0; i < nombreDeTics(2.0f); ++i) {
        escouade.mettreAJour(s_dureeTic, carte, {});
    }
    REQUIRE(ecartMinimalEntrePersonnages(escouade) >= s_ecartSansChevauchement - 0.5f);
}

TEST_CASE("un personnage dont la place est derriere un monstre non vise le contourne") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f}); // centre (60, 60), guerrier en (85, 60), prêtre en (35, 35), mage en (35, 85)
    // Sur la route du mage (35, 85) quand l'escouade part vers l'est ; hors de celle de l'ancre.
    const std::vector<EntiteVue> monstres = {{1, {{160.0f, 85.0f}, 15.0f}}};

    escouade.allerVers({340.0f, 60.0f}, carte);
    for (int i = 0; i < nombreDeTics(6.0f); ++i) {
        escouade.mettreAJour(s_dureeTic, carte, monstres);
        // Aucune place de formation n'est dans le monstre (centre à moins de 15 + 10 px).
        for (const sf::Vector2f& place : escouade.getPlacesDeFormation()) {
            REQUIRE((place - monstres[0].cercle.centre).length() >= 25.0f);
        }
    }

    // Les trois sont passés de l'autre côté, y compris le mage aligné avec le monstre
    // (indice 2), que sa place entrant dans le monstre calait contre lui.
    for (const Cercle& cercle : escouade.cerclesDesVivants()) {
        REQUIRE(cercle.centre.x > 200.0f);
    }
}

TEST_CASE("une place de formation dans un monstre est deplacee vers un point libre autour de l'ancre") {
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({150.0f, 150.0f}); // centre (160, 160)
    // Sur la place du guerrier : place théorique (185, 160) pour une formation vers l'est.
    const std::vector<EntiteVue> monstres = {{1, {{185.0f, 160.0f}, 15.0f}}};

    escouade.mettreAJour(s_dureeTic, carte, monstres);

    for (const sf::Vector2f& place : escouade.getPlacesDeFormation()) {
        REQUIRE((place - monstres[0].cercle.centre).length() >= 25.0f);
    }
}

namespace {
// La place avant (le guerrier, indice 0) est-elle plus proche du point que les deux places arrière ?
bool avantPlusProche(const Escouade& escouade, sf::Vector2f point) {
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    const float avant = (places[0] - point).length();
    const float arriereMin = std::min((places[1] - point).length(), (places[2] - point).length());
    return avant < arriereMin;
}
}

TEST_CASE("regarderVers fait pivoter la formation a l'arret") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({2000.0f, 2000.0f});  // centre (2010, 2010), formation vers l'est
    const sf::Vector2f point{2010.0f, 2500.0f};

    escouade.regarderVers(point);
    simuler(escouade, carte, 2.0f);

    REQUIRE(avantPlusProche(escouade, point));
}

TEST_CASE("regarderVers suit la marche pendant le trajet puis se tourne vers le point a l'arrivee") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({2000.0f, 2000.0f});
    const sf::Vector2f destination{1810.0f, 2010.0f};  // vers l'ouest : demi-tour de la formation
    const sf::Vector2f point{2010.0f, 2500.0f};

    escouade.allerVers(destination, carte);
    escouade.regarderVers(point);
    simuler(escouade, carte, 0.9f);  // trajet de 200 px à 180 px/s : encore en marche

    REQUIRE(avantPlusProche(escouade, destination));

    simuler(escouade, carte, 3.0f);

    REQUIRE(avantPlusProche(escouade, point));
}

TEST_CASE("un nouvel allerVers efface le point a regarder") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({2000.0f, 2000.0f});
    const sf::Vector2f loinAOuest{1000.0f, 2010.0f};

    escouade.regarderVers({2010.0f, 2500.0f});
    escouade.allerVers({1810.0f, 2010.0f}, carte);
    simuler(escouade, carte, 4.0f);

    REQUIRE(avantPlusProche(escouade, loinAOuest));
}

TEST_CASE("verrouiller efface le point a regarder") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({2000.0f, 2000.0f});
    const sf::Vector2f loinAEst{3000.0f, 2010.0f};

    escouade.regarderVers({2010.0f, 2500.0f});
    escouade.verrouiller(1);  // monstre absent de la liste : l'ancre reste à l'arrêt
    simuler(escouade, carte, 2.0f);

    REQUIRE(avantPlusProche(escouade, loinAEst));
}

TEST_CASE("l'approche du verrou efface le point a regarder") {
    const TileMap& carte = carteOuverte();
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({2000.0f, 2000.0f});
    const std::vector<EntiteVue> monstres = {{1, cibleMonstre({1700.0f, 2010.0f})}};

    escouade.verrouiller(1);
    escouade.regarderVers({2010.0f, 2500.0f});  // après verrouiller : seule l'approche peut l'effacer
    for (int i = 0; i < nombreDeTics(4.0f); ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur));
        escouade.mettreAJour(s_dureeTic, carte, monstres);
    }

    REQUIRE(avantPlusProche(escouade, monstres[0].cercle.centre));
}
