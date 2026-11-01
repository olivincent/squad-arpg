// ./tests/monstres/test_Monstre_soigneur.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_monstre.hpp"
#include "base/Cercle.hpp"
#include "combat/AllieVu.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/NatureDAttaque.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/TileMap.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monstres/RoleDeMonstre.hpp"

// Le rôle de soigneur (étape 107.4), vu du monstre : engager un soin sur le plus blessé du groupe, l'activation de 0,8 s, la
// recharge de 6 s, la marche vers le blessé. L'application du soin au monstre visé est celle de `Partie` (test_Partie_soigneur.cpp).
// Le soigneur de ces tests : `soigneurSansHasard()` (soin de 75, portée 150, 6 s, 0,8 s, seuil de blessure de 75 %), en
// (1000, 1000), avec un personnage à (1100, 1000) qui lui sert de proie ; les alliés sont des monstres de 300 points de vie.

namespace {
Monstre unSoigneur(sf::Vector2f centre) {
    return Monstre(centre, IdMonstre{1}, 0, soigneurSansHasard());
}

// Un allié de 300 points de vie maximum, de la taille d'un monstre d'infanterie.
AllieVu unAllie(int id, sf::Vector2f centre, int pointsDeVie) {
    return {IdMonstre{id}, cibleMonstre(centre), pointsDeVie, 300};
}

// Le soigneur voit ce personnage (sa proie, son seul candidat de frappe) et ces alliés pendant `duree` secondes.
void ecoulerAvecAllies(Monstre& monstre, float duree, Cercle personnage, const std::vector<AllieVu>& allies, const TileMap& carte) {
    const PersonnageVu proie{Place{1}, personnage};
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        monstre.mettreAJour(s_dureeTic, proie, {proie}, allies, carte, {}, {});
    }
}

const sf::Vector2f s_centre{1000.0f, 1000.0f};
const Cercle s_personnage = cibleMonstre({1100.0f, 1000.0f});
// Un blessé (100 points de 300) à 100 px : 70 px bord à bord, dans la portée de 150.
const std::vector<AllieVu> s_unBlesse = {unAllie(2, {900.0f, 1000.0f}, 100)};

// Le soigneur, son soin engagé sur le blessé.
Monstre unSoigneurQuiSoigne() {
    Monstre soigneur = unSoigneur(s_centre);
    ecoulerAvecAllies(soigneur, 0.1f, s_personnage, s_unBlesse, carteOuverte());
    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
    return soigneur;
}
}  // namespace

TEST_CASE("soigneur : engage son soin sur le blesse a portee et en vue") {
    Monstre soigneur = unSoigneur(s_centre);
    ecoulerAvecAllies(soigneur, 0.1f, s_personnage, s_unBlesse, carteOuverte());

    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));

    REQUIRE(soigneur.cibleDuSoinEngage() == IdMonstre{2});
    REQUIRE_FALSE(soigneur.cibleEngagee().has_value());
}

TEST_CASE("soigneur : personne a soigner, pas de soin") {
    // Plein, juste au seuil (75 % de 300 : 225 n'est pas « sous » 75 %), et personne du tout.
    const std::vector<std::vector<AllieVu>> groupes = {
        {unAllie(2, {900.0f, 1000.0f}, 300)}, {unAllie(2, {900.0f, 1000.0f}, 225)}, {}};
    for (const std::vector<AllieVu>& allies : groupes) {
        Monstre soigneur = unSoigneur(s_centre);
        ecoulerAvecAllies(soigneur, 0.1f, s_personnage, allies, carteOuverte());

        REQUIRE_FALSE(soigneur.engagerSoin(allies, carteOuverte()));
        REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());
    }
}

TEST_CASE("soigneur : un allie sous le seuil d'un point est soigne") {
    Monstre soigneur = unSoigneur(s_centre);
    const std::vector<AllieVu> allies = {unAllie(2, {900.0f, 1000.0f}, 224)};
    ecoulerAvecAllies(soigneur, 0.1f, s_personnage, allies, carteOuverte());

    REQUIRE(soigneur.engagerSoin(allies, carteOuverte()));
}

