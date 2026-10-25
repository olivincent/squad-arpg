// ./tests/objets/test_Equipement.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>

#include "objets/CatalogueObjets.hpp"
#include "objets/Equipement.hpp"

namespace {
int armureDe(IdObjet id) {
    return definitionObjet(id).armure;
}
}  // namespace

TEST_CASE("equipement : vide au depart, armure nulle") {
    const Equipement equipement;

    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        REQUIRE_FALSE(equipement.a(static_cast<Emplacement>(i)).has_value());
    }
    REQUIRE(equipement.armureTotale() == 0);
}

TEST_CASE("equipement : poser sur un emplacement vide ne rend rien") {
    Equipement equipement;

    const std::optional<Objet> ancien = equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeFer});

    REQUIRE_FALSE(ancien.has_value());
    REQUIRE(equipement.a(Emplacement::Tete) == Objet{IdObjet::CasqueDeFer});
    REQUIRE_FALSE(equipement.a(Emplacement::Torse).has_value());
}

TEST_CASE("equipement : poser sur un emplacement occupe rend l'ancien objet") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeFer}).has_value());

    const std::optional<Objet> ancien = equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeCuir});

    REQUIRE(ancien == Objet{IdObjet::CasqueDeFer});
    REQUIRE(equipement.a(Emplacement::Tete) == Objet{IdObjet::CasqueDeCuir});
}

TEST_CASE("equipement : retirer rend l'objet et vide l'emplacement") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Torse, {IdObjet::CuirasseDeMailles}).has_value());

    const std::optional<Objet> retire = equipement.retirer(Emplacement::Torse);

    REQUIRE(retire == Objet{IdObjet::CuirasseDeMailles});
    REQUIRE_FALSE(equipement.a(Emplacement::Torse).has_value());
}

TEST_CASE("equipement : retirer d'un emplacement vide ne rend rien") {
    Equipement equipement;

    REQUIRE_FALSE(equipement.retirer(Emplacement::Bottes).has_value());
}

TEST_CASE("equipement : les deux anneaux sont deux emplacements distincts") {
    Equipement equipement;

    REQUIRE_FALSE(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDeProtection}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Anneau2, {IdObjet::AnneauDeProtection}).has_value());

    REQUIRE(equipement.armureTotale() == 2 * armureDe(IdObjet::AnneauDeProtection));
}

TEST_CASE("armureTotale : partielle, somme des objets portes") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeFer}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Torse, {IdObjet::CuirasseDeMailles}).has_value());

    REQUIRE(equipement.armureTotale() == armureDe(IdObjet::CasqueDeFer) + armureDe(IdObjet::CuirasseDeMailles));
}

TEST_CASE("armureTotale : complete, tous les emplacements de defense garnis") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeFer}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Torse, {IdObjet::CuirasseDeMailles}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Jambes, {IdObjet::JambieresDeFer}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Gants, {IdObjet::GantsDeCuir}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Bottes, {IdObjet::BottesDeCuir}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::MainGauche, {IdObjet::BouclierRond}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Collier, {IdObjet::CollierDeProtection}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDeProtection}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Anneau2, {IdObjet::AnneauDeProtection}).has_value());

    int attendu = 0;
    for (const IdObjet id : {IdObjet::CasqueDeFer, IdObjet::CuirasseDeMailles, IdObjet::JambieresDeFer,
                             IdObjet::GantsDeCuir, IdObjet::BottesDeCuir, IdObjet::BouclierRond,
                             IdObjet::CollierDeProtection, IdObjet::AnneauDeProtection,
                             IdObjet::AnneauDeProtection}) {
        attendu += armureDe(id);
    }
    REQUIRE(equipement.armureTotale() == attendu);
}

TEST_CASE("armureTotale : suit les changements, sans rien garder d'un ancien etat") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeFer}).has_value());
    REQUIRE(equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeCuir}).has_value());

    REQUIRE(equipement.armureTotale() == armureDe(IdObjet::CasqueDeCuir));

    REQUIRE(equipement.retirer(Emplacement::Tete).has_value());
    REQUIRE(equipement.armureTotale() == 0);
}

