// ./tests/objets/test_Inventaire.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>

#include "objets/Inventaire.hpp"

namespace {
constexpr std::size_t s_capaciteDeTest = 3;
const Objet s_casque{IdObjet::HeaumeIntegral};
const Objet s_gants{IdObjet::GantsDeTissu};
const Objet s_bottes{IdObjet::Chaussons};
}  // namespace

TEST_CASE("inventaire : les cases sont toutes vides au depart") {
    const Inventaire sac(s_capaciteDeTest);

    REQUIRE(sac.capacite() == s_capaciteDeTest);
    for (std::size_t i = 0; i < sac.capacite(); ++i) {
        REQUIRE_FALSE(sac.a(i).has_value());
    }
}

TEST_CASE("inventaire : ajouter remplit la premiere case libre") {
    Inventaire sac(s_capaciteDeTest);

    REQUIRE(sac.ajouter(s_casque));
    REQUIRE(sac.ajouter(s_gants));

    REQUIRE(sac.a(0) == s_casque);
    REQUIRE(sac.a(1) == s_gants);
    REQUIRE_FALSE(sac.a(2).has_value());
}

TEST_CASE("inventaire : ajouter dans un sac plein echoue sans rien changer") {
    Inventaire sac(2);
    REQUIRE(sac.ajouter(s_casque));
    REQUIRE(sac.ajouter(s_gants));

    REQUIRE_FALSE(sac.ajouter(s_bottes));

    REQUIRE(sac.a(0) == s_casque);
    REQUIRE(sac.a(1) == s_gants);
}

TEST_CASE("inventaire : prendre vide la case et rend l'objet") {
    Inventaire sac(s_capaciteDeTest);
    REQUIRE(sac.ajouter(s_casque));

    const std::optional<Objet> pris = sac.prendre(0);

    REQUIRE(pris == s_casque);
    REQUIRE_FALSE(sac.a(0).has_value());
}

TEST_CASE("inventaire : prendre dans une case vide ne rend rien") {
    Inventaire sac(s_capaciteDeTest);

    REQUIRE_FALSE(sac.prendre(1).has_value());
}

TEST_CASE("inventaire : les positions restent stables apres une prise") {
    Inventaire sac(s_capaciteDeTest);
    REQUIRE(sac.ajouter(s_casque));
    REQUIRE(sac.ajouter(s_gants));
    REQUIRE(sac.ajouter(s_bottes));

    REQUIRE(sac.prendre(0).has_value());

    REQUIRE(sac.a(1) == s_gants);
    REQUIRE(sac.a(2) == s_bottes);
}

TEST_CASE("inventaire : ajouter reprend la premiere case liberee") {
    Inventaire sac(s_capaciteDeTest);
    REQUIRE(sac.ajouter(s_casque));
    REQUIRE(sac.ajouter(s_gants));
    REQUIRE(sac.prendre(0).has_value());

    REQUIRE(sac.ajouter(s_bottes));

    REQUIRE(sac.a(0) == s_bottes);
    REQUIRE(sac.a(1) == s_gants);
}

TEST_CASE("inventaire : poser dans une case vide ne rend rien") {
    Inventaire sac(s_capaciteDeTest);

    REQUIRE_FALSE(sac.poser(2, s_casque).has_value());

    REQUIRE(sac.a(2) == s_casque);
}

TEST_CASE("inventaire : poser dans une case occupee rend l'ancien objet") {
    Inventaire sac(s_capaciteDeTest);
    REQUIRE(sac.ajouter(s_casque));

    const std::optional<Objet> ancien = sac.poser(0, s_gants);

    REQUIRE(ancien == s_casque);
    REQUIRE(sac.a(0) == s_gants);
}

TEST_CASE("inventaire : casesLibres compte les cases vides, une case videe redevient libre") {
    Inventaire sac(s_capaciteDeTest);
    REQUIRE(sac.casesLibres() == s_capaciteDeTest);

    REQUIRE(sac.ajouter(s_casque));
    REQUIRE(sac.ajouter(s_gants));
    REQUIRE(sac.casesLibres() == 1);

    REQUIRE(sac.ajouter(s_bottes));
    REQUIRE(sac.casesLibres() == 0);

    REQUIRE(sac.prendre(1).has_value());
    REQUIRE(sac.casesLibres() == 1);
}
