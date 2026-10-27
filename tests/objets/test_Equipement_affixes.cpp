// ./tests/objets/test_Equipement_affixes.cpp
#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "combat/Caracteristiques.hpp"
#include "combat/Defense.hpp"
#include "combat/Element.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Equipement.hpp"

// Les affixes ont un effet (étape 95.4) : ils entrent dans les totaux de l'équipement (`armureTotale`, `bonusTotal`,
// `bonusDeCombatTotal`, `attaqueDeBase`), qui restent les seuls endroits qui calculent.
namespace {
const Objet s_masseFlamboyante{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};
const Objet s_masseSanctifiee{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Sanctifie, 4}};
const Objet s_arcAcereDuRenard{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}};
const Objet s_mailleRenforceeDEsquive{IdObjet::CuirasseDeMailles, Rarete::Rare, Affixe{IdAffixe::Renforce, 3},
                                      Affixe{IdAffixe::DEsquive, 3}};
const Objet s_bouclierDeBlocage{IdObjet::BouclierRond, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeBlocage, 5}};

void poser(Equipement& equipement, Emplacement emplacement, const Objet& objet) {
    static_cast<void>(equipement.poser(emplacement, objet));
}
}  // namespace

TEST_CASE("affixes : l'armure d'un prefixe s'ajoute a celle du modele") {
    Equipement equipement;
    poser(equipement, Emplacement::Torse, s_mailleRenforceeDEsquive);

    REQUIRE(equipement.armureTotale() == definitionObjet(IdObjet::CuirasseDeMailles).armure + 3);
}

TEST_CASE("affixes : l'esquive d'un suffixe s'ajoute, celle d'un objet sans bonus de combat comprise") {
    Equipement equipement;
    poser(equipement, Emplacement::Torse, s_mailleRenforceeDEsquive);
    REQUIRE(equipement.bonusDeCombatTotal() == StatistiquesDeCombat{.esquiveContact = 3});

    poser(equipement, Emplacement::Bottes, {IdObjet::BottesDeCuir});  // 5 % de leur modèle
    REQUIRE(equipement.bonusDeCombatTotal().esquiveContact == 3 + definitionObjet(IdObjet::BottesDeCuir).bonusDeCombat.esquiveContact);
}

TEST_CASE("affixes : l'esquive contre les projectiles du suffixe de deviation") {
    Equipement equipement;
    poser(equipement, Emplacement::Tete, {IdObjet::CasqueDeCuir, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeDeviation, 4}});

    REQUIRE(equipement.bonusDeCombatTotal().esquiveProjectile == definitionObjet(IdObjet::CasqueDeCuir).bonusDeCombat.esquiveProjectile + 4);
    REQUIRE(equipement.bonusDeCombatTotal().esquiveContact == 0);
}

TEST_CASE("affixes : un suffixe de caracteristique entre dans le bonus total, une seule caracteristique") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, s_arcAcereDuRenard);

    REQUIRE(equipement.bonusTotal() == Caracteristiques::seule(Caracteristique::Adresse, 2));
}

TEST_CASE("affixes : le bonus du modele et celui de l'affixe s'additionnent") {
    Equipement equipement;
    poser(equipement, Emplacement::Anneau1, {IdObjet::AnneauDePuissance, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}});

    REQUIRE(equipement.bonusTotal().valeur(Caracteristique::Puissance) == definitionObjet(IdObjet::AnneauDePuissance).bonus.valeur(Caracteristique::Puissance) + 2);
}

TEST_CASE("affixes : la chance de blocage s'ajoute a celle du bouclier, le montant reste celui du bouclier") {
    Equipement equipement;
    poser(equipement, Emplacement::MainGauche, s_bouclierDeBlocage);
    const DonneesDeBouclier& bouclier = definitionObjet(IdObjet::BouclierRond).bouclier.value();

    const StatistiquesDeCombat total = equipement.bonusDeCombatTotal();

    REQUIRE(total.chanceDeBlocage == bouclier.chanceDeBlocage + 5);
    REQUIRE(total.montantBloque == bouclier.montantBloque);
}

