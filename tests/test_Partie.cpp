// ./tests/test_Partie.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "EvenementDeCombat.hpp"
#include "Monstre.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"
#include "Skill.hpp"

namespace {

// Niveau dégagé de 15 × 15 cases de 40 px (600 × 600 px), sans mur
// intérieur ; l'escouade part de (200, 200), son centre est donc en (210, 210).
// Guerrier en (235, 210), tireurs en (185, 185) et (185, 235).
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
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}, {300.0f, 400.0f}}), s_graineDeTest);

    REQUIRE(partie.cliquer({300.0f, 400.0f}) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee() == 2);
}

TEST_CASE("cliquer au sol renvoie Sol et leve le verrou") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({400.0f, 400.0f}) == ResultatClic::Monstre);

    REQUIRE(partie.cliquer({100.0f, 500.0f}) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("un monstre tue est retire et le verrou est leve") {
    Partie partie = unePartie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.getMonstres().empty());
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("aucun monstre mort ne reste dans le vecteur apres mettreAJour") {
    Partie partie = unePartie(niveauDegage({{235.0f, 210.0f}, {245.0f, 240.0f}}), s_graineDeTest);

    for (int i = 0; i < 600; ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Monstre& monstre : partie.getMonstres()) {
            REQUIRE(monstre.estVivant());
        }
    }

    REQUIRE(partie.getMonstres().size() < 2);
}

TEST_CASE("un skill demande part dans mettreAJour et sa recharge demarre") {
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseFrappe));

    partie.demanderSkill(s_adresseFrappe);
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseFrappe)); // rien ne part avant mettreAJour
    partie.mettreAJour(s_dureeTic);

    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseFrappe));
}

TEST_CASE("un monstre approche poursuit et frappe") {
    Partie partie = unePartie(niveauDegage({{400.0f, 210.0f}}), s_graineDeTest);

    for (int i = 0; i < 600 && partie.getEscouade().getAgresseurs().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE_FALSE(partie.getEscouade().getAgresseurs().empty());
}

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

TEST_CASE("un skill et attaquer dans la meme image ne frappent pas deux fois le meme lanceur") {
    // Monstre à portée du guerrier (skill de 25) et des deux
    // tireurs (attaques de base de 5). Le guerrier qui lance son skill ne
    // frappe pas avec son attaque de base pendant l'activation ni juste après.
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);

    // 0,35 s : le skill est parti (activation de 0,3 s) et les tireurs ont eu
    // le temps d'exécuter leur coup (0,15 s), pas le mêlée, dont la recharge
    // de l'attaque de base a été relancée au complet (au moins 0,3 s) au départ.
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.35f));

    REQUIRE(partie.getMonstres().front().getPointsDeVie() == 100 - 25 - 5 - 5);
}

TEST_CASE("estPerdue est faux au depart") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("estPerdue est faux tant qu'un personnage survit et vrai quand les 4 sont morts") {
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

TEST_CASE("estGagnee est faux tant qu'il reste des monstres") {
    Partie partie = unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE(partie.monstresRestants() == 1);
}

TEST_CASE("estGagnee est vrai quand tous les monstres sont tues") {
    Partie partie = unePartie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && !partie.estGagnee(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.estGagnee());
    REQUIRE_FALSE(partie.estPerdue());
    REQUIRE(partie.monstresRestants() == 0);
}

TEST_CASE("une partie perdue n'est pas gagnee") {
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.estPerdue());
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("sans monstre au depart la partie est gagnee") {
    Partie partie = unePartie(niveauDegage({}), s_graineDeTest);

    REQUIRE(partie.estGagnee());
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
    Partie partie = unePartie(niveauDegage({{235.0f, 210.0f}}), s_graineDeTest);
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
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.estPerdue());

    partie = unePartie(niveauDegage({{400.0f, 400.0f}, {450.0f, 400.0f}, {500.0f, 400.0f},
                                  {400.0f, 450.0f}, {450.0f, 450.0f}, {500.0f, 450.0f}}), s_graineDeTest);

    REQUIRE_FALSE(partie.estPerdue());
    REQUIRE(partie.getMonstres().size() == 6);
    for (const Monstre& monstre : partie.getMonstres()) {
        REQUIRE(monstre.getPointsDeVie() == 100);
    }
    verifierPointProche(partie.getEscouade().getCentre(), {210.0f, 210.0f});
}

TEST_CASE("un monstre qui prepare son coup ne bouge pas, meme si l'escouade s'eloigne") {
    // Monstre au contact du guerrier (235, 210), à 35 px.
    Partie partie = unePartie(niveauDegage({{270.0f, 210.0f}}), s_graineDeTest);
    const sf::Vector2f depart = partie.getMonstres().front().getCentre();
    partie.mettreAJour(s_dureeTic);  // le coup est engagé

    partie.ordonnerDeplacement({100.0f, 500.0f});
    avancer(partie, nombreDeTics(0.2f));  // moins que les 0,3 s d'exécution

    verifierPointProche(partie.getMonstres().front().getCentre(), depart, 0.01f);
}

TEST_CASE("un monstre reprend la poursuite une fois son coup parti") {
    Partie partie = unePartie(niveauDegage({{270.0f, 210.0f}}), s_graineDeTest);
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
    Partie partie = unePartie(niveauDegageEnGroupes({{s_monstreProche, 0}, {s_monstreLoinPoste, 0}}), s_graineDeTest);

    avancer(partie, 5);

    REQUIRE(aBouge(partie, 0, s_monstreProche));
    REQUIRE(aBouge(partie, 1, s_monstreLoinPoste));
}

