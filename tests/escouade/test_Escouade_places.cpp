// ./tests/escouade/test_Escouade_places.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "escouade/Composition.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/TileMap.hpp"

namespace {

// Indices dans cerclesDesVivants() quand tous sont vivants, pour la composition par défaut :
// le guerrier (0), le prêtre (1) et le mage (2).
constexpr std::size_t s_guerrier = 0;
constexpr std::size_t s_pretre = 1;
constexpr std::size_t s_mage = 2;

// Pour une escouade à deux guerriers (uneEscouadeADeuxGuerriers) : deux mêlées (0 et 1), le prêtre et le mage.
constexpr std::size_t s_melee1 = 0;
constexpr std::size_t s_melee2 = 1;

constexpr float s_tolerance = 0.5f;

// Image de Partie, sans les monstres qui bougent : attaquer() puis mettreAJour(), les
// monstres comme obstacles. `apres` est appelée à la fin de chaque image.
template <typename Fonction>
void combattre(Escouade& escouade, const TileMap& carte, const std::vector<EntiteVue>& monstres, float duree,
               Generateur& generateur, Fonction apres) {
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, monstres, carte, generateur));
        escouade.mettreAJour(s_dureeTic, carte, {.physiques = monstres, .ciblables = monstres, .deControle = {}});
        apres();
    }
}

void combattre(Escouade& escouade, const TileMap& carte, const std::vector<EntiteVue>& monstres, float duree,
               Generateur& generateur) {
    combattre(escouade, carte, monstres, duree, generateur, [] {});
}

// Angle du personnage autour du monstre, en degrés dans ]-180, 180].
float angleAutour(Cercle personnage, Cercle monstre) {
    return (personnage.centre - monstre.centre).angle().wrapSigned().asDegrees();
}

// Écart angulaire entre deux personnages autour du monstre, signé, dans ]-180, 180].
float ecartAngulaire(Cercle a, Cercle b, Cercle monstre) {
    return (sf::degrees(angleAutour(a, monstre)) - sf::degrees(angleAutour(b, monstre))).wrapSigned().asDegrees();
}

bool estAPorteeDe(Cercle personnage, Cercle monstre, float portee) {
    return distanceBordABord(personnage, monstre) <= portee + s_tolerance;
}

}  // namespace

