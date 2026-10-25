// ./tests/jeu/test_Partie_evenements.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Blocage.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Defense.hpp"
#include "combat/Esquive.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/TypeSkill.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Partie.hpp"
#include "combat/Skill.hpp"

// Événements de combat (étape 79). Les monstres de `niveauDegage` ont une armure de 0 (le niveau n'en
// donne pas), sauf mention.

namespace {

// Une Puissance qui multiplie l'attaque de base par plus de 30 : elle tue en un coup un monstre ordinaire.
constexpr int s_puissanceMortelle = 1000;
// Celle qui fait de l'épée à deux mains (60) un coup d'environ 60 % des points de vie du monstre ordinaire.
constexpr int s_puissanceDeLEpeeADeuxMains = 77;

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

// Ce que le monstre de test oppose à un coup : pas d'esquive, l'armure donnée (`monstreAvecArmure`).
Defenses defensesDuMonstreDeTest(int armure = 0) {
    return {.armure = armure, .statistiques = monstreSansHasard().statistiques};
}

// Un guerrier increvable : il frappe sans cesse un monstre indestructible (`monstreIndestructible`), et reçoit ses
// coups sans mourir.
DefinitionPersonnage guerrierInfatigable() {
    return avecEnduranceEnorme(guerrierDeTest());
}

// Deux guerriers (en (235, 185) et (235, 235)) d'une Puissance telle que leur attaque de base tue d'un coup, et un
// monstre en (260, 210) qu'ils atteignent tous les deux : leurs deux coups partent au même tic (et le monstre
// est mort avant que la recharge de l'épée ne leur en laisse un autre).
Partie deuxGuerriersQuiTuentLeMemeMonstre() {
    std::vector<DefinitionPersonnage> composition = {avecPuissance(guerrierDeTest(), s_puissanceMortelle),
                                                     avecPuissance(guerrierDeTest(), s_puissanceMortelle)};
    return partieDeTest(niveauDegage({{260.0f, 210.0f}}), std::move(composition));
}

}  // namespace

TEST_CASE("evenements : un skill de degats donne un evenement sur le monstre, armure comprise") {
    constexpr int armure = 20;
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest()},
                                 s_graineDeTest, monstreAvecArmure(armure));
    partie.demanderSkill(s_adresseFrappe);
    const Defenses monstre = defensesDuMonstreDeTest(armure);
    const Attaque frappe{.degats = s_degatsFrappeDeTest,
                         .capaciteOffensive = partie.getEscouade().statistiquesDeCombatDuPersonnage(1)->capaciteOffensive,
                         .nature = NatureDAttaque::Contact};
    const Attaque base = *partie.getEscouade().attaqueDeBase(1);

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 0.5f);

    // Le critique est au tirage de la partie : chaque événement dit s'il l'était. La frappe, même sans critique,
    // frappe plus fort que l'attaque de base, même critique : on les distingue toujours.
    REQUIRE(degatsAttendus(base, monstre, true) < degatsAttendus(frappe, monstre, false));
    int frappes = 0;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        if (degats(evenement).montant == degatsAttendus(frappe, monstre, degats(evenement).critique)) {
            ++frappes;
            REQUIRE(evenement.idCible == 1);
            REQUIRE_FALSE(degats(evenement).opportunite);
            verifierPointProche(evenement.position, {235.0f, 170.0f});
        }
    }
    REQUIRE(frappes == 1);
}

TEST_CASE("evenements : le coup d'un monstre donne un evenement sur le personnage touche, armure comprise") {
    Partie guerrier = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest()});
    Partie mage = partieDeTest(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()});
    const Attaque attaqueDuMonstre = guerrier.getMonstres().front().getAttaque();
    static_cast<void>(jusquAuPremierCoup(guerrier, 1));
    static_cast<void>(jusquAuPremierCoup(mage, 1));

    const std::vector<EvenementDeCombat> surLeGuerrier = parmi<Degats>(guerrier.prendreEvenements(), Camp::Escouade);
    const std::vector<EvenementDeCombat> surLeMage = parmi<Degats>(mage.prendreEvenements(), Camp::Escouade);

    REQUIRE(surLeGuerrier.size() == 1);
    REQUIRE(degats(surLeGuerrier[0]).montant ==
            degatsAttendus(attaqueDuMonstre, defensesDuPersonnageDeTest(s_armureMeleeDeTest), degats(surLeGuerrier[0]).critique));
    REQUIRE(surLeGuerrier[0].idCible == 1);
    REQUIRE_FALSE(degats(surLeGuerrier[0]).opportunite);
    verifierPointProche(surLeGuerrier[0].position, *guerrier.getEscouade().centreDuPersonnage(1));
    REQUIRE(surLeMage.size() == 1);
    REQUIRE(degats(surLeMage[0]).montant ==
            degatsAttendus(attaqueDuMonstre, defensesDuPersonnageDeTest(0), degats(surLeMage[0]).critique));  // armure 0
}

