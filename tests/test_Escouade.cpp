// ./tests/test_Escouade.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "Cercle.hpp"
#include "Escouade.hpp"
#include "TileMap.hpp"

namespace {

// Cible ponctuelle (rayon nul), pour les tests qui ne portent pas sur
// la forme de la cible elle-même.
Cercle pointCible(sf::Vector2f centre) {
    return {centre, 0.0f};
}

// Cible de la taille d'un monstre (rayon 15 px), pour les tests de combat
// où la taille de la cible décide si les mêlées peuvent l'atteindre.
Cercle cibleMonstre(sf::Vector2f centre) {
    return {centre, 15.0f};
}

// Compare un point à un point attendu avec une tolérance : les positions
// manipulées par Escouade sont des flottants accumulés sur plusieurs tics
// (vitesse * dt, racines carrées de sf::Vector2f::length...), donc deux
// calculs mathématiquement égaux peuvent différer de quelques poussières
// d'erreur d'arrondi. Comparer avec `==` serait fragile.
void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

// Simule le temps qui passe par tics de 1/60 s (comme setFramerateLimit(60)
// dans main.cpp), pendant la durée demandée. Les tests passent une durée
// généreuse par rapport au trajet attendu et ne vérifient que l'état final.
void simuler(Escouade& escouade, const TileMap& carte, float dureeSecondes) {
    constexpr float dt = 1.0f / 60.0f;
    const int nombreTics = static_cast<int>(dureeSecondes / dt);
    for (int i = 0; i < nombreTics; ++i) {
        escouade.mettreAJour(dt, carte, {});
    }
}

// Simule attaquer() + mettreAJour() par tics de 1/60 s, et renvoie le
// total des dégâts cumulés sur toute la durée.
int simulerAttaque(Escouade& escouade, const TileMap& carte, Cercle cible, float dureeSecondes) {
    constexpr float dt = 1.0f / 60.0f;
    const int nombreTics = static_cast<int>(dureeSecondes / dt);
    int degats = 0;
    for (int i = 0; i < nombreTics; ++i) {
        degats += escouade.attaquer(cible, carte);
        escouade.mettreAJour(dt, carte, {});
    }
    return degats;
}

} // namespace

