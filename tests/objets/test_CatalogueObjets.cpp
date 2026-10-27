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
    REQUIRE(static_cast<std::size_t>(IdObjet::Arbalete) == catalogue.size() - 1);
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
    const Objet masseFlamboyante{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}};

    REQUIRE(Objet{IdObjet::GantsDeCuir} == Objet{IdObjet::GantsDeCuir});
    REQUIRE(Objet{IdObjet::GantsDeCuir} != Objet{IdObjet::BottesDeCuir});
    REQUIRE(masseFlamboyante == Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}});
    REQUIRE(masseFlamboyante != Objet{IdObjet::MasseAUneMain});                                                    // sans affixe
    REQUIRE(masseFlamboyante != Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 7}});  // autre valeur
    REQUIRE(masseFlamboyante != Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Sanctifie, 6}});   // autre affixe
    REQUIRE(masseFlamboyante != Objet{IdObjet::MasseAUneMain, Rarete::Rare, Affixe{IdAffixe::Flamboyant, 6}});     // autre rareté
    REQUIRE(masseFlamboyante != Objet{IdObjet::MasseAUneMain, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuLion, 6}});
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
    const Objet base{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 3, 70};

    REQUIRE(base == Objet{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 3, 70});
    REQUIRE(base != Objet{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 4, 70});  // autre niveau
    REQUIRE(base != Objet{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 3, 71});  // autre qualité
    REQUIRE(base != Objet{IdObjet::MasseAUneMain});                                                      // niveau 1, qualité 50
}

TEST_CASE("objetValide : le niveau est d'au moins 1 et la qualite va de 0 a 100") {
    const auto objet = [](int niveau, int qualite) {
        return Objet{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, niveau, qualite};
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
    REQUIRE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}));
    REQUIRE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique}));  // aucun affixe
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}, Affixe{IdAffixe::DuBoeuf, 2}}));
}

TEST_CASE("objetValide : un objet rare a un prefixe et un suffixe") {
    REQUIRE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, std::nullopt, Affixe{IdAffixe::DuRenard, 2}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare}));
}

TEST_CASE("objetValide : un objet commun n'a aucun affixe") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Commun, Affixe{IdAffixe::Flamboyant, 6}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, Affixe{IdAffixe::DuBoeuf, 2}}));
}

TEST_CASE("objetValide : un affixe est a sa position, un prefixe dans prefixe et un suffixe dans suffixe") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::DuBoeuf, 2}}));                    // suffixe en préfixe
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, std::nullopt, Affixe{IdAffixe::Flamboyant, 6}}));  // préfixe en suffixe
    REQUIRE_FALSE(objetValide(Objet{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::DuRenard, 2}, Affixe{IdAffixe::Acere, 15}}));  // inversés
}

TEST_CASE("objetValide : un affixe est permis pour la categorie du modele") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::CasqueDeFer, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}));  // du feu sur un casque
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Renforce, 2}}));           // de l'armure sur une arme
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeBlocage, 5}}));  // du blocage sur une arme
    REQUIRE_FALSE(objetValide(Objet{IdObjet::BouclierRond, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DEsquive, 3}}));  // pas sur un bouclier
    REQUIRE(objetValide(Objet{IdObjet::BouclierRond, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeBlocage, 5}}));
    REQUIRE(objetValide(Objet{IdObjet::CuirasseDeMailles, Rarete::Rare, Affixe{IdAffixe::Renforce, 3}, Affixe{IdAffixe::DEsquive, 3}}));
    REQUIRE(objetValide(Objet{IdObjet::AnneauDeProtection, Rarete::Magique, std::nullopt, Affixe{IdAffixe::DeLaChouette, 2}}));
}

TEST_CASE("objetValide : la valeur d'un affixe est strictement positive") {
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 0}}));
    REQUIRE_FALSE(objetValide(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, -6}}));
}

TEST_CASE("nomDeLObjet : prefixe, modele, suffixe") {
    REQUIRE(nomDeLObjet(Objet{IdObjet::MasseAUneMain}) == "Mace");
    REQUIRE(nomDeLObjet(Objet{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}}) == "Blazing Mace");
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
// Les gabarits du catalogue des armes, tels que le tableau de l'étape 102 les donne (le test du catalogue est l'un des rares à
// les lire). La référence est les dégâts moyens d'un objet de niveau 1 et de qualité moyenne.
struct GabaritAttendu {
    IdObjet id;
    TypeDArme type;
    int mains;
    NatureDAttaque nature;
    float portee;
    float execution;
    float rechargeMin;
    float rechargeMax;
    int reference;
    int ecart;
    int penetration;
    std::vector<ExigenceDeCaracteristique> exigences;
};

