// ./tests/objets/test_CatalogueObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <vector>

#include "combat/NatureDAttaque.hpp"
#include "combat/TypeDArme.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"

namespace {
bool vaQuelquePart(CategorieObjet categorie) {
    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        if (peutAller(categorie, static_cast<Emplacement>(i))) {
            return true;
        }
    }
    return false;
}
}  // namespace

TEST_CASE("catalogueObjets : suit l'ordre de IdObjet") {
    const std::vector<DefinitionObjet>& catalogue = catalogueObjets();

    REQUIRE_FALSE(catalogue.empty());
    for (std::size_t i = 0; i < catalogue.size(); ++i) {
        REQUIRE(static_cast<std::size_t>(catalogue[i].id) == i);
    }
    REQUIRE(static_cast<std::size_t>(IdObjet::Arc) == catalogue.size() - 1);
}

TEST_CASE("catalogueObjets : chaque objet a un nom, de l'armure, un bonus ou une arme, et une categorie qui va quelque part") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE_FALSE(definition.nom.empty());
        REQUIRE(definition.armure >= 0);
        int bonus = 0;
        for (const Caracteristique caracteristique : s_caracteristiques) {
            bonus += definition.bonus.valeur(caracteristique);
        }
        REQUIRE((definition.armure > 0 || bonus > 0 || definition.arme.has_value()));
        REQUIRE(vaQuelquePart(definition.categorie));
    }
}

TEST_CASE("definitionObjet : rend l'entree du catalogue de cet identifiant") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE(&definitionObjet(definition.id) == &definition);
    }
    REQUIRE(definitionObjet(IdObjet::BouclierRond).categorie == CategorieObjet::Bouclier);
    REQUIRE(definitionObjet(IdObjet::AnneauDeProtection).categorie == CategorieObjet::Anneau);
}

TEST_CASE("Objet : l'egalite est par valeur, modele, rarete et affixes compris") {
    const Objet masseFlamboyante{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};

    REQUIRE(Objet{IdObjet::GantsDeCuir} == Objet{IdObjet::GantsDeCuir});
    REQUIRE(Objet{IdObjet::GantsDeCuir} != Objet{IdObjet::BottesDeCuir});
    REQUIRE(masseFlamboyante == Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}});
    REQUIRE(masseFlamboyante != Objet{IdObjet::Masse});                                                    // sans affixe
    REQUIRE(masseFlamboyante != Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 7}});  // autre valeur
    REQUIRE(masseFlamboyante != Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Sanctifie, 6}});   // autre affixe
    REQUIRE(masseFlamboyante != Objet{IdObjet::Masse, Rarete::Rare, Affixe{IdAffixe::Flamboyant, 6}});     // autre rareté
    REQUIRE(masseFlamboyante != Objet{IdObjet::Masse, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuLion, 6}});
}

TEST_CASE("Objet : un objet sans autre precision est commun et sans affixe") {
    const Objet objet{IdObjet::Arc};

    REQUIRE(objet.rarete == Rarete::Commun);
    REQUIRE_FALSE(objet.prefixe.has_value());
    REQUIRE_FALSE(objet.suffixe.has_value());
}

TEST_CASE("objetValide : tout modele du catalogue est valide a l'etat commun") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE(objetValide(Objet{definition.id}));
    }
}

TEST_CASE("objetValide : un objet magique a exactement un affixe, prefixe ou suffixe") {
    REQUIRE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}));
    REQUIRE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique}));  // aucun affixe
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}, Affixe{IdAffixe::DuBoeuf, 2}}));
}

TEST_CASE("objetValide : un objet rare a un prefixe et un suffixe") {
    REQUIRE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, std::nullopt, Affixe{IdAffixe::DuRenard, 2}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare}));
}

TEST_CASE("objetValide : un objet commun n'a aucun affixe") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Commun, Affixe{IdAffixe::Flamboyant, 6}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Commun, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}}));
}

TEST_CASE("objetValide : un affixe est a sa position, un prefixe dans prefixe et un suffixe dans suffixe") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::DuBoeuf, 2}}));                    // suffixe en préfixe
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, std::nullopt, Affixe{IdAffixe::Flamboyant, 6}}));  // préfixe en suffixe
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::DuRenard, 2}, Affixe{IdAffixe::Acere, 15}}));  // inversés
}

