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

TEST_CASE("Objet : un objet sans autre precision est commun, sans affixe, de niveau 1 et de qualite moyenne") {
    const Objet objet{IdObjet::Arc};

    REQUIRE(objet.rarete == Rarete::Commun);
    REQUIRE_FALSE(objet.prefixe.has_value());
    REQUIRE_FALSE(objet.suffixe.has_value());
    REQUIRE(objet.niveau == 1);
    REQUIRE(objet.qualite == 50);
}

TEST_CASE("Objet : le niveau et la qualite comptent dans l'egalite") {
    const Objet base{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, 3, 70};

    REQUIRE(base == Objet{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, 3, 70});
    REQUIRE(base != Objet{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, 4, 70});  // autre niveau
    REQUIRE(base != Objet{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, 3, 71});  // autre qualité
    REQUIRE(base != Objet{IdObjet::Masse});                                                      // niveau 1, qualité 50
}

TEST_CASE("objetValide : le niveau est d'au moins 1 et la qualite va de 0 a 100") {
    const auto objet = [](int niveau, int qualite) {
        return Objet{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, niveau, qualite};
    };

    REQUIRE(objetValide(objet(1, 0)));
    REQUIRE(objetValide(objet(1, 100)));
    REQUIRE(objetValide(objet(20, 50)));
    REQUIRE_FALSE(objetValide(objet(0, 50)));
    REQUIRE_FALSE(objetValide(objet(-1, 50)));
    REQUIRE_FALSE(objetValide(objet(1, -1)));
    REQUIRE_FALSE(objetValide(objet(1, 101)));
}

TEST_CASE("facteurDeNiveau : 1 au niveau 1, puis 0,25 de plus par niveau") {
    REQUIRE(facteurDeNiveau(1) == 1.0);
    REQUIRE(facteurDeNiveau(2) == 1.25);
    REQUIRE(facteurDeNiveau(5) == 2.0);
    REQUIRE(facteurDeNiveau(9) == 3.0);
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

TEST_CASE("seuilsDeLObjet : 6 au niveau 1, puis la pente de l'exigence par niveau") {
    const auto seuilDeLaCuirasse = [](int niveau) {
        return seuilsDeLObjet(Objet{IdObjet::CuirasseDeMailles, Rarete::Commun, std::nullopt, std::nullopt, niveau});
    };

    REQUIRE(seuilDeLaCuirasse(1) == std::vector<SeuilDeCaracteristique>{{Caracteristique::Endurance, 6}});
    REQUIRE(seuilDeLaCuirasse(2) == std::vector<SeuilDeCaracteristique>{{Caracteristique::Endurance, 8}});
    REQUIRE(seuilDeLaCuirasse(5) == std::vector<SeuilDeCaracteristique>{{Caracteristique::Endurance, 14}});
    REQUIRE(seuilDeLaCuirasse(20) == std::vector<SeuilDeCaracteristique>{{Caracteristique::Endurance, 44}});
}

TEST_CASE("seuilsDeLObjet : le seuil ne depend que du modele et du niveau, ni de la rarete, ni des affixes, ni de la qualite") {
    const Objet simple{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, 3, 50};
    const Objet magique{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}, std::nullopt, 3, 50};
    const Objet excellente{IdObjet::Masse, Rarete::Commun, std::nullopt, std::nullopt, 3, 100};

    REQUIRE(seuilsDeLObjet(magique) == seuilsDeLObjet(simple));
    REQUIRE(seuilsDeLObjet(excellente) == seuilsDeLObjet(simple));
}

TEST_CASE("seuilsDeLObjet : les armes prototypes exigent leur caracteristique principale, les protections de fer l'Endurance") {
    const auto exigees = [](IdObjet id) {
        std::vector<Caracteristique> caracteristiques;
        for (const SeuilDeCaracteristique& seuil : seuilsDeLObjet(Objet{id})) {
            caracteristiques.push_back(seuil.caracteristique);
        }
        return caracteristiques;
    };
    using Liste = std::vector<Caracteristique>;

    REQUIRE(exigees(IdObjet::EpeeLongue) == Liste{Caracteristique::Puissance});
    REQUIRE(exigees(IdObjet::Masse) == Liste{Caracteristique::Puissance});
    REQUIRE(exigees(IdObjet::EpeeADeuxMains) == Liste{Caracteristique::Puissance});
    REQUIRE(exigees(IdObjet::Dague) == Liste{Caracteristique::Adresse});
    REQUIRE(exigees(IdObjet::Arc) == Liste{Caracteristique::Adresse});
    REQUIRE(exigees(IdObjet::Baton) == Liste{Caracteristique::Savoir});
    REQUIRE(exigees(IdObjet::Sceptre) == Liste{Caracteristique::Volonte});
    REQUIRE(exigees(IdObjet::BouclierRond) == Liste{Caracteristique::Endurance});
    for (const IdObjet fer : {IdObjet::CasqueDeFer, IdObjet::CuirasseDeMailles, IdObjet::JambieresDeFer}) {
        REQUIRE(exigees(fer) == Liste{Caracteristique::Endurance});
    }
}

TEST_CASE("catalogueObjets : les objets de cuir et les bijoux n'ont aucune exigence") {
    for (const IdObjet libre : {IdObjet::GantsDeCuir, IdObjet::BottesDeCuir, IdObjet::CasqueDeCuir, IdObjet::TuniqueMatelassee,
                                IdObjet::CollierDeProtection, IdObjet::AnneauDeProtection, IdObjet::AnneauDePuissance,
                                IdObjet::CollierDEndurance}) {
        REQUIRE(definitionObjet(libre).exigences.empty());
        REQUIRE(seuilsDeLObjet(Objet{libre, Rarete::Commun, std::nullopt, std::nullopt, 9}).empty());
    }
}

TEST_CASE("seuilsAtteints : tous les seuils doivent l'etre, un objet sans seuil l'est toujours") {
    const Objet gants{IdObjet::GantsDeCuir};
    const Objet dagueDeNiveau3{IdObjet::Dague, Rarete::Commun, std::nullopt, std::nullopt, 3};  // Adresse 10
    const int seuil = seuilsDeLObjet(dagueDeNiveau3).front().valeur;

    REQUIRE(seuilsAtteints(gants, Caracteristiques()));
    REQUIRE_FALSE(seuilsAtteints(dagueDeNiveau3, Caracteristiques({0, 0, seuil - 1, 0, 0})));
    REQUIRE(seuilsAtteints(dagueDeNiveau3, Caracteristiques({0, 0, seuil, 0, 0})));       // atteint, pas dépassé
    REQUIRE(seuilsAtteints(dagueDeNiveau3, Caracteristiques({0, 0, seuil + 5, 0, 0})));
    REQUIRE_FALSE(seuilsAtteints(dagueDeNiveau3, Caracteristiques({seuil, seuil, 0, seuil, seuil})));  // pas la bonne caractéristique
}

TEST_CASE("catalogueObjets : la dague perce 30 % de l'armure, aucune autre arme ni les mains nues") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (definition.arme.has_value()) {
            REQUIRE(definition.arme->attaque.penetration == (definition.id == IdObjet::Dague ? 30 : 0));
        }
    }
    REQUIRE(mainsNues().penetration == 0);
}
