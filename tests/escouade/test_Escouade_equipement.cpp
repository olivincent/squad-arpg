// ./tests/escouade/test_Escouade_equipement.cpp
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/DescriptionDAttaque.hpp"
#include "combat/Defense.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "escouade/Escouade.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Equipement.hpp"
#include "objets/Inventaire.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// Équipement et sac de l'escouade (étape 83). Places de uneEscouade : 1 guerrier (équipé), 2 prêtre
// et 3 mage (sans rien). Les attentes d'armure viennent du catalogue d'objets.
namespace {
constexpr std::size_t s_guerrier = 1;
constexpr std::size_t s_pretre = 2;
constexpr std::size_t s_mage = 3;
constexpr int s_coup = 10;  // petit : trois coups ne tuent personne, quelle que soit l'armure

// Les dégâts qu'un coup de `s_coup` fait à ce personnage, armure comprise.
int degatsSubis(Escouade& escouade, std::size_t place) {
    return subirCoup(escouade, 1, place, s_coup);
}

// L'escouade du jeu, sac de départ rangé dans les premières cases.
Escouade escouadeAvecSac(const std::vector<Objet>& objets) {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});
    for (const Objet& objet : objets) {
        REQUIRE(escouade.ramasser(objet));
    }
    return escouade;
}

PositionDObjet dansLeSac(std::size_t caseDuSac) {
    return PositionDansLeSac{caseDuSac};
}

PositionDObjet portee(std::size_t place, Emplacement emplacement) {
    return PositionPortee{place, emplacement};
}

const Objet s_gants{IdObjet::GantsDeTissu};
const Objet s_casqueDeCuir{IdObjet::Capuchon};
const Objet s_casqueDeFer{IdObjet::HeaumeIntegral};

// Un lot de quatre objets différents, tous à affixes : aucun n'est porté au départ, donc chacun a zéro exemplaire dans une
// escouade neuve (les objets sont comptés par valeur complète).
const std::vector<Objet> s_lotDeQuatre = {
    {IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}},
    {IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}},
    {IdObjet::HeaumeIntegral, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeDeviation, 3}},
    {IdObjet::BouclierRond, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeBlocage, 5}}};
}  // namespace

TEST_CASE("le sac d'une escouade neuve est vide") {
    const Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});

    REQUIRE(escouade.sac().capacite() > 0);
    for (std::size_t i = 0; i < escouade.sac().capacite(); ++i) {
        REQUIRE_FALSE(escouade.sac().a(i).has_value());
    }
}

TEST_CASE("equipementDuPersonnage : le guerrier est equipe, le mage non") {
    const Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == s_armureMeleeDeTest);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == 0);
}

TEST_CASE("ramasser : premiere case libre, puis faux quand le sac est plein") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});

    REQUIRE(escouade.ramasser(s_gants));
    REQUIRE(escouade.ramasser(s_casqueDeCuir));
    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(escouade.sac().a(1) == s_casqueDeCuir);

    for (std::size_t i = 2; i < escouade.sac().capacite(); ++i) {
        REQUIRE(escouade.ramasser(s_gants));
    }
    const Exemplaires avant = exemplaires(escouade);
    REQUIRE_FALSE(escouade.ramasser(s_casqueDeCuir));
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer du sac vers un emplacement vide : l'objet quitte le sac, l'emplacement vide le recoit, l'armure suit") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Gants)));

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == s_gants);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_mage) == appliquerArmure(s_coup, definitionObjet(IdObjet::GantsDeTissu).armure));
}

TEST_CASE("deplacer du sac vers un emplacement occupe echange avec la case du sac") {
    Escouade escouade = escouadeAvecSac({s_casqueDeCuir});
    const Exemplaires avant = exemplaires(escouade);
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::Tete)));

    REQUIRE(escouade.sac().a(0) == s_casqueDeFer);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeCuir);
    REQUIRE(exemplaires(escouade) == avant);
    const int armureApres = armureAvant - definitionObjet(IdObjet::HeaumeIntegral).armure +
                            definitionObjet(IdObjet::Capuchon).armure;
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armureApres);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureApres));
}

