// ./tests/escouade/test_Escouade_vue.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/Skill.hpp"
#include "monde/TileMap.hpp"


// Carte 15 x 10 (cases de 40 px) avec un mur vertical en colonne 6
// (x: [240, 280)) sur les rangées 3 à 7 : le mage (place 3), en
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
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {100.0f, 235.0f}, {EntiteVue{1, cibleMonstre({100.0f, 235.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : un point a portee mais derriere un mur met le sort en attente, sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({320.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {320.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    // Un sort est en attente : un autre skill est refusé.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone : un point dans un mur est refuse, sans attente ni recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({100.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {260.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    // Rien en attente : une provocation est acceptée.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carte, generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : le lanceur contourne le mur, puis le sort part des qu'il voit le point") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({320.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {320.0f, 235.0f}, monstres, carte, generateur).empty());

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 600) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.attaque.degats.total() == s_degatsZoneDeTest) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
        ++tics;
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
    REQUIRE(tics > 5);  // le lanceur a dû marcher
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : un monstre dans la zone mais derriere un mur est epargne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({285.0f, 235.0f})},  // dans la zone (60 px), de l'autre côté du mur
    };

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {230.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));  // le sort est parti quand même
}

TEST_CASE("Zone : un monstre dans la zone et en vue est touche, celui derriere le mur non") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({285.0f, 235.0f})},  // derrière le mur
        EntiteVue{2, cibleMonstre({200.0f, 235.0f})},  // du même côté que le point
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {230.0f, 235.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 2);
    REQUIRE(coups[0].attaque.degats.total() == s_degatsZoneDeTest);
}

TEST_CASE("Autodefense : un monstre a portee mais derriere un mur n'est pas frappe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Le mage en (185, 235) : portée de tir 120 px, monstre à 110 px bord à bord, mur entre les deux.
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({320.0f, 235.0f})}};

    REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
}

TEST_CASE("Autodefense : un monstre a portee et en vue est frappe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({100.0f, 235.0f})}};

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE_FALSE(coups.empty());
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Autodefense : le monstre le plus proche est cache, l'autre est frappe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({285.0f, 235.0f})},  // plus proche du mage, mais derrière le mur
        EntiteVue{2, cibleMonstre({185.0f, 130.0f})},  // plus loin, en vue
    };

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE_FALSE(coups.empty());
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 2);
    }
}

// Ne laisse en vie que le mage (en (185, 235)) : seules sa portée et sa vue comptent.
namespace {
void garderSeulementLeMage(Escouade& escouade) {
    infliger(escouade, 1, {235.0f, 210.0f}, s_coupMortel);
    infliger(escouade, 1, {185.0f, 185.0f}, s_coupMortel);
}
}  // namespace

TEST_CASE("Combat verrouille : une cible a portee mais cachee ne fait pas arreter l'ancre") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    garderSeulementLeMage(escouade);
    escouade.verrouiller(1);
    const sf::Vector2f depart = escouade.getCentre();
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({320.0f, 235.0f})}};

    for (int i = 0; i < 20; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }

    REQUIRE((escouade.getCentre() - depart).length() > 5.0f);
}

TEST_CASE("Combat verrouille : une cible a portee et en vue fait arreter l'ancre") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    garderSeulementLeMage(escouade);
    escouade.verrouiller(1);
    const sf::Vector2f depart = escouade.getCentre();
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({100.0f, 235.0f})}};

    for (int i = 0; i < 20; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }

    REQUIRE((escouade.getCentre() - depart).length() < 1.0f);
}

TEST_CASE("Agresseur a portee mais cache : le personnage va vers lui") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({283.0f, 235.0f})}};
    infliger(escouade, 1, {283.0f, 235.0f}, 1);  // enregistre l'agresseur (touche le plus proche)
    const sf::Vector2f depart = escouade.cerclesDesVivants()[2].centre;
    static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test

    for (int i = 0; i < 20; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }

    REQUIRE((escouade.cerclesDesVivants()[2].centre - depart).length() > 10.0f);
}