TEST_CASE("un monstre d'un autre groupe n'est pas alerte") {
    Partie partie = unePartie(niveauDegageEnGroupes({{s_monstreProche, 0}, {s_monstreLoinPoste, 1}}), s_graineDeTest);

    avancer(partie, 5);

    REQUIRE(aBouge(partie, 0, s_monstreProche));
    REQUIRE_FALSE(aBouge(partie, 1, s_monstreLoinPoste));
}

TEST_CASE("un monstre seul dans son groupe ne change rien pour les autres") {
    Partie partie = unePartie(niveauDegageEnGroupes({{s_monstreProche, 0}}), s_graineDeTest);

    avancer(partie, 5);

    REQUIRE(aBouge(partie, 0, s_monstreProche));
    REQUIRE(partie.getMonstres().size() == 1);
}

TEST_CASE("sans monstre en poursuite personne n'est alerte") {
    Partie partie = unePartie(niveauDegageEnGroupes({{s_monstreLoinPoste, 0}, {{560.0f, 480.0f}, 0}}), s_graineDeTest);

    avancer(partie, 30);

    REQUIRE_FALSE(aBouge(partie, 0, s_monstreLoinPoste));
    REQUIRE_FALSE(aBouge(partie, 1, {560.0f, 480.0f}));
}

TEST_CASE("un monstre touche par un skill de zone alerte son groupe") {
    // Le monstre touché (420, 350) est hors de la détection (plus de 200 px de
    // tout personnage) mais à portée du skill de zone (mage, place 3) ; il passe en
    // poursuite au départ du sort, et son voisin de groupe le suit.
    const sf::Vector2f touche{420.0f, 350.0f};
    Partie partie = unePartie(niveauDegageEnGroupes({{touche, 0}, {s_monstreLoinPoste, 0}}), s_graineDeTest);
    partie.lancerSkillDeZone(s_adresseExplosion, touche);

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
    Partie partie = unePartie(niveauDegage({{300.0f, 235.0f}}), s_graineDeTest);
    partie.demanderSkill(s_adresseProvocation);  // provocation du guerrier (place 1)

    bool aFrappeLeProvocateur = false;
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Monstre& monstre : partie.getMonstres()) {
            if (const std::optional<int> cible = monstre.cibleEngagee(); cible && i > nombreDeTics(0.3f)) {
                REQUIRE(*cible == 1);
                aFrappeLeProvocateur = true;
            }
        }
    }
    REQUIRE(aFrappeLeProvocateur);
}

// Retour au poste (étape 69). Couloir dégagé de 60 × 15 cases de 40 px (2400 × 600) : le
// monstre est à son poste (400, 300), l'escouade en (560, 300), à 160 px de lui. L'escouade
// s'enfuit vers l'est : le monstre la poursuit jusqu'à 600 px de son poste (x = 1000), puis
// abandonne et rentre.

namespace {

const sf::Vector2f s_posteDuFuyard{400.0f, 300.0f};

Niveau niveauDeLaFuite() {
    return {std::vector<std::string>(15, std::string(60, '.')), 40.0f, {550.0f, 290.0f}, {{s_posteDuFuyard, 0}}};
}

const Monstre& fuyard(const Partie& partie) {
    return partie.getMonstres().front();
}

// Envoie l'escouade au point d'arrivée et avance jusqu'à ce que le monstre abandonne (retour au poste).
void attirerHorsDuTerritoire(Partie& partie, sf::Vector2f arrivee) {
    partie.ordonnerDeplacement(arrivee);
    for (int i = 0; i < nombreDeTics(15.0f) && fuyard(partie).estCiblable(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE_FALSE(fuyard(partie).estCiblable());
}

}  // namespace

TEST_CASE("un clic sur un monstre qui rentre a son poste compte comme un clic au sol") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    attirerHorsDuTerritoire(partie, {1080.0f, 300.0f});

    REQUIRE(partie.cliquer(fuyard(partie).getCentre()) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("un monstre qui abandonne n'est plus provoque par une provocation") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    attirerHorsDuTerritoire(partie, {1080.0f, 300.0f});

    partie.demanderSkill(s_adresseProvocation);  // provocation du mêlée avant-droit : zone de 150 px
    for (int i = 0; i < nombreDeTics(0.5f); ++i) {
        partie.mettreAJour(s_dureeTic);
        REQUIRE(partie.getEscouade().getProvocations().empty());
    }
    REQUIRE_FALSE(fuyard(partie).estCiblable());
}

TEST_CASE("un monstre qui rentre a son poste bloque toujours les personnages") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    attirerHorsDuTerritoire(partie, {1080.0f, 300.0f});

    partie.ordonnerDeplacement({300.0f, 300.0f});  // l'escouade rentre plus vite que lui, à travers lui
    float ecartMin = 1e9f;
    float rayons = 0.0f;
    for (int i = 0; i < nombreDeTics(5.0f) && !fuyard(partie).estCiblable(); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Cercle& personnage : partie.getEscouade().cerclesDesVivants()) {
            const float ecart = (personnage.centre - fuyard(partie).getCentre()).length();
            ecartMin = std::min(ecartMin, ecart);
            rayons = personnage.rayon + fuyard(partie).getCercle().rayon;
            REQUIRE(ecart >= rayons - 0.5f);
        }
    }
    REQUIRE(ecartMin <= rayons + 1.0f);  // un personnage a bien touché le monstre sans le traverser
}

