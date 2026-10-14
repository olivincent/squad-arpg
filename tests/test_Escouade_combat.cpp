// ./tests/test_Escouade_combat.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "MonstreVu.hpp"
#include "TileMap.hpp"

namespace {

// Simule attaquer() + mettreAJour() par tics de 1/60 s, et renvoie le
// total des dégâts cumulés sur toute la durée.
int simulerAttaque(Escouade& escouade, const TileMap& carte, Cercle cible, float dureeSecondes) {
    constexpr float dt = s_dureeTic;
    const int nombreTics = nombreDeTics(dureeSecondes);
    int degats = 0;
    Generateur generateur = generateurDeTest();
    for (int i = 0; i < nombreTics; ++i) {
        degats += attaquerCible(escouade, carte, cible);
        escouade.mettreAJour(dt, carte, {}, generateur);
    }
    return degats;
}

}  // namespace


TEST_CASE("attaquer renvoie 0 pour une cible lointaine et l'escouade s'en rapproche") {
    Generateur generateur = generateurDeTest();
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
    escouade.verrouiller(1);

    const sf::Vector2f cible{380.0f, 380.0f};
    const float distanceInitiale = (cible - escouade.getCentre()).length();

    REQUIRE(attaquerCible(escouade, carte, pointCible(cible)) == 0);
    escouade.mettreAJour(1.0f / 60.0f, carte, {}, generateur);

    const float distanceApres = (cible - escouade.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("attaquer finit par infliger des degats sans que l'escouade atteigne la cible") {
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
    escouade.verrouiller(1);

    const sf::Vector2f cible{300.0f, 300.0f};
    const int degatsTotal = simulerAttaque(escouade, carte, cibleMonstre(cible), 5.0f);

    REQUIRE(degatsTotal > 0);
    REQUIRE((cible - escouade.getCentre()).length() > 1.0f);
}

TEST_CASE("attaquer ignore les personnages morts pour decider si l'escouade doit s'arreter") {
    Generateur generateur = generateurDeTest();
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
    escouade.verrouiller(1);

    escouade.recevoirDegats(1, {1000.0f, 1000.0f}, 200); // tue le personnage le plus proche de ce point

    escouade.allerVers({300.0f, 60.0f}, carte); // éloigne l'escouade, laisse le mort derrière
    simuler(escouade, carte, 10.0f); // largement assez pour arriver
    const sf::Vector2f centreAvant = escouade.getCentre();

    // À portée des 3 vivants (qui ont suivi l'ancre), loin du mort resté
    // en arrière.
    const sf::Vector2f cible = centreAvant + sf::Vector2f(30.0f, 0.0f);

    REQUIRE(attaquerCible(escouade, carte, cibleMonstre(cible)) == 15); // 3 vivants x 5 degats
    escouade.mettreAJour(1.0f / 60.0f, carte, {}, generateur);

    verifierPointProche(escouade.getCentre(), centreAvant);
}

TEST_CASE("attaquer renvoie les degats de tous les personnages pour une cible deja proche") {
    Generateur generateur = generateurDeTest();
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Centre initial (60, 60) ; mêlées en (85, 35) et (85, 85), tireurs en
    // (35, 35) et (35, 85). Une cible de la taille d'un monstre en (85, 60)
    // est à 25 px du centre de chaque mêlée, soit 0 px bord à bord : les
    // 4 personnages sont déjà à portée.
    const sf::Vector2f cible{85.0f, 60.0f};
    const sf::Vector2f centreInitial = escouade.getCentre();

    REQUIRE(attaquerCible(escouade, carte, cibleMonstre(cible)) == 20);
    escouade.mettreAJour(1.0f / 60.0f, carte, {}, generateur);

    verifierPointProche(escouade.getCentre(), centreInitial);
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
    Generateur generateur = generateurDeTest();
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
    // (portée 20 px) à l'avant, en (85, 35) et (85, 85). Rayons : 10 px
    // pour un personnage, 15 px pour la cible. Cette cible met déjà les
    // tireurs à portée (≈142 - 25 ≈ 117 px bord à bord) mais pas les mêlées
    // (≈93 - 25 ≈ 68 px) : l'ancre doit donc encore avancer.
    const sf::Vector2f cible{175.0f, 60.0f};

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;
    const sf::Vector2f meleeAvant = escouade.proieDuMonstre(1, {85.0f, 60.0f})->centre;

    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.mettreAJour(1.0f / 60.0f, carte, {}, generateur);
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;
    verifierPointProche(tireurApres, tireurAvant, 0.01f);

    const sf::Vector2f meleeApres = escouade.proieDuMonstre(1, {85.0f, 60.0f})->centre;
    REQUIRE((meleeApres - meleeAvant).length() > 5.0f);
}

TEST_CASE("Sans verrou, un personnage deja a portee suit tout de meme son emplacement") {
    Generateur generateur = generateurDeTest();
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
    escouade.allerVers(cible, carte); // sans verrou, attaquer() n'approche pas : l'ordre vient du joueur

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;

    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.mettreAJour(1.0f / 60.0f, carte, {}, generateur);
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;
    REQUIRE((tireurApres - tireurAvant).length() > 5.0f);
}

// Autodéfense : sans verrou, chaque personnage vivant frappe de lui-même un
// monstre à sa portée. Centre initial (60, 60) ; mêlées en (85, 35) et
// (85, 85) (portée 20), tireurs en (35, 35) et (35, 85) (portée 120).

TEST_CASE("Sans verrou, tous les personnages frappent un monstre a leur portee, sans bouger") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f centreInitial = escouade.getCentre();

    const std::vector<Coup> coups = escouade.attaquer({MonstreVu{4, cibleMonstre({85.0f, 60.0f})}}, carte, generateur);
    escouade.mettreAJour(1.0f / 60.0f, carte, {}, generateur);

    REQUIRE(coups.size() == 4);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 4);
        REQUIRE(coup.degats == 5);
    }
    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("Sans verrou, un monstre hors de portee de tous ne recoit aucun coup") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.attaquer({MonstreVu{1, cibleMonstre({300.0f, 60.0f})}}, carte, generateur).empty());
}

