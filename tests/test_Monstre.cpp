// ./tests/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "Cercle.hpp"
#include "Monstre.hpp"
#include "TileMap.hpp"

namespace {

// Même tolérance que les autres fichiers de test (positions accumulées
// en virgule flottante sur plusieurs tics).
void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

// Cible ponctuelle (rayon nul), pour les tests qui ne portent pas sur
// la forme de la cible elle-même.
Cercle pointCible(sf::Vector2f centre) {
    return {centre, 0.0f};
}

TileMap carteDegagee() {
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
    return TileMap(plan, 40.0f);
}

// Dégagée, mais assez longue pour les tests de territoire (une proie à
// 1000 px du poste, par exemple).
TileMap carteLongue() {
    const std::vector<std::string> plan = {
        "..............................",
        "..............................",
        "..............................",
        "..............................",
        "..............................",
    };
    return TileMap(plan, 40.0f);
}

// Simule le temps qui passe par tics de 1/60 s, comme dans main.cpp.
void ecoulerTemps(Monstre& monstre, float duree, std::optional<Cercle> cible, const TileMap& carte) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        monstre.mettreAJour(pas, cible, carte, {});
        tempsRestant -= pas;
    }
}

}  // namespace

// Monstre centré en (100, 100), taille 30x30 : bords en x: [85, 115),
// y: [85, 115).

TEST_CASE("contient renvoie vrai pour un point au centre") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.contient({100.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point à l'extérieur") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE_FALSE(monstre.contient({200.0f, 200.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord gauche") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.contient({85.0f, 100.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord haut") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.contient({100.0f, 85.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord droit") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE_FALSE(monstre.contient({115.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord bas") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE_FALSE(monstre.contient({100.0f, 115.0f}));
}

TEST_CASE("getCentre renvoie le centre du monstre") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le monstre, de rayon la moitie du cote") {
    const Monstre monstre({100.0f, 100.0f}, 1);
    const Cercle cercle = monstre.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 15.0f);
}

TEST_CASE("getId renvoie l'identifiant de chaque monstre") {
    const Monstre premier({100.0f, 100.0f}, 1);
    const Monstre second({200.0f, 200.0f}, 2);

    REQUIRE(premier.getId() == 1);
    REQUIRE(second.getId() == 2);
}

// Le monstre a 100 points de vie à la création.

TEST_CASE("Le monstre est vivant à la création") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est encore vivant après 80 points de dégâts") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(80);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort après 100 points de dégâts") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(100);

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort après un coup qui dépasse les points de vie restants") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(80); // reste 20 points de vie
    monstre.recevoirDegats(50); // dépasse largement le reste

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre reste mort après un coup supplémentaire") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(100);
    monstre.recevoirDegats(1);

    REQUIRE_FALSE(monstre.estVivant());
}

// Monstre centré en (100, 100), rayon 15 px, portée 15 px (bord à bord),
// dégâts 10, recharge 1 s.

TEST_CASE("Monstre.frapper renvoie les dégâts pour une cible à portée") {
    Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.frapper(pointCible({110.0f, 100.0f})) == 10);
}

TEST_CASE("Monstre.frapper renvoie 0 pour une cible hors de portée") {
    Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.frapper(pointCible({300.0f, 100.0f})) == 0);
}

TEST_CASE("Monstre.frapper renvoie de nouveau les dégâts après une durée simulée généreuse") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const Cercle cible = pointCible({110.0f, 100.0f});

    REQUIRE(monstre.frapper(cible) == 10);
    ecoulerTemps(monstre, 1.2f, std::nullopt, carte);
    REQUIRE(monstre.frapper(cible) == 10);
}

// Monstre centré en (100, 100), rayon de détection 200 px.

TEST_CASE("Monstre.mettreAJour approche une cible dans le rayon de détection") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(300.0f, 100.0f);
    const float distanceInitiale = (cible - monstre.getCentre()).length();

    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    const float distanceApres = (cible - monstre.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("Monstre.mettreAJour ignore une cible hors du rayon de détection") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(400.0f, 100.0f); // 300 px, hors des 200 px de détection
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour reste immobile sans cible") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, std::nullopt, carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour s'arrête à portée d'attaque sans atteindre la cible") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(300.0f, 100.0f);

    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distanceFinale = (cible - monstre.getCentre()).length();
    REQUIRE(distanceFinale <= 30.0f); // portée (15 px) + rayon du monstre (15 px)
    REQUIRE(monstre.getCentre() != cible);
}

// Territoire : une fois détectée (<= 200 px), une cible reste poursuivie
// tant que le MONSTRE lui-même ne s'éloigne pas de plus de 400 px de son
// poste (m_rayonTerritoire) ; peu importe à quelle distance la cible est.

TEST_CASE("Monstre.mettreAJour continue de poursuivre une cible qui s'éloigne, tant que le territoire n'est pas franchi") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection
    const sf::Vector2f centreApresDetection = monstre.getCentre();

    const sf::Vector2f cibleEloignee(400.0f, 100.0f); // hors détection, mais le monstre reste dans son territoire
    const float distanceAvant = (cibleEloignee - centreApresDetection).length();
    ecoulerTemps(monstre, 0.2f, pointCible(cibleEloignee), carte);
    const float distanceApres = (cibleEloignee - monstre.getCentre()).length();

    REQUIRE(distanceApres < distanceAvant);
}

TEST_CASE("Monstre.mettreAJour rejoint à portée une proie à 300 px de son poste, et y reste") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cible(400.0f, 100.0f); // 300 px du poste, dans le territoire (400 px)
    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distance = (cible - monstre.getCentre()).length();
    REQUIRE(distance <= 30.0f); // à portée de m_attaque (15 px) + rayon du monstre (15 px)
    REQUIRE_FALSE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f)); // pas revenu au poste
}

TEST_CASE("Monstre.mettreAJour retourne à son poste si la poursuite l'éloigne trop de son territoire") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cibleLointaine(1100.0f, 100.0f); // 1000 px du poste, hors du territoire
    ecoulerTemps(monstre, 20.0f, pointCible(cibleLointaine), carte); // durée généreuse : sort du territoire, puis rentre

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour ignore une proie hors de son territoire même proche de lui pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    // Proie à 600 px du poste : pousse le monstre hors de son territoire
    // (déclenchant le retour), puis reste assez proche de lui en tout début
    // de retour (moins de 200 px) sans jamais être elle-même dans le
    // territoire (toujours à plus de 400 px du poste) — elle ne doit donc
    // jamais être redétectée.
    ecoulerTemps(monstre, 20.0f, pointCible({700.0f, 100.0f}), carte); // durée généreuse

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour redétecte une proie revenue dans son territoire pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    ecoulerTemps(monstre, 4.0f, pointCible({900.0f, 100.0f}), carte); // franchit le territoire : début du retour
    const sf::Vector2f centrePendantLeRetour = monstre.getCentre();

    // Proie replacée dans le territoire (< 400 px du poste) et à moins de
    // 200 px du monstre : la poursuite doit reprendre.
    const sf::Vector2f cibleProche(centrePendantLeRetour.x + 100.0f, centrePendantLeRetour.y);
    const float distanceAvant = (cibleProche - centrePendantLeRetour).length();
    ecoulerTemps(monstre, 0.3f, pointCible(cibleProche), carte);
    const float distanceApres = (cibleProche - monstre.getCentre()).length();

    REQUIRE(distanceApres < distanceAvant);
}

TEST_CASE("Monstre.mettreAJour retourne à son poste si la cible disparaît après détection") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    ecoulerTemps(monstre, 10.0f, std::nullopt, carte); // durée généreuse : retour

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}
