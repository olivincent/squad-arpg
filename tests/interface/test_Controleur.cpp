// ./tests/interface/test_Controleur.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "interface/BarreDeRaccourcis.hpp"
#include "interface/Controleur.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Partie.hpp"

namespace {

// Niveau : niveauDegage (aides.hpp). Le lanceur du skill de zone (le mage, place 3)
// est le tireur en (185, 235).

// La barre par défaut de la composition par défaut : Q frappe, W provocation, A soin, Z explosion.
BarreDeRaccourcis barreDe(const Partie& partie) {
    return barreParDefaut(partie.getEscouade().adressesDesSkills());
}
constexpr std::size_t s_emplacementProvocation = 1;
constexpr std::size_t s_emplacementVide = 2;
constexpr std::size_t s_emplacementExplosion = 8;

constexpr sf::Vector2f s_pointDeLancer{250.0f, 250.0f};  // à portée du lanceur
// Hors de portée de détection : la partie n'est pas gagnée, donc le monde bouge.
constexpr sf::Vector2f s_monstreLointain{550.0f, 550.0f};

}  // namespace

TEST_CASE("un emplacement de zone entre en visee quand le skill est pret") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);

    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);

    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);
}

TEST_CASE("un emplacement de zone n'entre pas en visee si le skill n'est pas pret") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    controleur.appuiGauche(partie, s_pointDeLancer);
    avancer(partie, 1);
    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseExplosion));

    controleur.debutImage(partie);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);

    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
}

TEST_CASE("un clic en visee lance le sort et sort de la visee") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);

    controleur.appuiGauche(partie, s_pointDeLancer);

    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseExplosion)); // rien ne part avant mettreAJour
    avancer(partie, 1);
    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseExplosion));
}

TEST_CASE("clic droit, Echap et le meme emplacement annulent la visee sans consommer la recharge") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);

    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    controleur.appuiDroit();
    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());

    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    controleur.appuiEchap();
    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());

    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());

    avancer(partie, 1);
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseExplosion));
}

TEST_CASE("un bouton tenu a l'entree en visee ne deplace pas l'escouade avant un nouvel appui") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    const sf::Vector2f depart = partie.getEscouade().getCentre();
    const sf::Vector2f curseur{500.0f, 500.0f};

    controleur.appuiGauche(partie, curseur);  // arme le déplacement continu
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
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
    Partie partie = unePartie(niveauDegage(monstres), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    for (int i = 0; i < 5400 && partie.getEscouade().estVivante(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE_FALSE(partie.getEscouade().estVivante());
    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);

    controleur.debutImage(partie);

    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
}

TEST_CASE("un clic au sol arme le deplacement continu, un clic sur un monstre non") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);
    Controleur controleur;
    const sf::Vector2f depart = partie.getEscouade().getCentre();
    const sf::Vector2f curseur{20.0f, 200.0f};  // à l'opposé du monstre

    controleur.appuiGauche(partie, curseur);
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 10);  // court : le monstre doit rester dans la vision (320 px) pour être cliqué
    REQUIRE(partie.getEscouade().getCentre().x < depart.x);

    // Le combat engagé fait avancer l'escouade vers le monstre ; l'ordre du
    // curseur, à l'opposé, ne doit plus être donné.
    controleur.appuiGauche(partie, {400.0f, 400.0f});
    const float xAvant = partie.getEscouade().getCentre().x;
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);
    REQUIRE(partie.getEscouade().getCentre().x > xAvant);
}

TEST_CASE("un clic sur une porte desarme le deplacement continu") {
    std::vector<std::string> plan(11, std::string(15, '.'));
    for (int y = 0; y < 11; ++y) {
        plan[static_cast<std::size_t>(y)][7] = '#';
    }
    plan[4][7] = '.';
    plan[5][7] = '.';
    Niveau niveau{plan, 40.0f, {200.0f, 200.0f}, {{{60.0f, 60.0f}, 0}}};
    niveau.portes = {PorteDuNiveau{{{7, 4}, {7, 5}}}};
    Partie partie = unePartie(niveau, s_graineDeTest);
    Controleur controleur;
    controleur.appuiGauche(partie, {100.0f, 200.0f});  // sol : déplacement continu armé
    controleur.appuiGauche(partie, {300.0f, 200.0f});  // porte : désarmé

    for (int i = 0; i < 120; ++i) {
        // Bouton tenu, curseur loin de la porte : sans désarmement, l'ordre écraserait l'approche.
        controleur.mettreAJourDeplacement(partie, true, {100.0f, 200.0f});
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.cliquer({300.0f, 200.0f}) == ResultatClic::Sol);  // la porte s'est ouverte
}

