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

    const std::optional<Objet> ancien = equipement.poser(Emplacement::Tete, {IdObjet::HeaumeIntegral});

    REQUIRE_FALSE(ancien.has_value());
    REQUIRE(equipement.a(Emplacement::Tete) == Objet{IdObjet::HeaumeIntegral});
    REQUIRE_FALSE(equipement.a(Emplacement::Torse).has_value());
}

TEST_CASE("equipement : poser sur un emplacement occupe rend l'ancien objet") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::HeaumeIntegral}).has_value());

    const std::optional<Objet> ancien = equipement.poser(Emplacement::Tete, {IdObjet::Capuchon});

    REQUIRE(ancien == Objet{IdObjet::HeaumeIntegral});
    REQUIRE(equipement.a(Emplacement::Tete) == Objet{IdObjet::Capuchon});
}

TEST_CASE("equipement : retirer rend l'objet et vide l'emplacement") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Torse, {IdObjet::ArmureDePlaques}).has_value());

    const std::optional<Objet> retire = equipement.retirer(Emplacement::Torse);

    REQUIRE(retire == Objet{IdObjet::ArmureDePlaques});
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
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::HeaumeIntegral}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Torse, {IdObjet::ArmureDePlaques}).has_value());

    REQUIRE(equipement.armureTotale() == armureDe(IdObjet::HeaumeIntegral) + armureDe(IdObjet::ArmureDePlaques));
}

TEST_CASE("armureTotale : complete, tous les emplacements de defense garnis") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::HeaumeIntegral}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Torse, {IdObjet::ArmureDePlaques}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Jambes, {IdObjet::JambieresDePlaques}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Gants, {IdObjet::GantsDeTissu}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Bottes, {IdObjet::Chaussons}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::MainGauche, {IdObjet::Bouclier}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Collier, {IdObjet::CollierDeProtection}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDeProtection}).has_value());
    REQUIRE_FALSE(equipement.poser(Emplacement::Anneau2, {IdObjet::AnneauDeProtection}).has_value());

    int attendu = 0;
    for (const IdObjet id : {IdObjet::HeaumeIntegral, IdObjet::ArmureDePlaques, IdObjet::JambieresDePlaques,
                             IdObjet::GantsDeTissu, IdObjet::Chaussons, IdObjet::Bouclier,
                             IdObjet::CollierDeProtection, IdObjet::AnneauDeProtection,
                             IdObjet::AnneauDeProtection}) {
        attendu += armureDe(id);
    }
    REQUIRE(equipement.armureTotale() == attendu);
}

TEST_CASE("armureTotale : suit les changements, sans rien garder d'un ancien etat") {
    Equipement equipement;
    REQUIRE_FALSE(equipement.poser(Emplacement::Tete, {IdObjet::HeaumeIntegral}).has_value());
    REQUIRE(equipement.poser(Emplacement::Tete, {IdObjet::Capuchon}).has_value());

    REQUIRE(equipement.armureTotale() == armureDe(IdObjet::Capuchon));

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

    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::Solerets}));
    static_cast<void>(equipement.poser(Emplacement::Tete, {IdObjet::HeaumeIntegral}));
    static_cast<void>(equipement.poser(Emplacement::Gants, {IdObjet::GantsDeTissu}));  // aucun bonus de combat

    REQUIRE(equipement.bonusDeCombatTotal() ==
            definitionObjet(IdObjet::Solerets).bonusDeCombat + definitionObjet(IdObjet::HeaumeIntegral).bonusDeCombat);
    REQUIRE(equipement.bonusDeCombatTotal().esquiveContact == -2);  // le malus de chaque pièce lourde s'additionne
    REQUIRE(equipement.bonusDeCombatTotal().esquiveProjectile == -2);
}

TEST_CASE("equipement : retirer un objet retire son bonus de combat") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::Solerets}));

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
    static_cast<void>(equipement.poser(Emplacement::MainGauche, {IdObjet::Bouclier}));
    REQUIRE_FALSE(equipement.typeDArme().has_value());  // un bouclier n'est pas une arme
}