TEST_CASE("soigneur : soigne le plus blesse en proportion") {
    Monstre soigneur = unSoigneur(s_centre);
    // 100 sur 300 (33 %) contre 10 sur 60 (17 %) : le second a perdu moins de points, mais davantage en proportion.
    const std::vector<AllieVu> allies = {unAllie(2, {900.0f, 1000.0f}, 100),
                                         AllieVu{IdMonstre{3}, cibleMonstre({1000.0f, 920.0f}), 10, 60}};
    ecoulerAvecAllies(soigneur, 0.1f, s_personnage, allies, carteOuverte());

    REQUIRE(soigneur.engagerSoin(allies, carteOuverte()));

    REQUIRE(soigneur.cibleDuSoinEngage() == IdMonstre{3});
}

TEST_CASE("soigneur : un blesse hors de portee n'est pas soigne, le soigneur marche vers lui plutot que de frapper") {
    Monstre soigneur = unSoigneur(s_centre);
    const std::vector<AllieVu> allies = {unAllie(2, {600.0f, 1000.0f}, 100)};  // 370 px bord à bord, sa proie est à 70 px

    ecoulerAvecAllies(soigneur, 0.5f, s_personnage, allies, carteOuverte());

    REQUIRE(soigneur.getCentre().x < s_centre.x - 50.0f);
    REQUIRE_FALSE(soigneur.engagerSoin(allies, carteOuverte()));
    REQUIRE_FALSE(engager(soigneur, s_personnage, Place{1}, carteOuverte()));  // en marche : aucun coup non plus
}

TEST_CASE("soigneur : un blesse hors de vue n'est pas soigne, le soigneur le rejoint") {
    // Même géométrie sur une carte dégagée et sur une carte avec un mur entre eux : à 150 px l'un de l'autre (120 px bord à bord).
    const std::vector<AllieVu> allies = {unAllie(2, {250.0f, 100.0f}, 100)};
    const Cercle personnage = cibleMonstre({100.0f, 200.0f});

    Monstre enVue = unSoigneur({100.0f, 100.0f});
    ecoulerAvecAllies(enVue, 0.1f, personnage, allies, carteDegagee10x10());
    verifierPointProche(enVue.getCentre(), {100.0f, 100.0f});
    REQUIRE(enVue.engagerSoin(allies, carteDegagee10x10()));

    Monstre cache = unSoigneur({100.0f, 100.0f});
    ecoulerAvecAllies(cache, 0.1f, personnage, allies, carteAvecMur());
    REQUIRE((cache.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() > 1.0f);
    REQUIRE_FALSE(cache.engagerSoin(allies, carteAvecMur()));
}

TEST_CASE("soigneur : le soin s'active 0,8 s, immobile, puis part avec son montant") {
    Generateur generateur = generateurDeTest();
    Monstre soigneur = unSoigneurQuiSoigne();

    // Immobile pendant l'activation, même si sa proie s'éloigne.
    for (int i = 0; i < nombreDeTics(0.7f); ++i) {
        soigneur.mettreAJour(s_dureeTic, PersonnageVu{Place{1}, cibleMonstre({1190.0f, 1000.0f})}, {}, s_unBlesse, carteOuverte(), {}, {});
        REQUIRE_FALSE(soigneur.avancerSoin(s_dureeTic, generateur).has_value());
    }
    verifierPointProche(soigneur.getCentre(), s_centre, 0.01f);
    REQUIRE(soigneur.cibleDuSoinEngage().has_value());

    int departs = 0;
    std::optional<SoinDeMonstre> soin;
    for (int i = 0; i < nombreDeTics(0.3f); ++i) {
        if (const std::optional<SoinDeMonstre> parti = soigneur.avancerSoin(s_dureeTic, generateur)) {
            ++departs;
            soin = parti;
        }
    }
    REQUIRE(departs == 1);
    REQUIRE(soin->cible == IdMonstre{2});
    REQUIRE(soin->montant == 75);
    REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());
}

