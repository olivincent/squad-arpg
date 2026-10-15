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
#include "EntiteVue.hpp"
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
        escouade.mettreAJour(dt, carte, {});
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
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

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
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

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
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

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

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;
    const sf::Vector2f meleeAvant = escouade.proieDuMonstre(1, {85.0f, 60.0f})->cercle.centre;

    // Comme dans Partie, l'ordre du joueur est redonné à chaque image (bouton
    // tenu) : il annule le coup que le tireur vient d'engager, qui sinon le
    // garderait sur place pendant son exécution.
    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.allerVers(cible, carte);
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;
    verifierPointProche(tireurApres, tireurAvant, 0.01f);

    const sf::Vector2f meleeApres = escouade.proieDuMonstre(1, {85.0f, 60.0f})->cercle.centre;
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

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;

    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->cercle.centre;
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

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, {EntiteVue{4, cibleMonstre({85.0f, 60.0f})}}, carte, generateur);
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

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

    REQUIRE(escouade.attaquer(s_dureeTic, {EntiteVue{1, cibleMonstre({300.0f, 60.0f})}}, carte, generateur).empty());
}

TEST_CASE("Sans verrou, seuls les tireurs frappent un monstre eloigne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});

    // À 92 px bord à bord des tireurs, à 44 px des mêlées.
    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, {EntiteVue{1, cibleMonstre({150.0f, 60.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 2);
}

TEST_CASE("Un verrou hors de portee n'empeche pas de frapper un autre monstre a portee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
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
    Escouade escouade({50.0f, 50.0f});
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
    Escouade escouade({50.0f, 150.0f});

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
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 35.0f}, 200);
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 200);
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 200);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 200);

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
        escouade.mettreAJour(s_dureeTic, carte, {});
    }
    return -1;
}

}  // namespace

TEST_CASE("A l'arret, le coup part 0,15 s apres son engagement") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};

    const int tic = ticDuPremierCoup(escouade, monstres, carte, generateur, 0.5f);

    // Engagé au tic 0, il part à 0,15 s (9 ou 10 tics après), pas avant.
    REQUIRE(tic >= nombreDeTics(0.15f));
    REQUIRE(tic <= nombreDeTics(0.15f) + 2);
}

TEST_CASE("Aucun coup n'est engage pendant que l'ancre marche sous ordre du joueur") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    // Un monstre à portée de tir des tireurs pendant toute la marche (en vue, à côté du trajet).
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({200.0f, 120.0f})}};

    // L'ordre n'est donné qu'une fois : seul le mouvement des personnages (et
    // non l'annulation par un nouvel ordre) les empêche d'engager un coup.
    escouade.allerVers({360.0f, 60.0f}, carte);
    int coups = 0;
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        coups += static_cast<int>(escouade.attaquer(s_dureeTic, monstres, carte, generateur).size());
        escouade.mettreAJour(s_dureeTic, carte, {});
    }

    REQUIRE(coups == 0);
}

TEST_CASE("Une fois l'escouade arretee, les coups reprennent") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({200.0f, 120.0f})}};
    escouade.allerVers({200.0f, 60.0f}, carte);
    simuler(escouade, carte, 3.0f);  // largement assez pour arriver et se mettre en place

    REQUIRE(ticDuPremierCoup(escouade, monstres, carte, generateur, 0.5f) >= 0);
}

TEST_CASE("Un ordre de deplacement pendant l'execution annule le coup, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
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
    Escouade escouade({50.0f, 50.0f});
    const std::vector<EntiteVue> proche = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    const std::vector<EntiteVue> loin = {EntiteVue{1, cibleMonstre({380.0f, 380.0f})}};
    for (int i = 0; i < 3; ++i) {
        REQUIRE(attaquerCibleUnTic(escouade, proche, carte, generateur) == 0);
    }

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += attaquerCibleUnTic(escouade, loin, carte, generateur);
    }

    REQUIRE(total == 20);  // 4 personnages x 5 dégâts, sur un monstre devenu hors de portée
}

