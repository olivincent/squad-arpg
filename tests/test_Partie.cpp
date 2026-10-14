// ./tests/test_Partie.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"

namespace {

// Niveau dégagé de 15 × 15 cases de 40 px (600 × 600 px), sans mur
// intérieur ; l'escouade part de (200, 200), son centre est donc en (210, 210).
// Mêlées en (235, 185) et (235, 235), tireurs en (185, 185) et (185, 235).
Niveau niveauDegage(const std::vector<sf::Vector2f>& monstres) {
    return {std::vector<std::string>(15, std::string(15, '.')), 40.0f, {200.0f, 200.0f}, monstres};
}

// Fait avancer la partie de ce nombre de tics de 1/60 s.
void avancer(Partie& partie, int nombreDeTicsAFaire) {
    for (int i = 0; i < nombreDeTicsAFaire; ++i) {
        partie.mettreAJour(s_dureeTic);
    }
}

// Une grille de 30 monstres serrés devant l'escouade, assez pour la vaincre.
std::vector<sf::Vector2f> groupeDeMonstres() {
    std::vector<sf::Vector2f> monstres;
    for (int colonne = 0; colonne < 5; ++colonne) {
        for (int ligne = 0; ligne < 6; ++ligne) {
            monstres.push_back({300.0f + 35.0f * static_cast<float>(colonne), 150.0f + 35.0f * static_cast<float>(ligne)});
        }
    }
    return monstres;
}

}  // namespace

TEST_CASE("cliquer sur un monstre le verrouille") {
    Partie partie(niveauDegage({{400.0f, 400.0f}, {300.0f, 400.0f}}), s_graineDeTest);

    REQUIRE(partie.cliquer({300.0f, 400.0f}) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee() == 2);
}

TEST_CASE("cliquer au sol renvoie Sol et leve le verrou") {
    Partie partie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({400.0f, 400.0f}) == ResultatClic::Monstre);

    REQUIRE(partie.cliquer({100.0f, 500.0f}) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("un monstre tue est retire et le verrou est leve") {
    Partie partie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.getMonstres().empty());
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("aucun monstre mort ne reste dans le vecteur apres mettreAJour") {
    Partie partie(niveauDegage({{235.0f, 210.0f}, {245.0f, 240.0f}}), s_graineDeTest);

    for (int i = 0; i < 600; ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Monstre& monstre : partie.getMonstres()) {
            REQUIRE(monstre.estVivant());
        }
    }

    REQUIRE(partie.getMonstres().size() < 2);
}

TEST_CASE("un skill demande part dans mettreAJour et sa recharge demarre") {
    Partie partie(niveauDegage({{235.0f, 150.0f}}), s_graineDeTest);
    REQUIRE(partie.getEscouade().skillEstPret(1));

    partie.demanderSkill(1);
    REQUIRE(partie.getEscouade().skillEstPret(1)); // rien ne part avant mettreAJour
    partie.mettreAJour(s_dureeTic);

    REQUIRE_FALSE(partie.getEscouade().skillEstPret(1));
}

TEST_CASE("un monstre approche poursuit et frappe") {
    Partie partie(niveauDegage({{400.0f, 210.0f}}), s_graineDeTest);

    for (int i = 0; i < 600 && partie.getEscouade().getAgresseurs().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE_FALSE(partie.getEscouade().getAgresseurs().empty());
}

TEST_CASE("une fois l'escouade vaincue, les clics et les demandes sont sans effet") {
    Partie partie(niveauDegage(groupeDeMonstres()), s_graineDeTest);

    for (int i = 0; i < 5400 && partie.getEscouade().estVivante(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE_FALSE(partie.getEscouade().estVivante());
    REQUIRE_FALSE(partie.getMonstres().empty());

    const sf::Vector2f centre = partie.getEscouade().getCentre();
    const sf::Vector2f cible = partie.getMonstres().front().getCentre();
    REQUIRE(partie.cliquer(cible) == ResultatClic::Ignore);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());

    partie.demanderSkill(1);
    partie.ordonnerDeplacement({500.0f, 500.0f});
    avancer(partie, nombreDeTics(2.0f));

    verifierPointProche(partie.getEscouade().getCentre(), centre);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("un skill et attaquer dans la meme image ne frappent pas deux fois le meme lanceur") {
    // Monstre à portée du mêlée avant-gauche (skill de 25) et des deux
    // tireurs (attaques de base de 5). Le mêlée qui lance son skill ne
    // frappe pas avec son attaque de base dans la même image.
    Partie partie(niveauDegage({{235.0f, 150.0f}}), s_graineDeTest);

    // 0,25 s : les tireurs ont eu le temps d'exécuter leur coup (0,15 s),
    // pas le mêlée, dont la recharge a été relancée au complet (au moins 0,3 s).
    partie.demanderSkill(1);
    avancer(partie, nombreDeTics(0.25f));

    REQUIRE(partie.getMonstres().front().getPointsDeVie() == 100 - 25 - 5 - 5);
}

TEST_CASE("estPerdue est faux au depart") {
    Partie partie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("estPerdue est faux tant qu'un personnage survit et vrai quand les 4 sont morts") {
    Partie partie(niveauDegage(groupeDeMonstres()), s_graineDeTest);

    bool survivantUnique = false;
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
        if (partie.getEscouade().cerclesDesVivants().size() == 1) {
            survivantUnique = true;
            REQUIRE_FALSE(partie.estPerdue());
        }
    }

    REQUIRE(survivantUnique);
    REQUIRE(partie.estPerdue());
}

TEST_CASE("une partie perdue se fige") {
    Partie partie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.estPerdue());
    REQUIRE_FALSE(partie.getMonstres().empty());

    std::vector<sf::Vector2f> positions;
    std::vector<int> pointsDeVie;
    for (const Monstre& monstre : partie.getMonstres()) {
        positions.push_back(monstre.getCentre());
        pointsDeVie.push_back(monstre.getPointsDeVie());
    }

    avancer(partie, nombreDeTics(3.0f));

    REQUIRE(partie.getMonstres().size() == positions.size());
    for (std::size_t i = 0; i < positions.size(); ++i) {
        verifierPointProche(partie.getMonstres()[i].getCentre(), positions[i]);
        REQUIRE(partie.getMonstres()[i].getPointsDeVie() == pointsDeVie[i]);
    }
}

TEST_CASE("une partie reaffectee avec un niveau neuf recommence") {
    Partie partie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.estPerdue());

    partie = Partie(niveauDegage({{400.0f, 400.0f}, {450.0f, 400.0f}, {500.0f, 400.0f},
                                  {400.0f, 450.0f}, {450.0f, 450.0f}, {500.0f, 450.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estPerdue());
    REQUIRE(partie.getMonstres().size() == 6);
    for (const Monstre& monstre : partie.getMonstres()) {
        REQUIRE(monstre.getPointsDeVie() == 100);
    }
    verifierPointProche(partie.getEscouade().getCentre(), {210.0f, 210.0f});
}
