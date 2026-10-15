// ./tests/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Monstre.hpp"
#include "TileMap.hpp"

namespace {

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

// 10 × 10 cases de 40 px, avec un mur de 4 cases (colonne 4, lignes 0 à 3) :
// un monstre en (100, 100) ne voit pas (250, 100), mais voit (200, 260).
TileMap carteAvecMur() {
    std::vector<std::string> plan(10, "..........");
    for (int ligne = 0; ligne < 4; ++ligne) {
        plan[static_cast<std::size_t>(ligne)][4] = '#';
    }
    return TileMap(plan, 40.0f);
}

// Simule le temps qui passe par tics de 1/60 s, comme dans main.cpp.
void ecoulerTemps(Monstre& monstre, float duree, std::optional<Cercle> cible, const TileMap& carte) {
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        monstre.mettreAJour(s_dureeTic, cible, carte, {});
    }
}

// Frappe en un seul appel, comme l'ancien frapper() : engage le coup sur la
// cible (refusée hors de portée), puis avance l'exécution (0,3 s) jusqu'au
// départ. Renvoie les dégâts, 0 si rien ne part.
int frapperEnEntier(Monstre& monstre, Cercle cible, Generateur& generateur) {
    if (!monstre.engagerAttaque(cible, 1, carteOuverte())) {
        return 0;
    }
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        if (const int degats = monstre.avancerAttaque(s_dureeTic, generateur); degats > 0) {
            return degats;
        }
    }
    return 0;
}

}  // namespace

// Monstre centré en (100, 100), taille 30x30 : bords en x: [85, 115),
// y: [85, 115).

TEST_CASE("contient renvoie vrai pour un point au centre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(monstre.contient({100.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point a l'exterieur") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE_FALSE(monstre.contient({200.0f, 200.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord gauche") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(monstre.contient({85.0f, 100.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord haut") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(monstre.contient({100.0f, 85.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord droit") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE_FALSE(monstre.contient({115.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord bas") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE_FALSE(monstre.contient({100.0f, 115.0f}));
}

TEST_CASE("getCentre renvoie le centre du monstre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le monstre, de rayon la moitie du cote") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);
    const Cercle cercle = monstre.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 15.0f);
}

TEST_CASE("getId renvoie l'identifiant de chaque monstre") {
    const Monstre premier({100.0f, 100.0f}, 1, 0);
    const Monstre second({200.0f, 200.0f}, 2, 0);

    REQUIRE(premier.getId() == 1);
    REQUIRE(second.getId() == 2);
}

// Le monstre a 100 points de vie à la création.

TEST_CASE("Le monstre est vivant a la creation") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est encore vivant apres 80 points de degats") {
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(80);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres 100 points de degats") {
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(100);

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres un coup qui depasse les points de vie restants") {
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(80); // reste 20 points de vie
    monstre.recevoirDegats(50); // dépasse largement le reste

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre reste mort apres un coup supplementaire") {
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(100);
    monstre.recevoirDegats(1);

    REQUIRE_FALSE(monstre.estVivant());
}

// Monstre centré en (100, 100), rayon 15 px, portée 15 px (bord à bord),
// dégâts 10, recharge 1 s.

TEST_CASE("Monstre.frapper renvoie les degats pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(frapperEnEntier(monstre, pointCible({110.0f, 100.0f}), generateur) == 10);
}