TEST_CASE("deplacer du sac vers un emplacement : case vide ou emplacement refuse, rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(5), portee(s_mage, Emplacement::Gants)));       // case vide
    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Tete)));        // des gants ne vont pas sur la tete
    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::MainDroite)));  // ni dans la main droite

    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == 0);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_mage) == s_coup);
}

TEST_CASE("deplacer : un personnage mort peut etre equipe") {
    Escouade escouade = escouadeAvecSac({s_gants});
    REQUIRE(subirCoup(escouade, 1, s_mage, s_coupMortel) > 0);
    REQUIRE_FALSE(escouade.placeEstVivante(s_mage));

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Gants)));

    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == s_gants);
}

TEST_CASE("deplacer d'un emplacement vers une case vide : l'objet va dans le sac, l'armure baisse") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});
    const Exemplaires avant = exemplaires(escouade);
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), dansLeSac(7)));

    REQUIRE(escouade.sac().a(7) == s_casqueDeFer);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete).has_value());
    REQUIRE(exemplaires(escouade) == avant);
    const int armureApres = armureAvant - definitionObjet(IdObjet::HeaumeIntegral).armure;
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureApres));
}

TEST_CASE("deplacer d'un emplacement vers une case occupee par un objet compatible echange") {
    Escouade escouade = escouadeAvecSac({s_casqueDeCuir});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), dansLeSac(0)));

    REQUIRE(escouade.sac().a(0) == s_casqueDeFer);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeCuir);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer d'un emplacement vers une case occupee par un objet incompatible : rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), dansLeSac(0)));

    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeFer);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, s_armureMeleeDeTest));
}

TEST_CASE("deplacer d'un emplacement vide : faux, rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_mage, Emplacement::Gants), dansLeSac(3)));

    REQUIRE_FALSE(escouade.sac().a(3).has_value());
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("tout desequiper ramene l'armure a zero, tout reequiper la rend") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});
    const std::vector<Emplacement> portes = {Emplacement::Tete, Emplacement::Torse, Emplacement::Jambes};
    for (std::size_t i = 0; i < portes.size(); ++i) {
        REQUIRE(escouade.deplacerObjet(portee(s_guerrier, portes[i]), dansLeSac(i)));
    }
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == 0);
    REQUIRE(degatsSubis(escouade, s_guerrier) == s_coup);

    for (std::size_t i = 0; i < portes.size(); ++i) {
        REQUIRE(escouade.deplacerObjet(dansLeSac(i), portee(s_guerrier, portes[i])));
    }
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == s_armureMeleeDeTest);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, s_armureMeleeDeTest));
}

TEST_CASE("l'armure suit un objet d'un personnage a l'autre") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});
    const Exemplaires avant = exemplaires(escouade);
    const int armureJambieres = definitionObjet(IdObjet::JambieresDePlaques).armure;
    const int armureGuerrier = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Jambes), portee(s_mage, Emplacement::Jambes)));

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDePlaques});
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == armureJambieres);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_mage) == appliquerArmure(s_coup, armureJambieres));
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureGuerrier - armureJambieres));
}

TEST_CASE("echange entre deux personnages : les deux armures changent") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});
    REQUIRE(escouade.ramasser(s_casqueDeCuir));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Tete)));
    const Exemplaires avant = exemplaires(escouade);
    const int armureGuerrier = escouade.equipementDuPersonnage(s_guerrier).armureTotale();
    const int armureMage = escouade.equipementDuPersonnage(s_mage).armureTotale();
    const int ecart = definitionObjet(IdObjet::HeaumeIntegral).armure - definitionObjet(IdObjet::Capuchon).armure;

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), portee(s_mage, Emplacement::Tete)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeCuir);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Tete) == s_casqueDeFer);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armureGuerrier - ecart);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == armureMage + ecart);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer d'une case du sac a une autre : case vide, case occupee, meme case") {
    Escouade escouade = escouadeAvecSac({s_gants, s_casqueDeCuir});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), dansLeSac(9)));
    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.sac().a(9) == s_gants);

    REQUIRE(escouade.deplacerObjet(dansLeSac(9), dansLeSac(1)));
    REQUIRE(escouade.sac().a(9) == s_casqueDeCuir);
    REQUIRE(escouade.sac().a(1) == s_gants);

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), dansLeSac(1)));
    REQUIRE(escouade.sac().a(1) == s_gants);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer : source vide ou destination refusee, rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(5), dansLeSac(6)));                              // source vide
    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_mage, Emplacement::Gants), dansLeSac(0)));        // emplacement vide
    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Torse), dansLeSac(0)));    // gants ne vont pas au torse
    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::Anneau1)));  // ni a un anneau

    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("peutDeplacerObjet : repond comme deplacerObjet, sans rien changer") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Gants)));
    REQUIRE_FALSE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Tete)));
    REQUIRE_FALSE(escouade.peutDeplacerObjet(portee(s_guerrier, Emplacement::Torse), dansLeSac(0)));
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(escouade.sac().a(0) == s_gants);
}