const std::vector<GabaritAttendu>& gabaritsAttendus() {
    constexpr NatureDAttaque contact = NatureDAttaque::Contact;
    constexpr NatureDAttaque projectile = NatureDAttaque::Projectile;
    static const std::vector<GabaritAttendu> gabarits = {
        {IdObjet::Dague, TypeDArme::Dague, 1, contact, 20.0f, 0.15f, 0.20f, 0.50f, 10, 10, 15, {{Caracteristique::Adresse, 2}}},
        {IdObjet::EpeeAUneMain, TypeDArme::EpeeAUneMain, 1, contact, 20.0f, 0.15f, 0.30f, 0.70f, 15, 15, 0, {{Caracteristique::Puissance, 2}}},
        {IdObjet::HacheAUneMain, TypeDArme::HacheAUneMain, 1, contact, 20.0f, 0.15f, 0.40f, 1.00f, 20, 25, 0, {{Caracteristique::Puissance, 2}}},
        {IdObjet::MasseAUneMain, TypeDArme::MasseAUneMain, 1, contact, 20.0f, 0.15f, 0.60f, 1.40f, 26, 40, 0, {{Caracteristique::Puissance, 2}}},
        {IdObjet::Lance, TypeDArme::Lance, 1, contact, 35.0f, 0.15f, 0.70f, 1.45f, 28, 25, 30,
         {{Caracteristique::Adresse, 2}, {Caracteristique::Puissance, 1}}},
        {IdObjet::EpeeADeuxMains, TypeDArme::EpeeADeuxMains, 2, contact, 20.0f, 0.30f, 0.50f, 1.20f, 35, 15, 0,
         {{Caracteristique::Puissance, 2}, {Caracteristique::Adresse, 1}}},
        {IdObjet::HacheADeuxMains, TypeDArme::HacheADeuxMains, 2, contact, 20.0f, 0.30f, 0.80f, 1.80f, 48, 25, 0, {{Caracteristique::Puissance, 2}}},
        {IdObjet::MasseADeuxMains, TypeDArme::MasseADeuxMains, 2, contact, 20.0f, 0.30f, 0.80f, 1.80f, 48, 40, 0,
         {{Caracteristique::Puissance, 2}, {Caracteristique::Endurance, 1}}},
        {IdObjet::Baguette, TypeDArme::Baguette, 1, projectile, 120.0f, 0.15f, 0.30f, 0.70f, 15, 15, 0, {{Caracteristique::Savoir, 2}}},
        {IdObjet::Sceptre, TypeDArme::Sceptre, 1, projectile, 120.0f, 0.15f, 0.40f, 1.00f, 20, 15, 0, {{Caracteristique::Volonte, 2}}},
        {IdObjet::Baton, TypeDArme::Baton, 2, projectile, 120.0f, 0.30f, 0.55f, 0.85f, 30, 25, 0, {{Caracteristique::Savoir, 2}}},
        {IdObjet::Arc, TypeDArme::Arc, 2, projectile, 120.0f, 0.30f, 0.35f, 0.55f, 23, 25, 15, {{Caracteristique::Adresse, 2}}},
        {IdObjet::Arbalete, TypeDArme::Arbalete, 2, projectile, 140.0f, 0.30f, 0.55f, 0.85f, 30, 15, 30,
         {{Caracteristique::Adresse, 2}, {Caracteristique::Puissance, 1}}},
    };
    return gabarits;
}
}  // namespace