TEST_CASE("Places d'attaque : le guerrier arrive au contact, les tireurs s'arretent a portee, sans se superposer") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({300.0f, 500.0f});
    const EntiteVue monstre{1, cibleMonstre({700.0f, 510.0f})};
    escouade.verrouiller(1);

    combattre(escouade, carte, {monstre}, 8.0f, generateur);

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    REQUIRE(cercles.size() == 3);
    REQUIRE(estAPorteeDe(cercles[s_guerrier], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(estAPorteeDe(cercles[s_pretre], monstre.cercle, s_porteeTireurDeTest));
    REQUIRE(estAPorteeDe(cercles[s_mage], monstre.cercle, s_porteeTireurDeTest));
    for (std::size_t i = 0; i < cercles.size(); ++i) {
        for (std::size_t j = i + 1; j < cercles.size(); ++j) {
            REQUIRE((cercles[i].centre - cercles[j].centre).length() >= 34.0f - s_tolerance);
        }
    }
}

TEST_CASE("Places d'attaque : deux melees en combat verrouille arrivent de cotes differents") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeADeuxGuerriers({300.0f, 500.0f});
    const EntiteVue monstre{1, cibleMonstre({700.0f, 510.0f})};
    escouade.verrouiller(1);

    combattre(escouade, carte, {monstre}, 8.0f, generateur);

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    REQUIRE(cercles.size() == 4);
    // Les deux mêlées frappent (à portée de leur attaque), chacune de son côté, sans se superposer.
    REQUIRE(estAPorteeDe(cercles[s_melee1], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(estAPorteeDe(cercles[s_melee2], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(std::abs(ecartAngulaire(cercles[s_melee1], cercles[s_melee2], monstre.cercle)) >= 30.0f);
    REQUIRE((cercles[s_melee1].centre - cercles[s_melee2].centre).length() >= 34.0f - s_tolerance);
}

TEST_CASE("Places d'attaque : un tireur s'arrete des qu'il est a portee, sur un angle different") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({300.0f, 500.0f});
    const EntiteVue monstre{1, cibleMonstre({700.0f, 510.0f})};
    escouade.verrouiller(1);

    combattre(escouade, carte, {monstre}, 8.0f, generateur);

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    for (const std::size_t tireur : {s_pretre, s_mage}) {
        REQUIRE(estAPorteeDe(cercles[tireur], monstre.cercle, s_porteeTireurDeTest));
        // Arrêté à distance : bien plus loin que le contact du guerrier.
        REQUIRE(distanceBordABord(cercles[tireur], monstre.cercle) > 60.0f);
    }
    REQUIRE(std::abs(ecartAngulaire(cercles[s_pretre], cercles[s_mage], monstre.cercle)) >= 10.0f);
}

TEST_CASE("Places d'attaque : l'angle est garde quand le monstre bouge") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeADeuxGuerriers({300.0f, 500.0f});
    EntiteVue monstre{1, cibleMonstre({700.0f, 510.0f})};
    escouade.verrouiller(1);
    combattre(escouade, carte, {monstre}, 6.0f, generateur);
    const float avant = ecartAngulaire(escouade.cerclesDesVivants()[s_melee1], escouade.cerclesDesVivants()[s_melee2], monstre.cercle);

    // Le monstre s'éloigne de 80 px : les mêlées le suivent sans échanger leurs places.
    monstre.cercle.centre += sf::Vector2f(80.0f, 0.0f);
    combattre(escouade, carte, {monstre}, 6.0f, generateur);

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    const float apres = ecartAngulaire(cercles[s_melee1], cercles[s_melee2], monstre.cercle);
    REQUIRE(estAPorteeDe(cercles[s_melee1], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(estAPorteeDe(cercles[s_melee2], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(avant * apres > 0.0f);  // même ordre autour du monstre
}

TEST_CASE("Places d'attaque : au changement de cible, chacun prend sa place autour du nouveau monstre") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeADeuxGuerriers({300.0f, 500.0f});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({700.0f, 510.0f})},
                                             EntiteVue{2, cibleMonstre({500.0f, 800.0f})}};
    escouade.verrouiller(1);
    combattre(escouade, carte, monstres, 6.0f, generateur);

    escouade.verrouiller(2);
    combattre(escouade, carte, monstres, 8.0f, generateur);

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    const Cercle nouveau = monstres[1].cercle;
    REQUIRE(estAPorteeDe(cercles[s_melee1], nouveau, s_porteeMeleeDeTest));
    REQUIRE(estAPorteeDe(cercles[s_melee2], nouveau, s_porteeMeleeDeTest));
    REQUIRE(std::abs(ecartAngulaire(cercles[s_melee1], cercles[s_melee2], nouveau)) >= 30.0f);
}

TEST_CASE("Places d'attaque : une place dans un mur est sautee") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();  // mur de droite : x >= 760
    Escouade escouade = uneEscouadeADeuxGuerriers({300.0f, 300.0f});
    // À 25 px du mur : les places du côté du mur (centre à 43 px du monstre) sont dans le mur.
    const EntiteVue monstre{1, cibleMonstre({735.0f, 310.0f})};
    escouade.verrouiller(1);

    combattre(escouade, carte, {monstre}, 10.0f, generateur, [&] {
        for (const Cercle& cercle : escouade.cerclesDesVivants()) {
            REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect(cercle.centre - sf::Vector2f(10.0f, 10.0f), {20.0f, 20.0f})));
        }
    });

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    REQUIRE(estAPorteeDe(cercles[s_melee1], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(estAPorteeDe(cercles[s_melee2], monstre.cercle, s_porteeMeleeDeTest));
}

TEST_CASE("Places d'attaque : un agresseur sans verrou, le melee a portee frappe et l'autre le rejoint sans se superposer") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeADeuxGuerriers({300.0f, 500.0f});  // mêlées en (335, 485) et (335, 535)
    // À portée du premier mêlée seulement.
    const EntiteVue monstre{1, cibleMonstre({345.0f, 470.0f})};
    infliger(escouade, 1, monstre.cercle.centre, 10);  // le monstre devient agresseur

    // Premier tic : le mêlée à portée reste sur place, l'autre vise une place d'attaque, pas le centre.
    combattre(escouade, carte, {monstre}, 0.02f, generateur);
    const std::optional<sf::Vector2f> pointDuSecond = escouade.pointVise(2);
    REQUIRE(pointDuSecond.has_value());
    // Sur l'anneau d'attaque : rayon du monstre + rayon du personnage (10) + 90 % de la portée.
    const float anneau = monstre.cercle.rayon + 10.0f + 0.9f * s_porteeMeleeDeTest;
    const float ecartAuCentre = (*pointDuSecond - monstre.cercle.centre).length();
    REQUIRE(ecartAuCentre > anneau - 3.0f);
    REQUIRE(ecartAuCentre < anneau + 3.0f);

    combattre(escouade, carte, {monstre}, 2.0f, generateur);

    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    REQUIRE(estAPorteeDe(cercles[s_melee1], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE(estAPorteeDe(cercles[s_melee2], monstre.cercle, s_porteeMeleeDeTest));
    REQUIRE((cercles[s_melee1].centre - cercles[s_melee2].centre).length() >= 34.0f - s_tolerance);
}

TEST_CASE("Places d'attaque : sans place valide, le personnage vise le centre du monstre") {
    Generateur generateur = generateurDeTest();
    // Un monstre dans une case fermée : toutes les places autour de lui sont dans un mur.
    const TileMap carte({"##########",
                         "#........#",
                         "#...###..#",
                         "#...#.#..#",
                         "#...###..#",
                         "##########"},
                        40.0f);
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const EntiteVue monstre{1, cibleMonstre({220.0f, 140.0f})};
    escouade.verrouiller(1);

    combattre(escouade, carte, {monstre}, 0.02f, generateur);

    for (std::size_t place = 1; place <= 3; ++place) {
        const std::optional<sf::Vector2f> point = escouade.pointVise(place);
        REQUIRE(point.has_value());
        REQUIRE(point->x == monstre.cercle.centre.x);
        REQUIRE(point->y == monstre.cercle.centre.y);
    }
}

TEST_CASE("pointVise : place de formation hors combat, vide pour une place invalide") {
    Escouade escouade = uneEscouade({300.0f, 500.0f});
    escouade.mettreAJour(s_dureeTic, carteOuverte(), {});

    for (std::size_t place = 1; place <= 3; ++place) {
        const std::optional<sf::Vector2f> point = escouade.pointVise(place);
        REQUIRE(point.has_value());
        verifierPointProche(*point, escouade.getPlacesDeFormation()[place - 1], 0.01f);
    }
    REQUIRE_FALSE(escouade.pointVise(0).has_value());
    REQUIRE_FALSE(escouade.pointVise(4).has_value());
}