TEST_CASE("une partie commence avec un sac vide") {
    const Partie partie = unePartie(niveauDegage({}), s_graineDeTest);
    const Inventaire& sac = partie.getEscouade().sac();

    for (std::size_t i = 0; i < sac.capacite(); ++i) {
        REQUIRE_FALSE(sac.a(i).has_value());
    }
}

TEST_CASE("statistiquesDeCombatDuPersonnage : celles du personnage de la place, vides si la place est invalide") {
    DefinitionPersonnage rodeur = pretreDeTest();
    rodeur.caracteristiques = Caracteristiques({10, 10, 14, 8, 8});
    const Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrierDeTest(), rodeur});

    // Le guerrier de test porte des pièces lourdes : leur malus d'esquive est dans ses statistiques.
    REQUIRE(escouade.statistiquesDeCombatDuPersonnage(1) ==
            statistiquesDuPersonnage(Caracteristiques(), Equipement(s_equipementMeleeDeTest).bonusDeCombatTotal()));
    REQUIRE(escouade.statistiquesDeCombatDuPersonnage(2) == StatistiquesDeCombat{120, 120});
    REQUIRE_FALSE(escouade.statistiquesDeCombatDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.statistiquesDeCombatDuPersonnage(3).has_value());
}

TEST_CASE("la puissance de l'attaque de base lue par l'escouade (coup d'opportunite) suit les caracteristiques") {
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    DefinitionPersonnage definition = guerrierDeTest();
    definition.caracteristiques = Caracteristiques({s_valeurDeReference + 1, 10, 10, 10, 10});  // 11 de Puissance : l'épée est active (bonus de 3 % arrondi à 0)
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {definition});
    REQUIRE(dansLaPlageAvecPuissance(attaqueDeBaseDe(escouade, 1)->degats.total(), s_attaqueDeMeleeDeTest,
                                     escouade.caracteristiquesDuPersonnage(1).value()));

    REQUIRE(escouade.ramasser(Objet{IdObjet::AnneauDePuissance}));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(1, Emplacement::Anneau1)));

    // Le coup tiré dans la plage de l'épée, la Puissance de l'anneau sur chaque borne.
    const Caracteristiques avecAnneau = escouade.caracteristiquesDuPersonnage(1).value();
    const int degats = attaqueDeBaseDe(escouade, 1)->degats.total();
    REQUIRE(degats >= montantEffectif(s_degatsBaseMinDeTest, puissance, avecAnneau));
    REQUIRE(degats <= montantEffectif(s_degatsBaseMaxDeTest, puissance, avecAnneau));
    REQUIRE(montantEffectif(s_degatsBaseMaxDeTest, puissance, avecAnneau) > s_degatsBaseMaxDeTest);  // la Puissance joue vraiment
}

// Les deux mains (étape 91.3) : la règle est celle de `deplacerLesObjets` (`test_DeplacementDObjets.cpp`) ; ici,
// ce que l'escouade en fait : l'armure recalculée, la surbrillance qui suit l'action, rien ne bouge au refus.
namespace {
const Objet s_bouclier{IdObjet::BouclierRond};
const Objet s_epeeADeuxMains{IdObjet::EpeeADeuxMains};
const Objet s_epee{IdObjet::EpeeAUneMain};

// Le guerrier porte le bouclier rond ; le sac contient l'épée à deux mains en case 0.
Escouade escouadeDuGuerrierAuBouclier() {
    Escouade escouade = escouadeAvecSac({s_bouclier, s_epeeADeuxMains});
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainGauche)));
    return escouade;
}
}  // namespace

