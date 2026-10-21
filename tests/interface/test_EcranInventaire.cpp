// ./tests/interface/test_EcranInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <string>
#include <variant>
#include <vector>

#include "aides_escouade.hpp"
#include "interface/EcranInventaire.hpp"

namespace {

const std::vector<std::string> s_noms = {"Guerrier", "Pretre", "Mage"};

}  // namespace

TEST_CASE("inventaire : I et Echap le ferment") {
    const Escouade escouade = uneEscouade({100.0f, 100.0f});
    const EcranInventaire ecran(escouade, s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("inventaire : les autres touches sont ignorees") {
    const Escouade escouade = uneEscouade({100.0f, 100.0f});
    const EcranInventaire ecran(escouade, s_noms);

    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Q)));
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::F5)));
}

TEST_CASE("inventaire : laisse voir la partie dessous") {
    const Escouade escouade = uneEscouade({100.0f, 100.0f});
    const EcranInventaire ecran(escouade, s_noms);

    REQUIRE(ecran.laisseVoirDessous());
}
