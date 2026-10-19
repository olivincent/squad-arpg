// ./tests/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "EntiteVue.hpp"
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
// La cible est à la fois la proie (place 1) et l'unique candidat de frappe.
void ecoulerTemps(Monstre& monstre, float duree, std::optional<Cercle> cible, const TileMap& carte,
                  const std::vector<Cercle>& obstacles = {}, const std::vector<Cercle>& aContourner = {}) {
    std::optional<EntiteVue> proie;
    std::vector<EntiteVue> candidats;
    if (cible) {
        proie = EntiteVue{1, *cible};
        candidats.push_back(*proie);
    }
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, obstacles, aContourner);
    }
}

// Engage un coup sur cette cible, à la fois proie et unique candidat.
bool engager(Monstre& monstre, Cercle cible, int id, const TileMap& carte) {
    const EntiteVue entite{id, cible};
    return monstre.engagerAttaque(entite, {entite}, carte);
}

// Frappe en un seul appel, comme l'ancien frapper() : engage le coup sur la
// cible (refusée hors de portée), puis avance l'exécution (0,3 s) jusqu'au
// départ. Renvoie les dégâts, 0 si rien ne part.
int frapperEnEntier(Monstre& monstre, Cercle cible, Generateur& generateur) {
    if (!engager(monstre, cible, 1, carteOuverte())) {
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
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.contient({100.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point a l'exterieur") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.contient({200.0f, 200.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord gauche") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.contient({85.0f, 100.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord haut") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.contient({100.0f, 85.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord droit") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.contient({115.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord bas") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.contient({100.0f, 115.0f}));
}

TEST_CASE("getCentre renvoie le centre du monstre") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le monstre, de rayon la moitie du cote") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const Cercle cercle = monstre.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 15.0f);
}

TEST_CASE("getId renvoie l'identifiant de chaque monstre") {
    const Monstre premier({100.0f, 100.0f}, 1, 0, 0);
    const Monstre second({200.0f, 200.0f}, 2, 0, 0);

    REQUIRE(premier.getId() == 1);
    REQUIRE(second.getId() == 2);
}

// Le monstre a tous ses points de vie à la création. Leur nombre n'est pas injectable : les tests
// le lisent sur le monstre neuf (getPointsDeVie), sans l'écrire.

TEST_CASE("Le monstre est vivant a la creation") {
    const Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est encore vivant apres des degats inferieurs a ses points de vie") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie() - 20);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres autant de degats que ses points de vie") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie());

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort apres un coup qui depasse les points de vie restants") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie() - 20); // reste 20 points de vie
    monstre.recevoirDegats(50); // dépasse largement le reste

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre reste mort apres un coup supplementaire") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(monstre.getPointsDeVie());
    monstre.recevoirDegats(1);

    REQUIRE_FALSE(monstre.estVivant());
}

// Monstre centré en (100, 100), rayon 15 px, portée 15 px (bord à bord),
// dégâts 10, recharge 1 s.

TEST_CASE("Monstre.frapper renvoie les degats pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(frapperEnEntier(monstre, pointCible({110.0f, 100.0f}), generateur) == 10);
}

