// ./tests/escouade/test_Escouade_equipement.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>
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
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// Équipement et sac de l'escouade (étape 83). Places de uneEscouade : 1 guerrier (équipé), 2 prêtre
// et 3 mage (sans rien). Les attentes d'armure viennent du catalogue d'objets.
namespace {
constexpr std::size_t s_guerrier = 1;
constexpr std::size_t s_mage = 3;
constexpr int s_coup = 10;  // petit : trois coups ne tuent personne, quelle que soit l'armure

// Combien d'exemplaires de chaque objet existent dans le sac et dans tous les équipements.
std::vector<int> exemplaires(const Escouade& escouade) {
    std::vector<int> nombre(catalogueObjets().size(), 0);
    for (std::size_t i = 0; i < escouade.sac().capacite(); ++i) {
        if (const std::optional<Objet>& objet = escouade.sac().a(i)) {
            ++nombre[static_cast<std::size_t>(objet->id)];
        }
    }
    for (std::size_t place = 1; place <= 3; ++place) {
        for (std::size_t e = 0; e < s_nombreEmplacements; ++e) {
            if (const std::optional<Objet>& objet = escouade.equipementDuPersonnage(place).a(static_cast<Emplacement>(e))) {
                ++nombre[static_cast<std::size_t>(objet->id)];
            }
        }
    }
    return nombre;
}

// Les dégâts qu'un coup de `s_coup` fait à ce personnage, armure comprise.
int degatsSubis(Escouade& escouade, std::size_t place) {
    return subirCoup(escouade, 1, place, s_coup);
}

// L'escouade du jeu, sac de départ rangé dans les premières cases.
Escouade escouadeAvecSac(const std::vector<Objet>& objets) {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
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

const Objet s_gants{IdObjet::GantsDeCuir};
const Objet s_casqueDeCuir{IdObjet::CasqueDeCuir};
const Objet s_casqueDeFer{IdObjet::CasqueDeFer};
}  // namespace

TEST_CASE("le sac d'une escouade neuve est vide") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.sac().capacite() > 0);
    for (std::size_t i = 0; i < escouade.sac().capacite(); ++i) {
        REQUIRE_FALSE(escouade.sac().a(i).has_value());
    }
}

TEST_CASE("equipementDuPersonnage : le guerrier est equipe, le mage non") {
    const Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == s_armureMeleeDeTest);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == 0);
}

TEST_CASE("ramasser : premiere case libre, puis faux quand le sac est plein") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.ramasser(s_gants));
    REQUIRE(escouade.ramasser(s_casqueDeCuir));
    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(escouade.sac().a(1) == s_casqueDeCuir);

    for (std::size_t i = 2; i < escouade.sac().capacite(); ++i) {
        REQUIRE(escouade.ramasser(s_gants));
    }
    const std::vector<int> avant = exemplaires(escouade);
    REQUIRE_FALSE(escouade.ramasser(s_casqueDeCuir));
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer du sac vers un emplacement vide : l'objet quitte le sac, l'emplacement vide le recoit, l'armure suit") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Gants)));

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == s_gants);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_mage) == appliquerArmure(s_coup, definitionObjet(IdObjet::GantsDeCuir).armure));
}

TEST_CASE("deplacer du sac vers un emplacement occupe echange avec la case du sac") {
    Escouade escouade = escouadeAvecSac({s_casqueDeCuir});
    const std::vector<int> avant = exemplaires(escouade);
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::Tete)));

    REQUIRE(escouade.sac().a(0) == s_casqueDeFer);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeCuir);
    REQUIRE(exemplaires(escouade) == avant);
    const int armureApres = armureAvant - definitionObjet(IdObjet::CasqueDeFer).armure +
                            definitionObjet(IdObjet::CasqueDeCuir).armure;
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armureApres);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureApres));
}

TEST_CASE("deplacer du sac vers un emplacement : case vide ou emplacement refuse, rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const std::vector<int> avant = exemplaires(escouade);

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
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<int> avant = exemplaires(escouade);
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), dansLeSac(7)));

    REQUIRE(escouade.sac().a(7) == s_casqueDeFer);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete).has_value());
    REQUIRE(exemplaires(escouade) == avant);
    const int armureApres = armureAvant - definitionObjet(IdObjet::CasqueDeFer).armure;
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureApres));
}