TEST_CASE("Sans verrou, seuls les tireurs frappent un monstre eloigne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});

    // À 92 px bord à bord des tireurs, à 44 px des mêlées.
    const std::vector<Coup> coups = escouade.attaquer({MonstreVu{1, cibleMonstre({150.0f, 60.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
}

TEST_CASE("Un verrou hors de portee n'empeche pas de frapper un autre monstre a portee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(2);

    const std::vector<Coup> coups = escouade.attaquer(
        {MonstreVu{1, cibleMonstre({150.0f, 60.0f})}, MonstreVu{2, cibleMonstre({300.0f, 60.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 1);
    }
}

TEST_CASE("Le verrou a portee passe avant un monstre qui est a portee aussi") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(2);

    // Le monstre 1 est plus proche des tireurs, mais le 2 est verrouillé.
    const std::vector<Coup> coups = escouade.attaquer(
        {MonstreVu{1, cibleMonstre({150.0f, 60.0f})}, MonstreVu{2, cibleMonstre({150.0f, 90.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 2);
    }
}

TEST_CASE("Deux personnages peuvent frapper deux monstres differents dans la meme image") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 150.0f});

    // Le monstre 1 est à portée du tireur du haut seulement, le 2 du tireur du bas seulement
    // (tous deux dans la carte, en vue : la ligne de vue exige des points dans la carte).
    const std::vector<Coup> coups = escouade.attaquer(
        {MonstreVu{1, cibleMonstre({140.0f, 50.0f})}, MonstreVu{2, cibleMonstre({140.0f, 270.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre != coups[1].idMonstre);
}

TEST_CASE("Une escouade dont tous les personnages sont morts ne frappe plus") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 35.0f}, 200);
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 200);
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 200);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 200);

    REQUIRE(escouade.attaquer({MonstreVu{1, cibleMonstre({85.0f, 60.0f})}}, carte, generateur).empty());
}
