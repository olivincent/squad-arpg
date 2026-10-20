// ./tests/escouade/test_Separation.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "aides.hpp"
#include "escouade/Separation.hpp"

namespace {

// Valeurs de test, indépendantes de celles du jeu : avec un dt de 0,1 s, la poussée d'un membre
// est plafonnée à 10 px par appel.
constexpr float s_ecartDeTest = 30.0f;
constexpr float s_vitesseDeTest = 100.0f;
constexpr float s_dtDeTest = 0.1f;
constexpr float s_tolerance = 0.0001f;

const sf::Vector2f s_nulle(0.0f, 0.0f);

std::vector<sf::Vector2f> separer(const std::vector<MembreASeparer>& membres) {
    return pousseesDeSeparation(membres, s_ecartDeTest, s_vitesseDeTest, s_dtDeTest);
}

}  // namespace

TEST_CASE("pousseesDeSeparation : aucun membre, aucune poussee") {
    REQUIRE(separer({}).empty());
}

TEST_CASE("pousseesDeSeparation : deux membres assez ecartes ne sont pas pousses") {
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Libre}, {{30.0f, 0.0f}, EtatDeSeparation::Libre}});

    REQUIRE(poussees.size() == 2);
    verifierPointProche(poussees[0], s_nulle, s_tolerance);
    verifierPointProche(poussees[1], s_nulle, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : deux libres trop proches prennent chacun la moitie du manque") {
    // Centres à 20 px pour un écart de 30 : 10 px manquent, 5 chacun, en sens opposés.
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Libre}, {{20.0f, 0.0f}, EtatDeSeparation::Libre}});

    verifierPointProche(poussees[0], {-5.0f, 0.0f}, s_tolerance);
    verifierPointProche(poussees[1], {5.0f, 0.0f}, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : la poussee suit la droite des centres") {
    // Centres à 20 px en diagonale (12, 16) : direction (0,6 ; 0,8), 5 px chacun.
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Libre}, {{12.0f, 16.0f}, EtatDeSeparation::Libre}});

    verifierPointProche(poussees[0], {-3.0f, -4.0f}, s_tolerance);
    verifierPointProche(poussees[1], {3.0f, 4.0f}, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : un membre fixe n'est pas pousse et l'autre prend tout le manque") {
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Fixe}, {{20.0f, 0.0f}, EtatDeSeparation::Libre}});

    verifierPointProche(poussees[0], s_nulle, s_tolerance);
    verifierPointProche(poussees[1], {10.0f, 0.0f}, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : deux membres fixes ne sont pas pousses") {
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Fixe}, {{20.0f, 0.0f}, EtatDeSeparation::Fixe}});

    verifierPointProche(poussees[0], s_nulle, s_tolerance);
    verifierPointProche(poussees[1], s_nulle, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : un absent ne pousse pas et n'est pas pousse") {
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Absent}, {{20.0f, 0.0f}, EtatDeSeparation::Libre}});

    verifierPointProche(poussees[0], s_nulle, s_tolerance);
    verifierPointProche(poussees[1], s_nulle, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : centres confondus, le plus petit indice part vers la gauche") {
    // 30 px manquent : 15 chacun, plafonnés à 10.
    const auto poussees = separer({{{50.0f, 50.0f}, EtatDeSeparation::Libre}, {{50.0f, 50.0f}, EtatDeSeparation::Libre}});

    verifierPointProche(poussees[0], {-10.0f, 0.0f}, s_tolerance);
    verifierPointProche(poussees[1], {10.0f, 0.0f}, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : la poussee est plafonnee a vitesse fois dt") {
    // Centres à 2 px : 28 px manquent, 14 chacun, plafonnés à 100 × dt.
    const std::vector<MembreASeparer> membres = {{{0.0f, 0.0f}, EtatDeSeparation::Libre}, {{2.0f, 0.0f}, EtatDeSeparation::Libre}};

    const auto avecUnDixieme = pousseesDeSeparation(membres, s_ecartDeTest, s_vitesseDeTest, 0.1f);
    verifierPointProche(avecUnDixieme[0], {-10.0f, 0.0f}, s_tolerance);
    verifierPointProche(avecUnDixieme[1], {10.0f, 0.0f}, s_tolerance);

    const auto avecUnVingtieme = pousseesDeSeparation(membres, s_ecartDeTest, s_vitesseDeTest, 0.05f);
    verifierPointProche(avecUnVingtieme[0], {-5.0f, 0.0f}, s_tolerance);
    verifierPointProche(avecUnVingtieme[1], {5.0f, 0.0f}, s_tolerance);
}

TEST_CASE("pousseesDeSeparation : toutes les paires sont jugees sur les centres recus") {
    // En ligne, à 20 px d'écart : A-B et B-C manquent chacune de 10 px, A-C est assez loin. B reçoit
    // +5 de A et -5 de C : il ne bouge pas. Un calcul séquentiel (B déplacé avant la paire B-C)
    // donnerait un autre résultat.
    const auto poussees = separer({{{0.0f, 0.0f}, EtatDeSeparation::Libre},
                                   {{20.0f, 0.0f}, EtatDeSeparation::Libre},
                                   {{40.0f, 0.0f}, EtatDeSeparation::Libre}});

    verifierPointProche(poussees[0], {-5.0f, 0.0f}, s_tolerance);
    verifierPointProche(poussees[1], s_nulle, s_tolerance);
    verifierPointProche(poussees[2], {5.0f, 0.0f}, s_tolerance);
}