TEST_CASE("Monstre.frapper renvoie 0 pour une cible hors de portee") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(frapperEnEntier(monstre, pointCible({300.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("Monstre.frapper renvoie de nouveau les degats apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const Cercle cible = pointCible({110.0f, 100.0f});

    REQUIRE(frapperEnEntier(monstre, cible, generateur) == 10);
    ecoulerTemps(monstre, 1.5f, std::nullopt, carte);
    REQUIRE(frapperEnEntier(monstre, cible, generateur) == 10);
}

// Monstre centré en (100, 100), rayon de détection 200 px.

TEST_CASE("Monstre.mettreAJour approche une cible dans le rayon de detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const sf::Vector2f cible(300.0f, 100.0f);
    const float distanceInitiale = (cible - monstre.getCentre()).length();

    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    const float distanceApres = (cible - monstre.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("Monstre.mettreAJour ignore une cible hors du rayon de detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const sf::Vector2f cible(400.0f, 100.0f); // 300 px, hors des 200 px de détection
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour reste immobile sans cible") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, std::nullopt, carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour s'arrete a portee d'attaque sans atteindre la cible") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
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
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

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
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cible(400.0f, 100.0f); // 300 px du poste, dans le territoire (600 px)
    ecoulerTemps(monstre, 10.0f, pointCible(cible), carte); // durée généreuse

    const float distance = (cible - monstre.getCentre()).length();
    REQUIRE(distance <= 30.0f); // à portée de m_attaque (15 px) + rayon du monstre (15 px)
    REQUIRE_FALSE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f)); // pas revenu au poste
}

TEST_CASE("Monstre.mettreAJour retourne a son poste si la poursuite l'eloigne trop de son territoire") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // déclenche la détection

    const sf::Vector2f cibleLointaine(1100.0f, 100.0f); // 1000 px du poste, hors du territoire
    ecoulerTemps(monstre, 20.0f, pointCible(cibleLointaine), carte); // durée généreuse : sort du territoire, puis rentre

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("Monstre.mettreAJour ignore une proie hors de son territoire meme proche de lui pendant le retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

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
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    ecoulerTemps(monstre, 4.0f, pointCible({900.0f, 100.0f}), carte); // franchit le territoire : début du retour
    // Laisse le retour ramener le monstre assez loin de la frontière pour que la proie, 100 px plus loin, soit dans le territoire.
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > Monstre::s_rayonTerritoire - 150.0f) {
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
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    ecoulerTemps(monstre, 20.0f, std::nullopt, carte);                // la proie disparaît : retour, puis arrivée
    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.mettreAJour retourne a son poste si la cible disparait apres detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection

    ecoulerTemps(monstre, 10.0f, std::nullopt, carte); // durée généreuse : retour

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

// Coup reçu : un monstre touché dans son territoire poursuit sa proie
// habituelle, même hors du rayon de détection (200 px).

namespace {

// Amène le monstre en retour, sorti du territoire (plus de `Monstre::s_rayonTerritoire`
// de son poste) : la proie lointaine à 800 px de son poste n'est jamais détectée.
// `degatsEnPoursuite` est infligé avant le retour (en retour, le monstre est invulnérable).
void amenerEnRetourHorsTerritoire(Monstre& monstre, const TileMap& carte, int degatsEnPoursuite = 0) {
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    monstre.recevoirDegats(degatsEnPoursuite);
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() <= Monstre::s_rayonTerritoire) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte); // bascule en retour
}

}  // namespace

TEST_CASE("Monstre touche en garde poursuit une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre non touche reste en garde devant une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre.recevoirDegats de 0 ne reveille pas le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(0);
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre touche sans proie reste en garde et le coup est consomme") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 1.0f / 60.0f, std::nullopt, carte);
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre touche en retour dans son territoire ne se reveille pas et ne perd rien") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte, 50);
    // Laisse le retour ramener le monstre dans son territoire.
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > 300.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    const float xAvant = monstre.getCentre().x;
    const int vieAvant = monstre.getPointsDeVie();

    monstre.recevoirDegats(10);
    REQUIRE(monstre.getPointsDeVie() == vieAvant); // invulnérable
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // continue de rentrer, à l'ouest
}

TEST_CASE("Monstre.alerter en retour dans son territoire ne reveille pas le monstre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > 300.0f) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    const float xAvant = monstre.getCentre().x;

    monstre.alerter();
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant);
    REQUIRE_FALSE(monstre.estEnPoursuite());
}

TEST_CASE("Monstre.engagerAttaque en retour n'engage aucun coup meme avec une proie a portee") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);
    const Cercle proche{monstre.getCentre() + sf::Vector2f(20.0f, 0.0f), 10.0f};

    REQUIRE_FALSE(engager(monstre, proche, 1, carte));
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
}