TEST_CASE("deplacer d'un emplacement vers une case occupee par un objet compatible echange") {
    Escouade escouade = escouadeAvecSac({s_casqueDeCuir});
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), dansLeSac(0)));

    REQUIRE(escouade.sac().a(0) == s_casqueDeFer);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeCuir);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer d'un emplacement vers une case occupee par un objet incompatible : rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), dansLeSac(0)));

    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeFer);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, s_armureMeleeDeTest));
}

TEST_CASE("deplacer d'un emplacement vide : faux, rien ne bouge") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_mage, Emplacement::Gants), dansLeSac(3)));

    REQUIRE_FALSE(escouade.sac().a(3).has_value());
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("tout desequiper ramene l'armure a zero, tout reequiper la rend") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
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
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<int> avant = exemplaires(escouade);
    const int armureJambieres = definitionObjet(IdObjet::JambieresDeFer).armure;
    const int armureGuerrier = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Jambes), portee(s_mage, Emplacement::Jambes)));

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDeFer});
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == armureJambieres);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_mage) == appliquerArmure(s_coup, armureJambieres));
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureGuerrier - armureJambieres));
}

TEST_CASE("echange entre deux personnages : les deux armures changent") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.ramasser(s_casqueDeCuir));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Tete)));
    const std::vector<int> avant = exemplaires(escouade);
    const int armureGuerrier = escouade.equipementDuPersonnage(s_guerrier).armureTotale();
    const int armureMage = escouade.equipementDuPersonnage(s_mage).armureTotale();
    const int ecart = definitionObjet(IdObjet::CasqueDeFer).armure - definitionObjet(IdObjet::CasqueDeCuir).armure;

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Tete), portee(s_mage, Emplacement::Tete)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == s_casqueDeCuir);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Tete) == s_casqueDeFer);
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armureGuerrier - ecart);
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == armureMage + ecart);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("deplacer d'une case du sac a une autre : case vide, case occupee, meme case") {
    Escouade escouade = escouadeAvecSac({s_gants, s_casqueDeCuir});
    const std::vector<int> avant = exemplaires(escouade);

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
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(5), dansLeSac(6)));                              // source vide
    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_mage, Emplacement::Gants), dansLeSac(0)));        // emplacement vide
    REQUIRE_FALSE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::Torse), dansLeSac(0)));    // gants ne vont pas au torse
    REQUIRE_FALSE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::Anneau1)));  // ni a un anneau

    REQUIRE(escouade.sac().a(0) == s_gants);
    REQUIRE(exemplaires(escouade) == avant);
}

TEST_CASE("peutDeplacerObjet : repond comme deplacerObjet, sans rien changer") {
    Escouade escouade = escouadeAvecSac({s_gants});
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Gants)));
    REQUIRE_FALSE(escouade.peutDeplacerObjet(dansLeSac(0), portee(s_mage, Emplacement::Tete)));
    REQUIRE_FALSE(escouade.peutDeplacerObjet(portee(s_guerrier, Emplacement::Torse), dansLeSac(0)));
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(escouade.sac().a(0) == s_gants);
}

TEST_CASE("une partie commence avec le sac de depart dans le sac") {
    const Partie partie = unePartie(niveauDegage({}), s_graineDeTest);
    const Inventaire& sac = partie.getEscouade().sac();

    const std::vector<Objet> attendu = sacDeDepart();
    for (std::size_t i = 0; i < attendu.size(); ++i) {
        REQUIRE(sac.a(i) == attendu[i]);
    }
    for (std::size_t i = attendu.size(); i < sac.capacite(); ++i) {
        REQUIRE_FALSE(sac.a(i).has_value());
    }
}

