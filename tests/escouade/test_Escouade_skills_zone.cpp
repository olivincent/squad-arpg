// ./tests/escouade/test_Escouade_skills_zone.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/TileMap.hpp"
#include "combat/TypeSkill.hpp"

// Skill de zone (touche 4) : lanceur (mage) en (185, 235), 15 dégâts, recharge 8 s,
// zone de 60 px de rayon ; monstres de rayon 15, donc touchés jusqu'à 75 px
// de distance entre centres.

TEST_CASE("skillEstPret : vrai pour un lanceur vivant dont le skill est charge") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("skillEstPret : faux pour une place invalide") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE_FALSE(escouade.skillEstPret(AdresseSkill{0, IdSkill::FrappePuissante}));
    REQUIRE_FALSE(escouade.skillEstPret(AdresseSkill{5, IdSkill::FrappePuissante}));
}

TEST_CASE("skillEstPret : faux si le lanceur est mort") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    infliger(escouade, 1, {185.0f, 235.0f}, s_coupMortel);  // tue la place 3

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("skillEstPret : faux pendant la recharge, vrai apres 8 s") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));

    escouade.mettreAJour(8.1f, carte, {});
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : les monstres dans la zone sont touches, celui hors zone est epargne") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({300.0f, 300.0f})},
        EntiteVue{2, cibleMonstre({330.0f, 300.0f})},
        EntiteVue{3, cibleMonstre({500.0f, 300.0f})},
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre == 1);
    REQUIRE(coups[0].attaque.degats.total() == s_degatsZoneDeTest);
    REQUIRE(coups[1].idMonstre == 2);
    REQUIRE(coups[1].attaque.degats.total() == s_degatsZoneDeTest);
}

TEST_CASE("Zone : un monstre qui touche tout juste le bord de la zone est touche") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({375.0f, 300.0f})},
        EntiteVue{2, cibleMonstre({300.0f, 376.0f})},
    };

    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Zone : une zone vide part quand meme et lance la recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {EntiteVue{1, cibleMonstre({700.0f, 300.0f})}}, carte, generateur).empty());

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : un point a portee (250 px bord a bord) part tout de suite") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Lanceur (mage, place 3) en (185, 235), rayon 10 : (435, 235) est à 240 px du bord.
    const std::vector<Coup> coups = declencherZoneEtAttendre(escouade, s_adresseExplosion, {435.0f, 235.0f}, {EntiteVue{1, cibleMonstre({435.0f, 235.0f})}}, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].attaque.nature == NatureDAttaque::Zone);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : un point hors de portee met le sort en attente, sans coup ni recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({650.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    // Un sort est en attente : un autre skill est refusé.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : le lanceur se rapproche, puis le sort part au point fixe") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({650.0f, 235.0f})},
        EntiteVue{2, cibleMonstre({650.0f, 400.0f})},
    };
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 300) {
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
    // Plus rien en attente : un autre skill peut maintenant partir.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : elle touche les monstres presents au depart, pas ceux du clic") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, {EntiteVue{1, cibleMonstre({650.0f, 235.0f})}}, carte, generateur).empty());

    const std::vector<EntiteVue> ailleurs = {
        EntiteVue{1, cibleMonstre({650.0f, 500.0f})},
        EntiteVue{2, cibleMonstre({660.0f, 235.0f})},
    };
    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, ailleurs, carte, generateur)) {
            if (coup.attaque.degats.total() == s_degatsZoneDeTest) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = ailleurs, .deControle = {}});
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 2);
}

TEST_CASE("Zone en attente : la mort du lanceur l'annule sans recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({650.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());
    infliger(escouade, 1, {185.0f, 235.0f}, s_coupMortel);  // tue la place 3

    for (int i = 0; i < 300; ++i) {
        REQUIRE(escouade.attaquer(s_dureeTic, monstres, carte, generateur).empty());
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }

    // Plus rien en attente : une provocation est acceptée.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("Zone en attente : verrouiller, deverrouiller et un ordre de deplacement ne l'annulent pas") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {
        EntiteVue{1, cibleMonstre({650.0f, 235.0f})},
        EntiteVue{2, cibleMonstre({300.0f, 400.0f})},
    };
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {650.0f, 235.0f}, monstres, carte, generateur).empty());

    escouade.verrouiller(2);
    escouade.verrouiller(1);
    escouade.deverrouiller();
    escouade.allerVers({200.0f, 500.0f}, carte);

    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.attaque.degats.total() == s_degatsZoneDeTest) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {.physiques = {}, .ciblables = monstres, .deControle = {}});
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
}