TEST_CASE("Monstre.estCiblable est vrai en garde et en poursuite, faux en retour") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(monstre.estCiblable());

    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // poursuite
    REQUIRE(monstre.estEnPoursuite());
    REQUIRE(monstre.estCiblable());

    amenerEnRetourHorsTerritoire(monstre, carte);
    REQUIRE_FALSE(monstre.estCiblable());
}

// Régénération en retour : 50 % des points de vie maximum par seconde.

TEST_CASE("Monstre en retour regagne tous ses points de vie en deux secondes environ") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const int maximum = monstre.getPointsDeVie();
    amenerEnRetourHorsTerritoire(monstre, carte, maximum - 10);
    // Environ 10 : le premier tic du retour a pu rendre un point.
    const int vieEnRetour = monstre.getPointsDeVie();
    REQUIRE(vieEnRetour < maximum / 2);

    ecoulerTemps(monstre, 1.0f, pointCible({900.0f, 100.0f}), carte);
    REQUIRE(monstre.getPointsDeVie() >= vieEnRetour + maximum / 2 - 5);
    REQUIRE(monstre.getPointsDeVie() <= vieEnRetour + maximum / 2 + 5);

    ecoulerTemps(monstre, 1.0f, pointCible({900.0f, 100.0f}), carte);
    REQUIRE(monstre.getPointsDeVie() == maximum);
}

TEST_CASE("Monstre en retour ne depasse jamais son maximum") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const int maximum = monstre.getPointsDeVie();
    amenerEnRetourHorsTerritoire(monstre, carte, 10);

    ecoulerTemps(monstre, 3.0f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getPointsDeVie() == maximum);
}

TEST_CASE("Monstre en retour regagne des points de vie a 60 images par seconde") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const int maximum = monstre.getPointsDeVie();
    amenerEnRetourHorsTerritoire(monstre, carte, maximum - 10);
    const int vieEnRetour = monstre.getPointsDeVie();

    ecoulerTemps(monstre, 0.1f, pointCible({900.0f, 100.0f}), carte); // 6 images, moins d'un point chacune

    // 0,1 s à 50 % du maximum par seconde : la partie entière cumulée, pas 0 à chaque image.
    REQUIRE(monstre.getPointsDeVie() >= vieEnRetour + maximum / 20 - 1);
}

TEST_CASE("Monstre ne regagne aucun point de vie hors retour") {
    const TileMap carte = carteLongue();
    Monstre enGarde({100.0f, 100.0f}, 1, 0, 0);
    const int maximum = enGarde.getPointsDeVie();
    enGarde.recevoirDegats(50);
    ecoulerTemps(enGarde, 2.0f, std::nullopt, carte);
    REQUIRE(enGarde.getPointsDeVie() == maximum - 50);

    Monstre enPoursuite({100.0f, 100.0f}, 2, 0, 0);
    ecoulerTemps(enPoursuite, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    enPoursuite.recevoirDegats(50);
    ecoulerTemps(enPoursuite, 1.0f, pointCible({250.0f, 100.0f}), carte);
    REQUIRE(enPoursuite.estEnPoursuite());
    REQUIRE(enPoursuite.getPointsDeVie() == maximum - 50);
}

TEST_CASE("Monstre touche en retour hors de son territoire continue de rentrer") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);
    const float xAvant = monstre.getCentre().x;

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // va vers son poste, à l'ouest
}

TEST_CASE("Monstre touche en poursuite continue de poursuivre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    const float xAvant = monstre.getCentre().x;

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.3f, pointCible({400.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > xAvant);
}

TEST_CASE("Monstre.recevoirDegats applique son armure") {
    Monstre monstre({100.0f, 100.0f}, 1, 20, 0);
    const int maximum = monstre.getPointsDeVie();

    monstre.recevoirDegats(5);

    REQUIRE(monstre.getPointsDeVie() == maximum - 4);  // 5 x 100 / 120 = 4,17 -> 4
}

TEST_CASE("Monstre.recevoirDegats sans armure retire le montant entier") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const int maximum = monstre.getPointsDeVie();

    monstre.recevoirDegats(5);

    REQUIRE(monstre.getPointsDeVie() == maximum - 5);
}

