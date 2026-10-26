// ./tests/jeu/test_Partie_jeter.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <set>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "jeu/ObjetAuSol.hpp"
#include "jeu/ObjetsAuSol.hpp"
#include "jeu/Partie.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// Jeter un objet (étape 97.6, `jeterUnObjet` libre depuis 98.1) : et le compte des exemplaires à travers jeter puis ramasser.
namespace {
constexpr std::size_t s_guerrier = 1;
const sf::Vector2f s_coinLointain = {570.0f, 570.0f};

// Une partie avec un monstre endormi loin dans un coin (une partie sans monstre est gagnée, donc figée), dont les
// personnages peuvent porter tout le catalogue, au sac vide.
Partie partieDeLeJet() {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = capableDeToutPorter(std::move(definition));
    }
    Partie partie = partieDeTest(niveauDegage({s_coinLointain}), std::move(composition));
    viderLeSac(partie.getEscouade());
    return partie;
}

// Jette l'objet de cette position de l'escouade de la partie, sur le sol de la partie.
bool jeter(Partie& partie, const PositionDObjet& position) {
    return jeterUnObjet(partie.getEscouade(), partie.getSol(), partie.getCarte(), position);
}

float distance(sf::Vector2f a, sf::Vector2f b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

const Objet s_gants{IdObjet::GantsDeCuir};
const Objet s_masseFlamboyante{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};
}  // namespace

TEST_CASE("jeterUnObjet : un objet du sac arrive au sol pres de l'ancre, et quitte le sac") {
    Partie partie = partieDeLeJet();
    REQUIRE(partie.getEscouade().ramasser(s_gants));
    REQUIRE(partie.getSol().tous().empty());

    REQUIRE(jeter(partie, PositionDansLeSac{0}));

    REQUIRE_FALSE(partie.getEscouade().sac().a(0).has_value());
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getSol().tous().front().objet == s_gants);
    // Le premier objet tombe sur la case de l'ancre : à moins d'une demi-diagonale de case de celle-ci.
    REQUIRE(distance(partie.getSol().tous().front().position, partie.getEscouade().getCentre()) < 30.0f);
}

TEST_CASE("jeterUnObjet : un objet porte arrive au sol, l'emplacement se vide et l'armure baisse") {
    Partie partie = partieDeLeJet();
    const int armureAvant = partie.getEscouade().equipementDuPersonnage(s_guerrier).armureTotale();
    const std::optional<Objet> casque = partie.getEscouade().objetA(PositionPortee{s_guerrier, Emplacement::Tete});
    REQUIRE(casque.has_value());

    REQUIRE(jeter(partie, PositionPortee{s_guerrier, Emplacement::Tete}));

    REQUIRE_FALSE(partie.getEscouade().equipementDuPersonnage(s_guerrier).a(Emplacement::Tete).has_value());
    REQUIRE(partie.getEscouade().equipementDuPersonnage(s_guerrier).armureTotale() < armureAvant);
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getSol().tous().front().objet == *casque);
}

TEST_CASE("jeterUnObjet : jeter l'arme portee applique les regles du retrait ordinaire, l'attaque de base devient celle des mains nues") {
    Partie partie = partieDeLeJet();
    const std::optional<Attaque> avant = partie.getEscouade().attaqueDeBase(s_guerrier);
    REQUIRE(avant.has_value());

    REQUIRE(jeter(partie, PositionPortee{s_guerrier, Emplacement::MainDroite}));

    REQUIRE_FALSE(partie.getEscouade().equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());
    REQUIRE(partie.getEscouade().equipementDuPersonnage(s_guerrier).attaqueDeBase() == mainsNues());
    REQUIRE(definitionObjet(partie.getSol().tous().front().objet.modele).arme.has_value());
}

TEST_CASE("jeterUnObjet : une position vide ne jette rien") {
    Partie partie = partieDeLeJet();
    const Exemplaires avant = exemplaires(partie);

    REQUIRE_FALSE(jeter(partie, PositionDansLeSac{7}));
    REQUIRE_FALSE(jeter(partie, PositionPortee{3, Emplacement::Tete}));

    REQUIRE(partie.getSol().tous().empty());
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("jeterUnObjet : les objets jetes l'un apres l'autre ont des identifiants et des cases differents") {
    Partie partie = partieDeLeJet();
    for (int i = 0; i < 5; ++i) {
        REQUIRE(partie.getEscouade().ramasser(s_gants));
    }

    for (std::size_t i = 0; i < 5; ++i) {
        REQUIRE(jeter(partie, PositionDansLeSac{i}));
    }

    std::set<int> identifiants;
    std::set<std::pair<float, float>> positions;
    for (const ObjetAuSol& objet : partie.getSol().tous()) {
        identifiants.insert(objet.id);
        positions.insert({objet.position.x, objet.position.y});
    }
    REQUIRE(identifiants.size() == 5);
    REQUIRE(positions.size() == 5);
}

TEST_CASE("exemplaires : un objet jete puis ramasse n'est ni duplique ni perdu, sac, equipements et sol compris") {
    Partie partie = partieDeLeJet();
    REQUIRE(partie.getEscouade().ramasser(s_masseFlamboyante));
    const Exemplaires avant = exemplaires(partie);

    // Jeter : l'objet passe du sac au sol, le total ne change pas.
    REQUIRE(jeter(partie, PositionDansLeSac{0}));
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(exemplaires(partie) == avant);

    // Jeter l'arme portée aussi.
    REQUIRE(jeter(partie, PositionPortee{s_guerrier, Emplacement::MainDroite}));
    REQUIRE(partie.getSol().tous().size() == 2);
    REQUIRE(exemplaires(partie) == avant);

    // Ramasser les deux, un clic après l'autre : tout revient dans le sac, rien n'est resté au sol.
    for (int i = 0; i < 2; ++i) {
        REQUIRE(partie.cliquer(partie.getSol().tous().front().position) == ResultatClic::ObjetAuSol);
        const std::size_t restants = partie.getSol().tous().size();
        for (int tic = 0; tic < nombreDeTics(5.0f) && partie.getSol().tous().size() == restants; ++tic) {
            partie.mettreAJour(s_dureeTic);
        }
        REQUIRE(partie.getSol().tous().size() == restants - 1);
        REQUIRE(exemplaires(partie) == avant);
    }
    REQUIRE(partie.getSol().tous().empty());
}
