// ./tests/monstres/test_Monstre_etats.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/GenerateurDonjon.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"
#include "aides_monstre.hpp"

// Monstre centré en (100, 100), rayon de détection 200 px.

TEST_CASE("Monstre.mettreAJour approche une cible dans le rayon de detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const sf::Vector2f cible(300.0f, 100.0f);
    const float distanceInitiale = (cible - monstre.getCentre()).length();

    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    const float distanceApres = (cible - monstre.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("Monstre.mettreAJour ignore une cible hors du rayon de detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const sf::Vector2f cible(400.0f, 100.0f); // 300 px, hors des 200 px de détection
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour reste immobile sans cible") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, std::nullopt, carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour s'arrete a portee d'attaque sans atteindre la cible") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const sf::Vector2f cible(300.0f, 100.0f);

    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distanceFinale = (cible - monstre.getCentre()).length();
    REQUIRE(distanceFinale <= 30.0f); // portée (15 px) + rayon du monstre (15 px)
    REQUIRE(monstre.getCentre() != cible);
}

// Territoire : une fois détectée (<= 200 px), une cible reste poursuivie
// tant que le MONSTRE lui-même ne s'éloigne pas de plus de 600 px de son
// poste (m_rayonTerritoire) ; peu importe à quelle distance la cible est.

TEST_CASE("Monstre.mettreAJour continue de poursuivre une cible qui s'eloigne, tant que le territoire n'est pas franchi") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection
    const sf::Vector2f centreApresDetection = monstre.getCentre();

    const sf::Vector2f cibleEloignee(400.0f, 100.0f); // hors détection, mais le monstre reste dans son territoire
    const float distanceAvant = (cibleEloignee - centreApresDetection).length();
    ecoulerTemps(monstre, 0.2f, pointCible(cibleEloignee), carte);
    const float distanceApres = (cibleEloignee - monstre.getCentre()).length();

    REQUIRE(distanceApres < distanceAvant);
}

TEST_CASE("Monstre.mettreAJour rejoint a portee une proie a 300 px de son poste, et y reste") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cible(400.0f, 100.0f); // 300 px du poste, dans le territoire (600 px)
    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distance = (cible - monstre.getCentre()).length();
    REQUIRE(distance <= 30.0f); // à portée de m_attaque (15 px) + rayon du monstre (15 px)
    REQUIRE_FALSE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f)); // pas revenu au poste
}

TEST_CASE("Monstre.mettreAJour retourne a son poste si la poursuite l'eloigne trop de son territoire") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cibleLointaine(1100.0f, 100.0f); // 1000 px du poste, hors du territoire
    ecoulerTemps(monstre, 20.0f, pointCible(cibleLointaine), carte); // durée généreuse : sort du territoire, puis rentre

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour ignore une proie hors de son territoire meme proche de lui pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    // Proie à 700 px du poste : pousse le monstre hors de son territoire
    // (déclenchant le retour), puis reste assez proche de lui en tout début
    // de retour (moins de 200 px) sans jamais être elle-même dans le
    // territoire (toujours à plus de 600 px du poste) — elle ne doit donc
    // jamais être redétectée.
    ecoulerTemps(monstre, 20.0f, pointCible({800.0f, 100.0f}), carte); // durée généreuse

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour ne redetecte pas une proie revenue dans son territoire pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    ecoulerTemps(monstre, 4.0f, pointCible({900.0f, 100.0f}), carte); // franchit le territoire : début du retour
    // Laisse le retour ramener le monstre assez loin de la frontière pour que la proie, 100 px plus loin, soit dans le territoire.
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > definitionDeMonstre(CategorieDeMonstre::Infanterie).rayonTerritoire - 150.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    const sf::Vector2f centrePendantLeRetour = monstre.getCentre();

    // Proie replacée dans le territoire (< 600 px du poste) et à moins de
    // 200 px du monstre : en retour, il l'ignore et continue de rentrer.
    const sf::Vector2f cibleProche(centrePendantLeRetour.x + 100.0f, centrePendantLeRetour.y);
    const float distanceAvant = (cibleProche - centrePendantLeRetour).length();
    ecoulerTemps(monstre, 0.3f, pointCible(cibleProche), carte);
    const float distanceApres = (cibleProche - monstre.getCentre()).length();

    REQUIRE(distanceApres > distanceAvant);
}

TEST_CASE("Monstre.mettreAJour redetecte normalement une proie une fois arrive a son poste") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    ecoulerTemps(monstre, 20.0f, std::nullopt, carte);                // la proie disparaît : retour, puis arrivée
    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.mettreAJour retourne a son poste si la cible disparait apres detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    ecoulerTemps(monstre, 10.0f, std::nullopt, carte); // durée généreuse : retour

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

// Coup reçu : un monstre touché dans son territoire poursuit sa proie
// habituelle, même hors du rayon de détection (200 px).