TEST_CASE("le verrou pose sur un monstre est leve quand il abandonne") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    partie.ordonnerDeplacement({1150.0f, 300.0f});
    bool verrouille = false;
    for (int i = 0; i < nombreDeTics(15.0f) && fuyard(partie).estCiblable(); ++i) {
        partie.mettreAJour(s_dureeTic);
        if (!verrouille && fuyard(partie).getCentre().x > 985.0f) { // la frontière est à x = 1000
            REQUIRE(partie.cliquer(fuyard(partie).getCentre()) == ResultatClic::Monstre);
            verrouille = true;
        }
    }
    REQUIRE(verrouille);
    REQUIRE_FALSE(fuyard(partie).estCiblable());

    partie.mettreAJour(s_dureeTic);

    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

// L'exploit de la laisse : des tireurs immobiles juste au-delà du territoire touchaient le
// monstre à chaque aller-retour sans jamais être frappés. Maintenant, il rentre sans réagir et
// arrive plein à son poste.
TEST_CASE("un monstre qui abandonne rentre a son poste sans perdre de vie et sans revenir") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    partie.ordonnerDeplacement({1150.0f, 300.0f});

    bool aAbandonne = false;
    int vieAvant = fuyard(partie).getPointsDeVie();
    for (int i = 0; i < nombreDeTics(25.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        const Monstre& monstre = fuyard(partie);
        if (!monstre.estCiblable()) {
            aAbandonne = true;
            REQUIRE(monstre.getPointsDeVie() >= vieAvant);
            REQUIRE_FALSE(monstre.estEnPoursuite());
        }
        vieAvant = monstre.getPointsDeVie();
    }

    REQUIRE(aAbandonne);
    REQUIRE(fuyard(partie).estCiblable());
    verifierPointProche(fuyard(partie).getCentre(), s_posteDuFuyard);
    REQUIRE(fuyard(partie).getPointsDeVie() == 100);
}

namespace {

// Deux pièces de 15 × 11 cases (40 px) séparées par un mur en colonne 7
// (x: [280, 320)) percé d'une porte de 2 cases en (7, 4) et (7, 5), soit la zone
// [280, 320] × [160, 240]. L'escouade part de (200, 200), côté ouest.
Niveau niveauAvecPorte(const std::vector<MonstreDuNiveau>& monstres = {{{60.0f, 60.0f}, 0}}) {
    std::vector<std::string> plan(11, std::string(15, '.'));
    for (int y = 0; y < 11; ++y) {
        plan[static_cast<std::size_t>(y)][7] = '#';
    }
    plan[4][7] = '.';
    plan[5][7] = '.';
    Niveau niveau{plan, 40.0f, {200.0f, 200.0f}, monstres};
    niveau.portes = {PorteDuNiveau{{{7, 4}, {7, 5}}}};
    return niveau;
}

constexpr sf::Vector2f s_surLaPorte{300.0f, 200.0f};
constexpr sf::Vector2f s_coteEst{420.0f, 200.0f};

}  // namespace

TEST_CASE("porte : fermee a la construction, l'escouade ne la traverse pas") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);

    partie.ordonnerDeplacement(s_coteEst);
    avancer(partie, 300);

    REQUIRE(partie.getEscouade().getCentre().x < 280.0f);
}

TEST_CASE("porte : un clic dessus renvoie Objet et leve le verrou") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer({60.0f, 60.0f}) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee().has_value());

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("porte : l'escouade s'avance, la porte s'ouvre et le chemin passe") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    avancer(partie, 120);

    // Ouverte : un clic dessus compte comme un clic au sol.
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);
    // L'escouade s'est arrêtée devant, sans franchir la porte.
    REQUIRE(partie.getEscouade().getCentre().x < 300.0f);

    partie.ordonnerDeplacement(s_coteEst);
    avancer(partie, 300);
    REQUIRE(partie.getEscouade().getCentre().x > 400.0f);
}

TEST_CASE("porte : un ordre de deplacement annule l'interaction") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    partie.ordonnerDeplacement({100.0f, 200.0f});
    avancer(partie, 120);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);  // toujours fermée
}

TEST_CASE("porte : un clic au sol annule l'interaction") {
    Partie partie = unePartie(niveauAvecPorte(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    REQUIRE(partie.cliquer({100.0f, 200.0f}) == ResultatClic::Sol);
    avancer(partie, 120);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);
}

TEST_CASE("porte : un monstre ciblable sur la porte a la priorite") {
    Partie partie = unePartie(niveauAvecPorte({{s_surLaPorte, 0}}), s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Monstre);
}

TEST_CASE("porte : sans cote libre, le clic compte comme un clic au sol") {
    Niveau niveau = niveauAvecPorte();
    for (const int y : {4, 5}) {
        niveau.plan[static_cast<std::size_t>(y)][6] = '#';
        niveau.plan[static_cast<std::size_t>(y)][8] = '#';
    }
    Partie partie = unePartie(niveau, s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);
}

TEST_CASE("porte : une partie figee n'ouvre rien") {
    Partie partie = unePartie(niveauAvecPorte({}), s_graineDeTest);  // aucun monstre : gagnée
    REQUIRE(partie.estGagnee());
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    avancer(partie, 120);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);
}