TEST_CASE("equipement : le bonus total additionne les bonus des objets portes") {
    Equipement equipement;
    REQUIRE(equipement.bonusTotal() == Caracteristiques());

    static_cast<void>(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));
    static_cast<void>(equipement.poser(Emplacement::Anneau2, {IdObjet::AnneauDePuissance}));
    static_cast<void>(equipement.poser(Emplacement::Collier, {IdObjet::CollierDEndurance}));

    const Caracteristiques attendu = definitionObjet(IdObjet::AnneauDePuissance).bonus +
                                     definitionObjet(IdObjet::AnneauDePuissance).bonus +
                                     definitionObjet(IdObjet::CollierDEndurance).bonus;
    REQUIRE(equipement.bonusTotal() == attendu);
}

TEST_CASE("equipement : retirer un objet retire son bonus") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));

    static_cast<void>(equipement.retirer(Emplacement::Anneau1));

    REQUIRE(equipement.bonusTotal() == Caracteristiques());
}

TEST_CASE("equipement : le bonus de combat total additionne les bonus des objets portes") {
    Equipement equipement;
    REQUIRE(equipement.bonusDeCombatTotal() == StatistiquesDeCombat());

    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::BottesDeCuir}));
    static_cast<void>(equipement.poser(Emplacement::Tete, {IdObjet::CasqueDeCuir}));
    static_cast<void>(equipement.poser(Emplacement::Torse, {IdObjet::CuirasseDeMailles}));  // aucun bonus de combat

    REQUIRE(equipement.bonusDeCombatTotal() ==
            definitionObjet(IdObjet::BottesDeCuir).bonusDeCombat + definitionObjet(IdObjet::CasqueDeCuir).bonusDeCombat);
    REQUIRE(equipement.bonusDeCombatTotal().esquiveContact == 5);
    REQUIRE(equipement.bonusDeCombatTotal().esquiveProjectile == 5);
}

TEST_CASE("equipement : retirer un objet retire son bonus de combat") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::BottesDeCuir}));

    static_cast<void>(equipement.retirer(Emplacement::Bottes));

    REQUIRE(equipement.bonusDeCombatTotal() == StatistiquesDeCombat());
}

TEST_CASE("typeDArme : le type de l'arme de la main droite, vide sans arme") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.typeDArme().has_value());

    static_cast<void>(equipement.poser(Emplacement::MainDroite, {IdObjet::Dague}));
    REQUIRE(equipement.typeDArme() == TypeDArme::Dague);

    static_cast<void>(equipement.poser(Emplacement::MainDroite, {IdObjet::Arc}));
    REQUIRE(equipement.typeDArme() == TypeDArme::Arc);

    static_cast<void>(equipement.retirer(Emplacement::MainDroite));
    static_cast<void>(equipement.poser(Emplacement::MainGauche, {IdObjet::BouclierRond}));
    REQUIRE_FALSE(equipement.typeDArme().has_value());  // un bouclier n'est pas une arme
}

TEST_CASE("attaqueDeBase d'un equipement : celle de l'arme de la main droite, sinon les mains nues") {
    Equipement equipement;
    REQUIRE(equipement.attaqueDeBase() == mainsNues());

    static_cast<void>(equipement.poser(Emplacement::MainDroite, {IdObjet::EpeeADeuxMains}));
    REQUIRE(equipement.attaqueDeBase() == definitionObjet(IdObjet::EpeeADeuxMains).arme.value().attaque);
}

TEST_CASE("equipement : le bouclier compte dans les statistiques de combat totales, sans bonus de recuperation") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::MainGauche, {IdObjet::BouclierRond}));
    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::BottesDeCuir}));

    const StatistiquesDeCombat total = equipement.bonusDeCombatTotal();

    const DonneesDeBouclier& bouclier = definitionObjet(IdObjet::BouclierRond).bouclier.value();
    REQUIRE(total.chanceDeBlocage == bouclier.chanceDeBlocage);
    REQUIRE(total.montantBloque == bouclier.montantBloque);
    REQUIRE(total.esquiveContact == definitionObjet(IdObjet::BottesDeCuir).bonusDeCombat.esquiveContact);
}