TEST_CASE("Monstre touche en garde poursuit une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    subir(monstre, 10);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre non touche reste en garde devant une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre.recevoirDegats de 0 ne reveille pas le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    subir(monstre, 0);
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre touche sans proie reste en garde et le coup est consomme") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    subir(monstre, 10);
    ecoulerTemps(monstre, 1.0f / 60.0f, std::nullopt, carte);
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre touche en poursuite continue de poursuivre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    const float xAvant = monstre.getCentre().x;

    subir(monstre, 10);
    ecoulerTemps(monstre, 0.3f, pointCible({400.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > xAvant);
}

TEST_CASE("Monstre.alerter en garde dans son territoire poursuit une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    monstre.alerter();
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte); // 250 px, hors détection

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.alerter hors de son territoire ne reveille pas le monstre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    amenerEnRetourHorsTerritoire(monstre, carte);
    const float xAvant = monstre.getCentre().x;

    monstre.alerter();
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // continue de rentrer, à l'ouest
}

TEST_CASE("Monstre.alerter en poursuite n'a aucun effet") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    const float xAvant = monstre.getCentre().x;

    monstre.alerter();
    ecoulerTemps(monstre, 0.3f, pointCible({400.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > xAvant);
}

// Ligne de vue : la détection et la frappe exigent la vue ; la poursuite non.

TEST_CASE("Monstre ne detecte pas une proie a portee de detection derriere un mur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre detecte la meme distance de proie quand rien ne la cache") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 0.5f, pointCible({200.0f, 260.0f}), carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre en poursuite continue de suivre une proie qui disparait derriere le mur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    ecoulerTemps(monstre, 0.1f, pointCible({200.0f, 260.0f}), carte);  // détection
    const float xApresDetection = monstre.getCentre().x;

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);  // proie cachée

    REQUIRE(monstre.getCentre().x > xApresDetection);  // il avance vers elle (un retour au poste ramènerait x vers 100)
}

TEST_CASE("Monstre touche s'active meme sans voir son agresseur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    subir(monstre, 10);
    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre en poursuite contourne un autre monstre immobile") {
    const TileMap carte = carteDegagee10x10();
    const std::vector<Cercle> autre = {{{150.0f, 200.0f}, 15.0f}};
    Monstre monstre({60.0f, 200.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 6.0f, pointCible({240.0f, 200.0f}), carte, autre, autre);

    REQUIRE(monstre.getCentre().x > 150.0f); // passé de l'autre côté de l'autre monstre
}

TEST_CASE("Monstre en poursuite sans cercle a contourner reste bloque derriere un autre monstre") {
    const TileMap carte = carteDegagee10x10();
    const std::vector<Cercle> autre = {{{150.0f, 200.0f}, 15.0f}};
    Monstre monstre({60.0f, 200.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    ecoulerTemps(monstre, 6.0f, pointCible({240.0f, 200.0f}), carte, autre, {});

    REQUIRE(monstre.getCentre().x <= 120.0f);
}

namespace {
// Deux salles reliées par un couloir en L de la largeur du générateur : A (colonnes 1 à 6, lignes 1 à 6) et B (colonnes
// 10 à 15, lignes 8 à 13). Un monstre en A (centre de la salle) et sa proie en B sont à moins de 500 px à vol
// d'oiseau, donc dans son territoire, mais loin de sa détection : il faut l'alerter.
TileMap carteDeDeuxSallesReliees(SensCouloir sens) {
    const ParametresDonjon parametres;
    const std::vector<Salle> salles = {{1, 1, 6, 6}, {10, 8, 6, 6}};
    std::vector<std::string> plan(16, std::string(18, '#'));
    for (const Salle& salle : salles) {
        creuserSalle(plan, salle);
    }
    creuserCouloir(plan, salles[0].centre(), salles[1].centre(), sens, parametres.largeurCouloir);
    return TileMap(plan, parametres.tailleCase);
}
}  // namespace

TEST_CASE("chaque categorie de monstre traverse un couloir en L jusqu'a sa proie") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        for (const SensCouloir sens : {SensCouloir::HorizontalPuisVertical, SensCouloir::VerticalPuisHorizontal}) {
            INFO("categorie " << static_cast<int>(categorie) << ", sens " << static_cast<int>(sens));
            const TileMap carte = carteDeDeuxSallesReliees(sens);
            const sf::Vector2f poste = carte.centreDeCase(Salle{1, 1, 6, 6}.centre());
            const Cercle proie = cibleMonstre(carte.centreDeCase(Salle{10, 8, 6, 6}.centre()));
            Monstre monstre(poste, IdMonstre{1}, 0, definitionDeMonstre(categorie));
            monstre.alerter();

            ecoulerTemps(monstre, 20.0f, proie, carte);

            REQUIRE(monstre.peutAtteindre(proie, carte));
        }
    }
}
