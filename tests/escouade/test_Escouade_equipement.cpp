// ./tests/escouade/test_Escouade_equipement.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Defense.hpp"
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
    return escouade.recevoirCoup(1, place, s_coup);
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
    REQUIRE(escouade.recevoirCoup(1, s_mage, s_coupMortel) > 0);
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
    const std::vector<Emplacement> portes = {Emplacement::Tete, Emplacement::Torse, Emplacement::Jambes,
                                             Emplacement::MainGauche};
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
    const int armureBouclier = definitionObjet(IdObjet::BouclierRond).armure;
    const int armureGuerrier = escouade.equipementDuPersonnage(s_guerrier).armureTotale();

    REQUIRE(escouade.deplacerObjet(portee(s_guerrier, Emplacement::MainGauche), portee(s_mage, Emplacement::MainGauche)));

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::MainGauche).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::MainGauche) == Objet{IdObjet::BouclierRond});
    REQUIRE(escouade.equipementDuPersonnage(s_mage).armureTotale() == armureBouclier);
    REQUIRE(exemplaires(escouade) == avant);
    REQUIRE(degatsSubis(escouade, s_mage) == appliquerArmure(s_coup, armureBouclier));
    REQUIRE(degatsSubis(escouade, s_guerrier) == appliquerArmure(s_coup, armureGuerrier - armureBouclier));
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