TEST_CASE("attaqueDeBase d'un equipement : celle de l'arme de la main droite, sinon les mains nues") {
    Equipement equipement;
    REQUIRE(equipement.attaqueDeBase() == mainsNues());

    static_cast<void>(equipement.poser(Emplacement::MainDroite, {IdObjet::EpeeADeuxMains}));
    REQUIRE(equipement.attaqueDeBase() == descriptionDeLArme(Objet{IdObjet::EpeeADeuxMains}));
}

TEST_CASE("equipement : le bouclier compte dans les statistiques de combat totales, sans bonus de recuperation") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::MainGauche, {IdObjet::Bouclier}));
    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::Solerets}));

    const StatistiquesDeCombat total = equipement.bonusDeCombatTotal();

    const DonneesDeBouclier& bouclier = definitionObjet(IdObjet::Bouclier).bouclier.value();
    REQUIRE(total.chanceDeBlocage == bouclier.chanceAuNiveau1);
    REQUIRE(total.montantBloque == bouclier.montantAuNiveau1);
    REQUIRE(total.esquiveContact == definitionObjet(IdObjet::Solerets).bonusDeCombat.esquiveContact);
}

// Les objets actifs (étape 92.2), avec les objets du catalogue.
TEST_CASE("actifPour : l'epee a une main exige de la Puissance, l'anneau de puissance la lui donne, le retirer la rend inactive") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::MainDroite, {IdObjet::EpeeAUneMain}));
    const int seuil = seuilsDeLObjet(Objet{IdObjet::EpeeAUneMain}).front().valeur;
    const int bonus = definitionObjet(IdObjet::AnneauDePuissance).bonus.valeur(Caracteristique::Puissance);
    REQUIRE(bonus >= 1);
    const Caracteristiques sous({seuil - bonus, 0, 0, 0, 0});  // juste ce que l'anneau comble

    REQUIRE_FALSE(equipement.actifPour(sous).a(Emplacement::MainDroite).has_value());
    REQUIRE(equipement.a(Emplacement::MainDroite).has_value());  // elle reste portée
    REQUIRE(equipement.actifPour(Caracteristiques({seuil, 0, 0, 0, 0})).a(Emplacement::MainDroite).has_value());

    static_cast<void>(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));
    REQUIRE(equipement.actifPour(sous).a(Emplacement::MainDroite).has_value());

    static_cast<void>(equipement.retirer(Emplacement::Anneau1));
    REQUIRE_FALSE(equipement.actifPour(sous).a(Emplacement::MainDroite).has_value());  // inactive de nouveau
}

TEST_CASE("actifPour : un objet inactif n'a aucun effet, ni armure, ni arme, ni blocage, ni bonus") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::Torse, {IdObjet::ArmureDePlaques}));
    static_cast<void>(equipement.poser(Emplacement::MainGauche, {IdObjet::Bouclier}));
    static_cast<void>(equipement.poser(Emplacement::MainDroite, {IdObjet::Arc}));
    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::Chaussons}));  // pas de seuil
    const Equipement actif = equipement.actifPour(Caracteristiques());

    REQUIRE(actif.armureTotale() == definitionObjet(IdObjet::Chaussons).armure);
    REQUIRE(actif.bonusDeCombatTotal() == StatistiquesDeCombat());  // ni blocage, ni le malus d'esquive de la plaque
    REQUIRE(actif.attaqueDeBase() == mainsNues());
    REQUIRE_FALSE(actif.typeDArme().has_value());
    REQUIRE(equipement.armureTotale() > actif.armureTotale());  // l'équipement porté, lui, les garde
}

TEST_CASE("actifPour : sans objet a seuil, l'equipement actif est l'equipement porte") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::Bottes, {IdObjet::Chaussons}));
    static_cast<void>(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));

    REQUIRE(equipement.actifPour(Caracteristiques()) == equipement);
}

TEST_CASE("caracteristiquesEffectives : la base plus les bonus des seuls objets actifs") {
    Equipement equipement;
    static_cast<void>(equipement.poser(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));

    REQUIRE(caracteristiquesEffectives(Caracteristiques({10, 0, 0, 0, 0}), equipement) ==
            Caracteristiques({10 + definitionObjet(IdObjet::AnneauDePuissance).bonus.valeur(Caracteristique::Puissance), 0, 0, 0, 0}));
}