TEST_CASE("Monstre.frapper renvoie 0 pour une cible hors de portee") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE(frapperEnEntier(monstre, pointCible({300.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("Monstre.frapper renvoie de nouveau les degats apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    const Cercle cible = pointCible({110.0f, 100.0f});

    REQUIRE(frapperEnEntier(monstre, cible, generateur) == 10);
    ecoulerTemps(monstre, 1.5f, std::nullopt, carte);
    REQUIRE(frapperEnEntier(monstre, cible, generateur) == 10);
}

// Monstre centré en (100, 100), rayon de détection 200 px.

TEST_CASE("Monstre.mettreAJour approche une cible dans le rayon de detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    const sf::Vector2f cible(300.0f, 100.0f);
    const float distanceInitiale = (cible - monstre.getCentre()).length();

    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    const float distanceApres = (cible - monstre.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("Monstre.mettreAJour ignore une cible hors du rayon de detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    const sf::Vector2f cible(400.0f, 100.0f); // 300 px, hors des 200 px de détection
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour reste immobile sans cible") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, std::nullopt, carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour s'arrete a portee d'attaque sans atteindre la cible") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    const sf::Vector2f cible(300.0f, 100.0f);

    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distanceFinale = (cible - monstre.getCentre()).length();
    REQUIRE(distanceFinale <= 30.0f); // portée (15 px) + rayon du monstre (15 px)
    REQUIRE(monstre.getCentre() != cible);
}

// Territoire : une fois détectée (<= 200 px), une cible reste poursuivie
// tant que le MONSTRE lui-même ne s'éloigne pas de plus de 400 px de son
// poste (m_rayonTerritoire) ; peu importe à quelle distance la cible est.

TEST_CASE("Monstre.mettreAJour continue de poursuivre une cible qui s'eloigne, tant que le territoire n'est pas franchi") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

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
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cible(400.0f, 100.0f); // 300 px du poste, dans le territoire (400 px)
    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distance = (cible - monstre.getCentre()).length();
    REQUIRE(distance <= 30.0f); // à portée de m_attaque (15 px) + rayon du monstre (15 px)
    REQUIRE_FALSE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f)); // pas revenu au poste
}

TEST_CASE("Monstre.mettreAJour retourne a son poste si la poursuite l'eloigne trop de son territoire") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cibleLointaine(1100.0f, 100.0f); // 1000 px du poste, hors du territoire
    ecoulerTemps(monstre, 20.0f, pointCible(cibleLointaine), carte); // durée généreuse : sort du territoire, puis rentre

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour ignore une proie hors de son territoire meme proche de lui pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    // Proie à 600 px du poste : pousse le monstre hors de son territoire
    // (déclenchant le retour), puis reste assez proche de lui en tout début
    // de retour (moins de 200 px) sans jamais être elle-même dans le
    // territoire (toujours à plus de 400 px du poste) — elle ne doit donc
    // jamais être redétectée.
    ecoulerTemps(monstre, 20.0f, pointCible({700.0f, 100.0f}), carte); // durée généreuse

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour redetecte une proie revenue dans son territoire pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

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

TEST_CASE("Monstre.mettreAJour retourne a son poste si la cible disparait apres detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    ecoulerTemps(monstre, 10.0f, std::nullopt, carte); // durée généreuse : retour

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

// Coup reçu : un monstre touché dans son territoire poursuit sa proie
// habituelle, même hors du rayon de détection (200 px).

namespace {

// Amène le monstre en retour, sorti du territoire (plus de 400 px de son
// poste) : la proie lointaine à 800 px de son poste n'est jamais détectée.
void amenerEnRetourHorsTerritoire(Monstre& monstre, const TileMap& carte) {
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() <= 400.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte); // bascule en retour
}

}  // namespace

TEST_CASE("Monstre touche en garde poursuit une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre non touche reste en garde devant une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre.recevoirDegats de 0 ne reveille pas le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(0);
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre touche sans proie reste en garde et le coup est consomme") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 1.0f / 60.0f, std::nullopt, carte);
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre touche en retour dans son territoire poursuit") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);
    // Laisse le retour ramener le monstre dans son territoire.
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > 300.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    const float xAvant = monstre.getCentre().x;

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > xAvant); // va vers la proie, à l'est
}

TEST_CASE("Monstre touche en retour hors de son territoire continue de rentrer") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);
    const float xAvant = monstre.getCentre().x;

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // va vers son poste, à l'ouest
}

TEST_CASE("Monstre touche en poursuite continue de poursuivre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    const float xAvant = monstre.getCentre().x;

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.3f, pointCible({400.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > xAvant);
}

TEST_CASE("Monstre.recevoirDegats applique son armure") {
    Monstre monstre({100.0f, 100.0f}, 1, 20);

    monstre.recevoirDegats(5);

    REQUIRE(monstre.getPointsDeVie() == 96);  // 5 x 100 / 120 = 4,17 -> 4
}

TEST_CASE("Monstre.recevoirDegats sans armure retire le montant entier") {
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(5);

    REQUIRE(monstre.getPointsDeVie() == 95);
}

TEST_CASE("Monstre.recevoirDegats d'un coup de 1 retire 1 malgre l'armure et reveille le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 20);
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    monstre.recevoirDegats(1);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getPointsDeVie() == 99);
    REQUIRE(monstre.getCentre().x > 100.0f);
}

// Temps d'exécution de l'attaque (0,3 s) : le monstre frappe immobile, le coup
// engagé part à la fin de l'exécution, quelle que soit la cible du moment.

TEST_CASE("Monstre.engagerAttaque exige une cible a portee") {
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    REQUIRE_FALSE(monstre.engagerAttaque(pointCible({300.0f, 100.0f}), 3, carteOuverte()));
    REQUIRE_FALSE(monstre.proieEngagee().has_value());
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 3, carteOuverte()));
    REQUIRE(monstre.proieEngagee() == 3);
}

