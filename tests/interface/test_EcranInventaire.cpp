// ./tests/interface/test_EcranInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "aides_escouade.hpp"
#include "interface/DispositionInventaire.hpp"
#include "interface/EcranInventaire.hpp"
#include "jeu/Partie.hpp"
#include "objets/Objet.hpp"

namespace {

const std::vector<std::string> s_noms = {"Guerrier", "Pretre", "Mage"};
const sf::Vector2f s_vue{1200.0f, 800.0f};
constexpr std::size_t s_guerrier = 1;
constexpr std::size_t s_mage = 3;

sf::Vector2f centre(sf::FloatRect zone) {
    return zone.position + zone.size / 2.0f;
}

sf::Vector2f centreDeLaCase(std::size_t indice) {
    return centre(rectangleDeLaCaseDuSac(indice, s_vue));
}

sf::Vector2f centreDeLEmplacement(std::size_t place, Emplacement emplacement) {
    return centre(rectangleDeLEmplacement(place, emplacement, s_vue));
}

// Une partie sans monstre, avec l'escouade de test et un sac vide.
Partie partieSansMonstre() {
    Partie partie = unePartie(niveauDegage({}), s_graineDeTest);
    viderLeSac(partie.getEscouade());
    return partie;
}

// La même, avec des gants de cuir dans la case 0 du sac.
Partie partieAvecDesGants() {
    Partie partie = partieSansMonstre();
    REQUIRE(partie.getEscouade().ramasser(Objet{IdObjet::GantsDeCuir}));
    return partie;
}

// La même, dont les trois personnages peuvent porter tous les objets du catalogue.
Partie partieCapableDeToutPorter() {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = capableDeToutPorter(std::move(definition));
    }
    Partie partie = partieDeTest(niveauDegage({}), std::move(composition));
    viderLeSac(partie.getEscouade());
    return partie;
}

}  // namespace

TEST_CASE("inventaire : I ferme l'ecran") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie, s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
}

TEST_CASE("inventaire : Echap ferme l'ecran quand rien n'est saisi") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie, s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("inventaire : les autres touches sont ignorees") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie, s_noms);

    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Q)));
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::F5)));
}

TEST_CASE("inventaire : laisse voir la partie dessous") {
    Partie partie = partieSansMonstre();
    const EcranInventaire ecran(partie, s_noms);

    REQUIRE(ecran.laisseVoirDessous());
}

TEST_CASE("glissement : saisir un objet commence le glissement, une case vide non") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(5), s_vue);  // case vide
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    ecran.saisir({1.0f, 1.0f}, s_vue);  // hors des elements
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    ecran.saisir(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);  // emplacement vide
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionDansLeSac{0}});
}

TEST_CASE("glissement : rien ne bouge avant le relachement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sur un emplacement compatible equipe l'objet") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sur une case du sac range l'objet") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLaCase(12), s_vue);

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.sac().a(12) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : un emplacement incompatible refuse, rien ne bouge, le glissement se termine") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(s_guerrier, Emplacement::Tete), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == Objet{IdObjet::CasqueDeFer});
}

TEST_CASE("glissement : relacher dans un ecart du panneau ne bouge rien, ne jette rien et termine le glissement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centre(rectangleDeLEnTete(s_guerrier, s_vue)), s_vue);  // dans le panneau, sur aucun element

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(partie.getObjetsAuSol().empty());
}

TEST_CASE("jeter : lacher un objet du sac hors du panneau le pose au sol pres de l'ancre") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie, s_noms);
    const Exemplaires avant = exemplaires(partie);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher({1.0f, 1.0f}, s_vue);  // dans la marge, hors du panneau

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE_FALSE(partie.getEscouade().sac().a(0).has_value());
    REQUIRE(partie.getObjetsAuSol().size() == 1);
    REQUIRE(partie.getObjetsAuSol().front().objet == Objet{IdObjet::GantsDeCuir});
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("jeter : lacher un objet porte hors du panneau le pose au sol, l'emplacement se vide") {
    Partie partie = partieCapableDeToutPorter();
    EcranInventaire ecran(partie, s_noms);
    const Exemplaires avant = exemplaires(partie);

    ecran.saisir(centreDeLEmplacement(s_guerrier, Emplacement::MainDroite), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::MainDroite}});
    ecran.relacher({s_vue.x - 5.0f, s_vue.y - 5.0f}, s_vue);

    REQUIRE_FALSE(partie.getEscouade().equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());
    REQUIRE(partie.getObjetsAuSol().size() == 1);
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("jeter : annuler le glissement avant de lacher ne jette rien") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.annulerLeGlissement();
    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(partie.getObjetsAuSol().empty());
}

TEST_CASE("jeter : lacher hors du panneau sans glissement ne fait rien") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie, s_noms);

    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(partie.getObjetsAuSol().empty());
}

TEST_CASE("glissement : relacher sans glissement ne fait rien") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.relacher(centreDeLaCase(3), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE_FALSE(escouade.sac().a(3).has_value());
}

TEST_CASE("glissement : un objet porte se saisit et s'echange avec un autre personnage") {
    Partie partie = partieCapableDeToutPorter();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLEmplacement(s_guerrier, Emplacement::Jambes), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::Jambes}});
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Jambes), s_vue);

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDeFer});
}

TEST_CASE("glissement : Echap l'annule sans fermer, un deuxieme Echap ferme") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("glissement : I ferme l'ecran meme pendant un glissement, rien n'a bouge") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : annulerLeGlissement abandonne sans rien deplacer") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.annulerLeGlissement();
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants).has_value());
}