namespace {

// La place avant de la formation (le guerrier) est-elle plus proche du point que les deux places arrière ?
bool formationFaitFace(const Partie& partie, sf::Vector2f point) {
    const std::vector<sf::Vector2f>& places = partie.getEscouade().getPlacesDeFormation();
    const float avant = (places[0] - point).length();
    const float arriereMin = std::min((places[1] - point).length(), (places[2] - point).length());
    return avant < arriereMin;
}

// Monstre loin de la porte et de l'escouade, pour que la partie ne soit pas gagnée.
const std::vector<MonstreDuNiveau> s_monstreLointain = {{{560.0f, 400.0f}, 0}};

// Escouade à l'est de la porte (centre (360, 200)), tournée vers l'est : ses tireurs sont
// collés à la porte, qu'elle a dans le dos.
Niveau niveauCollePorteDeDos() {
    Niveau niveau = niveauAvecPorte(s_monstreLointain);
    niveau.departEscouade = {350.0f, 190.0f};
    return niveau;
}

}  // namespace

TEST_CASE("porte : un personnage deja a portee ouvre la porte sur place, sans deplacement") {
    Partie partie = unePartie(niveauCollePorteDeDos(), s_graineDeTest);
    const sf::Vector2f centreAvant = partie.getEscouade().getCentre();

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);  // déjà ouverte, avant tout mettreAJour
    avancer(partie, 30);
    verifierPointProche(partie.getEscouade().getCentre(), centreAvant, 0.01f);
}

TEST_CASE("porte : de dos et collee a la porte, la formation fait face a la porte apres l'ouverture sur place") {
    Partie partie = unePartie(niveauCollePorteDeDos(), s_graineDeTest);
    partie.mettreAJour(s_dureeTic);
    REQUIRE_FALSE(formationFaitFace(partie, s_surLaPorte));

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);
    avancer(partie, 60);

    REQUIRE(formationFaitFace(partie, s_surLaPorte));
}

TEST_CASE("porte : apres l'ouverture par approche, la formation est tournee vers la porte") {
    Niveau niveau = niveauAvecPorte(s_monstreLointain);
    niveau.departEscouade = {250.0f, 80.0f};  // au nord-ouest : l'approche se fait vers le sud
    Partie partie = unePartie(niveau, s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);
    avancer(partie, 200);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);  // ouverte
    REQUIRE(formationFaitFace(partie, s_surLaPorte));
}

TEST_CASE("porte : loin de la porte, la porte reste fermee tant que l'escouade n'est pas arrivee") {
    Partie partie = unePartie(niveauAvecPorte(s_monstreLointain), s_graineDeTest);  // départ (210, 210), à 70 px

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);

    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Objet);  // pas encore ouverte
    avancer(partie, 120);
    REQUIRE(partie.cliquer(s_surLaPorte) == ResultatClic::Sol);
}

namespace {

// Salle de 30 × 11 cases (40 px) ; l'escouade part de (200, 200), son centre est en (210, 210).
Niveau niveauLong(const std::vector<MonstreDuNiveau>& monstres) {
    return {std::vector<std::string>(11, std::string(30, '.')), 40.0f, {200.0f, 200.0f}, monstres};
}

// Un monstre à 610 px de l'escouade : hors de sa vision (320 px) et de sa détection (200 px).
const sf::Vector2f s_monstreAuLoin{820.0f, 220.0f};

}  // namespace

TEST_CASE("vision : un monstre de la salle de depart est cliquable des la construction") {
    Partie partie = unePartie(niveauLong({{{300.0f, 210.0f}, 0}}), s_graineDeTest);  // aucun mettreAJour

    REQUIRE(partie.cliquer({300.0f, 210.0f}) == ResultatClic::Monstre);
}

