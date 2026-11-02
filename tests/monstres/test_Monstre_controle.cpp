// ./tests/monstres/test_Monstre_controle.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_monstre.hpp"
#include "base/Cercle.hpp"
#include "combat/AllieVu.hpp"
#include "combat/Controle.hpp"
#include "combat/EntiteVue.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"

// Le contrôle subi par un monstre (étape 113.3), vu du monstre : étourdi il ne fait rien, ralenti il avance moins vite, entravé son
// attaque de base est plus lente, en Disruption il ne soigne plus mais frappe encore. Monstre d'infanterie centré en (100, 100), sauf mention.

namespace {
void subirLeControle(Monstre& monstre, const EffetActif& controle) {
    Generateur generateur = generateurDeTest();
    static_cast<void>(monstre.recevoirDegats(attaqueQuiControle(controle), generateur));
}

// Le chemin parcouru en `duree` secondes vers cette cible lointaine mais détectée (200 px de détection).
float distanceParcourue(Monstre& monstre, float duree) {
    const sf::Vector2f depart = monstre.getCentre();
    ecoulerTemps(monstre, duree, pointCible({300.0f, 100.0f}), carteDegagee10x10());
    return (monstre.getCentre() - depart).length();
}

// Les tics qu'il faut à un coup engagé pour partir.
int ticsAvantLeDepart(Monstre& monstre, Generateur& generateur) {
    for (int tic = 1; tic <= nombreDeTics(5.0f); ++tic) {
        if (monstre.avancerAttaque(s_dureeTic, generateur).has_value()) {
            return tic;
        }
    }
    return -1;
}
}  // namespace

TEST_CASE("controle monstre : etourdi, il ne bouge pas, puis il repart") {
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    subirLeControle(monstre, unControle(EtatInflige::Etourdissement, 2.0f));
    const sf::Vector2f depart = monstre.getCentre();

    ecoulerTemps(monstre, 1.5f, pointCible({300.0f, 100.0f}), carteDegagee10x10());
    REQUIRE(monstre.getCentre() == depart);

    ecoulerTemps(monstre, 1.0f, pointCible({300.0f, 100.0f}), carteDegagee10x10());  // l'étourdissement a pris fin
    REQUIRE(monstre.getCentre() != depart);
}

TEST_CASE("controle monstre : ralenti, il avance proportionnellement moins vite") {
    Monstre libre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    Monstre ralenti({100.0f, 100.0f}, IdMonstre{2}, 0, monstreAvecArmure(0));
    subirLeControle(ralenti, unControle(EtatInflige::Ralentissement, 10.0f, 50));

    const float distanceLibre = distanceParcourue(libre, 0.4f);
    const float distanceRalentie = distanceParcourue(ralenti, 0.4f);

    REQUIRE(distanceLibre > 0.0f);
    REQUIRE(distanceRalentie > distanceLibre * 0.4f);
    REQUIRE(distanceRalentie < distanceLibre * 0.6f);
}

TEST_CASE("controle monstre : l'entrave ne ralentit pas son deplacement") {
    Monstre libre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    Monstre entrave({100.0f, 100.0f}, IdMonstre{2}, 0, monstreAvecArmure(0));
    subirLeControle(entrave, unControle(EtatInflige::Entrave, 10.0f, 50));

    REQUIRE(distanceParcourue(entrave, 0.4f) == distanceParcourue(libre, 0.4f));
}

TEST_CASE("controle monstre : etourdi, il refuse d'engager un coup et le coup engage est annule") {
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const Cercle cible = pointCible({110.0f, 100.0f});
    REQUIRE(engager(monstre, cible, Place{1}, carteOuverte()));
    REQUIRE(monstre.cibleEngagee().has_value());

    subirLeControle(monstre, unControle(EtatInflige::Etourdissement, 2.0f));

    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE_FALSE(engager(monstre, cible, Place{1}, carteOuverte()));
    ecoulerTemps(monstre, 2.5f, std::nullopt, carteOuverte());
    REQUIRE(engager(monstre, cible, Place{1}, carteOuverte()));
}

TEST_CASE("controle monstre : le coup annule par l'etourdissement ne consomme pas la recharge") {
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    const Cercle cible = pointCible({110.0f, 100.0f});
    REQUIRE(engager(monstre, cible, Place{1}, carteOuverte()));

    subirLeControle(monstre, unControle(EtatInflige::Etourdissement, 0.5f));
    ecoulerTemps(monstre, 0.6f, std::nullopt, carteOuverte());

    REQUIRE(engager(monstre, cible, Place{1}, carteOuverte()));  // prêt tout de suite : aucune recharge tirée
}