TEST_CASE("Monstre.engagerAttaque refuse un monstre en marche") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.1f, pointCible({180.0f, 100.0f}), carte);  // en poursuite, pas encore à portée

    REQUIRE_FALSE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

TEST_CASE("Monstre.avancerAttaque rend les degats une seule fois, a la fin de l'execution") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.25f); ++i) {
        total += monstre.avancerAttaque(s_dureeTic, generateur);
    }
    REQUIRE(total == 0);

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += monstre.avancerAttaque(s_dureeTic, generateur);
    }
    REQUIRE(total == 10);
    REQUIRE_FALSE(monstre.proieEngagee().has_value());
}

TEST_CASE("Monstre.mettreAJour ne bouge pas pendant l'execution, meme si la cible s'eloigne") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));

    ecoulerTemps(monstre, 0.25f, pointCible({300.0f, 100.0f}), carte);

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f}, 0.01f);
}

TEST_CASE("Monstre.annulerAttaque ne consomme pas de recharge") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    REQUIRE(monstre.avancerAttaque(0.1f, generateur) == 0);

    monstre.annulerAttaque();

    REQUIRE_FALSE(monstre.proieEngagee().has_value());
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

TEST_CASE("Monstre : apres le depart du coup, la recharge court avant le suivant") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    REQUIRE(monstre.avancerAttaque(0.4f, generateur) == 10);

    REQUIRE_FALSE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    ecoulerTemps(monstre, 1.4f, std::nullopt, carte);  // recharge de 0,7 à 1,3 s écoulée
    REQUIRE(monstre.engagerAttaque(pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

// Ligne de vue : la détection et la frappe exigent la vue ; la poursuite non.

TEST_CASE("Monstre ne detecte pas une proie a portee de detection derriere un mur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre detecte la meme distance de proie quand rien ne la cache") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    ecoulerTemps(monstre, 0.5f, pointCible({200.0f, 260.0f}), carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre en poursuite continue de suivre une proie qui disparait derriere le mur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0);
    ecoulerTemps(monstre, 0.1f, pointCible({200.0f, 260.0f}), carte);  // détection
    const float xApresDetection = monstre.getCentre().x;

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);  // proie cachée

    REQUIRE(monstre.getCentre().x > xApresDetection);  // il avance vers elle (un retour au poste ramènerait x vers 100)
}

TEST_CASE("Monstre touche s'active meme sans voir son agresseur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0);

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(100.0f, 100.0f));
}

namespace {

// 5 × 5 cases de 40 px, un mur en case (1, 1) (x et y de 40 à 80). Un monstre en
// (30, 60) a (45, 35) à 29 px : à portée (15 px bord à bord, rayon du monstre
// 15 px), mais le segment passe par le coin du mur. (30, 35) est à portée et
// en vue ; (30, 150) est en vue mais trop loin.
TileMap carteAvecPilier() {
    std::vector<std::string> plan(5, ".....");
    plan[1][1] = '#';
    return TileMap(plan, 40.0f);
}

}  // namespace

TEST_CASE("Monstre.peutAtteindre exige la portee ET la vue") {
    const TileMap carte = carteAvecPilier();
    const Monstre monstre({30.0f, 60.0f}, 1, 0);

    REQUIRE_FALSE(monstre.peutAtteindre(pointCible({45.0f, 35.0f}), carte));   // à portée, caché
    REQUIRE_FALSE(monstre.peutAtteindre(pointCible({30.0f, 150.0f}), carte));  // en vue, trop loin
    REQUIRE(monstre.peutAtteindre(pointCible({30.0f, 35.0f}), carte));         // à portée et en vue
}

TEST_CASE("Monstre.engagerAttaque refuse une proie a portee mais cachee, l'accepte en vue") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({30.0f, 60.0f}, 1, 0);

    REQUIRE_FALSE(monstre.engagerAttaque(pointCible({45.0f, 35.0f}), 1, carte));
    REQUIRE_FALSE(monstre.proieEngagee().has_value());
    REQUIRE(monstre.engagerAttaque(pointCible({30.0f, 35.0f}), 1, carte));
    REQUIRE(monstre.proieEngagee() == 1);
}

TEST_CASE("Monstre en poursuite ne s'arrete pas a portee mais cachee derriere un coin") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0);  // son carré touche le mur sans le chevaucher
    const Cercle proie{{45.0f, 35.0f}, 10.0f};  // à 36 px : à portée (bord à bord), mais cachée
    REQUIRE_FALSE(monstre.peutAtteindre(proie, carte));
    monstre.recevoirDegats(1);  // actif sans dépendre de la vue

    ecoulerTemps(monstre, 0.3f, proie, carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(25.0f, 65.0f));
}