TEST_CASE("affixes : l'armure d'un bouclier renforce s'ajoute aussi") {
    Equipement equipement;
    poser(equipement, Emplacement::MainGauche, {IdObjet::BouclierRond, Rarete::Magique, Affixe{IdAffixe::Renforce, 2}});

    REQUIRE(equipement.armureTotale() == definitionObjet(IdObjet::BouclierRond).armure + 2);
}

TEST_CASE("affixes : une masse flamboyante frappe en physique et en feu") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, s_masseFlamboyante);
    const DescriptionDAttaque masse = definitionObjet(IdObjet::Masse).arme.value().attaque;

    const DescriptionDAttaque attaque = equipement.attaqueDeBase();

    REQUIRE(attaque.degats[Element::Physique] == masse.degats[Element::Physique]);
    REQUIRE(attaque.degats[Element::Feu] == 6);
    REQUIRE(attaque.degats[Element::Sacre] == 0);
    // Le reste de l'attaque est celui de l'arme.
    REQUIRE(attaque.portee == masse.portee);
    REQUIRE(attaque.nature == masse.nature);
    REQUIRE(attaque.penetration == masse.penetration);
    REQUIRE(attaque.rechargeMin == masse.rechargeMin);
}

TEST_CASE("affixes : une masse sanctifiee frappe en physique et en sacre") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, s_masseSanctifiee);

    const DescriptionDAttaque attaque = equipement.attaqueDeBase();

    REQUIRE(attaque.degats[Element::Sacre] == 4);
    REQUIRE(attaque.degats[Element::Feu] == 0);
    REQUIRE(attaque.degats[Element::Physique] == definitionObjet(IdObjet::Masse).arme.value().attaque.degats[Element::Physique]);
}

TEST_CASE("affixes : la penetration d'un prefixe acere s'ajoute a celle de l'arme") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, s_arcAcereDuRenard);
    REQUIRE(equipement.attaqueDeBase().penetration == 15);  // l'arc ne perce rien

    poser(equipement, Emplacement::MainDroite, {IdObjet::Dague, Rarete::Magique, Affixe{IdAffixe::Acere, 15}});
    REQUIRE(equipement.attaqueDeBase().penetration == definitionObjet(IdObjet::Dague).arme->attaque.penetration + 15);
}

TEST_CASE("affixes : la penetration cumulee n'est pas plafonnee dans l'attaque, le plafond de 75 % joue a l'application") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, {IdObjet::Dague, Rarete::Magique, Affixe{IdAffixe::Acere, 60}});

    const int penetration = equipement.attaqueDeBase().penetration;

    REQUIRE(penetration == definitionObjet(IdObjet::Dague).arme->attaque.penetration + 60);
    REQUIRE(penetration > s_penetrationMaximale);
    REQUIRE(armureAppliquee(100.0f, 40, penetration) == armureAppliquee(100.0f, 40, s_penetrationMaximale));
}

TEST_CASE("affixes : un affixe de l'arme ne change pas l'attaque des mains nues ni celle d'une autre arme") {
    Equipement equipement;
    poser(equipement, Emplacement::Torse, s_mailleRenforceeDEsquive);  // pas une arme
    REQUIRE(equipement.attaqueDeBase() == mainsNues());

    poser(equipement, Emplacement::MainDroite, {IdObjet::Masse});
    REQUIRE(equipement.attaqueDeBase() == definitionObjet(IdObjet::Masse).arme.value().attaque);
}

