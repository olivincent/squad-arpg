// ./tests/test_Escouade_vue.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "MonstreVu.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"


// Carte 15 x 10 (cases de 40 px) avec un mur vertical en colonne 6
// (x: [240, 280)) sur les rangées 3 à 7 : le lanceur de la place 4, en
// (185, 235), est d'un côté, et (320, 235) de l'autre. On passe par le haut
// (rangées 0 à 2) ou par le bas (rangées 8 et 9).
namespace {
TileMap carteAvecMurVertical() {
    const std::vector<std::string> plan = {
        "...............",
        "...............",
        "...............",
        "......#........",
        "......#........",
        "......#........",
        "......#........",
        "......#........",
        "...............",
        "...............",
    };
    return TileMap(plan, 40.0f);
}
}  // namespace

TEST_CASE("Zone : un point a portee et en vue part tout de suite (carte avec mur)") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});

    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {100.0f, 235.0f}, {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un point a portee mais derriere un mur met le sort en attente, sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkillDeZone(4, {320.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(4));
    // Un sort est en attente : un autre skill est refusé.
    REQUIRE(escouade.declencherSkill(2, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(2));
}

TEST_CASE("Zone : un point dans un mur est refuse, sans attente ni recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkillDeZone(4, {260.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(4));
    // Rien en attente : une provocation (place 2) est acceptée.
    REQUIRE(escouade.declencherSkill(2, monstres, carte, generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : le lanceur contourne le mur, puis le sort part des qu'il voit le point") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};
    REQUIRE(escouade.declencherSkillDeZone(4, {320.0f, 235.0f}, monstres, carte, generateur).empty());

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 600) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.degats == 15) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {}, generateur);
        ++tics;
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
    REQUIRE(tics > 5);  // le lanceur a dû marcher
    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un monstre dans la zone mais derriere un mur est epargne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({285.0f, 235.0f})},  // dans la zone (60 px), de l'autre côté du mur
    };

    REQUIRE(escouade.declencherSkillDeZone(4, {230.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(4));  // le sort est parti quand même
}

TEST_CASE("Zone : un monstre dans la zone et en vue est touche, celui derriere le mur non") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({285.0f, 235.0f})},  // derrière le mur
        MonstreVu{2, cibleMonstre({200.0f, 235.0f})},  // du même côté que le point
    };

    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {230.0f, 235.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 2);
    REQUIRE(coups[0].degats == 15);
}

TEST_CASE("Autodefense : un monstre a portee mais derriere un mur n'est pas frappe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    // Place 4 en (185, 235) : portée de tir 120 px, monstre à 110 px bord à bord, mur entre les deux.
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};

    REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
}

TEST_CASE("Autodefense : un monstre a portee et en vue est frappe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}};

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE_FALSE(coups.empty());
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Autodefense : le monstre le plus proche est cache, l'autre est frappe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({285.0f, 235.0f})},  // plus proche de la place 4, mais derrière le mur
        MonstreVu{2, cibleMonstre({185.0f, 130.0f})},  // plus loin, en vue
    };

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE_FALSE(coups.empty());
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 2);
    }
}

// Ne laisse en vie que la place 4 (tireur en (185, 235)) : seule sa portée et sa vue comptent.
namespace {
void garderSeulementLaPlace4(Escouade& escouade) {
    escouade.recevoirDegats(1, {235.0f, 185.0f}, 200);
    escouade.recevoirDegats(1, {235.0f, 235.0f}, 200);
    escouade.recevoirDegats(1, {185.0f, 185.0f}, 200);
}
}  // namespace

TEST_CASE("Combat verrouille : une cible a portee mais cachee ne fait pas arreter l'ancre") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    garderSeulementLaPlace4(escouade);
    escouade.verrouiller(1);
    const sf::Vector2f depart = escouade.getCentre();
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};

    for (int i = 0; i < 20; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(0.016f, carte, {}, generateur);
    }

    REQUIRE((escouade.getCentre() - depart).length() > 5.0f);
}

TEST_CASE("Combat verrouille : une cible a portee et en vue fait arreter l'ancre") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    garderSeulementLaPlace4(escouade);
    escouade.verrouiller(1);
    const sf::Vector2f depart = escouade.getCentre();
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}};

    for (int i = 0; i < 20; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(0.016f, carte, {}, generateur);
    }

    REQUIRE((escouade.getCentre() - depart).length() < 1.0f);
}

