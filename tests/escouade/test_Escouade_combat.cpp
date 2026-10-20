// ./tests/escouade/test_Escouade_combat.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/TileMap.hpp"

namespace {

// Simule attaquer() + mettreAJour() par tics de 1/60 s, et renvoie le
// total des dégâts cumulés sur toute la durée.
int simulerAttaque(Escouade& escouade, const TileMap& carte, Cercle cible, float dureeSecondes) {
    constexpr float dt = s_dureeTic;
    const int nombreTics = nombreDeTics(dureeSecondes);
    int degats = 0;
    for (int i = 0; i < nombreTics; ++i) {
        degats += attaquerCible(escouade, carte, cible);
        escouade.mettreAJour(dt, carte, {.physiques = {}, .ciblables = unSeulMonstre(cible), .deControle = {}});
    }
    return degats;
}

}  // namespace


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
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const sf::Vector2f cible{380.0f, 380.0f};
    const float distanceInitiale = (cible - escouade.getCentre()).length();

    REQUIRE(attaquerCible(escouade, carte, pointCible(cible)) == 0);
    escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = unSeulMonstre(pointCible(cible)), .deControle = {}});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const sf::Vector2f cible{300.0f, 300.0f};
    const int degatsTotal = simulerAttaque(escouade, carte, cibleMonstre(cible), 5.0f);

    REQUIRE(degatsTotal > 0);
    REQUIRE((cible - escouade.getCentre()).length() > 1.0f);
}

TEST_CASE("attaquer ignore les personnages morts pour decider si l'escouade doit s'arreter") {
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
    escouade.verrouiller(1);

    infliger(escouade, 1, {1000.0f, 1000.0f}, 200); // tue le personnage le plus proche de ce point

    escouade.allerVers({300.0f, 60.0f}, carte); // éloigne l'escouade, laisse le mort derrière
    simuler(escouade, carte, 10.0f); // largement assez pour arriver
    const sf::Vector2f centreAvant = escouade.getCentre();

    // À portée des 2 vivants (qui ont suivi l'ancre), loin du mort resté
    // en arrière.
    const sf::Vector2f cible = centreAvant + sf::Vector2f(30.0f, 0.0f);

    REQUIRE(attaquerCible(escouade, carte, cibleMonstre(cible)) == 2 * s_degatsBaseDeTest); // les 2 vivants
    escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = unSeulMonstre(cibleMonstre(cible)), .deControle = {}});

    verifierPointProche(escouade.getCentre(), centreAvant);
}

TEST_CASE("attaquer renvoie les degats de tous les personnages pour une cible deja proche") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Centre initial (60, 60) ; guerrier en (85, 60), tireurs en
    // (35, 35) et (35, 85). Une cible de la taille d'un monstre en (85, 60)
    // est sur le guerrier, soit 0 px bord à bord : les
    // 3 personnages sont déjà à portée.
    const sf::Vector2f cible{85.0f, 60.0f};
    const sf::Vector2f centreInitial = escouade.getCentre();

    REQUIRE(attaquerCible(escouade, carte, cibleMonstre(cible)) == 3 * s_degatsBaseDeTest);
    escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = unSeulMonstre(cibleMonstre(cible)), .deControle = {}});

    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("getCibleVerrouillee est vide a la creation") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("verrouiller pose la cible donnee") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    escouade.verrouiller(3);

    REQUIRE(escouade.getCibleVerrouillee() == std::optional<int>(3));
}

TEST_CASE("verrouiller remplace une cible deja posee") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    escouade.verrouiller(3);
    escouade.verrouiller(7);

    REQUIRE(escouade.getCibleVerrouillee() == std::optional<int>(7));
}

TEST_CASE("deverrouiller vide la cible") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(1); // id arbitraire, Escouade ne connaît pas Monstre

    // Centre initial (60, 60). Les deux tireurs (portée 120 px bord à bord)
    // démarrent à l'arrière, en (35, 35) et (35, 85) ; le guerrier
    // (portée 20 px) à l'avant, en (85, 60). Rayons : 10 px
    // pour un personnage, 15 px pour la cible. Cette cible met déjà les
    // tireurs à portée (≈142 - 25 ≈ 117 px bord à bord) mais pas le guerrier
    // (90 - 25 = 65 px) : l'ancre doit donc encore avancer.
    const sf::Vector2f cible{175.0f, 60.0f};

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;
    const sf::Vector2f meleeAvant = escouade.proieDuMonstre(1, {85.0f, 60.0f})->cercle.centre;

    // Comme dans Partie, l'ordre du joueur est redonné à chaque image (bouton
    // tenu) : il annule le coup que le tireur vient d'engager, qui sinon le
    // garderait sur place pendant son exécution.
    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.allerVers(cible, carte);
        escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = unSeulMonstre(cibleMonstre(cible)), .deControle = {}});
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;
    verifierPointProche(tireurApres, tireurAvant, 0.01f);

    const sf::Vector2f meleeApres = escouade.proieDuMonstre(1, {85.0f, 60.0f})->cercle.centre;
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
    Escouade escouade = uneEscouade({50.0f, 50.0f}); // pas de verrouiller()

    const sf::Vector2f cible{175.0f, 60.0f}; // même configuration que le test précédent
    escouade.allerVers(cible, carte); // sans verrou, attaquer() n'approche pas : l'ordre vient du joueur

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;

    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = unSeulMonstre(cibleMonstre(cible)), .deControle = {}});
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;
    REQUIRE((tireurApres - tireurAvant).length() > 5.0f);
}

