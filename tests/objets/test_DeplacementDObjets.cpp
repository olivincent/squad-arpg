// ./tests/objets/test_DeplacementDObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>
#include <vector>

#include "objets/DeplacementDObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

// La règle des déplacements d'objets sur un état en valeurs (sac et équipements), dont les deux mains : une
// arme à deux mains exige la main gauche vide, et range ce qui la gêne dans la première case libre du sac.
namespace {
constexpr std::size_t s_capaciteDuSac = 6;
constexpr std::size_t s_placeA = 1;
constexpr std::size_t s_placeB = 2;

const Objet s_bouclier{IdObjet::BouclierRond};
const Objet s_epee{IdObjet::EpeeLongue};
const Objet s_epeeADeuxMains{IdObjet::EpeeADeuxMains};
const Objet s_arc{IdObjet::Arc};
const Objet s_gants{IdObjet::GantsDeCuir};
const Objet s_bottes{IdObjet::BottesDeCuir};
const Objet s_casque{IdObjet::CasqueDeCuir};

PositionDObjet dansLeSac(std::size_t caseDuSac) {
    return PositionDansLeSac{caseDuSac};
}

PositionDObjet portee(std::size_t place, Emplacement emplacement) {
    return PositionPortee{place, emplacement};
}

// Deux personnages sans rien, un sac vide.
EtatDesObjets etatVide() {
    return {Inventaire(s_capaciteDuSac), std::vector<Equipement>(2)};
}

void rangerDansLeSac(EtatDesObjets& etat, std::size_t caseDuSac, Objet objet) {
    REQUIRE_FALSE(etat.sac.poser(caseDuSac, objet).has_value());
}

void porter(EtatDesObjets& etat, std::size_t place, Emplacement emplacement, Objet objet) {
    REQUIRE_FALSE(etat.equipements[place - 1].poser(emplacement, objet).has_value());
}

// Tout ce que l'état contient, dans un ordre fixe : le sac, puis chaque emplacement de chaque personnage.
std::vector<std::optional<Objet>> contenu(const EtatDesObjets& etat) {
    std::vector<std::optional<Objet>> tout;
    for (std::size_t i = 0; i < etat.sac.capacite(); ++i) {
        tout.push_back(etat.sac.a(i));
    }
    for (const Equipement& equipement : etat.equipements) {
        for (std::size_t e = 0; e < s_nombreEmplacements; ++e) {
            tout.push_back(equipement.a(static_cast<Emplacement>(e)));
        }
    }
    return tout;
}

// Remplit le sac avec des objets quelconques, sauf les cases de `sauf`.
void remplirLeSacSauf(EtatDesObjets& etat, const std::vector<std::size_t>& sauf) {
    for (std::size_t i = 0; i < etat.sac.capacite(); ++i) {
        bool exclu = false;
        for (const std::size_t autre : sauf) {
            exclu = exclu || autre == i;
        }
        if (!exclu) {
            rangerDansLeSac(etat, i, s_gants);
        }
    }
}
}  // namespace

TEST_CASE("deux mains : une arme a deux mains range l'objet de la main gauche dans la premiere case libre du sac") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    rangerDansLeSac(etat, 3, s_epeeADeuxMains);

    REQUIRE(deplacerLesObjets(etat, dansLeSac(3), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE_FALSE(etat.equipements[0].a(Emplacement::MainGauche).has_value());
    REQUIRE(etat.sac.a(0) == s_bouclier);  // la première case libre
    REQUIRE_FALSE(etat.sac.a(3).has_value());
}

TEST_CASE("deux mains : la case que l'arme vient de quitter est libre pour le bouclier, meme dans un sac plein") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    remplirLeSacSauf(etat, {});
    static_cast<void>(etat.sac.poser(4, s_epeeADeuxMains));  // la case 4 contenait des gants : l'arme les remplace

    REQUIRE(deplacerLesObjets(etat, dansLeSac(4), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE(etat.sac.a(4) == s_bouclier);
}

TEST_CASE("deux mains : sac plein et rien ne se libere, le deplacement est refuse et rien ne bouge") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epeeADeuxMains);
    porter(etat, s_placeB, Emplacement::MainDroite, s_epee);
    porter(etat, s_placeB, Emplacement::MainGauche, s_bouclier);
    remplirLeSacSauf(etat, {});
    const std::vector<std::optional<Objet>> avant = contenu(etat);

    // L'épée à deux mains de A va chez B, qui a un bouclier : il faudrait le ranger, et le sac est plein (l'épée
    // longue de B prend la place de l'épée de A, aucune case du sac ne se libère).
    REQUIRE_FALSE(deplacerLesObjets(etat, portee(s_placeA, Emplacement::MainDroite), portee(s_placeB, Emplacement::MainDroite)));

    REQUIRE(contenu(etat) == avant);
}