TEST_CASE("statistiquesDeCombatDuPersonnage : celles du personnage de la place, vides si la place est invalide") {
    DefinitionPersonnage rodeur = pretreDeTest();
    rodeur.caracteristiques = Caracteristiques({10, 10, 14, 8, 8});
    const Escouade escouade({50.0f, 50.0f}, {guerrierDeTest(), rodeur});

    REQUIRE(escouade.statistiquesDeCombatDuPersonnage(1) == statistiquesDuPersonnage(Caracteristiques(), StatistiquesDeCombat()));
    REQUIRE(escouade.statistiquesDeCombatDuPersonnage(2) == StatistiquesDeCombat{120, 120});
    REQUIRE_FALSE(escouade.statistiquesDeCombatDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.statistiquesDeCombatDuPersonnage(3).has_value());
}

TEST_CASE("la puissance de l'attaque de base lue par l'escouade (coup d'opportunite) suit les caracteristiques") {
    const int degats = s_degatsBaseDeTest;
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    DefinitionPersonnage definition = guerrierDeTest();
    definition.caracteristiques = Caracteristiques({s_valeurDeReference, 10, 10, 10, 10});
    Escouade escouade({50.0f, 50.0f}, {definition});
    REQUIRE(escouade.attaqueDeBase(1)->montant == degats);

    REQUIRE(escouade.ramasser(Objet{IdObjet::AnneauDePuissance}));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(1, Emplacement::Anneau1)));

    REQUIRE(escouade.attaqueDeBase(1)->montant ==
            montantEffectif(degats, puissance, escouade.caracteristiquesDuPersonnage(1).value()));
}

// Les deux mains (étape 91.3) : la règle est celle de `deplacerLesObjets` (`test_DeplacementDObjets.cpp`) ; ici,
// ce que l'escouade en fait : l'armure recalculée, la surbrillance qui suit l'action, rien ne bouge au refus.
namespace {
const Objet s_bouclier{IdObjet::BouclierRond};
const Objet s_epeeADeuxMains{IdObjet::EpeeADeuxMains};
const Objet s_epee{IdObjet::EpeeLongue};

// Le guerrier porte le bouclier rond ; le sac contient l'épée à deux mains en case 0.
Escouade escouadeDuGuerrierAuBouclier() {
    Escouade escouade = escouadeAvecSac({s_bouclier, s_epeeADeuxMains});
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainGauche)));
    return escouade;
}
}  // namespace

TEST_CASE("deux mains : l'epee a deux mains range le bouclier du guerrier dans le sac, et son armure baisse") {
    Escouade escouade = escouadeDuGuerrierAuBouclier();
    const std::vector<int> avant = exemplaires(escouade);
    const int armureAvant = escouade.equipementDuPersonnage(s_guerrier).armureTotale();
    REQUIRE(escouade.peutDeplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainDroite)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche).has_value());
    REQUIRE(escouade.sac().a(0) == s_bouclier);   // la première case libre
    REQUIRE(escouade.sac().a(1) == s_epee);       // l'épée longue du guerrier a pris la place de l'arme (échange)
    REQUIRE(exemplaires(escouade) == avant);
    const int armureApres = armureAvant - definitionObjet(IdObjet::BouclierRond).armure;
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).armureTotale() == armureApres);
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureApres));
}

TEST_CASE("deux mains : un bouclier range l'arme a deux mains que le guerrier porte") {
    Escouade escouade = escouadeAvecSac({s_epeeADeuxMains, s_bouclier});
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    const std::vector<int> avant = exemplaires(escouade);

    REQUIRE(escouade.deplacerObjet(dansLeSac(1), portee(s_guerrier, Emplacement::MainGauche)));

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche) == s_bouclier);
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());
    REQUIRE(escouade.sac().a(0) == s_epee);              // l'épée longue rendue par l'échange de l'arme à deux mains
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
    const std::vector<int> avant = exemplaires(escouade);
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
    Escouade escouade({50.0f, 50.0f}, {guerrierDeTest()});  // seul : tous les coups sont les siens
    REQUIRE(escouade.ramasser(Objet{IdObjet::Dague}));
    REQUIRE(escouade.deplacerObjet(dansLeSac(0), portee(s_guerrier, Emplacement::MainDroite)));
    const DescriptionDAttaque dague = definitionObjet(IdObjet::Dague).arme.value().attaque;
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