TEST_CASE("Zone : refusee pendant la recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({300.0f, 300.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur).size() == 1);

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur).empty());
}

TEST_CASE("Zone : refusee si le lanceur est mort") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    infliger(escouade, 1, {185.0f, 235.0f}, s_coupMortel);  // tue la place 3

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {EntiteVue{1, cibleMonstre({300.0f, 300.0f})}}, carte, generateur).empty());
}

TEST_CASE("Zone : refusee si un skill est en attente") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({300.0f, 300.0f})}};
    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carteOuverte(), generateur).empty());  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, monstres, carte, generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
}

TEST_CASE("Zone : refusee pour une place invalide ou dont le skill n'est pas de zone") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({235.0f, 235.0f})}};

    REQUIRE(declencherZoneEtAttendre(escouade, AdresseSkill{0, IdSkill::FrappePuissante}, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, AdresseSkill{5, IdSkill::FrappePuissante}, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseFrappe, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseProvocation, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseSoin, {235.0f, 235.0f}, monstres, carte, generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
    REQUIRE(escouade.skillEstPret(s_adresseSoin));
}

TEST_CASE("declencherSkill sans point refuse un skill de zone, et choisirCible ne renvoie rien") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({185.0f, 235.0f})}};

    REQUIRE(declencherEtAttendre(escouade, s_adresseExplosion, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Zone, s_adresseExplosion, monstres, carteOuverte()).has_value());
}

TEST_CASE("Zone : relance la recharge de l'attaque de base du lanceur") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const TileMap carte = carteDegageeEntouree();
    // Ce monstre est à portée de tir des places 2 et 3 (120 px) et hors de
    // portée du guerrier : sans le skill, les deux tireurs frapperaient.
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({285.0f, 235.0f})}};
    REQUIRE(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur).empty());

    const std::vector<Coup> coups = attaquerJusquAuDepart(escouade, monstres, carte, generateur);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].attaque.degats.total() == s_degatsBaseDeTest);
}

TEST_CASE("rayonZoneDuSkill : celui de l'explosion et de la provocation, 0 sans zone ou place invalide") {
    Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(escouade.rayonZoneDuSkill(s_adresseExplosion) == s_rayonZoneDeTest);
    REQUIRE(escouade.rayonZoneDuSkill(s_adresseProvocation) == s_rayonProvocationDeTest);
    REQUIRE(escouade.rayonZoneDuSkill(s_adresseFrappe) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(AdresseSkill{0, IdSkill::FrappePuissante}) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(AdresseSkill{5, IdSkill::FrappePuissante}) == 0.0f);
}

// Écart accepté à l'étape 82.4 : le lanceur d'un skill de zone en attente va à un point, il n'approche
// aucun monstre ; il contourne donc aussi le monstre verrouillé (avant, en combat, personne ne contournait
// le verrou : marchant droit dessus, il restait calé contre lui). Seul le mage est vivant, en (185, 235) ;
// le verrou est sur son chemin vers le point, assez près pour que l'ancre reste à l'arrêt (le mage
// l'atteint déjà).
TEST_CASE("Zone en attente : en combat verrouille, le lanceur contourne le monstre verrouille") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    infliger(escouade, 9, {235.0f, 210.0f}, s_coupMortel);  // tue le guerrier
    infliger(escouade, 9, {185.0f, 185.0f}, s_coupMortel);  // tue le prêtre
    const Cercle verrou = cibleMonstre({300.0f, 235.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, verrou}};
    escouade.verrouiller(1);
    REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, {600.0f, 235.0f}, monstres, carte, generateur).empty());  // hors de portée : en attente

    for (int i = 0; i < nombreDeTics(5.0f) && escouade.skillEstPret(s_adresseExplosion); ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur)); // avance l'état : les coups ne sont pas l'objet du test
        escouade.mettreAJour(s_dureeTic, carte, {.physiques = monstres, .ciblables = monstres, .deControle = {}});
    }

    REQUIRE_FALSE(escouade.skillEstPret(s_adresseExplosion));  // arrivé à portée du point : le sort est lancé
    REQUIRE(escouade.cerclesDesVivants()[0].centre.x > verrou.centre.x + verrou.rayon);  // passé de l'autre côté du verrou
}
