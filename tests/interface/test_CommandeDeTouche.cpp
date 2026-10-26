// ./tests/interface/test_CommandeDeTouche.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <variant>

#include "interface/CommandeDeTouche.hpp"
#include "interface/Reglages.hpp"

namespace {

const TouchesDesRaccourcis s_touches = Reglages{}.touchesDesRaccourcis;

template <class Commande>
bool estCommande(sf::Keyboard::Scancode touche, bool finDePartie) {
    return std::holds_alternative<Commande>(commandePourTouche(touche, finDePartie, s_touches));
}

}  // namespace

TEST_CASE("touche : Echap annule, en jeu comme en fin de partie") {
    REQUIRE(estCommande<commande::Annuler>(sf::Keyboard::Scan::Escape, false));
    REQUIRE(estCommande<commande::Annuler>(sf::Keyboard::Scan::Escape, true));
}

TEST_CASE("touche : R ramene au choix seulement en fin de partie") {
    REQUIRE(estCommande<commande::RetourAuChoix>(sf::Keyboard::Scan::R, true));
    // Hors fin de partie, R est la touche de l'emplacement 3 de la barre.
    REQUIRE(estCommande<commande::ActiverEmplacement>(sf::Keyboard::Scan::R, false));
}

TEST_CASE("touche : les touches de debogage valent en tout temps") {
    for (const bool fin : {false, true}) {
        REQUIRE(estCommande<commande::NouvelleCarte>(sf::Keyboard::Scan::F5, fin));
        REQUIRE(estCommande<commande::BasculerPointsVises>(sf::Keyboard::Scan::F3, fin));
        REQUIRE(estCommande<commande::BasculerAncre>(sf::Keyboard::Scan::F4, fin));
        REQUIRE(estCommande<commande::BasculerBrouillard>(sf::Keyboard::Scan::F6, fin));
        REQUIRE(estCommande<commande::AjouterDesObjetsAleatoires>(sf::Keyboard::Scan::F7, fin));
        REQUIRE(estCommande<commande::DescendreDUnEtage>(sf::Keyboard::Scan::F8, fin));
        REQUIRE(estCommande<commande::GagnerUnNiveau>(sf::Keyboard::Scan::F9, fin));
    }
}

TEST_CASE("touche : une touche de la barre active son emplacement") {
    const CommandeDeTouche commande = commandePourTouche(sf::Keyboard::Scan::V, false, s_touches);

    REQUIRE(std::holds_alternative<commande::ActiverEmplacement>(commande));
    REQUIRE(std::get<commande::ActiverEmplacement>(commande).emplacement == 11);
}

TEST_CASE("touche : les raccourcis de la barre sont ignores en fin de partie") {
    REQUIRE(estCommande<commande::Rien>(sf::Keyboard::Scan::Q, true));
}

TEST_CASE("touche : une touche inconnue ne fait rien") {
    REQUIRE(estCommande<commande::Rien>(sf::Keyboard::Scan::P, false));
}

TEST_CASE("touche : I ouvre l'inventaire, sauf en fin de partie") {
    REQUIRE(estCommande<commande::OuvrirInventaire>(sf::Keyboard::Scan::I, false));
    REQUIRE(estCommande<commande::Rien>(sf::Keyboard::Scan::I, true));
}
