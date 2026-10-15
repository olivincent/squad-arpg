// ./tests/test_Partie.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"

namespace {

// Niveau dégagé de 15 × 15 cases de 40 px (600 × 600 px), sans mur
// intérieur ; l'escouade part de (200, 200), son centre est donc en (210, 210).
// Mêlées en (235, 185) et (235, 235), tireurs en (185, 185) et (185, 235).
Niveau niveauDegageEnGroupes(const std::vector<MonstreDuNiveau>& monstres) {
    return {std::vector<std::string>(15, std::string(15, '.')), 40.0f, {200.0f, 200.0f}, monstres};
}

// Chaque monstre a son propre groupe.
Niveau niveauDegage(const std::vector<sf::Vector2f>& monstres) {
    std::vector<MonstreDuNiveau> deLaPartie;
    for (const sf::Vector2f position : monstres) {
        deLaPartie.push_back({position, static_cast<int>(deLaPartie.size())});
    }
    return niveauDegageEnGroupes(deLaPartie);
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
    // frappe pas avec son attaque de base pendant l'activation ni juste après.
    Partie partie(niveauDegage({{235.0f, 150.0f}}), s_graineDeTest);

    // 0,35 s : le skill est parti (activation de 0,3 s) et les tireurs ont eu
    // le temps d'exécuter leur coup (0,15 s), pas le mêlée, dont la recharge
    // de l'attaque de base a été relancée au complet (au moins 0,3 s) au départ.
    partie.demanderSkill(1);
    avancer(partie, nombreDeTics(0.35f));

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

TEST_CASE("estGagnee est faux tant qu'il reste des monstres") {
    Partie partie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE(partie.monstresRestants() == 1);
}

TEST_CASE("estGagnee est vrai quand tous les monstres sont tues") {
    Partie partie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && !partie.estGagnee(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.estGagnee());
    REQUIRE_FALSE(partie.estPerdue());
    REQUIRE(partie.monstresRestants() == 0);
}

TEST_CASE("une partie perdue n'est pas gagnee") {
    Partie partie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.estPerdue());
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("sans monstre au depart la partie est gagnee") {
    Partie partie(niveauDegage({}), s_graineDeTest);

    REQUIRE(partie.estGagnee());
    REQUIRE(partie.monstresRestants() == 0);
}

TEST_CASE("le compteur de monstres baisse quand un monstre meurt") {
    Partie partie(niveauDegage({{235.0f, 210.0f}, {550.0f, 550.0f}}), s_graineDeTest);
    REQUIRE(partie.monstresRestants() == 2);

    for (int i = 0; i < 600 && partie.monstresRestants() == 2; ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.monstresRestants() == 1);
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("une partie gagnee se fige") {
    Partie partie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
    for (int i = 0; i < 600 && !partie.estGagnee(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.estGagnee());

    const sf::Vector2f centre = partie.getEscouade().getCentre();
    partie.ordonnerDeplacement({500.0f, 500.0f});
    avancer(partie, nombreDeTics(2.0f));

    verifierPointProche(partie.getEscouade().getCentre(), centre);
    REQUIRE(partie.estGagnee());
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

TEST_CASE("un monstre qui prepare son coup ne bouge pas, meme si l'escouade s'eloigne") {
    // Monstre au contact du mêlée arrière-droit (235, 235), à 35 px.
    Partie partie(niveauDegage({{270.0f, 235.0f}}), s_graineDeTest);
    const sf::Vector2f depart = partie.getMonstres().front().getCentre();
    partie.mettreAJour(s_dureeTic);  // le coup est engagé

    partie.ordonnerDeplacement({100.0f, 500.0f});
    avancer(partie, nombreDeTics(0.2f));  // moins que les 0,3 s d'exécution

    verifierPointProche(partie.getMonstres().front().getCentre(), depart, 0.01f);
}

TEST_CASE("un monstre reprend la poursuite une fois son coup parti") {
    Partie partie(niveauDegage({{270.0f, 235.0f}}), s_graineDeTest);
    partie.mettreAJour(s_dureeTic);
    partie.ordonnerDeplacement({100.0f, 500.0f});
    avancer(partie, nombreDeTics(1.0f));

    REQUIRE((partie.getMonstres().front().getCentre() - sf::Vector2f(270.0f, 235.0f)).length() > 5.0f);
}

// Alerte du groupe. Le monstre proche (360, 210) voit l'escouade (150 px) ; le
// monstre lointain (560, 560) est à plus de 300 px de tout personnage : seule
// une alerte le fait bouger.

namespace {

const sf::Vector2f s_monstreProche{360.0f, 210.0f};
const sf::Vector2f s_monstreLoinPoste{560.0f, 560.0f};

bool aBouge(const Partie& partie, std::size_t indice, sf::Vector2f depart) {
    return partie.getMonstres()[indice].getCentre() != depart;
}

}  // namespace

TEST_CASE("un monstre qui detecte l'escouade alerte son groupe meme hors de sa detection") {
    Partie partie(niveauDegageEnGroupes({{s_monstreProche, 0}, {s_monstreLoinPoste, 0}}), s_graineDeTest);

    avancer(partie, 5);

    REQUIRE(aBouge(partie, 0, s_monstreProche));
    REQUIRE(aBouge(partie, 1, s_monstreLoinPoste));
}

TEST_CASE("un monstre d'un autre groupe n'est pas alerte") {
    Partie partie(niveauDegageEnGroupes({{s_monstreProche, 0}, {s_monstreLoinPoste, 1}}), s_graineDeTest);

    avancer(partie, 5);

    REQUIRE(aBouge(partie, 0, s_monstreProche));
    REQUIRE_FALSE(aBouge(partie, 1, s_monstreLoinPoste));
}

TEST_CASE("un monstre seul dans son groupe ne change rien pour les autres") {
    Partie partie(niveauDegageEnGroupes({{s_monstreProche, 0}}), s_graineDeTest);

    avancer(partie, 5);

    REQUIRE(aBouge(partie, 0, s_monstreProche));
    REQUIRE(partie.getMonstres().size() == 1);
}

TEST_CASE("sans monstre en poursuite personne n'est alerte") {
    Partie partie(niveauDegageEnGroupes({{s_monstreLoinPoste, 0}, {{560.0f, 480.0f}, 0}}), s_graineDeTest);

    avancer(partie, 30);

    REQUIRE_FALSE(aBouge(partie, 0, s_monstreLoinPoste));
    REQUIRE_FALSE(aBouge(partie, 1, {560.0f, 480.0f}));
}

TEST_CASE("un monstre touche par un skill de zone alerte son groupe") {
    // Le monstre touché (420, 350) est hors de la détection (plus de 200 px de
    // tout personnage) mais à portée du skill de zone (place 4) ; il passe en
    // poursuite au départ du sort, et son voisin de groupe le suit.
    const sf::Vector2f touche{420.0f, 350.0f};
    Partie partie(niveauDegageEnGroupes({{touche, 0}, {s_monstreLoinPoste, 0}}), s_graineDeTest);
    partie.lancerSkillDeZone(4, touche);

    avancer(partie, nombreDeTics(0.7f)); // activation de 0,8 s : rien n'est encore parti
    REQUIRE_FALSE(aBouge(partie, 0, touche));
    REQUIRE_FALSE(aBouge(partie, 1, s_monstreLoinPoste));

    avancer(partie, nombreDeTics(0.5f));

    REQUIRE(aBouge(partie, 0, touche));
    REQUIRE(aBouge(partie, 1, s_monstreLoinPoste));
}

// Cible de frappe d'un monstre : un monstre provoqué ne frappe que son
// provocateur, même collé à un autre personnage.
TEST_CASE("un monstre provoque ne frappe que son provocateur") {
    Partie partie(niveauDegage({{300.0f, 235.0f}}), s_graineDeTest);
    partie.demanderSkill(2);  // provocation du mêlée avant-droit (place 2)

    bool aFrappeLeProvocateur = false;
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Monstre& monstre : partie.getMonstres()) {
            if (const std::optional<int> cible = monstre.cibleEngagee(); cible && i > nombreDeTics(0.3f)) {
                REQUIRE(*cible == 2);
                aFrappeLeProvocateur = true;
            }
        }
    }
    REQUIRE(aFrappeLeProvocateur);
}