TEST_CASE("Monstre.recevoirDegats d'un coup de 1 retire 1 malgre l'armure et reveille le monstre") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 20, 0);
    const int maximum = monstre.getPointsDeVie();
    const sf::Vector2f cible(350.0f, 100.0f); // 250 px, hors détection

    monstre.recevoirDegats(1);
    ecoulerTemps(monstre, 0.5f, pointCible(cible), carte);

    REQUIRE(monstre.getPointsDeVie() == maximum - 1);
    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.alerter en garde dans son territoire poursuit une proie hors detection") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.alerter();
    ecoulerTemps(monstre, 0.5f, pointCible({350.0f, 100.0f}), carte); // 250 px, hors détection

    REQUIRE(monstre.getCentre().x > 100.0f);
}

TEST_CASE("Monstre.alerter hors de son territoire ne reveille pas le monstre") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);
    const float xAvant = monstre.getCentre().x;

    monstre.alerter();
    ecoulerTemps(monstre, 0.3f, pointCible({900.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x < xAvant); // continue de rentrer, à l'ouest
}

TEST_CASE("Monstre.alerter en poursuite n'a aucun effet") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    const float xAvant = monstre.getCentre().x;

    monstre.alerter();
    ecoulerTemps(monstre, 0.3f, pointCible({400.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre().x > xAvant);
}

TEST_CASE("Monstre.getGroupe rend le groupe recu a la construction") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 7);

    REQUIRE(monstre.getGroupe() == 7);
}

// Temps d'exécution de l'attaque (0,3 s) : le monstre frappe immobile, le coup
// engagé part à la fin de l'exécution, quelle que soit la cible du moment.

TEST_CASE("Monstre.engagerAttaque exige une cible a portee") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(engager(monstre, pointCible({300.0f, 100.0f}), 3, carteOuverte()));
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 3, carteOuverte()));
    REQUIRE(monstre.cibleEngagee() == 3);
}

TEST_CASE("Monstre.engagerAttaque refuse un monstre en marche") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.1f, pointCible({180.0f, 100.0f}), carte);  // en poursuite, pas encore à portée

    REQUIRE_FALSE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

TEST_CASE("Monstre.avancerAttaque rend les degats une seule fois, a la fin de l'execution") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.25f); ++i) {
        total += monstre.avancerAttaque(s_dureeTic, generateur);
    }
    REQUIRE(total == 0);

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += monstre.avancerAttaque(s_dureeTic, generateur);
    }
    REQUIRE(total == 10);
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
}

TEST_CASE("Monstre.mettreAJour ne bouge pas pendant l'execution, meme si la cible s'eloigne") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));

    ecoulerTemps(monstre, 0.25f, pointCible({300.0f, 100.0f}), carte);

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f}, 0.01f);
}

TEST_CASE("Monstre.annulerAttaque ne consomme pas de recharge") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    REQUIRE(monstre.avancerAttaque(0.1f, generateur) == 0);

    monstre.annulerAttaque();

    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

TEST_CASE("Monstre : apres le depart du coup, la recharge court avant le suivant") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    REQUIRE(monstre.avancerAttaque(0.4f, generateur) == 10);

    REQUIRE_FALSE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    ecoulerTemps(monstre, 1.4f, std::nullopt, carte);  // recharge de 0,7 à 1,3 s écoulée
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

// Ligne de vue : la détection et la frappe exigent la vue ; la poursuite non.

TEST_CASE("Monstre ne detecte pas une proie a portee de detection derriere un mur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre detecte la meme distance de proie quand rien ne la cache") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.5f, pointCible({200.0f, 260.0f}), carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Monstre en poursuite continue de suivre une proie qui disparait derriere le mur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    ecoulerTemps(monstre, 0.1f, pointCible({200.0f, 260.0f}), carte);  // détection
    const float xApresDetection = monstre.getCentre().x;

    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);  // proie cachée

    REQUIRE(monstre.getCentre().x > xApresDetection);  // il avance vers elle (un retour au poste ramènerait x vers 100)
}