TEST_CASE("soigneur : la recharge du soin est de 6 s") {
    Generateur generateur = generateurDeTest();
    Monstre soigneur = unSoigneurQuiSoigne();
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        static_cast<void>(soigneur.avancerSoin(s_dureeTic, generateur));  // le départ du soin est testé ailleurs
    }
    REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());

    ecoulerAvecAllies(soigneur, 5.5f, s_personnage, s_unBlesse, carteOuverte());
    REQUIRE_FALSE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));

    ecoulerAvecAllies(soigneur, 1.0f, s_personnage, s_unBlesse, carteOuverte());
    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
}

TEST_CASE("soigneur : un soin annule ne consomme pas de recharge") {
    Monstre soigneur = unSoigneurQuiSoigne();

    soigneur.annulerSoin();

    REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());
    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
}

TEST_CASE("soigneur : un soin et un coup ne sont jamais engages ensemble") {
    SECTION("un soin engage empeche le coup") {
        Monstre soigneur = unSoigneurQuiSoigne();

        REQUIRE_FALSE(engager(soigneur, s_personnage, Place{1}, carteOuverte()));
        REQUIRE_FALSE(soigneur.cibleEngagee().has_value());
    }
    SECTION("un coup engage empeche le soin") {
        Monstre soigneur = unSoigneur(s_centre);
        ecoulerAvecAllies(soigneur, 0.1f, s_personnage, {}, carteOuverte());  // personne à soigner : il frappe
        REQUIRE(engager(soigneur, s_personnage, Place{1}, carteOuverte()));

        REQUIRE_FALSE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
        REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());
    }
}

TEST_CASE("soigneur : sans rien a soigner, c'est un tireur faible sans recul") {
    Monstre soigneur = unSoigneur(s_centre);
    REQUIRE(soigneur.getZoneDeControle() == 0.0f);
    REQUIRE(soigneur.getAttaque().nature == NatureDAttaque::Projectile);

    // Au contact d'un personnage : il ne recule pas.
    ecoulerAvecAllies(soigneur, 1.0f, cibleMonstre({1040.0f, 1000.0f}), {}, carteOuverte());
    verifierPointProche(soigneur.getCentre(), s_centre);

    // Un personnage à 70 px bord à bord : il frappe de là.
    Monstre autre = unSoigneur(s_centre);
    ecoulerAvecAllies(autre, 0.1f, s_personnage, {}, carteOuverte());
    REQUIRE(engager(autre, s_personnage, Place{1}, carteOuverte()));
}

TEST_CASE("soigneur : un monstre qui n'a pas ce role n'engage aucun soin") {
    Monstre infanterie({1000.0f, 1000.0f}, IdMonstre{1}, 0, monstreSansHasard());
    ecoulerAvecAllies(infanterie, 0.1f, s_personnage, s_unBlesse, carteOuverte());

    REQUIRE_FALSE(infanterie.engagerSoin(s_unBlesse, carteOuverte()));
    REQUIRE_FALSE(infanterie.cibleDuSoinEngage().has_value());
}

TEST_CASE("recevoirSoin : plafonne au maximum, rien en retour au poste") {
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreSansHasard());
    REQUIRE(subir(monstre, 100) == 100);

    REQUIRE(monstre.recevoirSoin(75) == 75);
    REQUIRE(monstre.recevoirSoin(500) == 25);  // 300 - (100 - 75)
    REQUIRE(monstre.getPointsDeVie() == monstre.getPointsDeVieMax());

    Monstre enRetour({100.0f, 100.0f}, IdMonstre{2}, 0, monstreSansHasard());
    amenerEnRetourHorsTerritoire(enRetour, carteLongue(), 100);
    REQUIRE(enRetour.recevoirSoin(75) == 0);
}