TEST_CASE("deux mains : l'epee a deux mains range le bouclier du guerrier dans le sac, et son armure baisse") {
    Escouade escouade = escouadeDuGuerrierAuBouclier();
    const Exemplaires avant = exemplaires(escouade);
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();
    REQUIRE(escouade.peutDeplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche).has_value());
    REQUIRE(escouade.sac().a(0) == s_bouclier);   // la première case libre
    REQUIRE(escouade.sac().a(1) == s_epee);       // l'épée à une main du guerrier a pris la place de l'arme (échange)
    REQUIRE(exemplaires(escouade) == avant);
    const int armureApres = armureAvant - definitionObjet(IdObjet::BouclierRond).armure;
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armureApres);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureApres));
}

TEST_CASE("deux mains : un bouclier range l'arme a deux mains que le guerrier porte") {
    Escouade escouade = escouadeAvecSac({s_epeeADeuxMains, s_bouclier});
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainGauche)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche) == s_bouclier);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());
    REQUIRE(escouade.sac().a(0) == s_epee);              // l'épée à une main rendue par l'échange de l'arme à deux mains
    REQUIRE(escouade.sac().a(1) == s_epeeADeuxMains);    // la première case libre : celle que le bouclier a quittée
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deux mains : une arme a une main laisse le bouclier en place") {
    Escouade escouade = escouadeAvecSac({s_bouclier, s_epee});
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainGauche)));

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite) == s_epee);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche) == s_bouclier);
}

TEST_CASE("deux mains : sac plein, l'epee a deux mains est refusee (ni surbrillance ni deplacement), rien ne bouge") {
    Escouade escouade = escouadeDuGuerrierAuBouclier();
    // Depuis le sac, la case de l'épée se libère et recevrait le bouclier : le déplacement serait permis. On met donc
    // l'épée dans la main du mage, puis on remplit le sac.
    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_mage, Emplacement::MainDroite)));
    while (escouade.ramasser(s_gants)) {
    }
    const Exemplaires avant = exemplaires(escouade);
    const int armure = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    // L'épée du mage va chez le guerrier (qui a son bouclier) : le sac est plein, aucune case ne se libère.
    REQUIRE_FALSE(escouade.peutDeplacerObjet(portee(s_mage, Emplacement::MainDroite), portee(s_guerrier, Emplacement::MainDroite)));
    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_mage, Emplacement::MainDroite), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche) == s_bouclier);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armure);
}

TEST_CASE("changer d'arme dans l'inventaire relance la recharge : le premier coup attend l'intervalle de la nouvelle arme") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    // Seul : tous les coups sont les siens ; capable de porter la dague.
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {capableDePorter(guerrierDeTest(), {IdObjet::Dague})});
    REQUIRE(escouade.ramasser(Objet{IdObjet::Dague}));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    const DescriptionDAttaque dague = descriptionDeLArme(Objet{IdObjet::Dague});
    const std::vector<EntiteVue> monstres = {EntiteVue{1, cibleMonstre({110.0f, 60.0f})}};  // au contact du guerrier, en (85, 60)
    const auto coupsPendant = [&](float duree) {
        std::size_t coups = 0;
        for (int i = 0; i < nombreDeTics(duree); ++i) {
            coups += escouade.attaquer(s_dureeTic, monstres, carte, generateur).size();
            escouade.mettreAJour(s_dureeTic, carte, {.physiques = monstres, .ciblables = monstres, .deControle = {}});
        }
        return coups;
    };

    // Une arme neuve n'est pas prête : ni la recharge minimale de la dague, ni le temps d'exécution ne suffisent
    // à faire partir un coup avant la fin de l'intervalle (un coup exige la recharge puis l'exécution).
    REQUIRE(coupsPendant(dague.rechargeMin * 0.9f) == 0);
    // Passé l'intervalle et l'exécution, le coup part.
    REQUIRE(coupsPendant(dague.rechargeMax + dague.tempsExecution + 0.2f) >= 1);
}