TEST_CASE("catalogueObjets : les treize gabarits d'armes, leurs types, leurs mains, leur vitesse, leur portee et leurs exigences") {
    REQUIRE(gabaritsAttendus().size() == 13);

    for (const GabaritAttendu& attendu : gabaritsAttendus()) {
        const DefinitionObjet& definition = definitionObjet(attendu.id);
        INFO(definition.nom);
        REQUIRE(definition.categorie == CategorieObjet::Arme);
        REQUIRE(definition.armure == 0);
        REQUIRE(definition.arme.has_value());
        const DonneesDArme& arme = *definition.arme;  // NOLINT(bugprone-unchecked-optional-access): vérifié par le REQUIRE au-dessus
        REQUIRE(arme.type == attendu.type);
        REQUIRE(arme.mains == attendu.mains);
        REQUIRE(arme.nature == attendu.nature);
        REQUIRE(arme.portee == attendu.portee);
        REQUIRE(arme.tempsExecution == attendu.execution);
        REQUIRE(arme.rechargeMin == attendu.rechargeMin);
        REQUIRE(arme.rechargeMax == attendu.rechargeMax);
        REQUIRE(arme.degatsDeReference == attendu.reference);
        REQUIRE(arme.ecartPourCent == attendu.ecart);
        REQUIRE(arme.penetration == attendu.penetration);
        REQUIRE(definition.exigences == attendu.exigences);
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
    REQUIRE(types.size() == 13);
    for (std::size_t i = 0; i < types.size(); ++i) {
        for (std::size_t j = i + 1; j < types.size(); ++j) {
            REQUIRE(types[i] != types[j]);
        }
    }
}

TEST_CASE("estADeuxMains : les armes a deux mains, pas les autres armes ni les autres objets") {
    for (const GabaritAttendu& attendu : gabaritsAttendus()) {
        INFO(definitionObjet(attendu.id).nom);
        REQUIRE(estADeuxMains(attendu.id) == (attendu.mains == 2));
    }
    REQUIRE(estADeuxMains(IdObjet::Arbalete));
    REQUIRE(estADeuxMains(IdObjet::MasseADeuxMains));
    REQUIRE_FALSE(estADeuxMains(IdObjet::Lance));  // une lance se tient d'une main
    REQUIRE_FALSE(estADeuxMains(IdObjet::BouclierRond));
}

TEST_CASE("descriptionDeLArme : l'epee a une main de niveau 1 et de qualite moyenne fait 13 a 17, a 0,3 - 0,7 s") {
    const DescriptionDAttaque epee = descriptionDeLArme(Objet{IdObjet::EpeeAUneMain});

    REQUIRE(epee.degats.min == degatsPhysiques(13));
    REQUIRE(epee.degats.max == degatsPhysiques(17));
    REQUIRE(epee.rechargeMin == 0.30f);
    REQUIRE(epee.rechargeMax == 0.70f);
    REQUIRE(epee.tempsExecution == 0.15f);
    REQUIRE(epee.portee == 20.0f);
    REQUIRE(epee.nature == NatureDAttaque::Contact);
    REQUIRE(epee.penetration == 0);
}

TEST_CASE("descriptionDeLArme : la moyenne des degats suit le niveau (x 2 au niveau 5) et la qualite (de 0,85 a 1,15)") {
    const auto epee = [](int niveau, int qualite) {
        return descriptionDeLArme(Objet{IdObjet::EpeeAUneMain, Rarete::Commun, std::nullopt, std::nullopt, niveau, qualite});
    };

    REQUIRE(epee(5, 50).degats.min == degatsPhysiques(26));  // 30 de moyenne : 25,5 arrondi à 26
    REQUIRE(epee(5, 50).degats.max == degatsPhysiques(35));  // 34,5 arrondi à 35
    REQUIRE(epee(1, 0).degats.min == degatsPhysiques(11));   // 12,75 de moyenne
    REQUIRE(epee(1, 0).degats.max == degatsPhysiques(15));
    REQUIRE(epee(1, 100).degats.min == degatsPhysiques(15));  // 17,25 de moyenne
    REQUIRE(epee(1, 100).degats.max == degatsPhysiques(20));
}

TEST_CASE("descriptionDeLArme : une plage de niveau superieur ou de meilleure qualite n'est jamais plus basse") {
    for (const GabaritAttendu& attendu : gabaritsAttendus()) {
        INFO(definitionObjet(attendu.id).nom);
        int minPrecedent = 0;
        int maxPrecedent = 0;
        for (int niveau = 1; niveau <= 20; ++niveau) {
            const DescriptionDAttaque arme = descriptionDeLArme(Objet{attendu.id, Rarete::Commun, std::nullopt, std::nullopt, niveau});
            REQUIRE(arme.degats.min.total() >= minPrecedent);
            REQUIRE(arme.degats.max.total() >= maxPrecedent);
            minPrecedent = arme.degats.min.total();
            maxPrecedent = arme.degats.max.total();
        }
        const auto selonLaQualite = [&](int qualite) {
            return descriptionDeLArme(Objet{attendu.id, Rarete::Commun, std::nullopt, std::nullopt, 3, qualite});
        };
        REQUIRE(selonLaQualite(0).degats.min.total() <= selonLaQualite(50).degats.min.total());
        REQUIRE(selonLaQualite(50).degats.min.total() <= selonLaQualite(100).degats.min.total());
        REQUIRE(selonLaQualite(0).degats.max.total() <= selonLaQualite(50).degats.max.total());
        REQUIRE(selonLaQualite(50).degats.max.total() <= selonLaQualite(100).degats.max.total());
    }
}

TEST_CASE("descriptionDeLArme : la recharge, l'execution, la portee, la nature et la penetration ne dependent ni du niveau ni de la qualite") {
    for (const GabaritAttendu& attendu : gabaritsAttendus()) {
        INFO(definitionObjet(attendu.id).nom);
        const DescriptionDAttaque depart = descriptionDeLArme(Objet{attendu.id});
        for (const int niveau : {1, 5, 20}) {
            for (const int qualite : {0, 50, 100}) {
                const DescriptionDAttaque arme = descriptionDeLArme(Objet{attendu.id, Rarete::Commun, std::nullopt, std::nullopt, niveau, qualite});
                REQUIRE(arme.rechargeMin == depart.rechargeMin);
                REQUIRE(arme.rechargeMax == depart.rechargeMax);
                REQUIRE(arme.tempsExecution == depart.tempsExecution);
                REQUIRE(arme.portee == depart.portee);
                REQUIRE(arme.nature == depart.nature);
                REQUIRE(arme.penetration == depart.penetration);
            }
        }
    }
}

TEST_CASE("descriptionDeLArme : ni la rarete ni les affixes ne changent la plage de l'arme") {
    const Objet commune{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 3, 70};
    const Objet magique{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}, std::nullopt, 3, 70};

    REQUIRE(descriptionDeLArme(magique) == descriptionDeLArme(commune));
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
    const Objet simple{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 3, 50};
    const Objet magique{IdObjet::MasseAUneMain, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}, std::nullopt, 3, 50};
    const Objet excellente{IdObjet::MasseAUneMain, Rarete::Commun, std::nullopt, std::nullopt, 3, 100};

    REQUIRE(seuilsDeLObjet(magique) == seuilsDeLObjet(simple));
    REQUIRE(seuilsDeLObjet(excellente) == seuilsDeLObjet(simple));
}