TEST_CASE("deux mains : un bouclier range l'arme a deux mains de la main droite") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epeeADeuxMains);
    rangerDansLeSac(etat, 1, s_gants);
    rangerDansLeSac(etat, 4, s_bouclier);

    REQUIRE(deplacerLesObjets(etat, dansLeSac(4), portee(s_placeA, Emplacement::MainGauche)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainGauche) == s_bouclier);
    REQUIRE_FALSE(etat.equipements[0].a(Emplacement::MainDroite).has_value());
    REQUIRE(etat.sac.a(0) == s_epeeADeuxMains);  // la première case libre
    REQUIRE(etat.sac.a(1) == s_gants);
}

TEST_CASE("deux mains : un bouclier refuse quand le sac est plein et que l'arme a deux mains doit y aller") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epeeADeuxMains);
    porter(etat, s_placeB, Emplacement::MainGauche, s_bouclier);
    remplirLeSacSauf(etat, {});
    const std::vector<std::optional<Objet>> avant = contenu(etat);

    // Le bouclier de B va chez A et échange avec rien : A garde son arme à deux mains à ranger, sans case libre.
    REQUIRE_FALSE(deplacerLesObjets(etat, portee(s_placeB, Emplacement::MainGauche), portee(s_placeA, Emplacement::MainGauche)));

    REQUIRE(contenu(etat) == avant);
}

TEST_CASE("deux mains : une arme a une main et un bouclier se portent ensemble, sans rien ranger") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    rangerDansLeSac(etat, 0, s_epee);

    REQUIRE(deplacerLesObjets(etat, dansLeSac(0), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_epee);
    REQUIRE(etat.equipements[0].a(Emplacement::MainGauche) == s_bouclier);
    REQUIRE_FALSE(etat.sac.a(0).has_value());
}

TEST_CASE("deux mains : poser un bouclier sur une arme a une main ne range rien") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epee);
    rangerDansLeSac(etat, 0, s_bouclier);

    REQUIRE(deplacerLesObjets(etat, dansLeSac(0), portee(s_placeA, Emplacement::MainGauche)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_epee);
    REQUIRE(etat.equipements[0].a(Emplacement::MainGauche) == s_bouclier);
}

TEST_CASE("deux mains : l'arc, a deux mains, range aussi le bouclier") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    rangerDansLeSac(etat, 2, s_arc);

    REQUIRE(deplacerLesObjets(etat, dansLeSac(2), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_arc);
    REQUIRE(etat.sac.a(0) == s_bouclier);
}

TEST_CASE("deux mains : retirer l'arme a deux mains ne range rien") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epeeADeuxMains);

    REQUIRE(deplacerLesObjets(etat, portee(s_placeA, Emplacement::MainDroite), dansLeSac(5)));

    REQUIRE_FALSE(etat.equipements[0].a(Emplacement::MainDroite).has_value());
    REQUIRE(etat.sac.a(5) == s_epeeADeuxMains);
}