TEST_CASE("evenements : un critique est signale, et son montant est multiplie") {
    // Un guerrier d'Adresse énorme (capacité offensive 5 050 contre 100 : 50 % de critiques, ×2) et d'Endurance
    // énorme frappe sans cesse un monstre indestructible : une centaine de coups en 60 s, dont la chance de n'avoir
    // que des critiques ou que des coups normaux est de 2^-100 (la graine n'y change rien).
    DefinitionPersonnage adroit = guerrierInfatigable();
    adroit.caracteristiques = Caracteristiques({10, 2'000'000, 1000, 10, 10});
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), std::vector<DefinitionPersonnage>{adroit}, s_graineDeTest,
                                 monstreIndestructible());
    const Attaque attaque = *partie.getEscouade().attaqueDeBase(1);
    const Defenses monstre = defensesDuMonstreDeTest();

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 60.0f);

    int critiques = 0;
    int normaux = 0;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        REQUIRE(degats(evenement).montant == degatsAttendus(attaque, monstre, degats(evenement).critique));
        ++(degats(evenement).critique ? critiques : normaux);
    }
    REQUIRE(critiques > 0);
    REQUIRE(normaux > 0);
}

TEST_CASE("evenements : un skill de zone donne un evenement par monstre touche") {
    // Trois monstres hors de la détection, dans la zone de 60 px autour de (400, 340) ; l'explosion du
    // mage (activation de 0,8 s) part avant qu'ils aient pu atteindre quelqu'un.
    Partie partie = unePartie(niveauDegage({{400.0f, 340.0f}, {435.0f, 340.0f}, {400.0f, 375.0f}}), s_graineDeTest);
    partie.lancerSkillDeZone(s_adresseExplosion, {400.0f, 340.0f});
    const Attaque explosion{.degats = s_degatsZoneDeTest,
                            .capaciteOffensive = partie.getEscouade().statistiquesDeCombatDuPersonnage(3)->capaciteOffensive,
                            .nature = NatureDAttaque::Zone};

    const std::vector<EvenementDeCombat> evenements = evenementsPendant(partie, 1.0f);

    // Seule l'explosion touche les monstres (ils sont loin) ; son critique est au tirage, l'événement le dit.
    std::vector<int> touches;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        REQUIRE(degats(evenement).montant == degatsAttendus(explosion, defensesDuMonstreDeTest(), degats(evenement).critique));
        touches.push_back(evenement.idCible);
    }
    std::ranges::sort(touches);
    REQUIRE(touches == std::vector<int>{1, 2, 3});
}