TEST_CASE("La disparition de la cible annule le coup engage, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
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
    Escouade escouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    REQUIRE(attaquerCibleUnTic(escouade, monstres, carte, generateur) == 0);

    escouade.recevoirDegats(1, {85.0f, 35.0f}, 1000);  // tue le mêlée en (85, 35)

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += attaquerCibleUnTic(escouade, monstres, carte, generateur);
    }
    REQUIRE(total == 15);  // les 3 autres personnages seulement
}

TEST_CASE("Un skill qui part annule le coup en execution de son lanceur") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Escouade escouade({50.0f, 50.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({85.0f, 60.0f})}};
    REQUIRE(attaquerCibleUnTic(escouade, monstres, carte, generateur) == 0);

    // Mêlée avant-gauche : le skill commence son activation (il part à 0,3 s, après la fenêtre ci-dessous).
    REQUIRE(escouade.declencherSkill(1, monstres, carte, generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(1));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += attaquerCibleUnTic(escouade, monstres, carte, generateur);
    }
    REQUIRE(total == 15);  // le mêlée du skill ne frappe pas : les 3 autres seulement
}

TEST_CASE("recevoirCoup frappe le personnage de la place donnee, pas le plus proche") {
    Escouade escouade({50.0f, 50.0f});

    // Origine à côté du mêlée de la place 1 (85, 35) ; le coup vise la place 4 (35, 85).
    escouade.recevoirCoup(1, 4, {85.0f, 35.0f}, 1000);

    REQUIRE(escouade.cerclesDesVivants().size() == 3);
    REQUIRE(escouade.placeEstVivante(1));
    REQUIRE_FALSE(escouade.placeEstVivante(4));
}

TEST_CASE("proieDuMonstre donne la place du personnage vise") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.proieDuMonstre(1, {85.0f, 35.0f})->id == 1);
    REQUIRE(escouade.proieDuMonstre(1, {35.0f, 85.0f})->id == 4);
    escouade.provoquer(1, 2, 4.0f);
    REQUIRE(escouade.proieDuMonstre(1, {35.0f, 85.0f})->id == 2);
}

TEST_CASE("candidatsDeFrappe sans provocation donne tous les vivants avec leur place") {
    Escouade escouade({50.0f, 50.0f});

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 4);
    for (std::size_t i = 0; i < candidats.size(); ++i) {
        REQUIRE(candidats[i].id == static_cast<int>(i + 1));
    }
    verifierPointProche(candidats[0].cercle.centre, escouade.cerclesDesVivants()[0].centre);
}

TEST_CASE("candidatsDeFrappe exclut les morts") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirCoup(1, 4, {85.0f, 35.0f}, 1000);

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 3);
    for (const EntiteVue& candidat : candidats) {
        REQUIRE(candidat.id != 4);
    }
}

TEST_CASE("candidatsDeFrappe d'un monstre provoque ne donne que le provocateur") {
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(1, 2, 4.0f);

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 1);
    REQUIRE(candidats[0].id == 2);
    REQUIRE(escouade.candidatsDeFrappe(2).size() == 4);
}

TEST_CASE("candidatsDeFrappe revient a tous les vivants si le provocateur est mort") {
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(1, 2, 4.0f);
    escouade.recevoirCoup(9, 2, {85.0f, 35.0f}, 1000);

    const std::vector<EntiteVue> candidats = escouade.candidatsDeFrappe(1);

    REQUIRE(candidats.size() == 3);
    for (const EntiteVue& candidat : candidats) {
        REQUIRE(candidat.id != 2);
    }
}

TEST_CASE("candidatsDeFrappe est vide si personne n'est vivant") {
    Escouade escouade({50.0f, 50.0f});
    for (std::size_t place = 1; place <= 4; ++place) {
        escouade.recevoirCoup(9, place, {85.0f, 35.0f}, 1000);
    }

    REQUIRE(escouade.candidatsDeFrappe(1).empty());
}
