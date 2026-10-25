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
#include "combat/EvenementDeCombat.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Skill.hpp"
#include <variant>

// Zones de contrôle et attaque d'opportunité (étape 77). Le monstre n'a pas d'armure dans ces niveaux
// (Niveau::armureMonstres vaut 0) : les dégâts lus sur lui sont ceux des attaques de base.

TEST_CASE("zone de controle : un mage au contact qui s'eloigne recoit un coup gratuit du monstre") {
    // Un mage seul (zone 0) à 40 px du monstre, qui l'engage (zone de 20 px bord à bord).
    // Hypothèse : les personnages sont plus rapides que les monstres (le mage sort de la zone).
    Partie partie = partieDeTest(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()});
    const Attaque attaqueDuMonstre = partie.getMonstres().front().getAttaque();
    const int apresLePremierCoup = jusquAuPremierCoup(partie, 1);
    static_cast<void>(partie.prendreEvenements());

    partie.ordonnerDeplacement({210.0f, 20.0f});
    int pointsDeVie = apresLePremierCoup;
    std::vector<EvenementDeCombat> evenements;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, 1);
        const std::vector<EvenementDeCombat> duTic = partie.prendreEvenements();
        evenements.insert(evenements.end(), duTic.begin(), duTic.end());
    }

    // Un seul coup gratuit (armure 0), critique ou non selon le tirage de la partie.
    REQUIRE(evenements.size() == 1);
    const Degats& coupGratuit = std::get<Degats>(evenements[0].detail);
    REQUIRE(coupGratuit.opportunite);
    REQUIRE(pointsDeVie == apresLePremierCoup - degatsAttendus(attaqueDuMonstre, defensesDuPersonnageDeTest(0), coupGratuit.critique));
}

TEST_CASE("zone de controle : un guerrier qui reste au contact ne recoit aucun coup gratuit") {
    Partie partie = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest()});
    const int apresLePremierCoup = jusquAuPremierCoup(partie, 1);

    int pointsDeVie = apresLePremierCoup;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, 1);
    }

    REQUIRE(pointsDeVie == apresLePremierCoup);
}

namespace {

// Une Puissance qui rend le coup du premier guerrier reconnaissable : son attaque de base (15 pour une arme du
// catalogue, 60 pour l'épée à deux mains) fait plus de trois fois plus, quand les autres guerriers (Puissance de
// base) ne font que l'attaque de leur épée.
constexpr int s_puissanceReconnaissable = 77;

// Trois guerriers (le premier au nord, à (235, 160)) et un monstre à (270, 160), au contact du premier seulement.
// Le premier porte cette arme, de Puissance `s_puissanceReconnaissable` : son coup est reconnaissable. Le troisième
// guerrier (à (235, 260)) provoque le monstre dès le départ : il quitte le premier pour aller vers lui (si la
// provocation attendait le premier coup, les autres guerriers se seraient déjà portés au contact du monstre et
// l'auraient retenu dans la zone du premier). Renvoie le nombre de coups **gratuits** (marqués `opportunite`) de la force du coup du premier sur le
// monstre, sur 1,2 s. Un monstre indestructible : aucun coup ne le tue.
int coupsGratuitsDuPremier(IdObjet arme) {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0] = avecPuissance(capableDePorter(avecArme(composition[0], arme), {arme}), s_puissanceReconnaissable);
    Partie partie = partieDeTest(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest, monstreIndestructible());
    const Attaque coupDuPremier = *partie.getEscouade().attaqueDeBase(1);
    const Defenses monstre{.armure = 0, .statistiques = monstreSansHasard().statistiques};

    int gratuits = 0;
    partie.demanderSkill({3, IdSkill::Provocation});
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            const Degats* coup = std::get_if<Degats>(&evenement.detail);
            if (coup == nullptr || evenement.campCible != Camp::Monstres) {
                continue;
            }
            if (coup->opportunite && coup->montant == degatsAttendus(coupDuPremier, monstre, coup->critique)) {
                ++gratuits;
            }
        }
    }
    return gratuits;
}

}  // namespace

TEST_CASE("zone de controle : un monstre qui quitte un guerrier pour un provocateur est frappe gratuitement") {
    // L'épée à deux mains frappe une fois en 1,2 s (recharge d'au moins 1,2 s) : le coup gratuit est le seul de sa
    // force marqué comme tel.
    REQUIRE(coupsGratuitsDuPremier(IdObjet::EpeeADeuxMains) == 1);
}

TEST_CASE("zone de controle : un personnage sans zone ne frappe jamais gratuitement") {
    // Même scénario, mais le premier porte un arc (projectile) : sa zone de contrôle est nulle, aucun coup gratuit.
    REQUIRE(coupsGratuitsDuPremier(IdObjet::Arc) == 0);
}

TEST_CASE("zone de controle : un monstre tue par un coup gratuit est retire et le verrou est leve") {
    // Le premier coup (60 % des points de vie du monstre, 90 % s'il est critique : il reste en vie) en laisse au
    // plus 40 % ; le coup gratuit, au moins aussi fort, le tue. La provocation est demandée dès le départ.
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0] = avecPuissance(avecArme(composition[0], IdObjet::EpeeADeuxMains), s_puissanceReconnaissable);
    Partie partie = partieDeTest(niveauDegage({{270.0f, 160.0f}}), std::move(composition));
    const int vieDuMonstre = partie.getMonstres().front().getPointsDeVie();
    REQUIRE(partie.getEscouade().attaqueDeBase(1)->montant >= 6 * vieDuMonstre / 10);
    REQUIRE(partie.getEscouade().attaqueDeBase(1)->montant * 3 / 2 < vieDuMonstre);
    REQUIRE(partie.cliquer({270.0f, 160.0f}) == ResultatClic::Monstre);

    // En 1,2 s les autres guerriers ne retirent que quelques points : seul le coup gratuit peut le tuer.
    partie.demanderSkill({3, IdSkill::Provocation});
    for (int i = 0; i < nombreDeTics(1.2f) && !partie.getMonstres().empty(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }

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
// et sa proie est le guerrier, plus proche. Le guerrier frappe de son épée (15) ; le mage tire des coups bien plus
// forts : on reconnaît ainsi qui touche le monstre.

namespace {

// Le mage est d'une Puissance énorme : son attaque de base (le bâton) fait quatre fois celle de l'épée du guerrier,
// donc reconnaissable. Le monstre est indestructible : trois secondes de coups ne le tuent pas.
constexpr int s_puissanceDuMage = 110;
const int s_degatsDuMageDansLaPartie = montantEffectif(s_degatsBaseDeTest, Coefficients::seul(Caracteristique::Puissance, 1.0f),
                                                       avecPuissance(mageDeTest(), s_puissanceDuMage).caracteristiques);

Partie unGuerrierEtUnMageAuContact() {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), avecPuissance(mageDeTest(), s_puissanceDuMage)};
    return partieDeTest(niveauDegage({{219.0f, 232.0f}}), std::move(composition), s_graineDeTest, monstreIndestructible());
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