// Autodéfense : sans verrou, chaque personnage vivant frappe de lui-même un
// monstre à sa portée. Centre initial (60, 60) ; guerrier en (85, 60)
// (portée 20), tireurs en (35, 35) et (35, 85) (portée 120).

TEST_CASE("Sans verrou, tous les personnages frappent un monstre a leur portee, sans bouger") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const sf::Vector2f centreInitial = escouade.getCentre();

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, {EntiteVue{4, cibleMonstre({85.0f, 60.0f})}}, carte, generateur);
    escouade.mettreAJour(1.0f / 60.0f, carte, {.physiques = {}, .ciblables = {EntiteVue{4, cibleMonstre({85.0f, 60.0f})}}, .deControle = {}});

    REQUIRE(coups.size() == 3);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 4);
        REQUIRE(coup.degats == s_degatsBaseDeTest);
    }
    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("Sans verrou, un monstre hors de portee de tous ne recoit aucun coup") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.attaquer(s_dureeTic, {EntiteVue{1, cibleMonstre({300.0f, 60.0f})}}, carte, generateur).empty());
}

TEST_CASE("Sans verrou, seuls les tireurs frappent un monstre eloigne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    // À 92 px bord à bord des tireurs, à 55 px du guerrier.
    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, {EntiteVue{1, cibleMonstre({150.0f, 60.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
}

TEST_CASE("Un verrou hors de portee n'empeche pas de frapper un autre monstre a portee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(2);

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade,
        {EntiteVue{1, cibleMonstre({150.0f, 60.0f})}, EntiteVue{2, cibleMonstre({300.0f, 60.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 1);
    }
}

TEST_CASE("Le verrou a portee passe avant un monstre qui est a portee aussi") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(2);

    // Le monstre 1 est plus proche des tireurs, mais le 2 est verrouillé.
    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade,
        {EntiteVue{1, cibleMonstre({150.0f, 60.0f})}, EntiteVue{2, cibleMonstre({150.0f, 90.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 2);
    }
}

TEST_CASE("Deux personnages peuvent frapper deux monstres differents dans la meme image") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 150.0f});

    // Le monstre 1 est à portée du tireur du haut seulement, le 2 du tireur du bas seulement
    // (tous deux dans la carte, en vue : la ligne de vue exige des points dans la carte).
    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade,
        {EntiteVue{1, cibleMonstre({140.0f, 50.0f})}, EntiteVue{2, cibleMonstre({140.0f, 270.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre != coups[1].idMonstre);
}

TEST_CASE("Une escouade dont tous les personnages sont morts ne frappe plus") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    infliger(escouade, 1, {85.0f, 35.0f}, 200);
    infliger(escouade, 1, {85.0f, 85.0f}, 200);
    infliger(escouade, 1, {35.0f, 35.0f}, 200);
    infliger(escouade, 1, {35.0f, 85.0f}, 200);

    REQUIRE(escouade.attaquer(s_dureeTic, {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}}, carte, generateur).empty());
}

// Temps d'exécution de l'attaque de base (0,15 s) : on frappe immobile, le
// coup engagé part à la fin de l'exécution. Les boucles suivent l'ordre d'une
// image de Partie : attaquer, ordre du joueur, mettreAJour.

namespace {

// Un tic d'attaquer() ; renvoie le total des dégâts des coups partis à ce tic.
int attaquerCibleUnTic(Escouade& escouade, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    int total = 0;
    for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
        total += coup.degats;
    }
    return total;
}

// Numéro du tic (à partir de 0) où le premier coup part, -1 s'il n'en part
// aucun dans la durée donnée.
int ticDuPremierCoup(Escouade& escouade, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur,
                     float duree) {
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        if (!escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty()) {
            return i;
        }
        escouade.mettreAJour(s_dureeTic, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }
    return -1;
}

}  // namespace

TEST_CASE("A l'arret, le coup part 0,15 s apres son engagement") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};

    const int tic = ticDuPremierCoup(escouade, monstres, carte, generateur, 0.5f);

    // Engagé au tic 0, il part à 0,15 s (9 ou 10 tics après), pas avant.
    REQUIRE(tic >= nombreDeTics(0.15f));
    REQUIRE(tic <= nombreDeTics(0.15f) + 2);
}

TEST_CASE("Aucun coup n'est engage pendant que l'ancre marche sous ordre du joueur") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    // Un monstre à portée de tir des tireurs pendant toute la marche (en vue, à côté du trajet).
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({200.0f, 120.0f})}};

    // L'ordre n'est donné qu'une fois : seul le mouvement des personnages (et
    // non l'annulation par un nouvel ordre) les empêche d'engager un coup.
    escouade.allerVers({360.0f, 60.0f}, carte);
    int coups = 0;
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        coups += static_cast<int>(escouade.attaquer(s_dureeTic, monstres, carte, generateur).size());
        escouade.mettreAJour(s_dureeTic, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }

    REQUIRE(coups == 0);
}