namespace {
// Une cuirasse de niveau 20, dont le seuil d'Endurance dépasse celui de toutes les classes.
const Objet s_cuirasseExigeante{IdObjet::ArmureDePlaques, Rarete::Commun, std::nullopt, std::nullopt, 20};

// Une hache à deux mains de niveau 4 (elle n'exige que de la Puissance), dont le seuil est juste au-dessus de celui du guerrier de départ, mais à la
// portée du guerrier avec l'anneau de puissance (les `REQUIRE` du test le vérifient sans écrire les valeurs).
const Objet s_hacheADeuxMainsDeNiveau4{IdObjet::HacheADeuxMains, Rarete::Commun, std::nullopt, std::nullopt, 4};
}  // namespace

TEST_CASE("seuils : le mage ne peut pas recevoir la cuirasse de mailles, ni la surbrillance ni le deplacement") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});  // le mage n'a pas l'Endurance de la cuirasse
    const Objet cuirasse = s_cuirasseExigeante;
    REQUIRE(escouade.ramasser(cuirasse));
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Torse)));
    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Torse)));

    REQUIRE(escouade.sac().a(0) == cuirasse);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Torse).has_value());
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("seuils : le guerrier ne porte pas la hache a deux mains de niveau 4, avec l'anneau de puissance il le peut") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const int puissance = escouade.caracteristiquesDuPersonnage(s_guerrier)->valeur(Caracteristique::Puissance);
    const int bonus = definitionObjet(IdObjet::AnneauDePuissance).bonus.valeur(Caracteristique::Puissance);
    const int seuil = seuilsDeLObjet(s_hacheADeuxMainsDeNiveau4).front().valeur;
    REQUIRE(seuil > puissance);          // sans l'anneau, il n'atteint pas le seuil…
    REQUIRE(seuil <= puissance + bonus);  // …avec l'anneau, il l'atteint
    REQUIRE(escouade.ramasser(s_hacheADeuxMainsDeNiveau4));         // case 0
    REQUIRE(escouade.ramasser(Objet{IdObjet::AnneauDePuissance}));  // case 1
    REQUIRE_FALSE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::Anneau1)));

    REQUIRE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite) == s_hacheADeuxMainsDeNiveau4);
}

TEST_CASE("objets inactifs : retirer l'anneau de puissance rend la hache a deux mains inactive, sans la deplacer") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.ramasser(s_hacheADeuxMainsDeNiveau4));         // case 0
    REQUIRE(escouade.ramasser(Objet{IdObjet::AnneauDePuissance}));  // case 1
    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::Anneau1)));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    REQUIRE(escouade.equipementActifDuPersonnage(s_guerrier).a(Emplacement::MainDroite) == s_hacheADeuxMainsDeNiveau4);

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Anneau1), dansLeSac(5)));  // vers le sac : aucun seuil

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite) == s_hacheADeuxMainsDeNiveau4);   // toujours portée
    REQUIRE_FALSE(escouade.equipementActifDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());  // sans effet
    REQUIRE(attaqueDeBaseDe(escouade, s_guerrier)->degats.total() <
            montantEffectif(descriptionDeLArme(Objet{IdObjet::HacheADeuxMains}).degats.min[Element::Physique], Coefficients::seul(Caracteristique::Puissance, 1.0f),
                            *escouade.caracteristiquesDuPersonnage(s_guerrier)));
}

TEST_CASE("seuilsManques : ce que le mage n'atteint pas pour la cuirasse, rien pour des gants") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.ramasser(s_cuirasseExigeante));  // case 0
    REQUIRE(escouade.ramasser(s_gants));              // case 1

    const std::vector<SeuilDeCaracteristique> manques = escouade.seuilsManques(dansLeSac(0), portee(s_mage, Emplacement::Torse));

    REQUIRE(manques == seuilsDeLObjet(s_cuirasseExigeante));
    REQUIRE(escouade.seuilsManques(dansLeSac(1), portee(s_mage, Emplacement::Gants)).empty());
}