TEST_CASE("objetValide : un affixe est permis pour la categorie du modele") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::CasqueDeFer, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}));  // du feu sur un casque
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Renforce, 2}}));           // de l'armure sur une arme
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeBlocage, 5}}));  // du blocage sur une arme
    REQUIRE_FALSE(objetValide(Objet{IdObjet::BouclierRond, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DEsquive, 3}}));  // pas sur un bouclier
    REQUIRE(objetValide(Objet{IdObjet::BouclierRond, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeBlocage, 5}}));
    REQUIRE(objetValide(Objet{IdObjet::CuirasseDeMailles, Rarete::Rare, Affixe{IdAffixe::Renforce, 3}, Affixe{IdAffixe::DEsquive, 3}}));
    REQUIRE(objetValide(Objet{IdObjet::AnneauDeProtection, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeLaChouette, 2}}));
}

TEST_CASE("objetValide : la valeur d'un affixe est strictement positive") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 0}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, -6}}));
}

TEST_CASE("nomDeLObjet : prefixe, modele, suffixe") {
    REQUIRE(nomDeLObjet(Objet{IdObjet::Masse}) == "Mace");
    REQUIRE(nomDeLObjet(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}) == "Blazing Mace");
    REQUIRE(nomDeLObjet(Objet{IdObjet::Arc, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuRenard, 2}}) == "Bow of the Fox");
    REQUIRE(nomDeLObjet(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}}) ==
            "Keen Bow of the Fox");
    REQUIRE(nomDeLObjet(Objet{IdObjet::CuirasseDeMailles, Rarete::Rare, Affixe{IdAffixe::Renforce, 3}, Affixe{IdAffixe::DEsquive, 3}}) ==
            "Reinforced Chain Mail of Evasion");
}

TEST_CASE("sacDeDepart : des objets du catalogue, dont au moins deux anneaux") {
    const std::vector<Objet> sac = sacDeDepart();

    REQUIRE_FALSE(sac.empty());
    std::size_t anneaux = 0;
    for (const Objet& objet : sac) {
        if (definitionObjet(objet.modele).categorie == CategorieObjet::Anneau) {
            ++anneaux;
        }
    }
    REQUIRE(anneaux >= 2);
}

TEST_CASE("catalogueObjets : l'anneau de puissance donne de la Puissance, le collier d'endurance de l'Endurance") {
    const Caracteristiques& anneau = definitionObjet(IdObjet::AnneauDePuissance).bonus;
    const Caracteristiques& collier = definitionObjet(IdObjet::CollierDEndurance).bonus;

    REQUIRE(definitionObjet(IdObjet::AnneauDePuissance).categorie == CategorieObjet::Anneau);
    REQUIRE(definitionObjet(IdObjet::CollierDEndurance).categorie == CategorieObjet::Collier);
    REQUIRE(anneau.valeur(Caracteristique::Puissance) > 0);
    REQUIRE(collier.valeur(Caracteristique::Endurance) > 0);
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (caracteristique != Caracteristique::Puissance) {
            REQUIRE(anneau.valeur(caracteristique) == 0);
        }
        if (caracteristique != Caracteristique::Endurance) {
            REQUIRE(collier.valeur(caracteristique) == 0);
        }
    }
}

TEST_CASE("sacDeDepart : contient l'anneau de puissance et le collier d'endurance") {
    const std::vector<Objet> sac = sacDeDepart();

    REQUIRE(std::ranges::find(sac, Objet{IdObjet::AnneauDePuissance}) != sac.end());
    REQUIRE(std::ranges::find(sac, Objet{IdObjet::CollierDEndurance}) != sac.end());
}

TEST_CASE("catalogueObjets : les bottes de cuir donnent 5 % d'esquive contre le contact, le casque de cuir 5 % contre les projectiles") {
    const StatistiquesDeCombat& bottes = definitionObjet(IdObjet::BottesDeCuir).bonusDeCombat;
    const StatistiquesDeCombat& casque = definitionObjet(IdObjet::CasqueDeCuir).bonusDeCombat;

    REQUIRE(bottes == StatistiquesDeCombat{.esquiveContact = 5});
    REQUIRE(casque == StatistiquesDeCombat{.esquiveProjectile = 5});
}

TEST_CASE("catalogueObjets : le bouclier rond bloque 30 % des coups, 12 points, par ses donnees de bouclier") {
    const DefinitionObjet& bouclier = definitionObjet(IdObjet::BouclierRond);

    REQUIRE(bouclier.bouclier.has_value());
    REQUIRE(bouclier.bouclier->chanceDeBlocage == 30);
    REQUIRE(bouclier.bouclier->montantBloque == 12);
    REQUIRE(bouclier.bonusDeCombat == StatistiquesDeCombat());  // le blocage n'est pas un bonus de statistiques
}

TEST_CASE("catalogueObjets : seul un bouclier a des donnees de bouclier") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE(definition.bouclier.has_value() == (definition.categorie == CategorieObjet::Bouclier));
    }
}