TEST_CASE("Une fois l'escouade arretee, les coups reprennent") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({200.0f, 120.0f})}};
    escouade.allerVers({200.0f, 60.0f}, carte);
    simuler(escouade, carte, 3.0f);  // largement assez pour arriver et se mettre en place

    REQUIRE(ticDuPremierCoup(escouade, monstres, carte, generateur, 0.5f) >= 0);
}

TEST_CASE("Un ordre de deplacement pendant l'execution annule le coup, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    for (int i = 0; i < 5; ++i) {
        REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
    }

    escouade.allerVers(escouade.getCentre(), carte);  // un ordre, sans bouger l'ancre

    // Rien ne part du coup annulé...
    REQUIRE(ticDuPremierCoup(escouade, monstres, carte, generateur, 0.1f) == -1);
    // ... et l'attaque, prête, repart tout de suite : un nouveau coup en 0,15 s.
    // Avec une recharge consommée (au moins 0,3 s), il aurait été plus tardif.
    const int tic = ticDuPremierCoup(escouade, monstres, carte, generateur, 0.3f);
    REQUIRE(tic >= 0);
    REQUIRE(tic <= nombreDeTics(0.15f) + 2);
}

TEST_CASE("Un coup engage part meme si le monstre s'est eloigne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> proche = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    const std::vector<EntiteVue> loin = {EntiteVue{1, cibleMonstre({380.0f, 380.0f})}};
    for (int i = 0; i < 3; ++i) {
        REQUIRE(attaquerCibleUnTic(escouade, proche, carte, generateur) == 0);
    }

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += attaquerCibleUnTic(escouade, loin, carte, generateur);
    }

    REQUIRE(total == 3 * s_degatsBaseDeTest);  // les 3 personnages, sur un monstre devenu hors de portée
}

TEST_CASE("La disparition de la cible annule le coup engage, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    REQUIRE(attaquerCibleUnTic(escouade, monstres, carte, generateur) == 0);

    // Le monstre est mort : plus de monstre dans la liste.
    REQUIRE(ticDuPremierCoup(escouade, {}, carte, generateur, 0.3f) == -1);

    // Un nouveau monstre : le coup repart au plus tard 0,15 s plus tard.
    const int tic = ticDuPremierCoup(escouade, {EntiteVue{2, cibleMonstre({85.0f, 60.0f})}}, carte, generateur, 0.3f);
    REQUIRE(tic >= 0);
    REQUIRE(tic <= nombreDeTics(0.15f) + 2);
}

TEST_CASE("La mort de l'attaquant annule son coup") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    REQUIRE(attaquerCibleUnTic(escouade, monstres, carte, generateur) == 0);

    infliger(escouade, 1, {85.0f, 60.0f}, 1000);  // tue le guerrier en (85, 60)

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += attaquerCibleUnTic(escouade, monstres, carte, generateur);
    }
    REQUIRE(total == 2 * s_degatsBaseDeTest);  // les 2 autres personnages seulement
}

TEST_CASE("Un skill qui part annule le coup en execution de son lanceur") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    REQUIRE(attaquerCibleUnTic(escouade, monstres, carte, generateur) == 0);

    // Guerrier : le skill commence son activation (il part à 0,3 s, après la fenêtre ci-dessous).
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carte, generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += attaquerCibleUnTic(escouade, monstres, carte, generateur);
    }
    REQUIRE(total == 2 * s_degatsBaseDeTest);  // le guerrier du skill ne frappe pas : les 2 autres seulement
}

