// ./tests/jeu/test_Partie_zonesDeControle.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"
#include "combat/Skill.hpp"

// Zones de contrôle et attaque d'opportunité (étape 77). Le monstre n'a pas d'armure dans ces niveaux
// (Niveau::armureMonstres vaut 0) : les dégâts lus sur lui sont ceux des attaques de base.

TEST_CASE("zone de controle : un mage au contact qui s'eloigne recoit un coup gratuit du monstre") {
    // Un mage seul (zone 0) à 40 px du monstre, qui l'engage (zone de 20 px bord à bord).
    // Hypothèse : les personnages sont plus rapides que les monstres (le mage sort de la zone).
    Partie partie(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()}, s_graineDeTest);
    const int puissanceDuMonstre = partie.getMonstres().front().getPuissanceAttaque();
    const int apresLePremierCoup = jusquAuPremierCoup(partie, 1);

    partie.ordonnerDeplacement({210.0f, 20.0f});
    int pointsDeVie = apresLePremierCoup;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, 1);
    }

    REQUIRE(pointsDeVie == apresLePremierCoup - puissanceDuMonstre);  // un seul coup gratuit (armure 0)
}

TEST_CASE("zone de controle : un guerrier qui reste au contact ne recoit aucun coup gratuit") {
    Partie partie(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest()}, s_graineDeTest);
    const int apresLePremierCoup = jusquAuPremierCoup(partie, 1);

    int pointsDeVie = apresLePremierCoup;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, 1);
    }

    REQUIRE(pointsDeVie == apresLePremierCoup);
}

namespace {

// Trois guerriers (le premier au nord, à (235, 160)) et un monstre à (270, 160), au contact du
// premier seulement. L'attaque de base du premier est reconnaissable : `degats` dégâts, recharge
// de 100 s, donc après son premier coup tout nouveau coup de cette force sur le monstre est un coup
// gratuit. Dès que le monstre est touché, le troisième guerrier (à (235, 260)) le provoque : le
// monstre quitte le premier pour aller vers lui. Renvoie le nombre d'images où le monstre perd au
// moins `degats` points de vie, sur 1,2 s.
int coupsReconnaissables(float zoneDuPremier, int degats) {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0].attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = s_porteeMeleeDeTest,
                                          .puissance = degats, .rechargeMin = 100.0f, .rechargeMax = 100.0f,
                                          .tempsExecution = s_executionBaseDeTest});
    composition[0].zoneDeControle = zoneDuPremier;
    Partie partie(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest);

    int coups = 0;
    int pointsDeVie = 1000;
    bool provoque = false;
    for (int i = 0; i < nombreDeTics(1.2f) && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
        if (partie.getMonstres().empty()) {
            break;
        }
        const int apres = partie.getMonstres().front().getPointsDeVie();
        if (pointsDeVie - apres >= degats) {
            ++coups;
        }
        pointsDeVie = apres;
        if (!provoque && apres < 1000) {
            partie.demanderSkill({3, IdSkill::Provocation});
            provoque = true;
        }
    }
    return coups;
}

}  // namespace

TEST_CASE("zone de controle : un monstre qui quitte un guerrier pour un provocateur est frappe gratuitement") {
    // Le premier coup (normal) et le coup gratuit.
    REQUIRE(coupsReconnaissables(20.0f, 300) == 2);
}

TEST_CASE("zone de controle : un personnage sans zone ne frappe jamais gratuitement") {
    // Même scénario, mais le premier guerrier n'engage personne : seul son premier coup (normal) compte.
    REQUIRE(coupsReconnaissables(0.0f, 300) == 1);
}

TEST_CASE("zone de controle : un monstre tue par un coup gratuit est retire et le verrou est leve") {
    // Le premier coup (600) laisse le monstre à 400 points de vie ; le coup gratuit (600) le tue.
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0].attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = s_porteeMeleeDeTest, .puissance = 600,
                                          .rechargeMin = 100.0f, .rechargeMax = 100.0f,
                                          .tempsExecution = s_executionBaseDeTest});
    Partie partie(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest);
    REQUIRE(partie.cliquer({270.0f, 160.0f}) == ResultatClic::Monstre);

    // En 1,2 s les autres guerriers ne retirent que quelques points : seul le coup gratuit peut le tuer.
    bool provoque = false;
    for (int i = 0; i < nombreDeTics(1.2f) && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
        if (!provoque && !partie.getMonstres().empty() && partie.getMonstres().front().getPointsDeVie() < 1000) {
            partie.demanderSkill({3, IdSkill::Provocation});
            provoque = true;
        }
    }

    REQUIRE(provoque);
    REQUIRE(partie.getMonstres().empty());
    REQUIRE_FALSE(partie.getEscouade().getCibleVerrouillee().has_value());
}

