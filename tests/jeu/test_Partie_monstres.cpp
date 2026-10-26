// ./tests/jeu/test_Partie_monstres.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Niveau.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"

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

TEST_CASE("un monstre approche poursuit et frappe") {
    Partie partie = unePartie(niveauDegage({{400.0f, 210.0f}}), s_graineDeTest);

    for (int i = 0; i < 600 && partie.getEscouade().getAgresseurs().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE_FALSE(partie.getEscouade().getAgresseurs().empty());
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

// Retour au poste (étape 69) : niveauDeLaFuite, fuyard et attirerHorsDuTerritoire sont dans aides.hpp.

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

    // Hypothèse : les personnages sont plus rapides que les monstres (l'escouade le rattrape).
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
    const int vieDeDepart = fuyard(partie).getPointsDeVie();
    partie.ordonnerDeplacement({1150.0f, 300.0f});

    bool aAbandonne = false;
    int vieAvant = vieDeDepart;
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
    REQUIRE(fuyard(partie).getPointsDeVie() == vieDeDepart);
}

TEST_CASE("chaque monstre du niveau recoit la definition de sa categorie") {
    std::vector<MonstreDuNiveau> monstres;
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const float x = 100.0f + 100.0f * static_cast<float>(monstres.size());
        monstres.push_back({{x, 500.0f}, static_cast<int>(monstres.size()), categorie});
    }
    const Partie partie(niveauDegageEnGroupes(monstres), expeditionDeTest(), s_graineDeTest);

    REQUIRE(partie.getMonstres().size() == s_nombreDeCategoriesDeMonstre);
    for (std::size_t i = 0; i < partie.getMonstres().size(); ++i) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(s_categoriesDeMonstre[i]);
        const Monstre& monstre = partie.getMonstres()[i];
        REQUIRE(monstre.getPointsDeVie() == definition.pointsDeVieMax);
        REQUIRE(monstre.getCercle().rayon == definition.cote / 2.0f);
        REQUIRE(monstre.getAttaque().degats.total() == definition.degats.total());
        REQUIRE(monstre.getStatistiques() == definition.statistiques);
    }
}

TEST_CASE("les monstres d'un etage profond sont renforces par la profondeur de l'expedition") {
    std::vector<MonstreDuNiveau> monstres;
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const float x = 100.0f + 100.0f * static_cast<float>(monstres.size());
        monstres.push_back({{x, 500.0f}, static_cast<int>(monstres.size()), categorie});
    }
    Expedition expedition = expeditionDeTest();
    expedition.profondeur = 3;
    const Partie partie(niveauDegageEnGroupes(monstres), std::move(expedition), s_graineDeTest);

    REQUIRE(partie.getMonstres().size() == s_nombreDeCategoriesDeMonstre);
    for (std::size_t i = 0; i < partie.getMonstres().size(); ++i) {
        const DefinitionDeMonstre renforcee = renforcer(definitionDeMonstre(s_categoriesDeMonstre[i]), 3);
        const Monstre& monstre = partie.getMonstres()[i];
        REQUIRE(monstre.getPointsDeVie() == renforcee.pointsDeVieMax);
        REQUIRE(monstre.getPointsDeVie() > definitionDeMonstre(s_categoriesDeMonstre[i]).pointsDeVieMax);
        REQUIRE(monstre.getAttaque().degats.total() == renforcee.degats.total());
        REQUIRE(monstre.getStatistiques() == renforcee.statistiques);
    }
}