TEST_CASE("deux mains : l'echange d'une arme a une main contre une arme a deux mains range le bouclier et rend l'arme dans le sac") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epee);
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    rangerDansLeSac(etat, 0, s_epeeADeuxMains);
    rangerDansLeSac(etat, 1, s_gants);

    REQUIRE(deplacerLesObjets(etat, dansLeSac(0), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE_FALSE(etat.equipements[0].a(Emplacement::MainGauche).has_value());
    REQUIRE(etat.sac.a(0) == s_epee);  // l'échange
    REQUIRE(etat.sac.a(1) == s_gants);
    REQUIRE(etat.sac.a(2) == s_bouclier);  // la première case libre
}

TEST_CASE("deux mains : l'echange entre deux personnages traite les deux extremites") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epee);
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    porter(etat, s_placeB, Emplacement::MainDroite, s_epeeADeuxMains);

    REQUIRE(deplacerLesObjets(etat, portee(s_placeA, Emplacement::MainDroite), portee(s_placeB, Emplacement::MainDroite)));

    // B reçoit l'épée à une main ; A reçoit l'épée à deux mains, donc son bouclier va dans le sac.
    REQUIRE(etat.equipements[1].a(Emplacement::MainDroite) == s_epee);
    REQUIRE(etat.equipements[0].a(Emplacement::MainDroite) == s_epeeADeuxMains);
    REQUIRE_FALSE(etat.equipements[0].a(Emplacement::MainGauche).has_value());
    REQUIRE(etat.sac.a(0) == s_bouclier);
}

TEST_CASE("deux mains : aucun objet ne disparait ni ne se double") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    porter(etat, s_placeA, Emplacement::Tete, s_casque);
    rangerDansLeSac(etat, 1, s_epeeADeuxMains);
    rangerDansLeSac(etat, 2, s_bottes);
    const auto compter = [](const EtatDesObjets& e) {
        int total = 0;
        for (const std::optional<Objet>& objet : contenu(e)) {
            total += objet.has_value() ? 1 : 0;
        }
        return total;
    };

    REQUIRE(deplacerLesObjets(etat, dansLeSac(1), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(compter(etat) == 4);
}

TEST_CASE("deplacement : la regle de base des emplacements reste : arme en main gauche, bouclier en main droite refuses") {
    EtatDesObjets etat = etatVide();
    rangerDansLeSac(etat, 0, s_epee);
    rangerDansLeSac(etat, 1, s_bouclier);
    rangerDansLeSac(etat, 2, s_gants);
    const std::vector<std::optional<Objet>> avant = contenu(etat);

    REQUIRE_FALSE(deplacerLesObjets(etat, dansLeSac(0), portee(s_placeA, Emplacement::MainGauche)));
    REQUIRE_FALSE(deplacerLesObjets(etat, dansLeSac(1), portee(s_placeA, Emplacement::MainDroite)));
    REQUIRE_FALSE(deplacerLesObjets(etat, dansLeSac(2), portee(s_placeA, Emplacement::MainDroite)));
    REQUIRE_FALSE(deplacerLesObjets(etat, dansLeSac(4), dansLeSac(5)));  // source vide

    REQUIRE(contenu(etat) == avant);
}

TEST_CASE("deplacement : une position vers elle-meme est vraie sans effet") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epee);
    const std::vector<std::optional<Objet>> avant = contenu(etat);

    REQUIRE(deplacerLesObjets(etat, portee(s_placeA, Emplacement::MainDroite), portee(s_placeA, Emplacement::MainDroite)));

    REQUIRE(contenu(etat) == avant);
}

TEST_CASE("deplacementPossible : repond comme deplacerLesObjets, sans rien changer") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainGauche, s_bouclier);
    rangerDansLeSac(etat, 2, s_epeeADeuxMains);
    rangerDansLeSac(etat, 3, s_gants);
    const std::vector<std::optional<Objet>> avant = contenu(etat);

    REQUIRE(deplacementPossible(etat, dansLeSac(2), portee(s_placeA, Emplacement::MainDroite)));
    REQUIRE_FALSE(deplacementPossible(etat, dansLeSac(3), portee(s_placeA, Emplacement::MainDroite)));  // des gants en main droite
    REQUIRE(contenu(etat) == avant);
}

TEST_CASE("deplacementPossible : un sac plein qui ne peut pas ranger la main refuse aussi, sans rien changer") {
    EtatDesObjets etat = etatVide();
    porter(etat, s_placeA, Emplacement::MainDroite, s_epeeADeuxMains);
    porter(etat, s_placeB, Emplacement::MainGauche, s_bouclier);
    remplirLeSacSauf(etat, {});
    const std::vector<std::optional<Objet>> avant = contenu(etat);

    REQUIRE_FALSE(deplacementPossible(etat, portee(s_placeB, Emplacement::MainGauche), portee(s_placeA, Emplacement::MainGauche)));
    REQUIRE(contenu(etat) == avant);
}
