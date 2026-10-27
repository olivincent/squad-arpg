// ./tests/escouade/test_Personnage_affixes.cpp
#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Attaque.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Element.hpp"
#include "escouade/Personnage.hpp"
#include "objets/CatalogueObjets.hpp"

// L'attaque de base d'un personnage qui porte une arme à affixes (étape 95.4). Le coup d'opportunité d'`Escouade` lit
// `Personnage::getAttaqueDeBase`, comme l'attaque ordinaire : il en profite sans autre règle.
namespace {
const Coefficients s_puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);

// Un personnage avec cette Puissance (relevée au besoin pour porter l'arme), qui porte cet objet en main droite.
Personnage armeDe(const Objet& arme, int puissance) {
    Personnage personnage = unPersonnage({100.0f, 100.0f}, capableDePorter(avecPuissance(definitionArmee(IdObjet::MasseAUneMain), puissance), {arme.modele}));
    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, arme));
    return personnage;
}
}  // namespace

TEST_CASE("une masse flamboyante frappe en physique et en feu, la Puissance sur chacune des portions") {
    const Objet masseFlamboyante{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};
    const Personnage personnage = armeDe(masseFlamboyante, 20);  // +30 % sur chaque portion
    const DescriptionDAttaque masse = descriptionDeLArme(Objet{IdObjet::MasseAUneMain});

    const Attaque attaque = attaqueDeBaseDe(personnage);

    // Le coup de la masse est tiré dans sa plage, la Puissance sur le tirage ; le feu de l'affixe est fixe.
    REQUIRE(dansLaPlageAvecPuissance(attaque.degats[Element::Physique], masse, personnage.getCaracteristiques()));
    REQUIRE(attaque.degats[Element::Feu] == montantEffectif(6, s_puissance, personnage.getCaracteristiques()));
    REQUIRE(attaque.degats[Element::Physique] > masse.degats.min[Element::Physique]);  // la Puissance joue vraiment
    REQUIRE(attaque.degats[Element::Feu] > 6);
    REQUIRE(attaque.degats[Element::Sacre] == 0);
}

TEST_CASE("une masse sans affixe frappe comme avant, en physique seulement") {
    const Personnage personnage = armeDe({IdObjet::MasseAUneMain}, 10);

    const Attaque attaque = attaqueDeBaseDe(personnage);

    REQUIRE(dansLaPlage(attaque.degats[Element::Physique], descriptionDeLArme(Objet{IdObjet::MasseAUneMain})));
    REQUIRE(attaque.degats[Element::Feu] == 0);
    REQUIRE(attaque.degats[Element::Sacre] == 0);
}

TEST_CASE("un arc acere perce de plus que l'arc seul, sa nature reste le projectile") {
    Personnage personnage = armeDe({IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}}, 10);

    const Attaque attaque = attaqueDeBaseDe(personnage);

    REQUIRE(attaque.penetration == descriptionDeLArme(Objet{IdObjet::Arc}).penetration + 15);  // celle de l'arc, plus l'affixe
    REQUIRE(attaque.nature == NatureDAttaque::Projectile);
}

TEST_CASE("le suffixe de Dexterite d'un arc entre dans les caracteristiques du personnage") {
    const Personnage sans = armeDe({IdObjet::Arc}, 10);
    const Personnage avec = armeDe({IdObjet::Arc, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuRenard, 2}}, 10);

    REQUIRE(avec.getCaracteristiques().valeur(Caracteristique::Adresse) == sans.getCaracteristiques().valeur(Caracteristique::Adresse) + 2);
    REQUIRE(avec.getStatistiques().capaciteOffensive == sans.getStatistiques().capaciteOffensive + 10);  // 5 par point d'Adresse
}

TEST_CASE("changer pour une arme a affixes refait l'attaque de base, un anneau a affixes ne la refait pas") {
    Personnage personnage = unPersonnage({100.0f, 100.0f}, capableDePorter(definitionArmee(IdObjet::MasseAUneMain), {IdObjet::MasseAUneMain}));
    REQUIRE(personnage.engagerAttaque(1));  // une arme neuve est prête
    personnage.annulerAttaque();

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDeProtection, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeLOurs, 2}}));
    REQUIRE(personnage.engagerAttaque(1));  // la recharge n'a pas été relancée
    personnage.annulerAttaque();

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}));
    REQUIRE_FALSE(personnage.engagerAttaque(1));  // l'attaque a changé : recharge complète à relancer
}
