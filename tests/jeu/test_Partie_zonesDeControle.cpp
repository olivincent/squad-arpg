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
// (`monstreSansHasard` n'en a pas) : les dégâts lus sur lui sont ceux des attaques de base.

TEST_CASE("zone de controle : un mage au contact qui s'eloigne recoit un coup gratuit du monstre") {
    // Un mage seul (zone 0) à 40 px du monstre, qui l'engage (zone de 20 px bord à bord).
    // Hypothèse : les personnages sont plus rapides que les monstres (le mage sort de la zone).
    Partie partie = partieDeTest(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()});
    const Attaque attaqueDuMonstre = partie.getMonstres().front().getAttaque();
    const int apresLePremierCoup = jusquAuPremierCoup(partie, Place{1});
    static_cast<void>(partie.prendreEvenements());

    partie.ordonnerDeplacement({210.0f, 20.0f});
    int pointsDeVie = apresLePremierCoup;
    std::vector<EvenementDeCombat> evenements;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, Place{1});
        const std::vector<EvenementDeCombat> duTic = partie.prendreEvenements();
        evenements.insert(evenements.end(), duTic.begin(), duTic.end());
    }

    // Un seul coup gratuit (armure 0), critique ou non selon le tirage de la partie.
    REQUIRE(evenements.size() == 1);
    const DegatsSubis& coupGratuit = std::get<DegatsSubis>(evenements[0].detail);
    REQUIRE(coupGratuit.opportunite);
    REQUIRE(pointsDeVie == apresLePremierCoup - degatsAttendus(attaqueDuMonstre, defensesDuPersonnageDeTest(0), coupGratuit.critique));
}

TEST_CASE("zone de controle : un guerrier qui reste au contact ne recoit aucun coup gratuit") {
    Partie partie = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest()});
    const int apresLePremierCoup = jusquAuPremierCoup(partie, Place{1});

    int pointsDeVie = apresLePremierCoup;
    for (int i = 0; i < nombreDeTics(s_fenetreSansCoupNormal); ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, Place{1});
    }

    REQUIRE(pointsDeVie == apresLePremierCoup);
}

namespace {

// Une Puissance qui rend le coup du premier guerrier reconnaissable : son attaque de base (de 13 à 60 selon l'arme et le
// tirage) fait plus de trois fois plus, quand les autres guerriers (Puissance de base) ne font que l'attaque de leur épée.
constexpr int s_puissanceReconnaissable = 77;

// Trois guerriers (le premier au nord, à (235, 160)) et un monstre à (270, 160), au contact du premier seulement.
// Le premier porte cette arme, de Puissance `s_puissanceReconnaissable` : son coup est reconnaissable. Le troisième
// guerrier (à (235, 260)) provoque le monstre dès le départ : il quitte le premier pour aller vers lui (si la
// provocation attendait le premier coup, les autres guerriers se seraient déjà portés au contact du monstre et
// l'auraient retenu dans la zone du premier). Renvoie le nombre de coups **gratuits** (marqués `opportunite`) de la force du coup du premier sur le
// monstre, sur 1,2 s. Un monstre indestructible : aucun coup ne le tue.
// Le coup de base du premier guerrier à la borne basse et à la borne haute de la plage de son arme, avec sa Puissance : tout
// coup qu'il porte est entre les deux.
struct BornesDuCoup {
    Attaque basse;
    Attaque haute;
};

BornesDuCoup bornesDuCoupDuPremier(const Partie& partie, IdObjet arme) {
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    const Caracteristiques caracteristiques = partie.getEscouade().caracteristiquesDuPersonnage(Place{1}).value();
    const DescriptionDAttaque description = descriptionDeLArme(Objet{arme});
    Attaque basse = *attaqueDeBaseDe(partie.getEscouade(), Place{1});
    Attaque haute = basse;
    basse.degats = degatsPhysiques(montantEffectif(description.degats.min.total(), puissance, caracteristiques));
    haute.degats = degatsPhysiques(montantEffectif(description.degats.max.total(), puissance, caracteristiques));
    return {basse, haute};
}

int coupsGratuitsDuPremier(IdObjet arme) {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0] = avecPuissance(capableDePorter(avecArme(composition[0], arme), {arme}), s_puissanceReconnaissable);
    Partie partie = partieDeTest(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest, monstreIndestructible());
    const BornesDuCoup bornes = bornesDuCoupDuPremier(partie, arme);
    const Defenses monstre{.armure = 0, .statistiques = monstreSansHasard().statistiques};

    int gratuits = 0;
    partie.demanderSkill({Place{3}, IdSkill::Provocation});
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            const DegatsSubis* coup = std::get_if<DegatsSubis>(&evenement.detail);
            if (coup == nullptr || !std::holds_alternative<IdMonstre>(evenement.cible)) {
                continue;
            }
            if (coup->opportunite && coup->montant >= degatsAttendus(bornes.basse, monstre, coup->critique) &&
                coup->montant <= degatsAttendus(bornes.haute, monstre, coup->critique)) {
                ++gratuits;
            }
        }
    }
    return gratuits;
}

}  // namespace