TEST_CASE("catalogueObjets : les autres objets n'ont aucun bonus de combat") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (definition.id != IdObjet::BottesDeCuir && definition.id != IdObjet::CasqueDeCuir) {
            REQUIRE(definition.bonusDeCombat == StatistiquesDeCombat());
        }
    }
}

namespace {
// Les valeurs du catalogue des armes (le test du catalogue est l'un des rares à les lire).
struct ArmeAttendue {
    IdObjet id;
    TypeDArme type;
    int mains;
    int degats;
    float rechargeMin;
    float rechargeMax;
    float execution;
    float portee;
    NatureDAttaque nature;
};
}  // namespace

TEST_CASE("catalogueObjets : les sept armes, leurs types, leurs mains et leur attaque") {
    const std::vector<ArmeAttendue> attendues = {
        {IdObjet::EpeeLongue, TypeDArme::EpeeLongue, 1, 15, 0.3f, 0.7f, 0.15f, 20.0f, NatureDAttaque::Contact},
        {IdObjet::Masse, TypeDArme::Masse, 1, 15, 0.3f, 0.7f, 0.15f, 20.0f, NatureDAttaque::Contact},
        {IdObjet::Dague, TypeDArme::Dague, 1, 14, 0.4f, 0.8f, 0.15f, 20.0f, NatureDAttaque::Contact},
        {IdObjet::EpeeADeuxMains, TypeDArme::EpeeADeuxMains, 2, 60, 1.2f, 2.8f, 0.3f, 20.0f, NatureDAttaque::Contact},
        {IdObjet::Sceptre, TypeDArme::Sceptre, 1, 15, 0.4f, 0.6f, 0.15f, 120.0f, NatureDAttaque::Projectile},
        {IdObjet::Baton, TypeDArme::Baton, 2, 15, 0.4f, 0.6f, 0.15f, 120.0f, NatureDAttaque::Projectile},
        {IdObjet::Arc, TypeDArme::Arc, 2, 15, 0.4f, 0.6f, 0.15f, 120.0f, NatureDAttaque::Projectile},
    };

    for (const ArmeAttendue& attendue : attendues) {
        const DefinitionObjet& definition = definitionObjet(attendue.id);
        REQUIRE(definition.categorie == CategorieObjet::Arme);
        REQUIRE(definition.armure == 0);
        REQUIRE(definition.arme.has_value());
        const DonneesDArme& arme = *definition.arme;  // NOLINT(bugprone-unchecked-optional-access): vérifié par le REQUIRE au-dessus
        REQUIRE(arme.type == attendue.type);
        REQUIRE(arme.mains == attendue.mains);
        REQUIRE(arme.attaque.degats == degatsPhysiques(attendue.degats));
        REQUIRE(arme.attaque.rechargeMin == attendue.rechargeMin);
        REQUIRE(arme.attaque.rechargeMax == attendue.rechargeMax);
        REQUIRE(arme.attaque.tempsExecution == attendue.execution);
        REQUIRE(arme.attaque.portee == attendue.portee);
        REQUIRE(arme.attaque.nature == attendue.nature);
    }
}

TEST_CASE("catalogueObjets : seules les armes ont des donnees d'arme, et chaque type d'arme a son objet") {
    std::vector<TypeDArme> types;
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE(definition.arme.has_value() == (definition.categorie == CategorieObjet::Arme));
        if (definition.arme.has_value()) {
            types.push_back(definition.arme->type);
        }
    }
    REQUIRE(types.size() == 7);
    for (std::size_t i = 0; i < types.size(); ++i) {
        for (std::size_t j = i + 1; j < types.size(); ++j) {
            REQUIRE(types[i] != types[j]);
        }
    }
}

TEST_CASE("estADeuxMains : l'epee a deux mains, le baton et l'arc, pas les autres armes ni les autres objets") {
    REQUIRE(estADeuxMains(IdObjet::EpeeADeuxMains));
    REQUIRE(estADeuxMains(IdObjet::Baton));
    REQUIRE(estADeuxMains(IdObjet::Arc));
    REQUIRE_FALSE(estADeuxMains(IdObjet::EpeeLongue));
    REQUIRE_FALSE(estADeuxMains(IdObjet::Masse));
    REQUIRE_FALSE(estADeuxMains(IdObjet::Dague));
    REQUIRE_FALSE(estADeuxMains(IdObjet::Sceptre));
    REQUIRE_FALSE(estADeuxMains(IdObjet::BouclierRond));
}