TEST_CASE("Monstre touche s'active meme sans voir son agresseur") {
    const TileMap carte = carteAvecMur();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    monstre.recevoirDegats(10);
    ecoulerTemps(monstre, 0.5f, pointCible({250.0f, 100.0f}), carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(100.0f, 100.0f));
}

namespace {

// Depuis l'étape 73 la vue se décide en cases : deux points voisins ne sont cachés l'un à
// l'autre que si un mur d'une case se trouve entre eux. Deux fixtures :
//
// carteAvecMurMince : 20 × 20 cases de 10 px, un mur en case (4, 4) (x et y de 40 à 50).
// Un monstre en (25, 45), dont le carré touche le mur sans le chevaucher, a (60, 45) à
// 35 px : à portée (15 px bord à bord, rayon du monstre 15 px, cible ponctuelle), mais le
// mur est entre les deux. (25, 15) est à portée et en vue ; (25, 150) est en vue mais
// trop loin. Pour les tests qui ne déplacent pas le monstre : sur des cases aussi petites
// le A* le coincerait, son carré de 30 px ne passant pas entre les centres de cases.
TileMap carteAvecMurMince() {
    std::vector<std::string> plan(20, std::string(20, '.'));
    plan[4][4] = '#';
    return TileMap(plan, 10.0f);
}

// carteAvecPilier : 5 × 5 cases de 40 px, un mur en case (1, 1) (x et y de 40 à 80), pour
// les tests où le monstre doit pouvoir se déplacer. Un monstre en (25, 65) est dans la case
// (0, 1) ; une cible de rayon 55 centrée en (100, 60), dans la case (2, 1), est à 75 px :
// à portée (5 px bord à bord), mais cachée par le mur. Le rayon est grand parce que le mur
// fait 40 px d'épaisseur. (25, 45) est dans la case du monstre : à portée et en vue ;
// (25, 150) est en vue mais trop loin.
TileMap carteAvecPilier() {
    std::vector<std::string> plan(5, ".....");
    plan[1][1] = '#';
    return TileMap(plan, 40.0f);
}

}  // namespace

TEST_CASE("Monstre.peutAtteindre exige la portee ET la vue") {
    const TileMap carte = carteAvecMurMince();
    const Monstre monstre({25.0f, 45.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.peutAtteindre(pointCible({60.0f, 45.0f}), carte));   // à portée, caché
    REQUIRE_FALSE(monstre.peutAtteindre(pointCible({25.0f, 150.0f}), carte));  // en vue, trop loin
    REQUIRE(monstre.peutAtteindre(pointCible({25.0f, 15.0f}), carte));         // à portée et en vue
}

TEST_CASE("Monstre.engagerAttaque refuse une proie a portee mais cachee, l'accepte en vue") {
    const TileMap carte = carteAvecMurMince();
    Monstre monstre({25.0f, 45.0f}, 1, 0, 0);

    REQUIRE_FALSE(engager(monstre, pointCible({60.0f, 45.0f}), 1, carte));
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE(engager(monstre, pointCible({25.0f, 15.0f}), 1, carte));
    REQUIRE(monstre.cibleEngagee() == 1);
}

TEST_CASE("Monstre en poursuite ne s'arrete pas a portee mais cachee derriere un mur") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0, 0);  // son carré touche le mur sans le chevaucher
    const Cercle proie{{100.0f, 60.0f}, 55.0f};  // à 75 px : à portée (bord à bord), mais cachée
    REQUIRE_FALSE(monstre.peutAtteindre(proie, carte));
    monstre.recevoirDegats(1);  // actif sans dépendre de la vue

    ecoulerTemps(monstre, 0.3f, proie, carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(25.0f, 65.0f));
}

// Cible de frappe : la proie (place 1) si elle est atteignable, sinon le
// candidat atteignable le plus proche. Les identifiants sont des places.

namespace {

void ecoulerTempsAvecCandidats(Monstre& monstre, float duree, std::optional<EntiteVue> proie,
                               const std::vector<EntiteVue>& candidats, const TileMap& carte) {
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, {}, {});
    }
}

}  // namespace