TEST_CASE("recevoirCoup frappe le personnage de la place donnee, pas le plus proche") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    // Origine à côté du guerrier de la place 1 (85, 60) ; le coup vise la place 3 (35, 85).
    escouade.recevoirCoup(1, 3, 1000);

    REQUIRE(escouade.cerclesDesVivants().size() == 2);
    REQUIRE(escouade.placeEstVivante(1));
    REQUIRE_FALSE(escouade.placeEstVivante(3));
}

TEST_CASE("pointsDeVieDuPersonnage donne les points de vie de la place, vide si la place est invalide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::optional<int> guerrierAvant = escouade.pointsDeVieDuPersonnage(1);
    const std::optional<int> pretreAvant = escouade.pointsDeVieDuPersonnage(2);
    escouade.recevoirCoup(1, 2, 10);

    REQUIRE(escouade.pointsDeVieDuPersonnage(1) == guerrierAvant);
    REQUIRE(escouade.pointsDeVieDuPersonnage(2) == *pretreAvant - 10);  // le prêtre n'a pas d'armure
    REQUIRE_FALSE(escouade.pointsDeVieDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.pointsDeVieDuPersonnage(4).has_value());
}

TEST_CASE("entitesDeControle donne la place, le cercle et la zone de chaque vivant") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::vector<EntiteDeControle> entites = escouade.entitesDeControle();

    REQUIRE(entites.size() == 3);
    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    for (std::size_t i = 0; i < entites.size(); ++i) {
        REQUIRE(entites[i].id == static_cast<int>(i + 1));
        REQUIRE(entites[i].cercle.centre == cercles[i].centre);
        REQUIRE(entites[i].cercle.rayon == cercles[i].rayon);
    }
    REQUIRE(entites[0].zone == 20.0f);  // le guerrier
    REQUIRE(entites[1].zone == 0.0f);   // le prêtre
    REQUIRE(entites[2].zone == 0.0f);   // le mage
}

TEST_CASE("entitesDeControle exclut les morts mais garde la place des autres") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 1, 1000);  // le guerrier meurt

    const std::vector<EntiteDeControle> entites = escouade.entitesDeControle();

    REQUIRE(entites.size() == 2);
    REQUIRE(entites[0].id == 2);
    REQUIRE(entites[1].id == 3);
}

TEST_CASE("puissanceDeLAttaqueDeBase : les degats de la place, vide si la place est invalide") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.puissanceDeLAttaqueDeBase(1) == s_degatsBaseDeTest);
    REQUIRE(escouade.puissanceDeLAttaqueDeBase(3) == s_degatsBaseDeTest);
    REQUIRE_FALSE(escouade.puissanceDeLAttaqueDeBase(0).has_value());
    REQUIRE_FALSE(escouade.puissanceDeLAttaqueDeBase(4).has_value());
}

TEST_CASE("proieDuMonstre donne la place du personnage vise") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.proieDuMonstre(1, {85.0f, 35.0f})->id == 1);
    REQUIRE(escouade.proieDuMonstre(1, {35.0f, 85.0f})->id == 3);
    escouade.provoquer(1, 2, 4.0f);
    REQUIRE(escouade.proieDuMonstre(1, {35.0f, 85.0f})->id == 2);
}

TEST_CASE("candidatsDeFrappe sans provocation donne tous les vivants avec leur place") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 3);
    for (std::size_t i = 0; i < candidats.size(); ++i) {
        REQUIRE(candidats[i].id == static_cast<int>(i + 1));
    }
    verifierPointProche(candidats[0].cercle.centre, escouade.cerclesDesVivants()[0].centre);
}

TEST_CASE("candidatsDeFrappe exclut les morts") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 3, 1000);

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 2);
    for (const EntiteVue& candidat : candidats) {
        REQUIRE(candidat.id != 3);
    }
}

TEST_CASE("candidatsDeFrappe d'un monstre provoque ne donne que le provocateur") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.provoquer(1, 2, 4.0f);

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 1);
    REQUIRE(candidats[0].id == 2);
    REQUIRE(escouade.candidatsDeFrappe(2).size() == 3);
}

TEST_CASE("candidatsDeFrappe revient a tous les vivants si le provocateur est mort") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.provoquer(1, 2, 4.0f);
    escouade.recevoirCoup(9, 2, 1000);

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 2);
    for (const EntiteVue& candidat : candidats) {
        REQUIRE(candidat.id != 2);
    }
}

TEST_CASE("candidatsDeFrappe est vide si personne n'est vivant") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    for (std::size_t place = 1; place <= 3; ++place) {
        escouade.recevoirCoup(9, place, 1000);
    }

    REQUIRE(escouade.candidatsDeFrappe(1).empty());
}