TEST_CASE("seuilsDeLObjet : une arme a deux exigences les a toutes, la principale monte deux fois plus vite que la secondaire") {
    const Objet lance{IdObjet::Lance, Rarete::Commun, std::nullopt, std::nullopt, 5};  // Adresse +2 par niveau, Puissance +1

    REQUIRE(seuilsDeLObjet(lance) == std::vector<SeuilDeCaracteristique>{{Caracteristique::Adresse, 14}, {Caracteristique::Puissance, 10}});
    REQUIRE(seuilsDeLObjet(Objet{IdObjet::Lance}) ==
            std::vector<SeuilDeCaracteristique>{{Caracteristique::Adresse, 6}, {Caracteristique::Puissance, 6}});
}

TEST_CASE("seuilsDeLObjet : les protections de fer et le bouclier exigent l'Endurance") {
    for (const IdObjet fer : {IdObjet::CasqueDeFer, IdObjet::CuirasseDeMailles, IdObjet::JambieresDeFer, IdObjet::BouclierRond}) {
        const std::vector<SeuilDeCaracteristique> seuils = seuilsDeLObjet(Objet{fer});
        REQUIRE(seuils.size() == 1);
        REQUIRE(seuils.front().caracteristique == Caracteristique::Endurance);
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

TEST_CASE("catalogueObjets : la dague, l'arc, la lance et l'arbalete percent l'armure, aucune autre arme ni les mains nues") {
    for (const GabaritAttendu& attendu : gabaritsAttendus()) {
        const bool perce = attendu.id == IdObjet::Dague || attendu.id == IdObjet::Arc || attendu.id == IdObjet::Lance ||
                           attendu.id == IdObjet::Arbalete;
        INFO(definitionObjet(attendu.id).nom);
        REQUIRE((descriptionDeLArme(Objet{attendu.id}).penetration > 0) == perce);
    }
    REQUIRE(mainsNues().penetration == 0);
}

TEST_CASE("mainsNues : 6 de degats sans ecart, 0,4 - 0,6 s, contact") {
    const DescriptionDAttaque& mains = mainsNues();

    REQUIRE(mains.degats == plageFixe(degatsPhysiques(6)));
    REQUIRE(mains.rechargeMin == 0.4f);
    REQUIRE(mains.rechargeMax == 0.6f);
    REQUIRE(mains.nature == NatureDAttaque::Contact);
}