TEST_CASE("evenements : un coup gratuit est marque, dans les deux sens") {
    // Le mage seul recule d'un monstre qui l'engage : coup gratuit du monstre sur le mage.
    // Hypothèse : les personnages sont plus rapides que les monstres (le mage sort de la zone).
    Partie mage = partieDeTest(niveauDegage({{185.0f, 250.0f}}), std::vector<DefinitionPersonnage>{mageDeTest()});
    const Attaque attaqueDuMonstre = mage.getMonstres().front().getAttaque();
    static_cast<void>(jusquAuPremierCoup(mage, 1));
    static_cast<void>(mage.prendreEvenements());
    mage.ordonnerDeplacement({210.0f, 20.0f});
    const std::vector<EvenementDeCombat> surLeMage =
        parmi<Degats>(evenementsPendant(mage, s_fenetreSansCoupNormal), Camp::Escouade);
    REQUIRE(surLeMage.size() == 1);
    REQUIRE(degats(surLeMage[0]).opportunite);
    REQUIRE(degats(surLeMage[0]).montant ==
            degatsAttendus(attaqueDuMonstre, defensesDuPersonnageDeTest(0), degats(surLeMage[0]).critique));  // armure 0

    // Trois guerriers ; le premier frappe une fois, fort (l'épée à deux mains, de Puissance telle qu'elle prend 60 %
    // des points de vie du monstre, 90 % si critique, et de recharge d'au moins 1,2 s : une seule frappe en 1,2 s).
    // Le troisième le provoque dès le départ : il quitte le premier, qui le frappe gratuitement (le même coup).
    std::vector<DefinitionPersonnage> composition = {guerrierDeTest(), guerrierDeTest(), guerrierDeTest()};
    composition[0] = avecPuissance(avecArme(composition[0], IdObjet::EpeeADeuxMains), s_puissanceDeLEpeeADeuxMains);
    Partie guerriers = partieDeTest(niveauDegage({{270.0f, 160.0f}}), std::move(composition));
    const Attaque coupDuPremier = *guerriers.getEscouade().attaqueDeBase(1);
    std::vector<EvenementDeCombat> evenements;
    guerriers.demanderSkill({3, IdSkill::Provocation});
    for (int i = 0; i < nombreDeTics(1.2f); ++i) {
        guerriers.mettreAJour(s_dureeTic);
        const std::vector<EvenementDeCombat> duTic = guerriers.prendreEvenements();
        evenements.insert(evenements.end(), duTic.begin(), duTic.end());
    }
    int normaux = 0;
    int gratuits = 0;
    for (const EvenementDeCombat& evenement : parmi<Degats>(evenements, Camp::Monstres)) {
        if (degats(evenement).montant == degatsAttendus(coupDuPremier, defensesDuMonstreDeTest(), degats(evenement).critique)) {
            ++(degats(evenement).opportunite ? gratuits : normaux);
        }
    }
    REQUIRE(normaux == 1);
    REQUIRE(gratuits == 1);
}

TEST_CASE("evenements : un soin donne un evenement Soin sur l'allie soigne, avec les points reellement rendus") {
    // Le monstre blesse le guerrier ; le prêtre le soigne (30) : les points rendus sont ceux qui manquaient.
    Partie partie = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest(), pretreDeTest()});
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
    const Attaque coup = *partie.getEscouade().attaqueDeBase(1);

    const std::vector<EvenementDeCombat> evenements = parmi<Degats>(evenementsPendant(partie, 0.5f), Camp::Monstres);

    REQUIRE(partie.getMonstres().empty());
    REQUIRE(evenements.size() == 1);
    REQUIRE(degats(evenements[0]).montant == degatsAttendus(coup, defensesDuMonstreDeTest(), degats(evenements[0]).critique));
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
    Partie partie = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest(), pretreDeTest()});
    partie.demanderSkill({1, IdSkill::Provocation});

    REQUIRE(parmi<SkillLance>(partie.prendreEvenements(), Camp::Escouade).empty());  // l'activation commence seulement

    const std::vector<EvenementDeCombat> lances = parmi<SkillLance>(evenementsPendant(partie, 0.5f), Camp::Escouade);
    REQUIRE(lances.size() == 1);
    REQUIRE(lances[0].idCible == 1);
    REQUIRE(std::get<SkillLance>(lances[0].detail).skill == IdSkill::Provocation);
    verifierPointProche(lances[0].position, *partie.getEscouade().centreDuPersonnage(1));
}

TEST_CASE("evenements : la provocation donne un evenement Provoque au-dessus de chaque monstre provoque") {
    Partie partie = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrierDeTest(), pretreDeTest()});
    partie.demanderSkill({1, IdSkill::Provocation});

    const std::vector<EvenementDeCombat> provoques = parmi<Provoque>(evenementsPendant(partie, 0.5f), Camp::Monstres);
    REQUIRE(provoques.size() == 1);
    REQUIRE(provoques[0].idCible == partie.getMonstres()[0].getId());
    verifierPointProche(provoques[0].position, partie.getMonstres()[0].getCentre());
    REQUIRE(parmi<Provoque>(evenementsPendant(partie, 0.5f), Camp::Monstres).empty());  // rien sans relance
}