TEST_CASE("zone de controle : un monstre qui quitte un guerrier pour un provocateur est frappe gratuitement") {
    // L'épée à deux mains frappe peu en 1,2 s (exécution de 0,3 s, recharge d'au moins 0,5 s) : le coup gratuit est le seul de
    // sa force marqué comme tel.
    REQUIRE(coupsGratuitsDuPremier(IdObjet::EpeeADeuxMains) == 1);
}

TEST_CASE("zone de controle : un personnage sans zone ne frappe jamais gratuitement") {
    // Même scénario, mais le premier porte un arc (projectile) : sa zone de contrôle est nulle, aucun coup gratuit.
    REQUIRE(coupsGratuitsDuPremier(IdObjet::Arc) == 0);
}

TEST_CASE("zone de controle : un monstre tue par un coup gratuit est retire et le verrou est leve") {
    // Les points de vie du monstre sont deux fois le plus petit coup du premier guerrier : le premier coup, même le plus fort de
    // sa plage, ne le tue pas (le plus fort est moins que le double du plus faible) ; le coup gratuit, au moins aussi fort que le
    // plus faible, le tue. Aucun critique : le monstre a une capacité défensive énorme, donc le rapport des capacités est nul.
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0] = avecPuissance(capableDePorter(avecArme(composition[0], IdObjet::EpeeADeuxMains), {IdObjet::EpeeADeuxMains}),
                                   s_puissanceReconnaissable);
    const BornesDuCoup bornes = bornesDuCoupDuPremier(partieDeTest(niveauDegage({{270.0f, 160.0f}}), composition), IdObjet::EpeeADeuxMains);
    REQUIRE(bornes.haute.degats.total() < 2 * bornes.basse.degats.total());
    DefinitionDeMonstre definition = monstreSansHasard();
    definition.statistiques.capaciteDefensive = 1'000'000;
    definition.pointsDeVieMax = 2 * bornes.basse.degats.total();
    Partie partie = partieDeTest(niveauDegage({{270.0f, 160.0f}}), std::move(composition), s_graineDeTest, definition);
    REQUIRE(partie.cliquer({270.0f, 160.0f}) == ResultatClic::Monstre);

    // En 1,2 s les autres guerriers ne retirent que quelques points : seul le coup gratuit peut le tuer.
    partie.demanderSkill({Place{3}, IdSkill::Provocation});
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
    for (std::size_t numero = 1; numero <= 3; ++numero) {
        const Place place{numero};
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
        for (std::size_t numero = 1; numero <= 3; ++numero) {
            const Place place{numero};
            REQUIRE(partie.getEscouade().pointsDeVieDuPersonnage(place) == vieDesPersonnages[place.indice()]);
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
// Le moins qu'un coup de base du mage lui fait perdre : le minimum de la plage de son bâton, avec sa Puissance.
const int s_degatsDuMageDansLaPartie = montantEffectif(descriptionDeLArme(Objet{IdObjet::Baton}).degats.min.total(),
                                                       Coefficients::seul(Caracteristique::Puissance, 1.0f),
                                                       avecPuissance(mageDeTest(), s_puissanceDuMage).caracteristiques);

Partie unGuerrierEtUnMageAuContact() {
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), avecPuissance(mageDeTest(), s_puissanceDuMage)};
    return partieDeTest(niveauDegage({{219.0f, 232.0f}}), std::move(composition), s_graineDeTest, monstreIndestructible());
}

}  // namespace

TEST_CASE("degagement : le mage quitte le monstre tenu par le guerrier sans coup gratuit, puis tire de loin") {
    Partie partie = unGuerrierEtUnMageAuContact();
    const std::optional<int> vieDuMage = partie.getEscouade().pointsDeVieDuPersonnage(Place{2});
    const Cercle monstre = partie.getMonstres().front().getCercle();
    REQUIRE(distanceBordABord(partie.getEscouade().cerclesDesVivants()[0], monstre) <= 20.0f);
    REQUIRE(distanceBordABord(partie.getEscouade().cerclesDesVivants()[1], monstre) <= 20.0f);

    int pointsDuMonstreALArrivee = 0;
    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        partie.mettreAJour(s_dureeTic);
        const std::vector<Cercle> cercles = partie.getEscouade().cerclesDesVivants();
        REQUIRE(partie.getEscouade().pointsDeVieDuPersonnage(Place{2}) == vieDuMage);  // aucun coup, gratuit ou non
        REQUIRE(distanceBordABord(cercles[0], monstre) <= 20.0f);          // le guerrier reste au contact
        if (pointsDuMonstreALArrivee == 0 && !partie.getEscouade().degagementDuPersonnage(Place{2}).has_value() &&
            distanceBordABord(cercles[1], monstre) > 20.0f) {
            pointsDuMonstreALArrivee = partie.getMonstres().front().getPointsDeVie();
        }
    }

    const std::vector<Cercle> cercles = partie.getEscouade().cerclesDesVivants();
    REQUIRE_FALSE(partie.getEscouade().degagementDuPersonnage(Place{2}).has_value());
    REQUIRE(distanceBordABord(cercles[1], monstre) > 20.0f);
    REQUIRE(distanceBordABord(cercles[1], monstre) <= 120.0f);  // à portée de son attaque
    REQUIRE(pointsDuMonstreALArrivee > 0);
    REQUIRE(partie.getMonstres().front().getPointsDeVie() <= pointsDuMonstreALArrivee - s_degatsDuMageDansLaPartie);
}