TEST_CASE("Le centre initial est au centre du carré de départ") {
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

TEST_CASE("En terrain dégagé, l'escouade finit centrée sur le point exact demandé") {
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

TEST_CASE("Avec un mur entre le départ et la destination, l'escouade le contourne et l'atteint") {
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

TEST_CASE("À l'arrêt, un ordre vers un mur ne fait pas bouger l'escouade") {
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

TEST_CASE("Un ordre vers un mur pendant un déplacement n'interrompt pas le trajet en cours") {
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

TEST_CASE("Un second ordre valide pendant un déplacement remplace la destination") {
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

TEST_CASE("attaquer renvoie 0 pour une cible lointaine et l'escouade s'en rapproche") {
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

    const sf::Vector2f cible{380.0f, 380.0f};
    const float distanceInitiale = (cible - escouade.getCentre()).length();

    REQUIRE(escouade.attaquer(pointCible(cible), carte) == 0);
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    const float distanceApres = (cible - escouade.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("attaquer finit par infliger des dégâts sans que l'escouade atteigne la cible") {
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

    const sf::Vector2f cible{300.0f, 300.0f};
    const int degatsTotal = simulerAttaque(escouade, carte, cibleMonstre(cible), 5.0f);

    REQUIRE(degatsTotal > 0);
    REQUIRE((cible - escouade.getCentre()).length() > 1.0f);
}

TEST_CASE("attaquer ignore les personnages morts pour décider si l'escouade doit s'arrêter") {
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

    escouade.recevoirDegats({1000.0f, 1000.0f}, 100); // tue le personnage le plus proche de ce point

    escouade.allerVers({300.0f, 60.0f}, carte); // éloigne l'escouade, laisse le mort derrière
    simuler(escouade, carte, 10.0f); // largement assez pour arriver
    const sf::Vector2f centreAvant = escouade.getCentre();

    // À portée des 3 vivants (qui ont suivi l'ancre), loin du mort resté
    // en arrière.
    const sf::Vector2f cible = centreAvant + sf::Vector2f(30.0f, 0.0f);

    REQUIRE(escouade.attaquer(cibleMonstre(cible), carte) == 15); // 3 vivants x 5 degats
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    verifierPointProche(escouade.getCentre(), centreAvant);
}

TEST_CASE("attaquer renvoie les dégâts de tous les personnages pour une cible déjà proche") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    // Centre initial (60, 60) ; mêlées en (85, 35) et (85, 85), tireurs en
    // (35, 35) et (35, 85). Une cible de la taille d'un monstre en (85, 60)
    // est à 25 px du centre de chaque mêlée, soit 0 px bord à bord : les
    // 4 personnages sont déjà à portée.
    const sf::Vector2f cible{85.0f, 60.0f};
    const sf::Vector2f centreInitial = escouade.getCentre();

    REQUIRE(escouade.attaquer(cibleMonstre(cible), carte) == 20);
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    verifierPointProche(escouade.getCentre(), centreInitial);
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

TEST_CASE("personnageLePlusProche renvoie un centre plus proche du point que le centre de l'escouade") {
    const Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f pointLointain{1000.0f, 1000.0f};
    const std::optional<Cercle> cerclePersonnage = escouade.personnageLePlusProche(pointLointain);
    REQUIRE(cerclePersonnage.has_value());

    const float distancePersonnage = (cerclePersonnage->centre - pointLointain).length();
    const float distanceEscouade = (escouade.getCentre() - pointLointain).length();

    REQUIRE(distancePersonnage < distanceEscouade);
}

TEST_CASE("personnageLePlusProche ignore les degats insuffisants pour tuer") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> avant = escouade.personnageLePlusProche(point);
    REQUIRE(avant.has_value());

    escouade.recevoirDegats(point, 50);
    const std::optional<Cercle> apres = escouade.personnageLePlusProche(point);
    REQUIRE(apres.has_value());

    REQUIRE(apres->centre == avant->centre);
}

TEST_CASE("personnageLePlusProche change de cible une fois le plus proche mort") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> avant = escouade.personnageLePlusProche(point);
    REQUIRE(avant.has_value());

    escouade.recevoirDegats(point, 100);
    const std::optional<Cercle> apres = escouade.personnageLePlusProche(point);
    REQUIRE(apres.has_value());

    REQUIRE_FALSE(apres->centre == avant->centre);
}

TEST_CASE("personnageLePlusProche est vide une fois tous les personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(point, 100);
    }

    REQUIRE_FALSE(escouade.personnageLePlusProche(point).has_value());

    escouade.recevoirDegats(point, 100);
    REQUIRE_FALSE(escouade.personnageLePlusProche(point).has_value());
}

TEST_CASE("estVivante est vraie a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante reste vraie apres la mort d'un seul personnage") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    escouade.recevoirDegats(point, 100);

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante devient fausse une fois les 4 personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(point, 100);
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

    const std::optional<Cercle> futurMort = escouade.personnageLePlusProche(point);
    REQUIRE(futurMort.has_value());

    escouade.recevoirDegats(point, 100);
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
        escouade.recevoirDegats(point, 100);
    }

    REQUIRE(escouade.cerclesDesVivants().empty());
}

TEST_CASE("getCibleVerrouillee est vide a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("verrouiller pose la cible donnee") {
    Escouade escouade({50.0f, 50.0f});

    escouade.verrouiller(3);

    REQUIRE(escouade.getCibleVerrouillee() == std::optional<int>(3));
}

TEST_CASE("verrouiller remplace une cible deja posee") {
    Escouade escouade({50.0f, 50.0f});

    escouade.verrouiller(3);
    escouade.verrouiller(7);

    REQUIRE(escouade.getCibleVerrouillee() == std::optional<int>(7));
}

TEST_CASE("deverrouiller vide la cible") {
    Escouade escouade({50.0f, 50.0f});

    escouade.verrouiller(3);
    escouade.deverrouiller();

    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("En combat, un personnage a portee de la cible reste immobile pendant que les autres avancent") {
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
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1); // id arbitraire, Escouade ne connaît pas Monstre

    // Centre initial (60, 60). Les deux tireurs (portée 120 px bord à bord)
    // démarrent à l'arrière, en (35, 35) et (35, 85) ; les deux mêlées
    // (portée 10 px) à l'avant, en (85, 35) et (85, 85). Rayons : 10 px
    // pour un personnage, 15 px pour la cible. Cette cible met déjà les
    // tireurs à portée (≈142 - 25 ≈ 117 px bord à bord) mais pas les mêlées
    // (≈93 - 25 ≈ 68 px) : l'ancre doit donc encore avancer.
    const sf::Vector2f cible{175.0f, 60.0f};

    const sf::Vector2f tireurAvant = escouade.personnageLePlusProche({35.0f, 60.0f})->centre;
    const sf::Vector2f meleeAvant = escouade.personnageLePlusProche({85.0f, 60.0f})->centre;

    for (int i = 0; i < 10; ++i) {
        escouade.attaquer(cibleMonstre(cible), carte);
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    const sf::Vector2f tireurApres = escouade.personnageLePlusProche({35.0f, 60.0f})->centre;
    verifierPointProche(tireurApres, tireurAvant, 0.01f);

    const sf::Vector2f meleeApres = escouade.personnageLePlusProche({85.0f, 60.0f})->centre;
    REQUIRE((meleeApres - meleeAvant).length() > 5.0f);
}

TEST_CASE("Sans verrou, un personnage deja a portee suit tout de meme son emplacement") {
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
    Escouade escouade({50.0f, 50.0f}); // pas de verrouiller()

    const sf::Vector2f cible{175.0f, 60.0f}; // même configuration que le test précédent

    const sf::Vector2f tireurAvant = escouade.personnageLePlusProche({35.0f, 60.0f})->centre;

    for (int i = 0; i < 10; ++i) {
        escouade.attaquer(cibleMonstre(cible), carte);
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    const sf::Vector2f tireurApres = escouade.personnageLePlusProche({35.0f, 60.0f})->centre;
    REQUIRE((tireurApres - tireurAvant).length() > 5.0f);
}

// Pour les tests du skill : centre initial (60, 60), mêlées en (85, 35) et
// (85, 85). Une cible de la taille d'un monstre en (85, 60) est à 0 px bord
// à bord de chacun d'eux, donc à portée du skill (10 px, comme la mêlée).

TEST_CASE("declencherSkill renvoie 0 sans combat engage, meme avec une cible a portee") {
    Escouade escouade({50.0f, 50.0f}); // pas de verrouiller()

    REQUIRE(escouade.declencherSkill(1, cibleMonstre({85.0f, 60.0f})) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie les degats du seul personnage qui a un skill") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Les deux mêlées sont à portée, mais un seul a un skill : 25, pas 50.
    REQUIRE(escouade.declencherSkill(1, cibleMonstre({85.0f, 60.0f})) == 25);
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une cible hors de portee du skill") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    REQUIRE(escouade.declencherSkill(1, cibleMonstre({300.0f, 60.0f})) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie 0 au second appel immediat (recharge du skill)") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(escouade.declencherSkill(1, cible) == 25);
    REQUIRE(escouade.declencherSkill(1, cible) == 0);
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

TEST_CASE("En combat, declencherSkill renvoie 0 pour une place sans skill ou hors formation") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(escouade.declencherSkill(2, cible) == 0);
    REQUIRE(escouade.declencherSkill(0, cible) == 0);
    REQUIRE(escouade.declencherSkill(5, cible) == 0);
}
