// ./tests/test_Controleur.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Controleur.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"

namespace {

// Niveau dégagé de 15 × 15 cases de 40 px ; l'escouade part de (200, 200),
// son centre est en (210, 210). Le lanceur du skill de zone (place 4) est
// le tireur en (185, 235).
Niveau niveauDegage(const std::vector<sf::Vector2f>& monstres) {
    std::vector<MonstreDuNiveau> deLaPartie;
    for (const sf::Vector2f position : monstres) {
        deLaPartie.push_back({position, static_cast<int>(deLaPartie.size())});  // chacun son groupe
    }
    return {std::vector<std::string>(15, std::string(15, '.')), 40.0f, {200.0f, 200.0f}, deLaPartie};
}

constexpr sf::Vector2f s_pointDeLancer{250.0f, 250.0f};  // à portée du lanceur
// Hors de portée de détection : la partie n'est pas gagnée, donc le monde bouge.
constexpr sf::Vector2f s_monstreLointain{550.0f, 550.0f};

void avancer(Partie& partie, int nombreDeTicsAFaire) {
    for (int i = 0; i < nombreDeTicsAFaire; ++i) {
        partie.mettreAJour(s_dureeTic);
    }
}

}  // namespace

TEST_CASE("4 entre en visee quand le skill est pret") {
    Partie partie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;

    controleur.appuiTouche(partie, 4);

    REQUIRE(controleur.getPlaceEnVisee() == 4);
}

TEST_CASE("4 n'entre pas en visee si le skill n'est pas pret") {
    Partie partie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    controleur.appuiTouche(partie, 4);
    controleur.appuiGauche(partie, s_pointDeLancer);
    avancer(partie, 1);
    REQUIRE_FALSE(partie.getEscouade().skillEstPret(4));

    controleur.debutImage(partie);
    controleur.appuiTouche(partie, 4);

    REQUIRE_FALSE(controleur.getPlaceEnVisee().has_value());
}

TEST_CASE("un clic en visee lance le sort et sort de la visee") {
    Partie partie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    controleur.appuiTouche(partie, 4);

    controleur.appuiGauche(partie, s_pointDeLancer);

    REQUIRE_FALSE(controleur.getPlaceEnVisee().has_value());
    REQUIRE(partie.getEscouade().skillEstPret(4)); // rien ne part avant mettreAJour
    avancer(partie, 1);
    REQUIRE_FALSE(partie.getEscouade().skillEstPret(4));
}

TEST_CASE("clic droit, Echap et 4 annulent la visee sans consommer la recharge") {
    Partie partie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;

    controleur.appuiTouche(partie, 4);
    controleur.appuiDroit();
    REQUIRE_FALSE(controleur.getPlaceEnVisee().has_value());

    controleur.appuiTouche(partie, 4);
    controleur.appuiEchap();
    REQUIRE_FALSE(controleur.getPlaceEnVisee().has_value());

    controleur.appuiTouche(partie, 4);
    controleur.appuiTouche(partie, 4);
    REQUIRE_FALSE(controleur.getPlaceEnVisee().has_value());

    avancer(partie, 1);
    REQUIRE(partie.getEscouade().skillEstPret(4));
}

TEST_CASE("un bouton tenu a l'entree en visee ne deplace pas l'escouade avant un nouvel appui") {
    Partie partie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const sf::Vector2f depart = partie.getEscouade().getCentre();
    const sf::Vector2f curseur{500.0f, 500.0f};

    controleur.appuiGauche(partie, curseur);  // arme le déplacement continu
    controleur.appuiTouche(partie, 4);
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);
    verifierPointProche(partie.getEscouade().getCentre(), depart);

    controleur.appuiGauche(partie, s_pointDeLancer);  // lance, sort de la visée
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);
    verifierPointProche(partie.getEscouade().getCentre(), depart);

    controleur.appuiGauche(partie, curseur);  // nouvel appui au sol
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);
    REQUIRE((partie.getEscouade().getCentre() - depart).length() > 10.0f);
}

TEST_CASE("la mort du lanceur fait sortir de la visee au debutImage suivant") {
    std::vector<sf::Vector2f> monstres;
    for (int colonne = 0; colonne < 5; ++colonne) {
        for (int ligne = 0; ligne < 6; ++ligne) {
            monstres.push_back({300.0f + 35.0f * static_cast<float>(colonne), 150.0f + 35.0f * static_cast<float>(ligne)});
        }
    }
    Partie partie(niveauDegage(monstres), s_graineDeTest);
    Controleur controleur;
    controleur.appuiTouche(partie, 4);
    for (int i = 0; i < 5400 && partie.getEscouade().estVivante(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE_FALSE(partie.getEscouade().estVivante());
    REQUIRE(controleur.getPlaceEnVisee() == 4);

    controleur.debutImage(partie);

    REQUIRE_FALSE(controleur.getPlaceEnVisee().has_value());
}

TEST_CASE("un clic au sol arme le deplacement continu, un clic sur un monstre non") {
    Partie partie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);
    Controleur controleur;
    const sf::Vector2f depart = partie.getEscouade().getCentre();
    const sf::Vector2f curseur{20.0f, 200.0f};  // à l'opposé du monstre

    controleur.appuiGauche(partie, curseur);
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);
    REQUIRE(partie.getEscouade().getCentre().x < depart.x);

    // Le combat engagé fait avancer l'escouade vers le monstre ; l'ordre du
    // curseur, à l'opposé, ne doit plus être donné.
    controleur.appuiGauche(partie, {400.0f, 400.0f});
    const float xAvant = partie.getEscouade().getCentre().x;
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);
    REQUIRE(partie.getEscouade().getCentre().x > xAvant);
}