TEST_CASE("vision : un monstre hors de vue ne peut pas etre clique, le clic est un clic au sol") {
    Partie partie = unePartie(niveauLong({{s_monstreAuLoin, 0}}), s_graineDeTest);

    REQUIRE(partie.cliquer(s_monstreAuLoin) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("vision : un monstre cache par un mur n'est pas cliquable") {
    Niveau niveau = niveauLong({{{500.0f, 200.0f}, 0}});  // à 290 px : dans le rayon, mais derrière le mur
    for (std::string& ligne : niveau.plan) {
        ligne[8] = '#';
    }
    Partie partie = unePartie(niveau, s_graineDeTest);

    REQUIRE(partie.cliquer({500.0f, 200.0f}) == ResultatClic::Sol);
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("vision : le meme monstre devient cliquable quand l'escouade s'en approche") {
    Partie partie = unePartie(niveauLong({{s_monstreAuLoin, 0}}), s_graineDeTest);
    REQUIRE(partie.cliquer(s_monstreAuLoin) == ResultatClic::Sol);

    partie.ordonnerDeplacement({540.0f, 210.0f});  // à 280 px du monstre : vu, mais pas détecté
    avancer(partie, 150);

    REQUIRE(partie.cliquer(s_monstreAuLoin) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee() == 1);
}

namespace {

// Salle de 30 × 11 cases coupée par un mur en colonne 25, percé d'une porte de 2 cases en (25, 4)
// et (25, 5), soit la zone [1000, 1040] × [160, 240]. Un monstre au sud-ouest, loin de tout.
Niveau niveauPorteLointaine() {
    Niveau niveau = niveauLong({{{60.0f, 400.0f}, 0}});
    for (std::string& ligne : niveau.plan) {
        ligne[25] = '#';
    }
    niveau.plan[4][25] = '.';
    niveau.plan[5][25] = '.';
    niveau.portes = {PorteDuNiveau{{{25, 4}, {25, 5}}}};
    return niveau;
}

constexpr sf::Vector2f s_surLaPorteLointaine{1020.0f, 200.0f};

}  // namespace

TEST_CASE("vision : une porte dans l'inconnu n'est pas cliquable, le clic est un clic au sol") {
    Partie partie = unePartie(niveauPorteLointaine(), s_graineDeTest);

    REQUIRE(partie.cliquer(s_surLaPorteLointaine) == ResultatClic::Sol);
}

TEST_CASE("vision : une porte vue devient cliquable") {
    Partie partie = unePartie(niveauPorteLointaine(), s_graineDeTest);
    REQUIRE(partie.cliquer(s_surLaPorteLointaine) == ResultatClic::Sol);

    partie.ordonnerDeplacement({800.0f, 200.0f});  // à 5 cases de la porte
    avancer(partie, 250);

    REQUIRE(partie.cliquer(s_surLaPorteLointaine) == ResultatClic::Objet);
}

// Zones de contrôle et attaque d'opportunité (étape 77). Le monstre n'a pas d'armure dans ces niveaux
// (Niveau::armureMonstres vaut 0) : les dégâts lus sur lui sont ceux des attaques de base.

namespace {

// Fait avancer la partie d'un tic et renvoie les points de vie du personnage de cette place.
int pointsDeVieApresUnTic(Partie& partie, std::size_t place) {
    partie.mettreAJour(s_dureeTic);
    return *partie.getEscouade().pointsDeVieDuPersonnage(place);
}

// Avance jusqu'à ce que le personnage de cette place perde des points de vie (le premier coup du
// monstre) ; renvoie ses points de vie à ce moment. Le coup lance la recharge du monstre (0,7 s au
// moins) puis un nouveau coup exige 0,3 s d'exécution : aucun coup normal avant 1 s.
int jusquAuPremierCoup(Partie& partie, std::size_t place) {
    int pointsDeVie = 100;
    for (int i = 0; i < nombreDeTics(2.0f) && pointsDeVie == 100; ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, place);
    }
    REQUIRE(pointsDeVie < 100);
    return pointsDeVie;
}

constexpr float s_fenetreSansCoupNormal = 0.9f;  // en secondes, après le premier coup du monstre

}  // namespace

TEST_CASE("zone de controle : un mage au contact qui s'eloigne recoit un coup gratuit du monstre") {
    // Un mage seul (zone 0) à 40 px du monstre, qui l'engage (zone de 20 px bord à bord).
    Partie partie(niveauDegage({{185.0f, 250.0f}}), composer({IdSpecialite::Mage}), s_graineDeTest);
    const int apresLePremierCoup = jusquAuPremierCoup(partie, 1);

    partie.ordonnerDeplacement({210.0f, 20.0f});
    int pointsDeVie = apresLePremierCoup;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, 1);
    }

    REQUIRE(pointsDeVie == apresLePremierCoup - 10);  // un seul coup gratuit, de 10 dégâts (armure 0)
}

TEST_CASE("zone de controle : un guerrier qui reste au contact ne recoit aucun coup gratuit") {
    Partie partie(niveauDegage({{270.0f, 210.0f}}), composer({IdSpecialite::Guerrier}), s_graineDeTest);
    const int apresLePremierCoup = jusquAuPremierCoup(partie, 1);

    int pointsDeVie = apresLePremierCoup;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, 1);
    }

    REQUIRE(pointsDeVie == apresLePremierCoup);
}

namespace {

// Trois guerriers (le premier au nord, à (235, 160)) et un monstre à (270, 160), au contact du
// premier seulement. L'attaque de base du premier est reconnaissable : `degats` dégâts, recharge
// de 100 s, donc après son premier coup tout nouveau coup de cette force sur le monstre est un coup
// gratuit. Dès que le monstre est touché, le troisième guerrier (à (235, 260)) le provoque : le
// monstre quitte le premier pour aller vers lui. Renvoie le nombre d'images où le monstre perd au
// moins `degats` points de vie, sur 1,2 s.
int coupsReconnaissables(float zoneDuPremier, int degats) {
    std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Guerrier, IdSpecialite::Guerrier});
    composition[0].attaqueDeBase = Skill(TypeSkill::Degats, 20.0f, degats, 100.0f, 100.0f, 0.0f, 0.15f);
    composition[0].zoneDeControle = zoneDuPremier;
    Partie partie(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest);

    int coups = 0;
    int pointsDeVie = 100;
    bool provoque = false;
    for (int i = 0; i < nombreDeTics(1.2f) && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
        if (partie.getMonstres().empty()) {
            break;
        }
        const int apres = partie.getMonstres().front().getPointsDeVie();
        if (pointsDeVie - apres >= degats) {
            ++coups;
        }
        pointsDeVie = apres;
        if (!provoque && apres < 100) {
            partie.demanderSkill({3, IdSkill::Provocation});
            provoque = true;
        }
    }
    return coups;
}

}  // namespace

TEST_CASE("zone de controle : un monstre qui quitte un guerrier pour un provocateur est frappe gratuitement") {
    // Le premier coup (normal) et le coup gratuit.
    REQUIRE(coupsReconnaissables(20.0f, 30) == 2);
}

TEST_CASE("zone de controle : un personnage sans zone ne frappe jamais gratuitement") {
    // Même scénario, mais le premier guerrier n'engage personne : seul son premier coup (normal) compte.
    REQUIRE(coupsReconnaissables(0.0f, 30) == 1);
}