// Carte de 300 × 300 px en cases de 2 px, avec deux murs fins posés hors des
// bandes de repli de la formation (escouade en (200, 200), ancre en (210, 210)) :
// - x: [206, 214), y: [170, 198) : entre le prêtre (185, 185) et le guerrier (235, 210) ;
// - x: [250, 258), y: [150, 230) : à droite du guerrier.
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
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Guerrier en (235, 210) : le monstre est à 15 px bord à bord, mur entre les deux.
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({275.0f, 210.0f})}};
    escouade.verrouiller(1);

    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseFrappe));
    // Un skill est en attente : un autre est refusé.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Skill de degats en attente derriere un mur : il part des que la vue est obtenue") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({275.0f, 210.0f})}};
    escouade.verrouiller(1);
    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carte, generateur).empty());

    // Tant que le mur est là, le skill reste en attente : seule l'attaque de base du mêlée pourrait partir, et elle ne part pas non plus.
    REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));

    // La vue est obtenue (la carte n'a plus de mur) : le skill part, avec les dégâts de la frappe.
    const std::vector<Coup> coups = attendreLeDepartDesSkills(escouade, monstres, carteOuverte(), generateur);
    REQUIRE_FALSE(coups.empty());
    REQUIRE(coups[0].idMonstre == 1);
    REQUIRE(coups[0].attaque.degats.total() == s_degatsFrappeDeTest);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));
}

TEST_CASE("Skill de degats sans verrou : un monstre a portee mais cache n'est pas candidat, l'appui est refuse") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({275.0f, 210.0f})}};

    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseFrappe));
    // Rien en attente : une provocation est acceptée.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carte, generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Soin d'un allie a portee mais cache : le soin est mis en attente, puis part des que la vue est obtenue") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Soigneur (prêtre) en (185, 185), allié (guerrier) en (235, 210) : 36 px bord à bord, mur entre les deux.
    infliger(escouade, 1, {235.0f, 210.0f}, 20);

    REQUIRE(declencherEtAttendre(escouade, s_adresseSoin, {}, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseSoin));              // pas encore parti
    REQUIRE(placeAlliePourSoin(escouade) == 1);     // pas encore soigné

    // Le mur est là : rien ne part. La vue est obtenue (carte sans mur) : le soin part.
    escouade.mettreAJour(0.016f, carte, {});
    REQUIRE(placeAlliePourSoin(escouade) == 1);
    escouade.mettreAJour(0.016f, carteOuverte(), {});
    static_cast<void>(attendreLeDepartDesSkills(escouade, {}, carteOuverte(), generateur));  // l'activation avance et le soin part

    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseSoin));
}

TEST_CASE("Provocation : un monstre dans la zone mais derriere un mur n'est pas provoque, la recharge est consommee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Guerrier en (235, 210), zone de 150 px : le monstre est à 40 px, mur entre les deux.
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({275.0f, 210.0f})}};

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carte, generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Provocation : seuls les monstres dans la zone et en vue sont provoques") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteAvecMursFins();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({275.0f, 210.0f})},  // dans la zone, derrière le mur
        EntiteVue{2, cibleMonstre({200.0f, 290.0f})},  // dans la zone, en vue
    };

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carte, generateur).empty());

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 2);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

// Escouade en (200, 200) : le personnage de la place 1 (guerrier) est en (235, 210), rayon 10.
// La zone ci-dessous est à 30 px de son centre, donc à 20 px bord à bord.
TEST_CASE("unVivantPresDe : vrai quand un vivant est a l'ecart voulu ou moins") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const sf::FloatRect zone({265.0f, 195.0f}, {40.0f, 30.0f});

    REQUIRE(escouade.unVivantPresDe(zone, 20.0f));
    REQUIRE_FALSE(escouade.unVivantPresDe(zone, 19.0f));
}

TEST_CASE("unVivantPresDe : un mort pres de la zone ne compte pas") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const sf::FloatRect zone({265.0f, 195.0f}, {40.0f, 30.0f});
    infliger(escouade, 1, {235.0f, 210.0f}, s_coupMortel);

    REQUIRE_FALSE(escouade.unVivantPresDe(zone, 20.0f));
}
