// ./tests/jeu/test_Partie_evenements.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Defense.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Partie.hpp"
#include "combat/Skill.hpp"

// Événements de combat (étape 79). Les monstres de `niveauDegage` ont une armure de 0 (le niveau n'en
// donne pas), sauf mention.

namespace {

// Avance de cette durée, tic par tic, et rend tous les événements produits.
std::vector<EvenementDeCombat> evenementsPendant(Partie& partie, float duree) {
    std::vector<EvenementDeCombat> tous;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        partie.mettreAJour(s_dureeTic);
        const std::vector<EvenementDeCombat> duTic = partie.prendreEvenements();
        tous.insert(tous.end(), duTic.begin(), duTic.end());
    }
    return tous;
}

const Degats& degats(const EvenementDeCombat& evenement) {
    return std::get<Degats>(evenement.detail);
}

const Soin& soin(const EvenementDeCombat& evenement) {
    return std::get<Soin>(evenement.detail);
}

// Les événements d'une sorte de détail (Degats, Soin…) sur un camp.
template <class Detail>
std::vector<EvenementDeCombat> parmi(const std::vector<EvenementDeCombat>& evenements, Camp camp) {
    std::vector<EvenementDeCombat> choisis;
    for (const EvenementDeCombat& evenement : evenements) {
        if (std::holds_alternative<Detail>(evenement.detail) && evenement.campCible == camp) {
            choisis.push_back(evenement);
        }
    }
    return choisis;
}

// Deux guerriers (en (235, 185) et (235, 235)) dont l'attaque de base tue d'un coup (2000 dégâts), et un
// monstre en (260, 210) qu'ils atteignent tous les deux : leurs deux coups partent au même tic.
Partie deuxGuerriersQuiTuentLeMemeMonstre() {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest()};
    for (DefinitionPersonnage& definition : composition) {
        definition.attaqueDeBase = Skill(TypeSkill::Degats, s_porteeMeleeDeTest, 2000, 100.0f, 100.0f, 0.0f, s_executionBaseDeTest);
    }
    return Partie(niveauDegage({{260.0f, 210.0f}}), std::move(composition), s_graineDeTest);
}

}  // namespace

TEST_CASE("evenements : un skill de degats donne un evenement sur le monstre, armure comprise") {
    constexpr int armure = 20;
    Niveau niveau = niveauDegage({{235.0f, 170.0f}});
    niveau.armureMonstres = armure;
    Partie partie(niveau, std::vector<DefinitionPersonnage>{guerrierDeTest()}, s_graineDeTest);
    partie.demanderSkill(s_adresseFrappe);

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 0.5f);

    // La frappe après armure se distingue de l'attaque de base après armure.
    const int frappeApresArmure = appliquerArmure(s_degatsFrappeDeTest, armure);
    REQUIRE(frappeApresArmure != appliquerArmure(s_degatsBaseDeTest, armure));
    int frappes = 0;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        if (degats(evenement).montant == frappeApresArmure) {
            ++frappes;
            REQUIRE(evenement.idCible == 1);
            REQUIRE_FALSE(degats(evenement).opportunite);
            verifierPointProche(evenement.position, {235.0f, 170.0f});
        }
    }
    REQUIRE(frappes == 1);
}

TEST_CASE("evenements : le coup d'un monstre donne un evenement sur le personnage touche, armure comprise") {
    Partie guerrier(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest()}, s_graineDeTest);
    Partie mage(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()}, s_graineDeTest);
    const int puissanceDuMonstre = guerrier.getMonstres().front().getPuissanceAttaque();
    static_cast<void>(jusquAuPremierCoup(guerrier, 1));
    static_cast<void>(jusquAuPremierCoup(mage, 1));

    const std::vector<EvenementDeCombat> surLeGuerrier = parmi<Degats>(guerrier.prendreEvenements(), Camp::Escouade);
    const std::vector<EvenementDeCombat> surLeMage = parmi<Degats>(mage.prendreEvenements(), Camp::Escouade);

    REQUIRE(surLeGuerrier.size() == 1);
    REQUIRE(degats(surLeGuerrier[0]).montant == appliquerArmure(puissanceDuMonstre, s_armureMeleeDeTest));
    REQUIRE(surLeGuerrier[0].idCible == 1);
    REQUIRE_FALSE(degats(surLeGuerrier[0]).opportunite);
    verifierPointProche(surLeGuerrier[0].position, *guerrier.getEscouade().centreDuPersonnage(1));
    REQUIRE(surLeMage.size() == 1);
    REQUIRE(degats(surLeMage[0]).montant == puissanceDuMonstre);  // armure 0
}