TEST_CASE("zone de controle : un monstre tue par un coup gratuit est retire et le verrou est leve") {
    // Le premier coup (60) laisse le monstre à 40 points de vie ; le coup gratuit (60) le tue.
    std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Guerrier, IdSpecialite::Guerrier});
    composition[0].attaqueDeBase = Skill(TypeSkill::Degats, 20.0f, 60, 100.0f, 100.0f, 0.0f, 0.15f);
    Partie partie(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest);
    REQUIRE(partie.cliquer({270.0f, 160.0f}) == ResultatClic::Monstre);

    // En 1,2 s les autres guerriers ne retirent que quelques points : seul le coup gratuit peut le tuer.
    bool provoque = false;
    for (int i = 0; i < nombreDeTics(1.2f) && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
        if (!provoque && !partie.getMonstres().empty() && partie.getMonstres().front().getPointsDeVie() < 100) {
            partie.demanderSkill({3, IdSkill::Provocation});
            provoque = true;
        }
    }

    REQUIRE(provoque);
    REQUIRE(partie.getMonstres().empty());
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("zone de controle : un monstre qui rentre a son poste ne donne ni ne recoit de coup gratuit") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    attirerHorsDuTerritoire(partie, {1080.0f, 300.0f});

    // L'escouade rentre à travers le monstre qui rentre et finit par le toucher...
    partie.ordonnerDeplacement({300.0f, 300.0f});
    bool contact = false;
    for (int i = 0; i < nombreDeTics(5.0f) && !contact; ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Cercle& personnage : partie.getEscouade().cerclesDesVivants()) {
            contact = contact || distanceBordABord(personnage, fuyard(partie).getCercle()) <= 10.0f;
        }
    }
    REQUIRE(contact);
    REQUIRE_FALSE(fuyard(partie).estCiblable());

    // ... puis fait demi-tour : elle s'écarte du monstre (lui, il continue vers son poste). Un monstre
    // qui rentre n'engage personne : aucun personnage ne perd de points de vie, et lui non plus.
    partie.ordonnerDeplacement({1080.0f, 300.0f});
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (std::size_t place = 1; place <= 3; ++place) {
            REQUIRE(partie.getEscouade().pointsDeVieDuPersonnage(place) == 100);
        }
    }
    REQUIRE_FALSE(fuyard(partie).estCiblable());
    REQUIRE(fuyard(partie).getPointsDeVie() == 100);
}

// Dégagement des tireurs (étape 78). Guerrier (place 1, en (235, 210)) et mage (place 2, en (185, 210)),
// monstre en (219, 232) : il engage les deux (écarts bord à bord de 2 et 15,5 px pour une zone de 20 px),
// et sa proie est le guerrier, plus proche. Le guerrier frappe une seule fois (1 dégât, recharge de 100 s) ;
// le mage tire des coups de 7 dégâts : on reconnaît ainsi qui touche le monstre.

namespace {

constexpr int s_degatsDuMageDansLaPartie = 7;

Partie unGuerrierEtUnMageAuContact() {
    std::vector<DefinitionPersonnage> composition = composer({IdSpecialite::Guerrier, IdSpecialite::Mage});
    composition[0].attaqueDeBase = Skill(TypeSkill::Degats, 20.0f, 1, 100.0f, 100.0f, 0.0f, 0.15f);
    composition[1].attaqueDeBase = Skill(TypeSkill::Degats, 120.0f, s_degatsDuMageDansLaPartie, 0.3f, 0.3f, 0.0f, 0.15f);
    return Partie(niveauDegage({{219.0f, 232.0f}}), std::move(composition), s_graineDeTest);
}

}  // namespace

TEST_CASE("degagement : le mage quitte le monstre tenu par le guerrier sans coup gratuit, puis tire de loin") {
    Partie partie = unGuerrierEtUnMageAuContact();
    const Cercle monstre = partie.getMonstres().front().getCercle();
    REQUIRE(distanceBordABord(partie.getEscouade().cerclesDesVivants()[0], monstre) <= 20.0f);
    REQUIRE(distanceBordABord(partie.getEscouade().cerclesDesVivants()[1], monstre) <= 20.0f);

    int pointsDuMonstreALArrivee = 0;
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        const std::vector<Cercle> cercles = partie.getEscouade().cerclesDesVivants();
        REQUIRE(partie.getEscouade().pointsDeVieDuPersonnage(2) == 100);  // aucun coup, gratuit ou non
        REQUIRE(distanceBordABord(cercles[0], monstre) <= 20.0f);          // le guerrier reste au contact
        if (pointsDuMonstreALArrivee == 0 && !partie.getEscouade().degagementDuPersonnage(2).has_value() &&
            distanceBordABord(cercles[1], monstre) > 20.0f) {
            pointsDuMonstreALArrivee = partie.getMonstres().front().getPointsDeVie();
        }
    }

    const std::vector<Cercle> cercles = partie.getEscouade().cerclesDesVivants();
    REQUIRE_FALSE(partie.getEscouade().degagementDuPersonnage(2).has_value());
    REQUIRE(distanceBordABord(cercles[1], monstre) > 20.0f);
    REQUIRE(distanceBordABord(cercles[1], monstre) <= 120.0f);  // à portée de son attaque
    REQUIRE(pointsDuMonstreALArrivee > 0);
    REQUIRE(partie.getMonstres().front().getPointsDeVie() <= pointsDuMonstreALArrivee - s_degatsDuMageDansLaPartie);
}

// Événements de combat (étape 79). Les monstres de `niveauDegage` ont une armure de 0 (le niveau n'en
// donne pas), sauf mention.

namespace {

// Avance de cette durée, tic par tic, et rend tous les événements produits.
std::vector<EvenementDeCombat> evenementsPendant(Partie& partie, float duree) {
    std::vector<EvenementDeCombat> tous;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        partie.mettreAJour(s_dureeTic);
        const std::vector<EvenementDeCombat> duTic = partie.prendreEvenements();
        tous.insert(tous.end(), duTic.begin(), duTic.end());
    }
    return tous;
}