TEST_CASE("affixes : un objet inactif perd tout, ses proprietes de modele et ses affixes") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, s_masseFlamboyante);   // exige de la Puissance
    poser(equipement, Emplacement::Torse, s_mailleRenforceeDEsquive);  // exige de l'Endurance
    const Caracteristiques faibles({s_seuilAuNiveau1 - 1, s_seuilAuNiveau1 - 1, 0, 0, 0});  // sous le seuil du niveau 1

    const Equipement actif = equipement.actifPour(faibles);

    REQUIRE(actif.armureTotale() == 0);
    REQUIRE(actif.bonusDeCombatTotal() == StatistiquesDeCombat());
    REQUIRE(actif.attaqueDeBase() == mainsNues());
    REQUIRE(equipement.armureTotale() > 0);  // l'équipement porté garde l'objet et ses affixes
    REQUIRE(equipement.attaqueDeBase().degats[Element::Feu] == 6);
}

TEST_CASE("affixes : les seuils ne dependent que du modele, jamais des affixes") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, s_masseFlamboyante);

    const int seuil = seuilsDeLObjet(s_masseFlamboyante).front().valeur;  // la Puissance exigée par la masse de base
    REQUIRE_FALSE(equipement.actifPour(Caracteristiques({seuil - 1, 0, 0, 0, 0})).a(Emplacement::MainDroite).has_value());
    REQUIRE(equipement.actifPour(Caracteristiques({seuil, 0, 0, 0, 0})).a(Emplacement::MainDroite).has_value());
}

TEST_CASE("affixes : un suffixe de caracteristique peut rendre actif un autre objet") {
    Equipement equipement;
    poser(equipement, Emplacement::MainDroite, {IdObjet::EpeeLongue});
    const int seuil = seuilsDeLObjet(Objet{IdObjet::EpeeLongue}).front().valeur;  // de Puissance
    const Caracteristiques base({seuil - 2, 0, 0, 0, 0});
    REQUIRE_FALSE(equipement.actifPour(base).a(Emplacement::MainDroite).has_value());

    poser(equipement, Emplacement::Anneau1, {IdObjet::AnneauDeProtection, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}});

    REQUIRE(equipement.actifPour(base).a(Emplacement::MainDroite).has_value());
    REQUIRE(caracteristiquesEffectives(base, equipement) == Caracteristiques({seuil, 0, 0, 0, 0}));
}

TEST_CASE("affixes : les affixes d'un objet inactif ne comptent pas dans les caracteristiques effectives") {
    Equipement equipement;
    // La cuirasse exige de l'Endurance : inactive sous son seuil, donc son suffixe de Puissance ne compte pas.
    poser(equipement, Emplacement::Torse, {IdObjet::CuirasseDeMailles, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuBoeuf, 5}});
    const Caracteristiques base({s_seuilAuNiveau1 - 1, s_seuilAuNiveau1 - 1, 0, 0, 0});

    REQUIRE_FALSE(equipement.actifPour(base).a(Emplacement::Torse).has_value());
    REQUIRE(caracteristiquesEffectives(base, equipement) == base);
}

TEST_CASE("affixes : une chaine d'objets actifs par leurs affixes se resout comme avec les modeles") {
    Equipement equipement;
    // Arme : Puissance exigée, que l'anneau « du Bœuf » +2 comble. Cuirasse : Endurance exigée, que le collier « de l'Ours » +3
    // comble (la base est au seuil du niveau 1 moins 2 : l'anneau suffit à l'arme, le collier à la cuirasse).
    poser(equipement, Emplacement::MainDroite, {IdObjet::EpeeLongue});
    poser(equipement, Emplacement::Anneau1, {IdObjet::AnneauDeProtection, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}});
    poser(equipement, Emplacement::Torse, {IdObjet::CuirasseDeMailles});
    poser(equipement, Emplacement::Collier, {IdObjet::CollierDeProtection, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeLOurs, 3}});

    const Equipement actif = equipement.actifPour(Caracteristiques({s_seuilAuNiveau1 - 2, s_seuilAuNiveau1 - 2, 0, 0, 0}));

    REQUIRE(actif == equipement);
}
