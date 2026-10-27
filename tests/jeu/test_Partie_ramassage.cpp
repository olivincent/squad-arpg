// ./tests/jeu/test_Partie_ramassage.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "jeu/ObjetAuSol.hpp"
#include "jeu/Partie.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Niveau.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "objets/CatalogueObjets.hpp"

// Le ramassage d'un objet au sol (étape 97.5) : un clic sur l'objet, l'ancre qui arrive, le sac plein.
namespace {
const sf::Vector2f s_departDeLEscouade = {200.0f, 200.0f};
const sf::Vector2f s_positionDuMonstreTue = {440.0f, 200.0f};
const sf::Vector2f s_coinLointain = {570.0f, 570.0f};

// Une partie où un objet est tombé, à 260 px de l'escouade : le mage tue d'un coup d'explosion un monstre sans défense,
// qui laisse un objet sur la case (11, 5), dont le centre est (460, 220). `autres` : des monstres de plus (par défaut, un
// seul, loin dans un coin, pour que la partie ne soit pas gagnée : une partie gagnée se fige). Le monstre tué est
// toujours le premier ; il laisse un butin certain d'un objet, les autres n'en laissent aucun.
Partie partieAvecUnObjetAuSol(const std::vector<MonstreDuNiveau>& autres = {{s_coinLointain, 1}}) {
    DefinitionDeMonstre faible = monstreSansHasard();
    faible.pointsDeVieMax = 1;
    faible.chanceDeButin = 100;
    faible.nombreDObjets = 1;
    std::vector<MonstreDuNiveau> monstres = {{s_positionDuMonstreTue, 0}};
    monstres.insert(monstres.end(), autres.begin(), autres.end());
    Partie partie = partieDeTest(niveauDegageEnGroupes(monstres), compositionDeTest(), s_graineDeTest, faible);
    partie.lancerSkillDeZone(s_adresseExplosion, s_positionDuMonstreTue);
    for (int i = 0; i < nombreDeTics(5.0f) && partie.getSol().tous().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.getSol().tous().size() == 1);
    return partie;
}

float distance(sf::Vector2f a, sf::Vector2f b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

// Avance jusqu'à ce que l'objet au sol ait disparu (ramassé), au plus `secondes`.
void avancerJusquAuRamassage(Partie& partie, float secondes) {
    for (int i = 0; i < nombreDeTics(secondes) && !partie.getSol().tous().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
}

bool sacContient(const Partie& partie, const Objet& objet) {
    for (std::size_t i = 0; i < partie.getEscouade().sac().capacite(); ++i) {
        if (partie.getEscouade().sac().a(i) == objet) {
            return true;
        }
    }
    return false;
}

void remplirLeSac(Partie& partie) {
    while (partie.getEscouade().sac().casesLibres() > 0) {
        REQUIRE(partie.getEscouade().ramasser(Objet{IdObjet::GantsDeTissu}));
    }
}

int combienDeSacPlein(const std::vector<EvenementDeCombat>& evenements) {
    int nombre = 0;
    for (const EvenementDeCombat& evenement : evenements) {
        if (std::holds_alternative<SacPlein>(evenement.detail)) {
            ++nombre;
        }
    }
    return nombre;
}
}  // namespace

TEST_CASE("ramassage : un clic sur un objet envoie l'escouade, et il passe dans le sac quand l'ancre arrive") {
    Partie partie = partieAvecUnObjetAuSol();
    const ObjetAuSol objet = partie.getSol().tous().front();
    REQUIRE(distance(partie.getEscouade().getCentre(), objet.position) > s_distanceDeRamassage);
    const std::size_t libresAvant = partie.getEscouade().sac().casesLibres();

    REQUIRE(partie.cliquer(objet.position) == ResultatClic::ObjetAuSol);
    REQUIRE(partie.getSol().tous().size() == 1);  // un ordre, pas un ramassage immédiat : l'ancre est loin
    avancerJusquAuRamassage(partie, 10.0f);

    REQUIRE(partie.getSol().tous().empty());
    REQUIRE(sacContient(partie, objet.objet));
    REQUIRE(partie.getEscouade().sac().casesLibres() == libresAvant - 1);
    REQUIRE(distance(partie.getEscouade().getCentre(), objet.position) < s_distanceDeRamassage + 5.0f);
}

TEST_CASE("ramassage : l'objet va dans la premiere case libre du sac") {
    Partie partie = partieAvecUnObjetAuSol();
    const ObjetAuSol objet = partie.getSol().tous().front();
    std::size_t premiereLibre = 0;
    while (partie.getEscouade().sac().a(premiereLibre).has_value()) {
        ++premiereLibre;
    }

    static_cast<void>(partie.cliquer(objet.position));
    avancerJusquAuRamassage(partie, 10.0f);

    REQUIRE(partie.getEscouade().sac().a(premiereLibre) == objet.objet);
}

TEST_CASE("ramassage : un clic sur le sol avant d'arriver remplace la visee, rien n'est ramasse") {
    Partie partie = partieAvecUnObjetAuSol();
    const ObjetAuSol objet = partie.getSol().tous().front();
    const std::size_t libresAvant = partie.getEscouade().sac().casesLibres();

    REQUIRE(partie.cliquer(objet.position) == ResultatClic::ObjetAuSol);
    avancer(partie, 10);
    REQUIRE(partie.cliquer({100.0f, 100.0f}) == ResultatClic::Sol);
    avancer(partie, nombreDeTics(10.0f));

    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getEscouade().sac().casesLibres() == libresAvant);
}

TEST_CASE("ramassage : un ordre de deplacement remplace aussi la visee") {
    Partie partie = partieAvecUnObjetAuSol();
    const ObjetAuSol objet = partie.getSol().tous().front();
    const std::size_t libresAvant = partie.getEscouade().sac().casesLibres();

    REQUIRE(partie.cliquer(objet.position) == ResultatClic::ObjetAuSol);
    avancer(partie, 10);
    partie.ordonnerDeplacement({100.0f, 100.0f});
    avancer(partie, nombreDeTics(10.0f));

    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getEscouade().sac().casesLibres() == libresAvant);
}