namespace {

// Les événements de chaque image, séparément : l'ordre et la simultanéité comptent.
std::vector<std::vector<EvenementDeCombat>> evenementsParImage(Partie& partie, float duree) {
    std::vector<std::vector<EvenementDeCombat>> images;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        partie.mettreAJour(s_dureeTic);
        images.push_back(partie.prendreEvenements());
    }
    return images;
}

}  // namespace

TEST_CASE("evenements : un coup esquive par un monstre donne Esquive, et aucun degat dans la meme image") {
    // Un seul attaquant (un guerrier qui frappe 6 fois par seconde) : une image ne contient qu'un coup sur le monstre.
    // Le monstre esquive la moitié des coups (le plafond) : sur 40 s, la chance de ne voir aucune esquive ou
    // aucun coup touché est de l'ordre de 2^-200, quelle que soit la graine.
    DefinitionDeMonstre esquiveur = monstreIndestructible();
    esquiveur.statistiques.esquiveContact = s_esquiveMaximale;
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), std::vector<DefinitionPersonnage>{guerrierInfatigable()},
                                 s_graineDeTest, esquiveur);
    int esquives = 0;
    int touches = 0;

    for (const std::vector<EvenementDeCombat>& image : evenementsParImage(partie, 40.0f)) {
        const std::vector<EvenementDeCombat> esquivesDeLImage = parmi<Esquive>(image, Camp::Monstres);
        const std::vector<EvenementDeCombat> degatsDeLImage = parmi<Degats>(image, Camp::Monstres);
        esquives += static_cast<int>(esquivesDeLImage.size());
        touches += static_cast<int>(degatsDeLImage.size());
        if (!esquivesDeLImage.empty()) {
            REQUIRE(degatsDeLImage.empty());
            verifierPointProche(esquivesDeLImage[0].position, partie.getMonstres()[0].getCentre());
        }
    }

    REQUIRE(esquives > 0);
    REQUIRE(touches > 0);
}

TEST_CASE("evenements : un blocage partiel donne les degats puis Bloque, puis rien pendant la recuperation") {
    DefinitionPersonnage guerrier = guerrierInfatigable();
    guerrier.equipementDeDepart.push_back({Emplacement::MainGauche, IdObjet::BouclierRond});  // 30 %, 40 points, 3 s
    const DefinitionDeMonstre monstre = monstreIndestructible();
    Partie partie = partieDeTest(niveauDegage({{270.0f, 210.0f}}), std::vector<DefinitionPersonnage>{guerrier}, s_graineDeTest, monstre);
    const StatistiquesDeCombat bouclier = *partie.getEscouade().statistiquesDeCombatDuPersonnage(1);
    const std::vector<std::vector<EvenementDeCombat>> images = evenementsParImage(partie, 120.0f);
    std::vector<int> imagesDesBlocages;

    for (std::size_t i = 0; i < images.size(); ++i) {
        std::vector<EvenementDeCombat> surLeGuerrier;
        for (const EvenementDeCombat& evenement : images[i]) {
            if (evenement.campCible == Camp::Escouade) {
                surLeGuerrier.push_back(evenement);
            }
        }
        for (std::size_t k = 0; k < surLeGuerrier.size(); ++k) {
            if (const Bloque* bloque = std::get_if<Bloque>(&surLeGuerrier[k].detail)) {
                // Le monstre frappe plus fort que ce que le bouclier absorbe : il ne l'absorbe jamais en entier.
                REQUIRE(bouclier.montantBloque < monstre.attaque.puissance);
                REQUIRE(bloque->montant == bouclier.montantBloque);
                REQUIRE_FALSE(bloque->total);
                REQUIRE(k > 0);  // les dégâts du reste viennent juste avant, donc le texte du blocage est au-dessus
                REQUIRE(std::holds_alternative<Degats>(surLeGuerrier[k - 1].detail));
                imagesDesBlocages.push_back(static_cast<int>(i));
            }
        }
    }

    REQUIRE(imagesDesBlocages.size() >= 3);
    for (std::size_t k = 1; k < imagesDesBlocages.size(); ++k) {
        REQUIRE(imagesDesBlocages[k] - imagesDesBlocages[k - 1] >= nombreDeTics(s_recuperationDuBlocage) - 1);  // la récupération (commune) au moins
    }
}
