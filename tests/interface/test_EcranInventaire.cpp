// ./tests/interface/test_EcranInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "aides_escouade.hpp"
#include "interface/DispositionInventaire.hpp"
#include "interface/EcranInventaire.hpp"
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

// L'escouade du jeu avec des gants de cuir dans la case 0 du sac.
Escouade escouadeAvecDesGants() {
    Escouade escouade = uneEscouade({100.0f, 100.0f});
    REQUIRE(escouade.ramasser(Objet{IdObjet::GantsDeCuir}));
    return escouade;
}

}  // namespace

TEST_CASE("inventaire : I ferme l'ecran") {
    Escouade escouade = uneEscouade({100.0f, 100.0f});
    EcranInventaire ecran(escouade, s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
}

TEST_CASE("inventaire : Echap ferme l'ecran quand rien n'est saisi") {
    Escouade escouade = uneEscouade({100.0f, 100.0f});
    EcranInventaire ecran(escouade, s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("inventaire : les autres touches sont ignorees") {
    Escouade escouade = uneEscouade({100.0f, 100.0f});
    EcranInventaire ecran(escouade, s_noms);

    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Q)));
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::F5)));
}

TEST_CASE("inventaire : laisse voir la partie dessous") {
    Escouade escouade = uneEscouade({100.0f, 100.0f});
    const EcranInventaire ecran(escouade, s_noms);

    REQUIRE(ecran.laisseVoirDessous());
}

TEST_CASE("glissement : saisir un objet commence le glissement, une case vide non") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

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
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sur un emplacement compatible equipe l'objet") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sur une case du sac range l'objet") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLaCase(12), s_vue);

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.sac().a(12) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : un emplacement incompatible refuse, rien ne bouge, le glissement se termine") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(s_guerrier, Emplacement::Tete), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == Objet{IdObjet::CasqueDeFer});
}

TEST_CASE("glissement : relacher dans le vide ne bouge rien et termine le glissement") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sans glissement ne fait rien") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.relacher(centreDeLaCase(3), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE_FALSE(escouade.sac().a(3).has_value());
}

TEST_CASE("glissement : un objet porte se saisit et s'echange avec un autre personnage") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({100.0f, 100.0f});
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLEmplacement(s_guerrier, Emplacement::Jambes), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::Jambes}});
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Jambes), s_vue);

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDeFer});
}

TEST_CASE("glissement : Echap l'annule sans fermer, un deuxieme Echap ferme") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("glissement : I ferme l'ecran meme pendant un glissement, rien n'a bouge") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : annulerLeGlissement abandonne sans rien deplacer") {
    Escouade escouade = escouadeAvecDesGants();
    EcranInventaire ecran(escouade, s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.annulerLeGlissement();
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants).has_value());
}