TEST_CASE("objetPorteEstInactif : un objet porte dont le seuil n'est pas atteint, pas un objet actif ni un emplacement vide") {
    // Le prêtre porte, dès le départ, une cuirasse dont le seuil d'Endurance dépasse la sienne.
    std::vector<MembreDEscouade> membres = membresDeTest({guerrierDeTest(), pretreDeTest()});
    static_cast<void>(membres[1].equipement.poser(Emplacement::Torse, s_cuirasseExigeante));
    static_cast<void>(membres[1].equipement.poser(Emplacement::Gants, s_gants));
    const Escouade escouade({50.0f, 50.0f}, std::move(membres));

    REQUIRE_FALSE(escouade.objetPorteEstInactif(s_guerrier, Emplacement::Torse));  // le guerrier porte une cuirasse de niveau 1 : actif
    REQUIRE(escouade.objetPorteEstInactif(2, Emplacement::Torse));                 // porté, sans effet
    REQUIRE_FALSE(escouade.objetPorteEstInactif(2, Emplacement::Gants));           // sans seuil
    REQUIRE_FALSE(escouade.objetPorteEstInactif(2, Emplacement::Tete));            // vide
}

TEST_CASE("la penetration de l'attaque de base lue par l'escouade (coup d'opportunite) est celle de l'arme") {
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {capableDePorter(guerrierDeTest(), {IdObjet::Dague})});
    REQUIRE(attaqueDeBaseDe(escouade, s_guerrier)->penetration == 0);  // l'épée à une main
    REQUIRE(escouade.ramasser(Objet{IdObjet::Dague}));

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(attaqueDeBaseDe(escouade, s_guerrier)->penetration == descriptionDeLArme(Objet{IdObjet::Dague}).penetration);
}

TEST_CASE("un objet a affixes garde ses affixes du sac a l'emplacement et retour, et se compte par valeur complete") {
    const Objet masseFlamboyante{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};
    const Objet masseCommune{IdObjet::MasseAUneMain};
    Escouade escouade = escouadeAvecSac({masseFlamboyante, masseCommune});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(std::ranges::count(avant, cle(masseFlamboyante)) == 1);  // la masse commune est comptée à part, même modèle
    REQUIRE(std::ranges::count(avant, cle(masseCommune)) >= 1);
    REQUIRE(cle(masseFlamboyante) != cle(masseCommune));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::MainDroite)));
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::MainDroite) == masseFlamboyante);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(escouade.deplacerObjet(portee(s_mage, Emplacement::MainDroite), dansLeSac(0)));
    REQUIRE(escouade.sac().a(0) == masseFlamboyante);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("ramasserTout : tous les objets dans les premieres cases libres, dans l'ordre") {
    Escouade escouade = escouadeAvecSac({s_gants});

    REQUIRE(escouade.ramasserTout(s_lotDeQuatre));

    REQUIRE(escouade.sac().a(0) == s_gants);
    for (std::size_t i = 0; i < s_lotDeQuatre.size(); ++i) {
        REQUIRE(escouade.sac().a(i + 1) == s_lotDeQuatre[i]);
    }
    REQUIRE_FALSE(escouade.sac().a(s_lotDeQuatre.size() + 1).has_value());
}

TEST_CASE("ramasserTout : sans assez de cases libres, rien n'est ajoute (tout ou rien)") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});
    const std::size_t libres = 3;  // il en faut quatre
    for (std::size_t i = 0; i + libres < escouade.sac().capacite(); ++i) {
        REQUIRE(escouade.ramasser(s_gants));
    }
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.ramasserTout(s_lotDeQuatre));

    REQUIRE(exemplaires(escouade) == avant);
    for (std::size_t i = escouade.sac().capacite() - libres; i < escouade.sac().capacite(); ++i) {
        REQUIRE_FALSE(escouade.sac().a(i).has_value());  // aucune des trois cases libres n'a été remplie
    }
}