TEST_CASE("catalogueObjets : les seuils des armes, des armures de fer et du bouclier") {
    using Seuils = std::vector<SeuilDeCaracteristique>;
    const auto seuils = [](IdObjet id) { return definitionObjet(id).seuils; };

    REQUIRE(seuils(IdObjet::EpeeLongue) == Seuils{{Caracteristique::Puissance, 11}});
    REQUIRE(seuils(IdObjet::Masse) == Seuils{{Caracteristique::Puissance, 11}});
    REQUIRE(seuils(IdObjet::EpeeADeuxMains) == Seuils{{Caracteristique::Puissance, 13}, {Caracteristique::Endurance, 12}});
    REQUIRE(seuils(IdObjet::Dague) == Seuils{{Caracteristique::Adresse, 10}});
    REQUIRE(seuils(IdObjet::Arc) == Seuils{{Caracteristique::Adresse, 12}});
    REQUIRE(seuils(IdObjet::Baton) == Seuils{{Caracteristique::Savoir, 12}});
    REQUIRE(seuils(IdObjet::Sceptre) == Seuils{{Caracteristique::Volonte, 11}});
    REQUIRE(seuils(IdObjet::BouclierRond) == Seuils{{Caracteristique::Endurance, 11}});
    for (const IdObjet fer : {IdObjet::CasqueDeFer, IdObjet::CuirasseDeMailles, IdObjet::JambieresDeFer}) {
        REQUIRE(seuils(fer) == Seuils{{Caracteristique::Endurance, 12}});
    }
}

TEST_CASE("catalogueObjets : les objets de cuir et les bijoux n'ont aucun seuil") {
    for (const IdObjet libre : {IdObjet::GantsDeCuir, IdObjet::BottesDeCuir, IdObjet::CasqueDeCuir, IdObjet::TuniqueMatelassee,
                                IdObjet::CollierDeProtection, IdObjet::AnneauDeProtection, IdObjet::AnneauDePuissance,
                                IdObjet::CollierDEndurance}) {
        REQUIRE(definitionObjet(libre).seuils.empty());
    }
}

TEST_CASE("seuilsAtteints : tous les seuils doivent l'etre, un objet sans seuil l'est toujours") {
    const Caracteristiques faibles({10, 10, 10, 10, 10});
    const Caracteristiques justes({13, 12, 0, 0, 0});

    REQUIRE(seuilsAtteints(IdObjet::GantsDeCuir, Caracteristiques()));
    REQUIRE_FALSE(seuilsAtteints(IdObjet::EpeeLongue, faibles));                          // Puissance 11 exigée
    REQUIRE(seuilsAtteints(IdObjet::Dague, faibles));                                     // Adresse 10 : atteint, pas dépassé
    REQUIRE_FALSE(seuilsAtteints(IdObjet::EpeeADeuxMains, Caracteristiques({13, 11, 0, 0, 0})));  // il manque 1 d'Endurance
    REQUIRE(seuilsAtteints(IdObjet::EpeeADeuxMains, justes));
}

TEST_CASE("catalogueObjets : la dague perce 30 % de l'armure, aucune autre arme ni les mains nues") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (definition.arme.has_value()) {
            REQUIRE(definition.arme->attaque.penetration == (definition.id == IdObjet::Dague ? 30 : 0));
        }
    }
    REQUIRE(mainsNues().penetration == 0);
}

TEST_CASE("objetsDeDemonstration : quatre objets valides, ceux de F7") {
    const std::vector<Objet> objets = objetsDeDemonstration();

    REQUIRE(objets.size() == 4);
    for (const Objet& objet : objets) {
        REQUIRE(objetValide(objet));
    }
    REQUIRE(nomDeLObjet(objets[0]) == "Blazing Mace");
    REQUIRE(objets[0].rarete == Rarete::Magique);
    REQUIRE(objets[0].prefixe == Affixe{IdAffixe::Flamboyant, 6});
    REQUIRE(nomDeLObjet(objets[1]) == "Keen Bow of the Fox");
    REQUIRE(objets[1].rarete == Rarete::Rare);
    REQUIRE(objets[1].prefixe == Affixe{IdAffixe::Acere, 15});
    REQUIRE(objets[1].suffixe == Affixe{IdAffixe::DuRenard, 2});
    REQUIRE(nomDeLObjet(objets[2]) == "Reinforced Chain Mail of Evasion");
    REQUIRE(objets[2].rarete == Rarete::Rare);
    REQUIRE(objets[2].prefixe == Affixe{IdAffixe::Renforce, 3});
    REQUIRE(objets[2].suffixe == Affixe{IdAffixe::DEsquive, 3});
    REQUIRE(nomDeLObjet(objets[3]) == "Round Shield of Blocking");
    REQUIRE(objets[3].rarete == Rarete::Magique);
    REQUIRE(objets[3].suffixe == Affixe{IdAffixe::DeBlocage, 5});
}