TEST_CASE("ramassage : une nouvelle visee remplace la precedente, un seul ordre a la fois") {
    // Deux objets : le monstre tué laisse deux objets sur deux cases voisines ; on vise le premier puis le second.
    DefinitionDeMonstre faible = monstreSansHasard();
    faible.pointsDeVieMax = 1;
    faible.chanceDeButin = 100;
    faible.nombreDObjets = 2;
    Partie partie = partieDeTest(niveauDegageEnGroupes({{s_positionDuMonstreTue, 0}, {s_coinLointain, 1}}), compositionDeTest(),
                                 s_graineDeTest, faible);
    partie.lancerSkillDeZone(s_adresseExplosion, s_positionDuMonstreTue);
    for (int i = 0; i < nombreDeTics(5.0f) && partie.getSol().tous().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.getSol().tous().size() == 2);
    const ObjetAuSol premier = partie.getSol().tous()[0];
    const ObjetAuSol second = partie.getSol().tous()[1];

    REQUIRE(partie.cliquer(premier.position) == ResultatClic::ObjetAuSol);
    REQUIRE(partie.cliquer(second.position) == ResultatClic::ObjetAuSol);
    avancer(partie, nombreDeTics(10.0f));

    // Seul le second est ramassé : la première visée a été remplacée. L'ancre, au second, est assez loin du premier
    // (une case, 40 px) pour ne pas être passée sur lui par hasard.
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getSol().tous().front().id == premier.id);
    REQUIRE(sacContient(partie, second.objet));
}

TEST_CASE("ramassage : sac plein, l'objet reste au sol, l'evenement est emis une fois et l'ordre est termine") {
    Partie partie = partieAvecUnObjetAuSol();
    remplirLeSac(partie);
    const ObjetAuSol objet = partie.getSol().tous().front();
    static_cast<void>(partie.prendreEvenements());  // ceux de l'explosion

    REQUIRE(partie.cliquer(objet.position) == ResultatClic::ObjetAuSol);
    std::vector<EvenementDeCombat> evenements;
    for (int i = 0; i < nombreDeTics(10.0f) && combienDeSacPlein(evenements) == 0; ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            evenements.push_back(evenement);
        }
    }

    REQUIRE(combienDeSacPlein(evenements) == 1);
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getSol().tous().front() == objet);
    REQUIRE(partie.getEscouade().sac().casesLibres() == 0);

    // L'ordre est terminé : l'escouade ne tente plus rien, aucun autre événement.
    avancer(partie, nombreDeTics(3.0f));
    REQUIRE(combienDeSacPlein(partie.prendreEvenements()) == 0);
    REQUIRE(partie.getSol().tous().size() == 1);
}