TEST_CASE("controle monstre : entrave, le coup engage part plus tard") {
    Generateur generateur = generateurDeTest();
    Monstre libre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    Monstre entrave({100.0f, 100.0f}, IdMonstre{2}, 0, monstreAvecArmure(0));
    subirLeControle(entrave, unControle(EtatInflige::Entrave, 20.0f, 50));
    const Cercle cible = pointCible({110.0f, 100.0f});
    REQUIRE(engager(libre, cible, Place{1}, carteOuverte()));
    REQUIRE(engager(entrave, cible, Place{1}, carteOuverte()));

    const int ticsLibre = ticsAvantLeDepart(libre, generateur);
    const int ticsEntrave = ticsAvantLeDepart(entrave, generateur);

    REQUIRE(ticsLibre > 0);
    // La moitié de la vitesse : environ le double du temps (le dernier tic compte entier, d'où la marge).
    REQUIRE(ticsEntrave >= 2 * ticsLibre - 3);
    REQUIRE(ticsEntrave <= 2 * ticsLibre + 1);
}

TEST_CASE("controle monstre : entrave, la recharge de l'attaque de base est plus lente") {
    Generateur generateur = generateurDeTest();
    Monstre libre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    Monstre entrave({100.0f, 100.0f}, IdMonstre{2}, 0, monstreAvecArmure(0));
    const Cercle cible = pointCible({110.0f, 100.0f});
    static_cast<void>(frapperEnEntier(libre, cible, generateur));
    static_cast<void>(frapperEnEntier(entrave, cible, generateur));
    subirLeControle(entrave, unControle(EtatInflige::Entrave, 30.0f, 80));  // après le coup : seule la recharge en pâtit

    // Après 1,5 s, le libre a fini sa recharge (1,3 s au plus), l'entravé en est à 0,3 s sur au moins 0,7.
    ecoulerTemps(libre, 1.5f, std::nullopt, carteOuverte());
    ecoulerTemps(entrave, 1.5f, std::nullopt, carteOuverte());

    REQUIRE(engager(libre, cible, Place{1}, carteOuverte()));
    REQUIRE_FALSE(engager(entrave, cible, Place{1}, carteOuverte()));
}

TEST_CASE("controle monstre : en Disruption, il frappe encore") {
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));
    subirLeControle(monstre, unControle(EtatInflige::Disruption, 5.0f));

    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), Place{1}, carteOuverte()));
}

namespace {
const sf::Vector2f s_centreDuSoigneur{1000.0f, 1000.0f};
const Cercle s_personnageDuSoigneur = cibleMonstre({1100.0f, 1000.0f});
const std::vector<AllieVu> s_unBlesse = {{IdMonstre{2}, cibleMonstre({900.0f, 1000.0f}), 100, 300}};

void ecoulerSoigneur(Monstre& monstre, float duree) {
    const PersonnageVu proie{Place{1}, s_personnageDuSoigneur};
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        monstre.mettreAJour(s_dureeTic, proie, {proie}, s_unBlesse, carteOuverte(), {}, {});
    }
}
}  // namespace

TEST_CASE("controle monstre : en Disruption, le soigneur ne soigne pas") {
    Monstre soigneur(s_centreDuSoigneur, IdMonstre{1}, 0, soigneurSansHasard());
    ecoulerSoigneur(soigneur, 0.1f);
    subirLeControle(soigneur, unControle(EtatInflige::Disruption, 3.0f));

    REQUIRE_FALSE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));

    ecoulerSoigneur(soigneur, 3.5f);  // la Disruption a pris fin
    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
}

TEST_CASE("controle monstre : la Disruption annule le soin en activation, sans recharge") {
    Monstre soigneur(s_centreDuSoigneur, IdMonstre{1}, 0, soigneurSansHasard());
    ecoulerSoigneur(soigneur, 0.1f);
    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
    REQUIRE(soigneur.cibleDuSoinEngage().has_value());

    subirLeControle(soigneur, unControle(EtatInflige::Disruption, 0.5f));

    REQUIRE_FALSE(soigneur.cibleDuSoinEngage().has_value());
    ecoulerSoigneur(soigneur, 0.6f);
    REQUIRE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));  // aucune recharge consommée
}

TEST_CASE("controle monstre : etourdi, le soigneur ne soigne pas non plus") {
    Monstre soigneur(s_centreDuSoigneur, IdMonstre{1}, 0, soigneurSansHasard());
    ecoulerSoigneur(soigneur, 0.1f);
    subirLeControle(soigneur, unControle(EtatInflige::Etourdissement, 2.0f));

    REQUIRE_FALSE(soigneur.engagerSoin(s_unBlesse, carteOuverte()));
}

TEST_CASE("controle monstre : la pose est notee, une attaque qui controle alerte le monstre") {
    Monstre monstre({100.0f, 100.0f}, IdMonstre{1}, 0, monstreAvecArmure(0));

    subirLeControle(monstre, unControle(EtatInflige::Ralentissement, 3.0f, 30));

    const std::vector<ControleApplique> poses = monstre.prendreLesControles();
    REQUIRE(poses.size() == 1);
    REQUIRE(poses[0].etat == EtatInflige::Ralentissement);
    // Alerté sans voir sa proie à 230 px (hors détection, mais dans le territoire) : il la poursuit.
    const sf::Vector2f depart = monstre.getCentre();
    ecoulerTemps(monstre, 0.5f, pointCible({330.0f, 100.0f}), carteDegagee10x10());
    REQUIRE(monstre.getCentre() != depart);
}