TEST_CASE("un emplacement vide ne fait rien") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);

    controleur.activerEmplacement(partie, barre, s_emplacementVide);
    avancer(partie, 1);

    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseProvocation));
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseExplosion));
}

TEST_CASE("un emplacement de skill direct le demande a la partie") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);

    controleur.activerEmplacement(partie, barre, s_emplacementProvocation);
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseProvocation));  // rien ne part avant mettreAJour
    avancer(partie, 1);

    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseProvocation));
    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
}

TEST_CASE("un skill direct pendant la visee est permis et la visee continue") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);

    controleur.activerEmplacement(partie, barre, s_emplacementProvocation);
    avancer(partie, 1);

    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);
    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseProvocation));
}

TEST_CASE("la meme adresse sur deux emplacements : le second la fait sortir de la visee") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    BarreDeRaccourcis barre = barreDe(partie);
    barre.placer(s_emplacementVide, s_adresseExplosion);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);
    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);

    controleur.activerEmplacement(partie, barre, s_emplacementVide);

    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
}

TEST_CASE("un autre skill de zone remplace la visee") {
    // Deux mages : le second est à la place 4.
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition.push_back(composition.back());
    Partie partie(niveauDegage({s_monstreLointain}), composition, s_graineDeTest);
    const AdresseSkill explosionDuSecondMage{4, IdSkill::Explosion};
    BarreDeRaccourcis barre(s_emplacementsPrincipaux);
    barre.placer(0, s_adresseExplosion);
    barre.placer(1, explosionDuSecondMage);
    Controleur controleur;

    controleur.activerEmplacement(partie, barre, 0);
    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);
    controleur.activerEmplacement(partie, barre, 1);
    REQUIRE(controleur.getAdresseEnVisee() == explosionDuSecondMage);

    controleur.activerEmplacement(partie, barre, 1);  // la même adresse : on sort
    REQUIRE_FALSE(controleur.getAdresseEnVisee().has_value());
}

TEST_CASE("un clic de barre active l'emplacement comme sa touche") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);

    controleur.appuiInterface(partie, barre, {Barre::Raccourcis, s_emplacementExplosion});
    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);

    controleur.appuiInterface(partie, barre, {Barre::Raccourcis, s_emplacementProvocation});
    avancer(partie, 1);
    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseProvocation));
}

TEST_CASE("un clic de barre desarme le deplacement continu, meme sur un emplacement vide") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    const sf::Vector2f depart = partie.getEscouade().getCentre();
    const sf::Vector2f curseur{500.0f, 500.0f};
    controleur.appuiGauche(partie, curseur);  // clic au sol : deplacement continu arme

    controleur.appuiInterface(partie, barre, {Barre::Raccourcis, s_emplacementVide});
    controleur.mettreAJourDeplacement(partie, true, curseur);
    avancer(partie, 30);

    verifierPointProche(partie.getEscouade().getCentre(), depart);
}

TEST_CASE("en visee, un clic de barre n'est jamais le point d'impact") {
    Partie partie = unePartie(niveauDegage({s_monstreLointain}), s_graineDeTest);
    Controleur controleur;
    const BarreDeRaccourcis barre = barreDe(partie);
    controleur.activerEmplacement(partie, barre, s_emplacementExplosion);

    controleur.appuiInterface(partie, barre, {Barre::Raccourcis, s_emplacementVide});
    avancer(partie, 1);

    REQUIRE(controleur.getAdresseEnVisee() == s_adresseExplosion);    // toujours en visee
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseExplosion));  // rien n'est lance
}