TEST_CASE("ramasserTout : deux appels de suite sur un sac vide donnent deux exemplaires de chaque objet") {
    Escouade escouade = uneEscouadeCapableDeToutPorter({50.0f, 50.0f});

    REQUIRE(escouade.ramasserTout(s_lotDeQuatre));
    REQUIRE(escouade.ramasserTout(s_lotDeQuatre));

    const Exemplaires tous = exemplaires(escouade);
    for (const Objet& objet : s_lotDeQuatre) {
        REQUIRE(std::ranges::count(tous, cle(objet)) == 2);  // deux exemplaires de la même valeur complète
    }
}

TEST_CASE("ramasserTout : une liste vide reussit sans rien changer") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE(escouade.ramasserTout({}));
    REQUIRE(exemplaires(escouade) == avant);
}

// retirerObjet (étape 97.6) : le retrait ordinaire, sans destination.
TEST_CASE("retirerObjet : un objet du sac est rendu et sa case reste vide, les autres ne bougent pas") {
    Escouade escouade = escouadeAvecSac({s_gants, s_casqueDeCuir});

    const std::optional<Objet> retire = escouade.retirerObjet(dansLeSac(0));

    REQUIRE(retire == s_gants);
    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.sac().a(1) == s_casqueDeCuir);
}

TEST_CASE("retirerObjet : un objet porte est rendu, l'emplacement se vide et l'armure baisse") {
    Escouade escouade = escouadeAvecSac({});
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    const std::optional<Objet> retire = escouade.retirerObjet(portee(s_guerrier, Emplacement::Tete));

    REQUIRE(retire == s_casqueDeFer);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() < armureAvant);
    REQUIRE(escouade.equipementActifDuPersonnage(s_guerrier).armureTotale() < armureAvant);
}

TEST_CASE("retirerObjet : une position vide ne rend rien et ne change rien") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.retirerObjet(dansLeSac(5)).has_value());
    REQUIRE_FALSE(escouade.retirerObjet(portee(s_mage, Emplacement::Tete)).has_value());

    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("retirerObjet : l'objet n'est plus nulle part, sauf chez celui qui l'a retire") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const Exemplaires avant = exemplaires(escouade);

    const std::optional<Objet> retire = escouade.retirerObjet(dansLeSac(0));

    REQUIRE(retire.has_value());
    Exemplaires attendu = avant;
    attendu.erase(std::ranges::find(attendu, cle(*retire)));
    REQUIRE(exemplaires(escouade) == attendu);
}

TEST_CASE("retirerObjet : retirer l'anneau de puissance rend la hache a deux mains inactive, comme un deplacement vers le sac") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.ramasser(s_hacheADeuxMainsDeNiveau4));          // case 0
    REQUIRE(escouade.ramasser(Objet{IdObjet::AnneauDePuissance}));  // case 1
    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::Anneau1)));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    REQUIRE_FALSE(escouade.objetPorteEstInactif(s_guerrier, Emplacement::MainDroite));

    REQUIRE(escouade.retirerObjet(portee(s_guerrier, Emplacement::Anneau1)).has_value());

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());  // toujours portée
    REQUIRE(escouade.objetPorteEstInactif(s_guerrier, Emplacement::MainDroite));
}

// Les points libres (étape 100.1) : la dépense passe par l'escouade, qui désigne le personnage par sa place.
TEST_CASE("Escouade::depenserDesPoints : le personnage de cette place depense, les autres non") {
    Escouade escouade(sf::Vector2f{50.0f, 50.0f}, membresDeTest(), Inventaire(Escouade::s_capaciteDuSac), 3);  // 6 points libres chacun
    const Caracteristiques baseDuGuerrier = escouade.caracteristiquesDeBaseDuPersonnage(s_guerrier).value();
    const int maximumDuMage = escouade.pointsDeVieMaxDuPersonnage(s_mage).value();

    REQUIRE(escouade.depenserDesPoints(s_mage, Caracteristiques::seule(Caracteristique::Endurance, 4)));

    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques::seule(Caracteristique::Endurance, 4));
    REQUIRE(escouade.pointsLibresDuPersonnage(s_mage) == 2);
    REQUIRE(escouade.pointsDeVieMaxDuPersonnage(s_mage) == maximumDuMage + 4 * s_pvParPointDEndurance);
    REQUIRE(escouade.pointsDepensesDuPersonnage(s_guerrier) == Caracteristiques());
    REQUIRE(escouade.pointsLibresDuPersonnage(s_guerrier) == 6);
    REQUIRE(escouade.caracteristiquesDeBaseDuPersonnage(s_guerrier) == baseDuGuerrier);
}