std::vector<EvenementDeCombat> parmi(const std::vector<EvenementDeCombat>& evenements, TypeEvenement type, Camp camp) {
    std::vector<EvenementDeCombat> choisis;
    for (const EvenementDeCombat& evenement : evenements) {
        if (evenement.type == type && evenement.campCible == camp) {
            choisis.push_back(evenement);
        }
    }
    return choisis;
}

// Deux guerriers (en (235, 185) et (235, 235)) dont l'attaque de base tue d'un coup (200 dégâts), et un
// monstre en (260, 210) qu'ils atteignent tous les deux : leurs deux coups partent au même tic.
Partie deuxGuerriersQuiTuentLeMemeMonstre() {
    std::vector<DefinitionPersonnage> composition = composer({IdSpecialite::Guerrier, IdSpecialite::Guerrier});
    for (DefinitionPersonnage& definition : composition) {
        definition.attaqueDeBase = Skill(TypeSkill::Degats, 20.0f, 200, 100.0f, 100.0f, 0.0f, 0.15f);
    }
    return Partie(niveauDegage({{260.0f, 210.0f}}), std::move(composition), s_graineDeTest);
}

}  // namespace

TEST_CASE("evenements : un skill de degats donne un evenement sur le monstre, armure comprise") {
    Niveau niveau = niveauDegage({{235.0f, 170.0f}});
    niveau.armureMonstres = 20;
    Partie partie(niveau, composer({IdSpecialite::Guerrier}), s_graineDeTest);
    partie.demanderSkill(s_adresseFrappe);

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 0.5f);

    int frappes = 0;
    for (const EvenementDeCombat& evenement : parmi(evenements, TypeEvenement::Degats, Camp::Monstres)) {
        if (evenement.montant == 21) {  // 25 x 100 / 120 = 20,8 -> 21 ; l'attaque de base fait 4
            ++frappes;
            REQUIRE(evenement.idCible == 1);
            REQUIRE_FALSE(evenement.opportunite);
            verifierPointProche(evenement.position, {235.0f, 170.0f});
        }
    }
    REQUIRE(frappes == 1);
}

TEST_CASE("evenements : le coup d'un monstre donne un evenement sur le personnage touche, armure comprise") {
    Partie guerrier(niveauDegage({{270.0f, 210.0f}}), composer({IdSpecialite::Guerrier}), s_graineDeTest);
    Partie mage(niveauDegage({{185.0f, 250.0f}}), composer({IdSpecialite::Mage}), s_graineDeTest);
    static_cast<void>(jusquAuPremierCoup(guerrier, 1));
    static_cast<void>(jusquAuPremierCoup(mage, 1));

    const std::vector<EvenementDeCombat> surLeGuerrier = parmi(guerrier.prendreEvenements(), TypeEvenement::Degats, Camp::Escouade);
    const std::vector<EvenementDeCombat> surLeMage = parmi(mage.prendreEvenements(), TypeEvenement::Degats, Camp::Escouade);

    REQUIRE(surLeGuerrier.size() == 1);
    REQUIRE(surLeGuerrier[0].montant == 7);  // 10 x 100 / 150 = 6,7 -> 7
    REQUIRE(surLeGuerrier[0].idCible == 1);
    REQUIRE_FALSE(surLeGuerrier[0].opportunite);
    verifierPointProche(surLeGuerrier[0].position, *guerrier.getEscouade().centreDuPersonnage(1));
    REQUIRE(surLeMage.size() == 1);
    REQUIRE(surLeMage[0].montant == 10);
}

TEST_CASE("evenements : un skill de zone donne un evenement par monstre touche") {
    // Trois monstres hors de la détection, dans la zone de 60 px autour de (400, 340) ; l'explosion du
    // mage (15 dégâts, activation de 0,8 s) part avant qu'ils aient pu atteindre quelqu'un.
    Partie partie = unePartie(niveauDegage({{400.0f, 340.0f}, {435.0f, 340.0f}, {400.0f, 375.0f}}), s_graineDeTest);
    partie.lancerSkillDeZone(s_adresseExplosion, {400.0f, 340.0f});

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 1.0f);

    std::vector<int> touches;
    for (const EvenementDeCombat& evenement : parmi(evenements, TypeEvenement::Degats, Camp::Monstres)) {
        if (evenement.montant == 15) {
            touches.push_back(evenement.idCible);
        }
    }
    std::ranges::sort(touches);
    REQUIRE(touches == std::vector<int>{1, 2, 3});
}