TEST_CASE("evenements : un skill de zone donne un evenement par monstre touche") {
    // Trois monstres hors de la détection, dans la zone de 60 px autour de (400, 340) ; l'explosion du
    // mage (activation de 0,8 s) part avant qu'ils aient pu atteindre quelqu'un.
    Partie partie = unePartie(niveauDegage({{400.0f, 340.0f}, {435.0f, 340.0f}, {400.0f, 375.0f}}), s_graineDeTest);
    partie.lancerSkillDeZone(s_adresseExplosion, {400.0f, 340.0f});

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 1.0f);

    std::vector<int> touches;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        if (degats(evenement).montant == s_degatsZoneDeTest) {
            touches.push_back(evenement.idCible);
        }
    }
    std::ranges::sort(touches);
    REQUIRE(touches == std::vector<int>{1, 2, 3});
}

TEST_CASE("evenements : un coup gratuit est marque, dans les deux sens") {
    // Le mage seul recule d'un monstre qui l'engage : coup gratuit du monstre sur le mage.
    // Hypothèse : les personnages sont plus rapides que les monstres (le mage sort de la zone).
    Partie mage(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()}, s_graineDeTest);
    const int puissanceDuMonstre = mage.getMonstres().front().getPuissanceAttaque();
    static_cast<void>(jusquAuPremierCoup(mage, 1));
    static_cast<void>(mage.prendreEvenements());
    mage.ordonnerDeplacement({210.0f, 20.0f});
    const std::vector<EvenementDeCombat> surLeMage =
        parmi<Degats>(evenementsPendant(mage, s_fenetreSansCoupNormal), Camp::Escouade);
    REQUIRE(surLeMage.size() == 1);
    REQUIRE(degats(surLeMage[0]).opportunite);
    REQUIRE(degats(surLeMage[0]).montant == puissanceDuMonstre);  // armure 0

    // Trois guerriers ; le premier frappe une fois (300 dégâts, recharge de 100 s). Dès que le monstre est
    // touché, le troisième le provoque : il quitte le premier, qui le frappe gratuitement (300 dégâts).
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0].attaqueDeBase = Skill(TypeSkill::Degats, s_porteeMeleeDeTest, 300, 100.0f, 100.0f, 0.0f, s_executionBaseDeTest);
    Partie guerriers(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest);
    std::vector<EvenementDeCombat> evenements;
    bool provoque = false;
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        guerriers.mettreAJour(s_dureeTic);
        const std::vector<EvenementDeCombat> duTic = guerriers.prendreEvenements();
        evenements.insert(evenements.end(), duTic.begin(), duTic.end());
        if (!provoque && !evenements.empty()) {
            guerriers.demanderSkill({3, IdSkill::Provocation});
            provoque = true;
        }
    }
    int normaux = 0;
    int gratuits = 0;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        if (degats(evenement).montant == 300) {
            ++(degats(evenement).opportunite ? gratuits : normaux);
        }
    }
    REQUIRE(normaux == 1);
    REQUIRE(gratuits == 1);
}

TEST_CASE("evenements : un soin donne un evenement Soin sur l'allie soigne, avec les points reellement rendus") {
    // Le monstre blesse le guerrier ; le prêtre le soigne (30) : les points rendus sont ceux qui manquaient.
    Partie partie(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest(), pretreDeTest()}, s_graineDeTest);
    const int vieDeDepart = *partie.getEscouade().pointsDeVieDuPersonnage(1);
    static_cast<void>(jusquAuPremierCoup(partie, 1));
    std::vector<EvenementDeCombat> evenements = partie.prendreEvenements();
    partie.demanderSkill({2, IdSkill::Soin});

    const std::vector<EvenementDeCombat> suite = evenementsPendant(partie, 0.8f);  // activation de 0,6 s
    evenements.insert(evenements.end(), suite.begin(), suite.end());

    const std::vector<EvenementDeCombat> soins = parmi<Soin>(evenements, Camp::Escouade);
    REQUIRE(soins.size() == 1);
    REQUIRE(soins[0].idCible == 1);
    int totalDegats = 0;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Escouade)) {
        totalDegats += degats(evenement).montant;
    }
    REQUIRE(soin(soins[0]).montant > 0);
    REQUIRE(soin(soins[0]).montant <= totalDegats);
    REQUIRE(*partie.getEscouade().pointsDeVieDuPersonnage(1) == vieDeDepart - totalDegats + soin(soins[0]).montant);
    verifierPointProche(soins[0].position, *partie.getEscouade().centreDuPersonnage(1));
}