TEST_CASE("Monstre.engagerAttaque prefere la proie atteignable a un candidat plus proche") {
    Monstre monstre({30.0f, 60.0f}, 1, 0, 0);
    const EntiteVue proie{1, pointCible({55.0f, 60.0f})};
    const EntiteVue plusProche{2, pointCible({40.0f, 60.0f})};

    REQUIRE(monstre.engagerAttaque(proie, {plusProche, proie}, carteOuverte()));

    REQUIRE(monstre.cibleEngagee() == 1);
}

TEST_CASE("Monstre.engagerAttaque frappe un autre candidat si la proie est cachee") {
    const TileMap carte = carteAvecMurMince();
    Monstre monstre({25.0f, 45.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, pointCible({60.0f, 45.0f})};
    const EntiteVue enVue{2, pointCible({25.0f, 15.0f})};

    REQUIRE(monstre.engagerAttaque(proieCachee, {proieCachee, enVue}, carte));

    REQUIRE(monstre.cibleEngagee() == 2);
}

TEST_CASE("Monstre.engagerAttaque choisit le candidat atteignable le plus proche") {
    Monstre monstre({30.0f, 60.0f}, 1, 0, 0);
    const EntiteVue proieLointaine{1, pointCible({30.0f, 300.0f})};
    const EntiteVue moinsProche{2, pointCible({55.0f, 60.0f})};
    const EntiteVue plusProche{3, pointCible({40.0f, 60.0f})};

    REQUIRE(monstre.engagerAttaque(proieLointaine, {proieLointaine, moinsProche, plusProche}, carteOuverte()));

    REQUIRE(monstre.cibleEngagee() == 3);
}

TEST_CASE("Monstre.engagerAttaque est refuse sans aucun candidat atteignable") {
    Monstre monstre({30.0f, 60.0f}, 1, 0, 0);
    const EntiteVue loin{1, pointCible({300.0f, 60.0f})};

    REQUIRE_FALSE(monstre.engagerAttaque(loin, {loin}, carteOuverte()));
    REQUIRE_FALSE(monstre.engagerAttaque(std::nullopt, {}, carteOuverte()));
}

TEST_CASE("Monstre.mettreAJour s'arrete en poursuite quand un autre candidat devient atteignable") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, {{100.0f, 60.0f}, 55.0f}};
    const EntiteVue enVue{2, {{25.0f, 45.0f}, 10.0f}};
    REQUIRE(monstre.peutAtteindre(enVue.cercle, carte));
    monstre.recevoirDegats(1);  // actif sans dépendre de la vue

    ecoulerTempsAvecCandidats(monstre, 0.3f, proieCachee, {proieCachee, enVue}, carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(25.0f, 65.0f));
}

TEST_CASE("Monstre.mettreAJour continue vers sa proie si aucun candidat n'est atteignable") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, {{100.0f, 60.0f}, 55.0f}};
    const EntiteVue tropLoin{2, {{25.0f, 150.0f}, 10.0f}};
    monstre.recevoirDegats(1);

    ecoulerTempsAvecCandidats(monstre, 0.3f, proieCachee, {proieCachee, tropLoin}, carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(25.0f, 65.0f));
}

TEST_CASE("Monstre le coup engage touche la place choisie et rend les degats") {
    const TileMap carte = carteAvecMurMince();
    Monstre monstre({25.0f, 45.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, pointCible({60.0f, 45.0f})};
    const EntiteVue enVue{2, pointCible({25.0f, 15.0f})};
    REQUIRE(monstre.engagerAttaque(proieCachee, {proieCachee, enVue}, carte));
    Generateur generateur(s_graineDeTest);

    int degats = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && degats == 0; ++i) {
        REQUIRE(monstre.cibleEngagee() == 2);
        degats = monstre.avancerAttaque(s_dureeTic, generateur);
    }

    REQUIRE(degats == 10);
}

