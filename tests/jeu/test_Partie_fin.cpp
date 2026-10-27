// ./tests/jeu/test_Partie_fin.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/Partie.hpp"

TEST_CASE("une fois l'escouade vaincue, les clics et les demandes sont sans effet") {
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);

    for (int i = 0; i < 5400 && partie.getEscouade().estVivante(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE_FALSE(partie.getEscouade().estVivante());
    REQUIRE_FALSE(partie.getMonstres().empty());

    const sf::Vector2f centre = partie.getEscouade().getCentre();
    const sf::Vector2f cible = partie.getMonstres().front().getCentre();
    REQUIRE(partie.cliquer(cible) == ResultatClic::Ignore);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());

    partie.demanderSkill(s_adresseFrappe);
    partie.ordonnerDeplacement({500.0f, 500.0f});
    avancer(partie, nombreDeTics(2.0f));

    verifierPointProche(partie.getEscouade().getCentre(), centre);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("estPerdue est faux au depart") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("estPerdue est faux tant qu'un personnage survit et vrai quand tous sont morts") {
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);

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
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
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

TEST_CASE("estGagnee est faux au depart, monstres ou non") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE(partie.monstresRestants() == 1);
}

TEST_CASE("tuer tous les monstres ne gagne plus la partie : il faut prendre l'escalier du dernier etage") {
    Partie partie = unePartie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && partie.monstresRestants() > 0; ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.monstresRestants() == 0);
    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("une partie perdue n'est pas gagnee") {
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.estPerdue());
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("sans monstre au depart la partie n'est pas gagnee") {
    Partie partie = unePartie(niveauDegage({}), s_graineDeTest);

    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE(partie.monstresRestants() == 0);
}

TEST_CASE("le compteur de monstres baisse quand un monstre meurt") {
    Partie partie = unePartie(niveauDegage({{235.0f, 210.0f}, {550.0f, 550.0f}}), s_graineDeTest);
    REQUIRE(partie.monstresRestants() == 2);

    for (int i = 0; i < 600 && partie.monstresRestants() == 2; ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.monstresRestants() == 1);
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("une partie gagnee se fige") {
    Expedition expedition = expeditionDeTest();
    expedition.profondeur = s_profondeurMaximale;
    Partie partie(niveauDegage({{550.0f, 550.0f}}), std::move(expedition), s_graineDeTest, catalogueDeTest(monstreSansHasard()));
    partie.forcerLaDescente();
    REQUIRE(partie.estGagnee());

    const sf::Vector2f centre = partie.getEscouade().getCentre();
    partie.ordonnerDeplacement({500.0f, 500.0f});
    avancer(partie, nombreDeTics(2.0f));

    verifierPointProche(partie.getEscouade().getCentre(), centre);
    REQUIRE(partie.estGagnee());
}

TEST_CASE("une partie reaffectee avec un niveau neuf recommence") {
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.estPerdue());

    partie = unePartie(niveauDegage({{400.0f, 400.0f}, {450.0f, 400.0f}, {500.0f, 400.0f},
                                  {400.0f, 450.0f}, {450.0f, 450.0f}, {500.0f, 450.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estPerdue());
    REQUIRE(partie.getMonstres().size() == 6);
    const int vieDUnMonstreNeuf = Monstre({0.0f, 0.0f}, IdMonstre{1}, 0, definitionDeMonstre(CategorieDeMonstre::Infanterie)).getPointsDeVie();
    for (const Monstre& monstre : partie.getMonstres()) {
        REQUIRE(monstre.getPointsDeVie() == vieDUnMonstreNeuf);
    }
    verifierPointProche(partie.getEscouade().getCentre(), {210.0f, 210.0f});
}
