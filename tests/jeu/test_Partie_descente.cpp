// ./tests/jeu/test_Partie_descente.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/Partie.hpp"
#include "monde/Niveau.hpp"
#include "monde/TileMap.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// Descendre et gagner (étape 98.5). Niveau dégagé de 15 × 15 cases de 40 px, l'escalier sur la case (8, 8) : l'ancre part en
// (210, 210).
namespace {
const sf::Vector2i s_caseDeLEscalier = {8, 8};
const sf::Vector2f s_centreDeLEscalier = {340.0f, 340.0f};
const sf::Vector2f s_coinLointain = {570.0f, 570.0f};

Niveau niveauAvecEscalier(const std::vector<MonstreDuNiveau>& monstres) {
    Niveau niveau = niveauDegageEnGroupes(monstres);
    niveau.plan[static_cast<std::size_t>(s_caseDeLEscalier.y)][static_cast<std::size_t>(s_caseDeLEscalier.x)] = s_caractereDEscalier;
    return niveau;
}

// Une partie à cette profondeur, avec un monstre endormi dans un coin.
Partie partieALaProfondeur(int profondeur, const std::vector<MonstreDuNiveau>& monstres = {{s_coinLointain, 0}}) {
    Expedition expedition = expeditionDeTest();
    expedition.profondeur = profondeur;
    return Partie(niveauAvecEscalier(monstres), std::move(expedition), s_graineDeTest, catalogueDeTest(monstreSansHasard()));
}

// Clique sur l'escalier et avance jusqu'à ce que l'escalier soit pris (au plus 10 s).
void prendreLEscalier(Partie& partie) {
    REQUIRE(partie.cliquer(s_centreDeLEscalier) == ResultatClic::Escalier);
    for (int i = 0; i < nombreDeTics(10.0f) && !partie.doitDescendre() && !partie.estGagnee(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
}
}  // namespace

TEST_CASE("descente : une partie commence a la profondeur de son expedition") {
    REQUIRE(partieALaProfondeur(1).getProfondeur() == 1);
    REQUIRE(partieALaProfondeur(4).getProfondeur() == 4);
}

TEST_CASE("descente : prendre l'escalier d'un etage ordinaire demande de descendre, sans gagner") {
    Partie partie = partieALaProfondeur(1);
    REQUIRE_FALSE(partie.doitDescendre());

    prendreLEscalier(partie);

    REQUIRE(partie.doitDescendre());
    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("descente : etageSuivant ajoute un etage et ne change rien d'autre") {
    const Expedition depart = expeditionDeTest();

    const Expedition suivante = etageSuivant(depart);

    REQUIRE(suivante.profondeur == depart.profondeur + 1);
    REQUIRE(suivante.graine == depart.graine);
    REQUIRE(suivante.membres.size() == depart.membres.size());
    for (std::size_t i = 0; i < depart.membres.size(); ++i) {
        REQUIRE(suivante.membres[i].equipement == depart.membres[i].equipement);
    }
    for (std::size_t i = 0; i < depart.sac.capacite(); ++i) {
        REQUIRE(suivante.sac.a(i) == depart.sac.a(i));
    }
}

TEST_CASE("descente : l'etage suivant a la profondeur suivante et garde l'equipement et le sac") {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = capableDeToutPorter(std::move(definition));
    }
    Expedition expedition = expeditionDeTest(std::move(composition));
    Partie partie(niveauAvecEscalier({{s_coinLointain, 0}}), expedition, s_graineDeTest, catalogueDeTest(monstreSansHasard()));
    Escouade& escouade = partie.getEscouade();
    // Un sac et un équipement qui ont bougé pendant l'étage.
    viderLeSac(escouade);
    REQUIRE(escouade.ramasser(Objet{IdObjet::GantsDeCuir}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{0}, PositionDansLeSac{17}));
    REQUIRE(escouade.ramasser(Objet{IdObjet::GantsDeCuir}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{0}, PositionPortee{3, Emplacement::Gants}));
    const Exemplaires avant = exemplaires(partie);
    prendreLEscalier(partie);
    REQUIRE(partie.doitDescendre());

    const Partie suivante(niveauAvecEscalier({{s_coinLointain, 0}}), etageSuivant(partie.expedition()), s_graineDeTest,
                          catalogueDeTest(monstreSansHasard()));

    REQUIRE(suivante.getProfondeur() == 2);
    REQUIRE(exemplaires(suivante) == avant);
    REQUIRE(suivante.getEscouade().sac().a(17) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(suivante.getEscouade().equipementDuPersonnage(3).a(Emplacement::Gants) == Objet{IdObjet::GantsDeCuir});
    for (std::size_t place = 1; place <= 3; ++place) {
        REQUIRE(suivante.getEscouade().equipementDuPersonnage(place) == escouade.equipementDuPersonnage(place));
    }
}

TEST_CASE("descente : prendre l'escalier du dernier etage gagne la partie") {
    Partie partie = partieALaProfondeur(s_profondeurMaximale);
    REQUIRE_FALSE(partie.estGagnee());

    prendreLEscalier(partie);

    REQUIRE(partie.estGagnee());
    REQUIRE_FALSE(partie.doitDescendre());  // rien dessous
    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("descente : au dernier etage, l'escalier n'est pas la victoire tant qu'on ne l'a pas pris") {
    Partie partie = partieALaProfondeur(s_profondeurMaximale);

    REQUIRE(partie.cliquer(s_centreDeLEscalier) == ResultatClic::Escalier);
    partie.mettreAJour(s_dureeTic);

    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("descente : tuer tous les monstres ne donne pas la victoire") {
    Partie partie = partieALaProfondeur(s_profondeurMaximale, {{{235.0f, 210.0f}, 0}});
    REQUIRE(partie.cliquer({235.0f, 210.0f}) == ResultatClic::Monstre);

    for (int i = 0; i < 600 && partie.monstresRestants() > 0; ++i) {
        partie.mettreAJour(s_dureeTic);
    }

    REQUIRE(partie.monstresRestants() == 0);
    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE_FALSE(partie.doitDescendre());
    REQUIRE_FALSE(partie.estPerdue());
}

TEST_CASE("descente : une carte sans monstre n'est pas gagnee des le depart, et la partie n'est pas figee") {
    Partie partie = partieALaProfondeur(1, {});

    REQUIRE_FALSE(partie.estGagnee());
    REQUIRE_FALSE(partie.doitDescendre());
    const sf::Vector2f avant = partie.getEscouade().getCentre();
    partie.ordonnerDeplacement({400.0f, 400.0f});
    avancer(partie, nombreDeTics(1.0f));
    REQUIRE(partie.getEscouade().getCentre() != avant);
}

TEST_CASE("descente : on descend sans avoir tue personne, un monstre toujours en vie") {
    Partie partie = partieALaProfondeur(1, {{s_coinLointain, 0}, {{300.0f, 215.0f}, 1}});

    prendreLEscalier(partie);

    REQUIRE(partie.doitDescendre());
    REQUIRE(partie.monstresRestants() >= 1);
}

TEST_CASE("descente : forcerLaDescente (F8) fait descendre d'un etage ordinaire, comme l'escalier") {
    Partie partie = partieALaProfondeur(2);

    partie.forcerLaDescente();

    REQUIRE(partie.doitDescendre());
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("descente : forcerLaDescente au dernier etage donne la victoire") {
    Partie partie = partieALaProfondeur(s_profondeurMaximale);

    partie.forcerLaDescente();

    REQUIRE(partie.estGagnee());
    REQUIRE_FALSE(partie.doitDescendre());
}

TEST_CASE("descente : l'escouade vaincue ne descend pas et ne gagne pas, F8 est sans effet") {
    Partie partie = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !partie.estPerdue(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.estPerdue());

    partie.forcerLaDescente();

    REQUIRE_FALSE(partie.doitDescendre());
    REQUIRE_FALSE(partie.estGagnee());
}

TEST_CASE("descente : une partie qui a pris l'escalier se fige") {
    Partie partie = partieALaProfondeur(1, {{s_coinLointain, 0}});
    prendreLEscalier(partie);
    REQUIRE(partie.doitDescendre());
    const sf::Vector2f centre = partie.getEscouade().getCentre();
    const sf::Vector2f monstre = partie.getMonstres().front().getCentre();

    partie.ordonnerDeplacement({100.0f, 100.0f});
    avancer(partie, nombreDeTics(2.0f));

    verifierPointProche(partie.getEscouade().getCentre(), centre);
    verifierPointProche(partie.getMonstres().front().getCentre(), monstre);
    REQUIRE(partie.prendreEvenements().empty());
}

TEST_CASE("descente : les monstres de l'etage suivant sont plus forts que ceux du premier") {
    const Partie premier = partieALaProfondeur(1);
    const Partie deuxieme = partieALaProfondeur(2);

    REQUIRE(deuxieme.getMonstres().front().getPointsDeVie() > premier.getMonstres().front().getPointsDeVie());
    REQUIRE(deuxieme.getMonstres().front().getAttaque().degats.total() > premier.getMonstres().front().getAttaque().degats.total());
}