TEST_CASE("zone de controle : un monstre qui rentre a son poste ne donne ni ne recoit de coup gratuit") {
    Partie partie = unePartie(niveauDeLaFuite(), s_graineDeTest);
    const int vieDeDepart = fuyard(partie).getPointsDeVie();
    std::vector<int> vieDesPersonnages;
    for (std::size_t place = 1; place <= 3; ++place) {
        vieDesPersonnages.push_back(*partie.getEscouade().pointsDeVieDuPersonnage(place));
    }
    attirerHorsDuTerritoire(partie, {1080.0f, 300.0f});

    // L'escouade rentre à travers le monstre qui rentre et finit par le toucher...
    partie.ordonnerDeplacement({300.0f, 300.0f});
    bool contact = false;
    for (int i = 0; i < nombreDeTics(5.0f) && !contact; ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const Cercle& personnage : partie.getEscouade().cerclesDesVivants()) {
            contact = contact || distanceBordABord(personnage, fuyard(partie).getCercle()) <= 10.0f;
        }
    }
    REQUIRE(contact);
    REQUIRE_FALSE(fuyard(partie).estCiblable());

    // ... puis fait demi-tour : elle s'écarte du monstre (lui, il continue vers son poste). Un monstre
    // qui rentre n'engage personne : aucun personnage ne perd de points de vie, et lui non plus.
    partie.ordonnerDeplacement({1080.0f, 300.0f});
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (std::size_t place = 1; place <= 3; ++place) {
            REQUIRE(partie.getEscouade().pointsDeVieDuPersonnage(place) == vieDesPersonnages[place - 1]);
        }
    }
    REQUIRE_FALSE(fuyard(partie).estCiblable());
    REQUIRE(fuyard(partie).getPointsDeVie() == vieDeDepart);
}

// Dégagement des tireurs (étape 78). Guerrier (place 1, en (235, 210)) et mage (place 2, en (185, 210)),
// monstre en (219, 232) : il engage les deux (écarts bord à bord de 2 et 15,5 px pour une zone de 20 px),
// et sa proie est le guerrier, plus proche. Le guerrier frappe une seule fois (1 dégât, recharge de 100 s) ;
// le mage tire des coups de 7 dégâts : on reconnaît ainsi qui touche le monstre.

namespace {

constexpr int s_degatsDuMageDansLaPartie = 70;

Partie unGuerrierEtUnMageAuContact() {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), mageDeTest()};
    composition[0].attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = s_porteeMeleeDeTest, .puissance = 1,
                                          .rechargeMin = 100.0f, .rechargeMax = 100.0f,
                                          .tempsExecution = s_executionBaseDeTest});
    composition[1].attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 120.0f,
                                          .puissance = s_degatsDuMageDansLaPartie, .rechargeMin = 0.3f,
                                          .rechargeMax = 0.3f, .tempsExecution = 0.15f});
    return Partie(niveauDegage({{219.0f, 232.0f}}), std::move(composition), s_graineDeTest);
}

}  // namespace

TEST_CASE("degagement : le mage quitte le monstre tenu par le guerrier sans coup gratuit, puis tire de loin") {
    Partie partie = unGuerrierEtUnMageAuContact();
    const std::optional<int> vieDuMage = partie.getEscouade().pointsDeVieDuPersonnage(2);
    const Cercle monstre = partie.getMonstres().front().getCercle();
    REQUIRE(distanceBordABord(partie.getEscouade().cerclesDesVivants()[0], monstre) <= 20.0f);
    REQUIRE(distanceBordABord(partie.getEscouade().cerclesDesVivants()[1], monstre) <= 20.0f);

    int pointsDuMonstreALArrivee = 0;
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        const std::vector<Cercle> cercles = partie.getEscouade().cerclesDesVivants();
        REQUIRE(partie.getEscouade().pointsDeVieDuPersonnage(2) == vieDuMage);  // aucun coup, gratuit ou non
        REQUIRE(distanceBordABord(cercles[0], monstre) <= 20.0f);          // le guerrier reste au contact
        if (pointsDuMonstreALArrivee == 0 && !partie.getEscouade().degagementDuPersonnage(2).has_value() &&
            distanceBordABord(cercles[1], monstre) > 20.0f) {
            pointsDuMonstreALArrivee = partie.getMonstres().front().getPointsDeVie();
        }
    }

    const std::vector<Cercle> cercles = partie.getEscouade().cerclesDesVivants();
    REQUIRE_FALSE(partie.getEscouade().degagementDuPersonnage(2).has_value());
    REQUIRE(distanceBordABord(cercles[1], monstre) > 20.0f);
    REQUIRE(distanceBordABord(cercles[1], monstre) <= 120.0f);  // à portée de son attaque
    REQUIRE(pointsDuMonstreALArrivee > 0);
    REQUIRE(partie.getMonstres().front().getPointsDeVie() <= pointsDuMonstreALArrivee - s_degatsDuMageDansLaPartie);
}