TEST_CASE("Agresseur a portee mais cache : le personnage va vers lui") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({283.0f, 235.0f})}};
    escouade.recevoirDegats(1, {283.0f, 235.0f}, 1);  // enregistre l'agresseur (touche le plus proche)
    const sf::Vector2f depart = escouade.cerclesDesVivants()[3].centre;
    static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test

    for (int i = 0; i < 20; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(0.016f, carte, {}, generateur);
    }

    REQUIRE((escouade.cerclesDesVivants()[3].centre - depart).length() > 10.0f);
}

// Carte de 300 × 300 px en cases de 2 px, avec deux murs fins posés hors des
// bandes de repli de la formation (escouade en (200, 200), ancre en (210, 210)) :
// - x: [206, 214), y: [170, 198) : entre les places 3 (185, 185) et 1 (235, 185) ;
// - x: [250, 258), y: [150, 230) : à droite du mêlée de la place 1.
namespace {
TileMap carteAvecMursFins() {
    std::vector<std::string> plan(150, std::string(150, '.'));
    for (int ligne = 85; ligne < 99; ++ligne) {
        for (int colonne = 103; colonne < 107; ++colonne) {
            plan[ligne][colonne] = '#';
        }
    }
    for (int ligne = 75; ligne < 115; ++ligne) {
        for (int colonne = 125; colonne < 129; ++colonne) {
            plan[ligne][colonne] = '#';
        }
    }
    return TileMap(plan, 2.0f);
}
}  // namespace

TEST_CASE("Skill de degats avec verrou : un monstre a portee mais cache met le skill en attente, sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade({200.0f, 200.0f});
    // Mêlée de la place 1 en (235, 185) : le monstre est à 15 px bord à bord, mur entre les deux.
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({275.0f, 185.0f})}};
    escouade.verrouiller(1);

    REQUIRE(escouade.declencherSkill(1, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(1));
    // Un skill est en attente : un autre est refusé.
    REQUIRE(escouade.declencherSkill(2, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(2));
}

TEST_CASE("Skill de degats en attente derriere un mur : il part des que la vue est obtenue") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({275.0f, 185.0f})}};
    escouade.verrouiller(1);
    REQUIRE(escouade.declencherSkill(1, monstres, carte, generateur).empty());

    // Tant que le mur est là, le skill reste en attente : seule l'attaque de base du mêlée (5) pourrait partir, et elle ne part pas non plus.
    REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(1));

    // La vue est obtenue (la carte n'a plus de mur) : le skill part avec ses 25 dégâts.
    const std::vector<Coup> coups = escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur);
    REQUIRE_FALSE(coups.empty());
    REQUIRE(coups[0].idMonstre == 1);
    REQUIRE(coups[0].degats == 25);
    REQUIRE_FALSE(escouade.skillEstPret(1));
}

TEST_CASE("Skill de degats sans verrou : un monstre a portee mais cache n'est pas candidat, l'appui est refuse") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({275.0f, 185.0f})}};

    REQUIRE(escouade.declencherSkill(1, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(1));
    // Rien en attente : une provocation (place 2) est acceptée.
    REQUIRE(escouade.declencherSkill(2, monstres, carte, generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Soin d'un allie a portee mais cache : le soin est mis en attente, puis part des que la vue est obtenue") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade({200.0f, 200.0f});
    // Soigneur de la place 3 en (185, 185), allié de la place 1 en (235, 185) : 30 px bord à bord, mur entre les deux.
    escouade.recevoirDegats(1, {235.0f, 185.0f}, 20);

    REQUIRE(escouade.declencherSkill(3, {}, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(3));              // pas encore parti
    REQUIRE(placeAlliePourSoin(escouade) == 1);     // pas encore soigné

    // Le mur est là : rien ne part. La vue est obtenue (carte sans mur) : le soin part.
    escouade.mettreAJour(0.016f, carte, {}, generateur);
    REQUIRE(placeAlliePourSoin(escouade) == 1);
    escouade.mettreAJour(0.016f, carteOuverte(), {}, generateur);

    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
    REQUIRE_FALSE(escouade.skillEstPret(3));
}

TEST_CASE("Provocation : un monstre dans la zone mais derriere un mur n'est pas provoque, la recharge est consommee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade({200.0f, 200.0f});
    // Place 2 en (235, 235), zone de 150 px : le monstre est à 64 px, mur entre les deux.
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({275.0f, 185.0f})}};

    REQUIRE(escouade.declencherSkill(2, monstres, carte, generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Provocation : seuls les monstres dans la zone et en vue sont provoques") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({275.0f, 185.0f})},  // dans la zone, derrière le mur
        MonstreVu{2, cibleMonstre({200.0f, 290.0f})},  // dans la zone, en vue
    };

    REQUIRE(escouade.declencherSkill(2, monstres, carte, generateur).empty());

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 2);
    REQUIRE_FALSE(escouade.skillEstPret(2));
}