TEST_CASE("ramassage : l'evenement du sac plein est au-dessus de l'escouade entiere, a la position de l'ancre") {
    Partie partie = partieAvecUnObjetAuSol();
    remplirLeSac(partie);
    static_cast<void>(partie.prendreEvenements());
    static_cast<void>(partie.cliquer(partie.getSol().tous().front().position));

    std::optional<EvenementDeCombat> plein;
    for (int i = 0; i < nombreDeTics(10.0f) && !plein.has_value(); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            if (std::holds_alternative<SacPlein>(evenement.detail)) {
                plein = evenement;
            }
        }
    }

    REQUIRE(plein.has_value());
    REQUIRE(plein->campCible == Camp::Escouade);
    REQUIRE(plein->idCible == 0);
    // L'ancre a un peu bougé depuis l'événement, dans la même image : une marge de quelques pixels.
    REQUIRE(distance(plein->position, partie.getEscouade().getCentre()) < 5.0f);
}

TEST_CASE("ramassage : avec un monstre et un objet sous le curseur, le clic vise le monstre") {
    // Un monstre lourd juste à côté du monstre tué, de sorte que le point de l'objet est dans son carré.
    const sf::Vector2f positionDuLourd = {450.0f, 215.0f};
    Partie partie = partieAvecUnObjetAuSol({{positionDuLourd, 1, CategorieDeMonstre::InfanterieLourde}});
    const ObjetAuSol objet = partie.getSol().tous().front();
    REQUIRE(partie.getMonstres().size() == 1);
    const Monstre& lourd = partie.getMonstres().front();
    REQUIRE(lourd.contient(objet.position));  // sinon le test ne prouve rien
    REQUIRE(partie.objetAuSolSous(objet.position) != nullptr);

    REQUIRE(partie.cliquer(objet.position) == ResultatClic::Monstre);

    REQUIRE(partie.getEscouade().getCibleVerrouillee() == lourd.getId());
}

TEST_CASE("ramassage : un clic sur un objet rompt le combat, comme un clic sur le sol") {
    Partie partie = partieAvecUnObjetAuSol({{{440.0f, 330.0f}, 1, CategorieDeMonstre::InfanterieLourde}});
    const ObjetAuSol objet = partie.getSol().tous().front();
    REQUIRE(partie.getMonstres().size() == 1);
    REQUIRE(partie.cliquer(partie.getMonstres().front().getCentre()) == ResultatClic::Monstre);
    REQUIRE(partie.getEscouade().getCibleVerrouillee().has_value());

    REQUIRE(partie.cliquer(objet.position) == ResultatClic::ObjetAuSol);

    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("ramassage : un clic juste hors de la zone de survol de l'objet est un clic au sol") {
    Partie partie = partieAvecUnObjetAuSol();
    const ObjetAuSol objet = partie.getSol().tous().front();

    REQUIRE(partie.cliquer(objet.position + sf::Vector2f(0.0f, s_rayonDeSurvolDObjet + 5.0f)) == ResultatClic::Sol);
}

TEST_CASE("ramassage : un objet qu'on ne peut pas voir (case inconnue) ne se clique pas") {
    // Le mage voit à 50 px seulement : l'objet, à 260 px, tombe sur une case inconnue.
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition.rayonVision = 50.0f;
    }
    DefinitionDeMonstre faible = monstreSansHasard();
    faible.pointsDeVieMax = 1;
    faible.chanceDeButin = 100;
    faible.nombreDObjets = 1;
    Partie partie = partieDeTest(niveauDegageEnGroupes({{s_positionDuMonstreTue, 0}, {s_coinLointain, 1}}), std::move(composition),
                                 s_graineDeTest, faible);
    partie.lancerSkillDeZone(s_adresseExplosion, s_positionDuMonstreTue);
    for (int i = 0; i < nombreDeTics(5.0f) && partie.getSol().tous().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.getSol().tous().size() == 1);

    REQUIRE(partie.cliquer(partie.getSol().tous().front().position) == ResultatClic::Sol);
}
