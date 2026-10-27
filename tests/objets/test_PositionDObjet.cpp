// ./tests/objets/test_PositionDObjet.cpp
#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// deplacementPermis : toutes les combinaisons de départ et d'arrivée.
namespace {
const PositionDObjet s_sac0 = PositionDansLeSac{0};
const PositionDObjet s_sac1 = PositionDansLeSac{1};
const PositionDObjet s_teteGuerrier = PositionPortee{1, Emplacement::Tete};
const PositionDObjet s_gantsMage = PositionPortee{3, Emplacement::Gants};
const PositionDObjet s_anneau1Guerrier = PositionPortee{1, Emplacement::Anneau1};
const PositionDObjet s_anneau2Guerrier = PositionPortee{1, Emplacement::Anneau2};
const PositionDObjet s_anneau1Mage = PositionPortee{3, Emplacement::Anneau1};
const PositionDObjet s_collierGuerrier = PositionPortee{1, Emplacement::Collier};
const PositionDObjet s_mainDroite = PositionPortee{1, Emplacement::MainDroite};

const std::optional<Objet> s_vide = std::nullopt;
const std::optional<Objet> s_casqueDeFer = Objet{IdObjet::HeaumeIntegral};
const std::optional<Objet> s_casqueDeCuir = Objet{IdObjet::Capuchon};
const std::optional<Objet> s_gants = Objet{IdObjet::GantsDeTissu};
const std::optional<Objet> s_bottes = Objet{IdObjet::Chaussons};
const std::optional<Objet> s_anneau = Objet{IdObjet::AnneauDeProtection};
}  // namespace

TEST_CASE("deplacement : source vide, jamais permis") {
    REQUIRE_FALSE(deplacementPermis(s_sac0, s_vide, s_sac1, s_vide));
    REQUIRE_FALSE(deplacementPermis(s_sac0, s_vide, s_sac1, s_gants));
    REQUIRE_FALSE(deplacementPermis(s_teteGuerrier, s_vide, s_sac0, s_vide));
    REQUIRE_FALSE(deplacementPermis(s_sac0, s_vide, s_sac0, s_vide));
}

TEST_CASE("deplacement : une position vers elle-meme est permise") {
    REQUIRE(deplacementPermis(s_sac0, s_gants, s_sac0, s_gants));
    REQUIRE(deplacementPermis(s_teteGuerrier, s_casqueDeFer, s_teteGuerrier, s_casqueDeFer));
}

TEST_CASE("deplacement sac vers sac : case vide, case occupee") {
    REQUIRE(deplacementPermis(s_sac0, s_gants, s_sac1, s_vide));
    REQUIRE(deplacementPermis(s_sac0, s_gants, s_sac1, s_bottes));  // echange, toujours permis
}

TEST_CASE("deplacement sac vers emplacement : compatible, incompatible, occupe") {
    REQUIRE(deplacementPermis(s_sac0, s_gants, s_gantsMage, s_vide));
    REQUIRE_FALSE(deplacementPermis(s_sac0, s_gants, s_teteGuerrier, s_casqueDeFer));  // gants sur la tete
    REQUIRE_FALSE(deplacementPermis(s_sac0, s_gants, s_mainDroite, s_vide));
    // Occupe : echange, l'objet porte va dans le sac sans contrainte.
    REQUIRE(deplacementPermis(s_sac0, s_casqueDeCuir, s_teteGuerrier, s_casqueDeFer));
}

TEST_CASE("deplacement emplacement vers sac : case vide, compatible, incompatible") {
    REQUIRE(deplacementPermis(s_teteGuerrier, s_casqueDeFer, s_sac0, s_vide));
    REQUIRE(deplacementPermis(s_teteGuerrier, s_casqueDeFer, s_sac0, s_casqueDeCuir));
    REQUIRE_FALSE(deplacementPermis(s_teteGuerrier, s_casqueDeFer, s_sac0, s_gants));
}

TEST_CASE("deplacement emplacement vers emplacement : anneaux, entre personnages, incompatible") {
    REQUIRE(deplacementPermis(s_anneau1Guerrier, s_anneau, s_anneau2Guerrier, s_vide));
    REQUIRE(deplacementPermis(s_anneau1Guerrier, s_anneau, s_anneau2Guerrier, s_anneau));
    REQUIRE(deplacementPermis(s_anneau1Guerrier, s_anneau, s_anneau1Mage, s_vide));
    REQUIRE(deplacementPermis(s_teteGuerrier, s_casqueDeFer, PositionDObjet{PositionPortee{3, Emplacement::Tete}}, s_casqueDeCuir));
    // Un anneau ne va ni au collier ni aux bottes.
    REQUIRE_FALSE(deplacementPermis(s_anneau1Guerrier, s_anneau, s_collierGuerrier, s_vide));
    REQUIRE_FALSE(deplacementPermis(s_anneau1Guerrier, s_anneau, PositionDObjet{PositionPortee{1, Emplacement::Bottes}}, s_vide));
}

TEST_CASE("deplacement emplacement vers emplacement : incompatible dans les deux sens") {
    REQUIRE_FALSE(deplacementPermis(s_gantsMage, s_gants, s_anneau1Mage, s_anneau));
    // Aucun des deux objets ne va a la place de l autre.
    REQUIRE_FALSE(deplacementPermis(s_anneau1Mage, s_anneau, s_gantsMage, s_gants));
}