TEST_CASE("Monstre en poursuite contourne un autre monstre immobile") {
    const TileMap carte = carteDegagee10x10();
    const std::vector<Cercle> autre = {{{150.0f, 200.0f}, 15.0f}};
    Monstre monstre({60.0f, 200.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 6.0f, pointCible({240.0f, 200.0f}), carte, autre, autre);

    REQUIRE(monstre.getCentre().x > 150.0f); // passé de l'autre côté de l'autre monstre
}

TEST_CASE("Monstre en poursuite sans cercle a contourner reste bloque derriere un autre monstre") {
    const TileMap carte = carteDegagee10x10();
    const std::vector<Cercle> autre = {{{150.0f, 200.0f}, 15.0f}};
    Monstre monstre({60.0f, 200.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 6.0f, pointCible({240.0f, 200.0f}), carte, autre, {});

    REQUIRE(monstre.getCentre().x <= 120.0f);
}

TEST_CASE("Monstre ne contourne pas un personnage entre lui et sa proie : il s'arrete pour le frapper") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({60.0f, 200.0f}, 1, 0, 0);
    const EntiteVue proie{1, pointCible({240.0f, 200.0f})};
    const EntiteVue devant{2, {{110.0f, 200.0f}, 10.0f}};
    const std::vector<EntiteVue> candidats = {proie, devant};
    const std::vector<Cercle> obstacles = {proie.cercle, devant.cercle};

    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, obstacles, {});
    }

    REQUIRE(monstre.getCentre().x < 110.0f);
    REQUIRE(monstre.engagerAttaque(proie, candidats, carte));
    REQUIRE(monstre.cibleEngagee() == 2);
}

TEST_CASE("Monstre en retour bloque par un autre monstre recalcule et rentre a son poste") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte);

    // Un autre monstre se place en travers du retour, après le choix du chemin.
    const std::vector<Cercle> autre = {{{monstre.getCentre().x - 80.0f, 100.0f}, 15.0f}};
    ecoulerTemps(monstre, 10.0f, std::nullopt, carte, autre, autre);

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f});
}

TEST_CASE("un monstre a une zone de controle de 20 px et une attaque de 10 degats") {
    const Monstre monstre({100.0f, 100.0f}, 1, 20, 0);

    REQUIRE(monstre.getZoneDeControle() == 20.0f);
    REQUIRE(Monstre::s_zoneDeControle == 20.0f);
    REQUIRE(monstre.getPuissanceAttaque() == 10);
}

TEST_CASE("lire la puissance de l'attaque ne change ni le coup engage ni la recharge") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const TileMap carte = carteDegagee10x10();
    Generateur generateur = generateurDeTest();
    REQUIRE(engager(monstre, pointCible({125.0f, 100.0f}), 1, carte));

    REQUIRE(monstre.getPuissanceAttaque() == 10);

    REQUIRE(monstre.cibleEngagee() == 1);
    REQUIRE(monstre.avancerAttaque(0.4f, generateur) == 10);  // le coup engagé part normalement
    REQUIRE(monstre.getPuissanceAttaque() == 10);
    REQUIRE_FALSE(engager(monstre, pointCible({125.0f, 100.0f}), 1, carte));  // la recharge court toujours
}

TEST_CASE("Monstre.recevoirDegats renvoie les degats subis apres armure") {
    Monstre sansArmure({100.0f, 100.0f}, 1, 0, 0);
    Monstre arme({100.0f, 100.0f}, 2, 20, 0);

    REQUIRE(sansArmure.recevoirDegats(10) == 10);
    REQUIRE(arme.recevoirDegats(10) == 8);  // 10 x 100 / 120 = 8,33 -> 8
}

TEST_CASE("Monstre.recevoirDegats sur un monstre mort renvoie zero") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    monstre.recevoirDegats(monstre.getPointsDeVie());

    REQUIRE(monstre.recevoirDegats(10) == 0);
}

TEST_CASE("Monstre.recevoirDegats en retour renvoie zero") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    amenerEnRetourHorsTerritoire(monstre, carte, 50);

    REQUIRE(monstre.recevoirDegats(10) == 0);
}