TEST_CASE("Escouade::depenserDesPoints : une place invalide ou une depense refusee ne changent rien") {
    Escouade escouade(sf::Vector2f{50.0f, 50.0f}, membresDeTest(), Inventaire(Escouade::s_capaciteDuSac), 2);  // 3 points libres chacun

    REQUIRE_FALSE(escouade.depenserDesPoints(0, Caracteristiques::seule(Caracteristique::Puissance, 1)));
    REQUIRE_FALSE(escouade.depenserDesPoints(4, Caracteristiques::seule(Caracteristique::Puissance, 1)));
    REQUIRE_FALSE(escouade.depenserDesPoints(s_mage, Caracteristiques::seule(Caracteristique::Puissance, 4)));
    REQUIRE_FALSE(escouade.depenserDesPoints(s_mage, Caracteristiques({-1, 2, 0, 0, 0})));

    for (std::size_t place = s_guerrier; place <= s_mage; ++place) {
        REQUIRE(escouade.pointsDepensesDuPersonnage(place) == Caracteristiques());
        REQUIRE(escouade.pointsLibresDuPersonnage(place) == 3);
    }
    REQUIRE_FALSE(escouade.pointsDepensesDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.pointsLibresDuPersonnage(4).has_value());
}

TEST_CASE("Escouade::monterAuNiveau : les points libres augmentent de trois par niveau pour chaque personnage") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.pointsLibresDuPersonnage(s_pretre) == 0);

    escouade.monterAuNiveau(4);

    for (std::size_t place = s_guerrier; place <= s_mage; ++place) {
        REQUIRE(escouade.pointsLibresDuPersonnage(place) == 9);
    }
}

// Le signal « Points to spend » (étape 100.3) : tant qu'un personnage au moins a des points libres.
TEST_CASE("aDesPointsLibres : aucun au niveau 1, oui apres une montee de niveau") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE_FALSE(escouade.aDesPointsLibres());

    escouade.monterAuNiveau(2);

    REQUIRE(escouade.aDesPointsLibres());
}

TEST_CASE("aDesPointsLibres : tant qu'un seul personnage en a, et plus quand tous ont tout depense") {
    Escouade escouade(sf::Vector2f{50.0f, 50.0f}, membresDeTest(), Inventaire(Escouade::s_capaciteDuSac), 2);  // 3 chacun
    REQUIRE(escouade.depenserDesPoints(s_guerrier, Caracteristiques::seule(Caracteristique::Puissance, 3)));
    REQUIRE(escouade.depenserDesPoints(s_pretre, Caracteristiques::seule(Caracteristique::Volonte, 3)));
    REQUIRE(escouade.aDesPointsLibres());  // le mage n'a rien depense

    REQUIRE(escouade.depenserDesPoints(s_mage, Caracteristiques::seule(Caracteristique::Savoir, 2)));
    REQUIRE(escouade.aDesPointsLibres());  // il lui en reste un

    REQUIRE(escouade.depenserDesPoints(s_mage, Caracteristiques::seule(Caracteristique::Savoir, 1)));
    REQUIRE_FALSE(escouade.aDesPointsLibres());
}

TEST_CASE("aDesPointsLibres : un mort garde ses points, le signal reste") {
    Escouade escouade(sf::Vector2f{50.0f, 50.0f}, membresDeTest(), Inventaire(Escouade::s_capaciteDuSac), 2);
    REQUIRE(escouade.depenserDesPoints(s_guerrier, Caracteristiques::seule(Caracteristique::Puissance, 3)));
    REQUIRE(escouade.depenserDesPoints(s_pretre, Caracteristiques::seule(Caracteristique::Volonte, 3)));
    static_cast<void>(subirCoup(escouade, 1, s_mage, 1'000'000));  // le mage meurt avec ses 3 points
    REQUIRE(escouade.pointsDeVieDuPersonnage(s_mage) == 0);

    REQUIRE(escouade.aDesPointsLibres());
}