TEST_CASE("evenements : un coup sur un monstre deja mort dans l'image ne produit rien") {
    // Les deux coups (2000 dégâts) partent au même tic : le premier tue, le second tombe sur un mort (0).
    Partie partie = deuxGuerriersQuiTuentLeMemeMonstre();

    const std::vector<EvenementDeCombat> evenements = parmi<Degats>(evenementsPendant(partie, 0.5f), Camp::Monstres);

    REQUIRE(partie.getMonstres().empty());
    REQUIRE(evenements.size() == 1);
    REQUIRE(degats(evenements[0]).montant == 2000);
}

TEST_CASE("evenements : le monstre tue garde son evenement et sa position") {
    Partie partie = deuxGuerriersQuiTuentLeMemeMonstre();

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 0.5f);

    REQUIRE(partie.getMonstres().empty());
    const std::vector<EvenementDeCombat> surLeMonstre = parmi<Degats>(evenements, Camp::Monstres);
    REQUIRE(surLeMonstre.size() == 1);
    REQUIRE(surLeMonstre[0].idCible == 1);
    verifierPointProche(surLeMonstre[0].position, {260.0f, 210.0f});
}

TEST_CASE("evenements : prendreEvenements vide la file") {
    Partie partie = deuxGuerriersQuiTuentLeMemeMonstre();
    avancer(partie, nombreDeTics(0.5f));

    REQUIRE_FALSE(partie.prendreEvenements().empty());
    REQUIRE(partie.prendreEvenements().empty());
}

TEST_CASE("evenements : une partie gagnee ou perdue n'en produit plus") {
    Partie gagnee = deuxGuerriersQuiTuentLeMemeMonstre();
    avancer(gagnee, nombreDeTics(0.5f));
    REQUIRE(gagnee.estGagnee());
    static_cast<void>(gagnee.prendreEvenements());
    avancer(gagnee, nombreDeTics(1.0f));
    REQUIRE(gagnee.prendreEvenements().empty());

    Partie perdue = unePartie(niveauDegage(groupeDeMonstres()), s_graineDeTest);
    for (int i = 0; i < 5400 && !perdue.estPerdue(); ++i) {
        perdue.mettreAJour(s_dureeTic);
    }
    REQUIRE(perdue.estPerdue());
    static_cast<void>(perdue.prendreEvenements());
    avancer(perdue, nombreDeTics(1.0f));
    REQUIRE(perdue.prendreEvenements().empty());
}

TEST_CASE("evenements : un skill lance donne un evenement SkillLance au-dessus du lanceur, a la fin de l'activation") {
    Partie partie(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest(), pretreDeTest()}, s_graineDeTest);
    partie.demanderSkill({1, IdSkill::Provocation});

    REQUIRE(parmi<SkillLance>(partie.prendreEvenements(), Camp::Escouade).empty());  // l'activation commence seulement

    const std::vector<EvenementDeCombat> lances = parmi<SkillLance>(evenementsPendant(partie, 0.5f), Camp::Escouade);
    REQUIRE(lances.size() == 1);
    REQUIRE(lances[0].idCible == 1);
    REQUIRE(std::get<SkillLance>(lances[0].detail).skill == IdSkill::Provocation);
    verifierPointProche(lances[0].position, *partie.getEscouade().centreDuPersonnage(1));
}

TEST_CASE("evenements : la provocation donne un evenement Provoque au-dessus de chaque monstre provoque") {
    Partie partie(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest(), pretreDeTest()}, s_graineDeTest);
    partie.demanderSkill({1, IdSkill::Provocation});

    const std::vector<EvenementDeCombat> provoques = parmi<Provoque>(evenementsPendant(partie, 0.5f), Camp::Monstres);
    REQUIRE(provoques.size() == 1);
    REQUIRE(provoques[0].idCible == partie.getMonstres()[0].getId());
    verifierPointProche(provoques[0].position, partie.getMonstres()[0].getCentre());
    REQUIRE(parmi<Provoque>(evenementsPendant(partie, 0.5f), Camp::Monstres).empty());  // rien sans relance
}
