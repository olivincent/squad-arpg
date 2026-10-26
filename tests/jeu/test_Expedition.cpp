// ./tests/jeu/test_Expedition.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/Partie.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// L'expédition (étape 98.2) : ce qui survit d'un étage à l'autre, et la graine de chaque étage.
namespace {
constexpr std::size_t s_guerrier = 1;
constexpr std::size_t s_pretre = 2;
constexpr std::size_t s_mage = 3;
constexpr int s_degatsDeLaMort = 1'000'000;
const sf::Vector2f s_coinLointain = {570.0f, 570.0f};

// Une expédition dont les trois personnages peuvent porter tous les objets du catalogue.
Expedition expeditionCapableDeToutPorter() {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = capableDeToutPorter(std::move(definition));
    }
    return expeditionDeTest(std::move(composition));
}

Partie partieDe(Expedition expedition, const std::vector<sf::Vector2f>& monstres = {}) {
    return Partie(niveauDegage(monstres), std::move(expedition), s_graineDeTest, catalogueDeTest(monstreSansHasard()));
}

const Objet s_masseFlamboyante{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};
}  // namespace

TEST_CASE("nouvelleExpedition : profondeur 1, l'equipement de depart porte, un sac vide") {
    const std::vector<DefinitionPersonnage> composition = compositionDeTest();

    const Expedition expedition = nouvelleExpedition(composition, 777);

    REQUIRE(expedition.profondeur == 1);
    REQUIRE(expedition.graine == 777);
    REQUIRE(expedition.membres.size() == composition.size());
    for (std::size_t i = 0; i < composition.size(); ++i) {
        REQUIRE(expedition.membres[i].equipement == Equipement(composition[i].equipementDeDepart));
        REQUIRE(expedition.membres[i].definition.equipementDeDepart.empty());  // le kit est devenu l'équipement
        REQUIRE(expedition.membres[i].definition.caracteristiques == composition[i].caracteristiques);
        REQUIRE(expedition.membres[i].definition.couleur == composition[i].couleur);
    }
    for (std::size_t i = 0; i < expedition.sac.capacite(); ++i) {
        REQUIRE_FALSE(expedition.sac.a(i).has_value());
    }
}

TEST_CASE("Partie : une partie neuve porte l'equipement de depart de l'expedition, comme avant") {
    const std::vector<DefinitionPersonnage> composition = compositionDeTest();
    const Partie partie = partieDe(expeditionDeTest(composition));
    const Escouade& escouade = partie.getEscouade();

    for (std::size_t i = 0; i < composition.size(); ++i) {
        REQUIRE(escouade.equipementDuPersonnage(i + 1) == Equipement(composition[i].equipementDeDepart));
        REQUIRE(escouade.caracteristiquesDeBaseDuPersonnage(i + 1) == composition[i].caracteristiques);
    }
}