TEST_CASE("evenements : un coup gratuit est marque, dans les deux sens") {
    // Le mage seul recule d'un monstre qui l'engage : coup gratuit du monstre sur le mage.
    Partie mage(niveauDegage({{185.0f, 250.0f}}), composer({IdSpecialite::Mage}), s_graineDeTest);
    static_cast<void>(jusquAuPremierCoup(mage, 1));
    static_cast<void>(mage.prendreEvenements());
    mage.ordonnerDeplacement({210.0f, 20.0f});
    const std::vector<EvenementDeCombat> surLeMage =
        parmi(evenementsPendant(mage, s_fenetreSansCoupNormal), TypeEvenement::Degats, Camp::Escouade);
    REQUIRE(surLeMage.size() == 1);
    REQUIRE(surLeMage[0].opportunite);
    REQUIRE(surLeMage[0].montant == 10);

    // Trois guerriers ; le premier frappe une fois (30 dégâts, recharge de 100 s). Dès que le monstre est
    // touché, le troisième le provoque : il quitte le premier, qui le frappe gratuitement (30 dégâts).
    std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Guerrier, IdSpecialite::Guerrier});
    composition[0].attaqueDeBase = Skill(TypeSkill::Degats, 20.0f, 30, 100.0f, 100.0f, 0.0f, 0.15f);
    Partie guerriers(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest);
    std::vector<EvenementDeCombat> evenements;
    bool provoque = false;
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        guerriers.mettreAJour(s_dureeTic);
        const std::vector<EvenementDeCombat> duTic = guerriers.prendreEvenements();
        evenements.insert(evenements.end(), duTic.begin(), duTic.end());
        if (!provoque && !evenements.empty()) {
            guerriers.demanderSkill({3, IdSkill::Provocation});
            provoque = true;
        }
    }
    int normaux = 0;
    int gratuits = 0;
    for (const EvenementDeCombat& evenement : parmi(evenements, TypeEvenement::Degats, Camp::Monstres)) {
        if (evenement.montant == 30) {
            ++(evenement.opportunite ? gratuits : normaux);
        }
    }
    REQUIRE(normaux == 1);
    REQUIRE(gratuits == 1);
}

TEST_CASE("evenements : un soin donne un evenement Soin sur l'allie soigne, avec les points reellement rendus") {
    // Le monstre blesse le guerrier ; le prêtre le soigne (30) : les points rendus sont ceux qui manquaient.
    Partie partie(niveauDegage({{270.0f, 210.0f}}), composer({IdSpecialite::Guerrier, IdSpecialite::Pretre}), s_graineDeTest);
    static_cast<void>(jusquAuPremierCoup(partie, 1));
    std::vector<EvenementDeCombat> evenements = partie.prendreEvenements();
    partie.demanderSkill({2, IdSkill::Soin});

    const std::vector<EvenementDeCombat> suite = evenementsPendant(partie, 0.8f);  // activation de 0,6 s
    evenements.insert(evenements.end(), suite.begin(), suite.end());

    const std::vector<EvenementDeCombat> soins = parmi(evenements, TypeEvenement::Soin, Camp::Escouade);
    REQUIRE(soins.size() == 1);
    REQUIRE(soins[0].idCible == 1);
    REQUIRE_FALSE(soins[0].opportunite);
    int degats = 0;
    for (const EvenementDeCombat& evenement : parmi(evenements, TypeEvenement::Degats, Camp::Escouade)) {
        degats += evenement.montant;
    }
    REQUIRE(soins[0].montant > 0);
    REQUIRE(soins[0].montant <= degats);
    REQUIRE(*partie.getEscouade().pointsDeVieDuPersonnage(1) == 100 - degats + soins[0].montant);
    verifierPointProche(soins[0].position, *partie.getEscouade().centreDuPersonnage(1));
}

TEST_CASE("evenements : un coup sur un monstre deja mort dans l'image ne produit rien") {
    // Les deux coups (200 dégâts) partent au même tic : le premier tue, le second tombe sur un mort (0).
    Partie partie = deuxGuerriersQuiTuentLeMemeMonstre();

    const std::vector<EvenementDeCombat> evenements = parmi(evenementsPendant(partie, 0.5f), TypeEvenement::Degats, Camp::Monstres);

    REQUIRE(partie.getMonstres().empty());
    REQUIRE(evenements.size() == 1);
    REQUIRE(evenements[0].montant == 200);
}

TEST_CASE("evenements : le monstre tue garde son evenement et sa position") {
    Partie partie = deuxGuerriersQuiTuentLeMemeMonstre();

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 0.5f);

    REQUIRE(partie.getMonstres().empty());
    const std::vector<EvenementDeCombat> surLeMonstre = parmi(evenements, TypeEvenement::Degats, Camp::Monstres);
    REQUIRE(surLeMonstre.size() == 1);
    REQUIRE(surLeMonstre[0].idCible == 1);
    verifierPointProche(surLeMonstre[0].position, {260.0f, 210.0f});
}

TEST_CASE("evenements : prendreEvenements vide la file") {
    Partie partie = deuxGuerriersQuiTuentLeMemeMonstre();
    avancer(partie, nombreDeTics(0.5f));

    REQUIRE_FALSE(partie.prendreEvenements().empty());
    REQUIRE(partie.prendreEvenements().empty());
}

TEST_CASE("evenements : une partie gagnee ou perdue n'en produit plus") {
    Partie gagnee = deuxGuerriersQuiTuentLeMemeMonstre();
    avancer(gagnee, nombreDeTics(0.5f));
    REQUIRE(gagnee.estGagnee());
    static_cast<void>(gagnee.prendreEvenements());
    avancer(gagnee, nombreDeTics(1.0f));
    REQUIRE(gagnee.prendreEvenements().empty());

    Partie perdue = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !perdue.estPerdue(); ++i) {
        perdue.mettreAJour(s_dureeTic);
    }
    REQUIRE(perdue.estPerdue());
    static_cast<void>(perdue.prendreEvenements());
    avancer(perdue, nombreDeTics(1.0f));
    REQUIRE(perdue.prendreEvenements().empty());
}

TEST_CASE("estVisible : la salle de depart est visible, un point lointain non") {
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);

    REQUIRE(partie.estVisible(partie.getEscouade().getCentre()));
    REQUIRE(partie.estVisible({235.0f, 170.0f}));
    REQUIRE_FALSE(partie.estVisible({580.0f, 580.0f}));  // à plus de 320 px de tout personnage
}