TEST_CASE("expedition : aller-retour, l'equipement, le sac et les caracteristiques de base survivent") {
    Partie partie = partieDe(expeditionCapableDeToutPorter());
    Escouade& escouade = partie.getEscouade();
    // Un objet magique à la case 20 du sac (une case que `ramasser` ne donnerait pas : la place est gardée), une paire de gants
    // portés par le mage, la masse flamboyante dans la main droite du prêtre.
    REQUIRE(escouade.sac().capacite() > 20);
    REQUIRE(escouade.ramasser(Objet{IdObjet::GantsDeCuir}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{0}, PositionDansLeSac{20}));
    REQUIRE(escouade.ramasser(Objet{IdObjet::GantsDeCuir}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{0}, PositionPortee{s_mage, Emplacement::Gants}));
    REQUIRE(escouade.ramasser(s_masseFlamboyante));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{0}, PositionPortee{s_pretre, Emplacement::MainDroite}));
    const Exemplaires avant = exemplaires(partie);

    const Expedition expedition = partie.expedition();
    const Partie suivante = partieDe(expedition);
    const Escouade& arrivee = suivante.getEscouade();

    REQUIRE(exemplaires(suivante) == avant);
    REQUIRE(arrivee.sac().a(20) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(arrivee.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(arrivee.equipementDuPersonnage(s_pretre).a(Emplacement::MainDroite) == s_masseFlamboyante);
    for (std::size_t place = s_guerrier; place <= s_mage; ++place) {
        REQUIRE(arrivee.equipementDuPersonnage(place) == escouade.equipementDuPersonnage(place));
        REQUIRE(arrivee.caracteristiquesDeBaseDuPersonnage(place) == escouade.caracteristiquesDeBaseDuPersonnage(place));
        REQUIRE(arrivee.caracteristiquesDuPersonnage(place) == escouade.caracteristiquesDuPersonnage(place));
    }
    for (std::size_t i = 0; i < arrivee.sac().capacite(); ++i) {
        REQUIRE(arrivee.sac().a(i) == escouade.sac().a(i));
    }
}

TEST_CASE("expedition : un objet inactif (seuil non atteint) reste porte d'un etage a l'autre") {
    // Un personnage sans Endurance qui porte la cuirasse de mailles : l'objet est porté, inactif. La reconstruction ne vérifie
    // aucun seuil, elle reprend l'équipement tel quel.
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition[0] = {.couleur = sf::Color::White, .skills = {}, .equipementDeDepart = {{Emplacement::Torse, IdObjet::CuirasseDeMailles}},
                      .rayonVision = composition[0].rayonVision};
    Partie partie = partieDe(expeditionDeTest(composition));
    REQUIRE(partie.getEscouade().objetPorteEstInactif(s_guerrier, Emplacement::Torse));

    const Partie suivante = partieDe(partie.expedition());

    REQUIRE(suivante.getEscouade().equipementDuPersonnage(s_guerrier).a(Emplacement::Torse) == Objet{IdObjet::CuirasseDeMailles});
    REQUIRE(suivante.getEscouade().objetPorteEstInactif(s_guerrier, Emplacement::Torse));
}

TEST_CASE("expedition : l'etage suivant commence a pleine vie, un mort compris") {
    Partie partie = partieDe(expeditionDeTest(), {s_coinLointain});
    Escouade& escouade = partie.getEscouade();
    REQUIRE(subirCoup(escouade, 1, s_mage, s_degatsDeLaMort) > 0);  // le mage meurt
    REQUIRE(subirCoup(escouade, 1, s_pretre, 50) > 0);         // le prêtre est blessé
    REQUIRE_FALSE(escouade.placeEstVivante(s_mage));
    REQUIRE(escouade.pointsDeVieDuPersonnage(s_pretre).value() < escouade.pointsDeVieMaxDuPersonnage(s_pretre).value());

    const Partie suivante = partieDe(partie.expedition());
    const Escouade& arrivee = suivante.getEscouade();

    for (std::size_t place = s_guerrier; place <= s_mage; ++place) {
        REQUIRE(arrivee.placeEstVivante(place));
        REQUIRE(arrivee.pointsDeVieDuPersonnage(place) == arrivee.pointsDeVieMaxDuPersonnage(place));
        REQUIRE(arrivee.pointsDeVieMaxDuPersonnage(place) == escouade.pointsDeVieMaxDuPersonnage(place));
    }
}

TEST_CASE("expedition : rien de l'etage ne passe, ni les monstres, ni les objets au sol, ni les agresseurs") {
    Partie partie = partieDe(expeditionCapableDeToutPorter(), {s_coinLointain});
    REQUIRE(partie.getMonstres().size() == 1);
    REQUIRE(partie.getEscouade().ramasser(Objet{IdObjet::GantsDeCuir}));
    REQUIRE(jeterUnObjet(partie.getEscouade(), partie.getSol(), partie.getCarte(), PositionDansLeSac{0}));
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(subirCoup(partie.getEscouade(), 1, s_guerrier, 10) > 0);  // le monstre 1 devient un agresseur
    REQUIRE_FALSE(partie.getEscouade().getAgresseurs().empty());
    partie.getEscouade().verrouiller(1);
    partie.getEscouade().provoquer(1, s_guerrier, 4.0f);
    const sf::Vector2f ancreAvant = partie.getEscouade().getCentre();
    const Exemplaires exemplairesDeLEscouade = exemplaires(partie.getEscouade());

    // L'étage suivant : un autre niveau, deux monstres, une autre place de départ.
    Niveau autre = niveauDegage({{100.0f, 500.0f}, {500.0f, 100.0f}});
    autre.departEscouade = {300.0f, 300.0f};
    const Partie suivante(autre, partie.expedition(), s_graineDeTest, catalogueDeTest(monstreSansHasard()));

    REQUIRE(suivante.getMonstres().size() == 2);
    REQUIRE(suivante.getSol().tous().empty());
    REQUIRE(suivante.getEscouade().getAgresseurs().empty());
    REQUIRE(suivante.getEscouade().getProvocations().empty());
    REQUIRE_FALSE(suivante.getEscouade().getCibleVerrouillee().has_value());
    REQUIRE(suivante.getEscouade().getCentre() != ancreAvant);
    // Seuls les objets de l'escouade ont voyagé : celui qui était au sol est resté dans l'étage précédent.
    REQUIRE(exemplaires(suivante) == exemplairesDeLEscouade);
}

TEST_CASE("expedition : une expedition est une valeur, la copier ne partage rien") {
    Partie partie = partieDe(expeditionCapableDeToutPorter());
    const Expedition avant = partie.expedition();

    REQUIRE(partie.getEscouade().ramasser(s_masseFlamboyante));
    const Expedition apres = partie.expedition();

    REQUIRE(avant.sac.casesLibres() == apres.sac.casesLibres() + 1);
}

TEST_CASE("graineDEtage : la meme expedition et la meme profondeur donnent la meme graine") {
    REQUIRE(graineDEtage(12345, 3) == graineDEtage(12345, 3));
}

TEST_CASE("graineDEtage : chaque profondeur a sa graine") {
    std::set<Generateur::result_type> graines;
    for (int profondeur = 1; profondeur <= 20; ++profondeur) {
        graines.insert(graineDEtage(12345, profondeur));
    }

    REQUIRE(graines.size() == 20);
}

TEST_CASE("graineDEtage : deux expeditions n'ont pas les memes etages") {
    for (int profondeur = 1; profondeur <= 5; ++profondeur) {
        REQUIRE(graineDEtage(1, profondeur) != graineDEtage(2, profondeur));
    }
}

TEST_CASE("graineDEtage : la graine d'un etage n'est pas celle de l'expedition") {
    REQUIRE(graineDEtage(12345, 1) != 12345);
}
