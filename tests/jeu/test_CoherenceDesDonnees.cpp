// ./tests/jeu/test_CoherenceDesDonnees.cpp
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

#include "combat/Defense.hpp"
#include "combat/DescriptionDAttaque.hpp"
#include "combat/Element.hpp"
#include "combat/Esquive.hpp"
#include "escouade/Specialite.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/CompositionDeGroupe.hpp"
#include "monde/GenerateurDonjon.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "objets/CatalogueAffixes.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/DonneesDArme.hpp"
#include "objets/GenerationDObjets.hpp"
#include "objets/Objet.hpp"

// Ces tests vérifient les données des catalogues, pas du code : chaque donnée ajoutée plus tard (monstres,
// compositions de groupes) y trouve sa vérification.

TEST_CASE("chaque classe atteint les seuils de tout son equipement de depart") {
    for (const Specialite& classe : catalogue()) {
        for (const ObjetDeDepart& objet : classe.definition.equipementDeDepart) {
            INFO(classe.nom << " porte " << definitionObjet(objet.objet).nom);
            REQUIRE(seuilsAtteints(Objet{objet.objet}, classe.definition.caracteristiques));
        }
    }
}

TEST_CASE("toute classe peut porter tout objet du catalogue au niveau 1") {
    for (const Specialite& classe : catalogue()) {
        for (const DefinitionObjet& modele : catalogueObjets()) {
            INFO(classe.nom << " porte " << modele.nom);
            REQUIRE(seuilsAtteints(Objet{modele.id}, classe.definition.caracteristiques));
        }
    }
}

namespace {
// Les dégâts par seconde visés d'une arme de niveau 1 et de qualité moyenne (valeurs « à tester ») : toutes les armes d'une main
// visent le même, toutes les armes à deux mains aussi. Une arme au-dessus de ce but de plus de 10 % est trop forte pour sa famille ;
// une arme sous le but (la dague, qui perce l'armure) est compensée par autre chose.
constexpr double s_degatsParSecondeUneMain = 23.0;
constexpr double s_degatsParSecondeDeuxMains = 30.0;
constexpr double s_excesPermis = 1.10;

// Les dégâts par seconde du gabarit de cette arme : sa référence, par le temps d'un coup (exécution et recharge moyenne).
double degatsParSeconde(const DonneesDArme& arme) {
    const double recharge = (static_cast<double>(arme.rechargeMin) + static_cast<double>(arme.rechargeMax)) / 2.0;
    return arme.degatsDeReference / (static_cast<double>(arme.tempsExecution) + recharge);
}
}  // namespace

TEST_CASE("aucun gabarit d'arme ne depasse de plus de 10 % les degats par seconde vises de sa famille") {
    for (const DefinitionObjet& modele : catalogueObjets()) {
        if (!modele.arme.has_value()) {
            continue;
        }
        const DonneesDArme& arme = *modele.arme;
        const double vise = arme.mains == 2 ? s_degatsParSecondeDeuxMains : s_degatsParSecondeUneMain;
        INFO(modele.nom << " : " << degatsParSeconde(arme) << " degats par seconde pour " << vise << " vises");
        REQUIRE(degatsParSeconde(arme) <= vise * s_excesPermis);
    }
}

TEST_CASE("la plage de chaque arme contient sa moyenne, a tous les niveaux de 1 a 20 et a toutes les qualites, et son minimum est d'au moins 1") {
    for (const DefinitionObjet& modele : catalogueObjets()) {
        if (!modele.arme.has_value()) {
            continue;
        }
        for (int niveau = 1; niveau <= 20; ++niveau) {
            for (const int qualite : {0, 25, 50, 75, 100}) {
                INFO(modele.nom << ", niveau " << niveau << ", qualite " << qualite);
                const DescriptionDAttaque arme = descriptionDeLArme(Objet{modele.id, Rarete::Commun, std::nullopt, std::nullopt, niveau, qualite});
                // La moyenne se recalcule ici de la façon la plus simple : le gabarit, le niveau, la qualité.
                const double moyenne = modele.arme->degatsDeReference * facteurDeNiveau(niveau) * (0.85 + 0.30 * qualite / 100.0);
                REQUIRE(arme.degats.min.total() >= 1);
                REQUIRE(arme.degats.min.total() <= arme.degats.max.total());
                REQUIRE(arme.degats.min.total() <= static_cast<int>(std::lround(moyenne)));
                REQUIRE(arme.degats.max.total() >= static_cast<int>(std::lround(moyenne)));
            }
        }
    }
}

TEST_CASE("chaque exigence d'objet a une pente d'au moins 1, et aucun objet n'exige deux fois la meme caracteristique") {
    for (const DefinitionObjet& modele : catalogueObjets()) {
        INFO(modele.nom);
        for (std::size_t i = 0; i < modele.exigences.size(); ++i) {
            REQUIRE(modele.exigences[i].pente >= 1);
            for (std::size_t j = i + 1; j < modele.exigences.size(); ++j) {
                REQUIRE(modele.exigences[i].caracteristique != modele.exigences[j].caracteristique);
            }
        }
    }
}

TEST_CASE("chaque definition de monstre est valide") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(categorie);
        INFO("categorie " << static_cast<int>(categorie));
        REQUIRE(definition.cote > 0.0f);
        REQUIRE(definition.vitesse > 0.0f);
        REQUIRE(definition.pointsDeVieMax > 0);
        REQUIRE(definition.armure >= 0);
        REQUIRE(definition.degats.total() > 0);
        REQUIRE(definition.experience >= 1);
        REQUIRE(definition.attaque.rechargeMin > 0.0f);
        REQUIRE(definition.attaque.rechargeMin <= definition.attaque.rechargeMax);
        REQUIRE(definition.attaque.tempsExecution >= 0.0f);
        REQUIRE(definition.statistiques.capaciteOffensive > 0);
        REQUIRE(definition.statistiques.capaciteDefensive > 0);
        REQUIRE(definition.statistiques.esquiveContact >= 0);
        REQUIRE(definition.statistiques.esquiveContact <= s_esquiveMaximale);
        REQUIRE(definition.statistiques.esquiveProjectile >= 0);
        REQUIRE(definition.statistiques.esquiveProjectile <= s_esquiveMaximale);
        REQUIRE(definition.rayonDetection > 0.0f);
        REQUIRE(definition.rayonTerritoire >= definition.rayonDetection);
        REQUIRE(definition.chanceDeButin >= 0);
        REQUIRE(definition.chanceDeButin <= 100);
        REQUIRE((definition.chanceDeButin == 0 || definition.nombreDObjets >= 1));
        for (const Element element : s_elements) {
            INFO("element " << static_cast<int>(element));
            REQUIRE(definition.statistiques.resistances[element] >= s_resistanceMinimale);
            REQUIRE(definition.statistiques.resistances[element] <= s_resistanceMaximale);
        }
    }
}

TEST_CASE("seule la creature monstrueuse resiste au feu, et la resistance physique est nulle partout") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const Resistances& resistances = definitionDeMonstre(categorie).statistiques.resistances;
        INFO("categorie " << static_cast<int>(categorie));
        REQUIRE(resistances[Element::Physique] == 0);
        REQUIRE(resistances[Element::Sacre] == 0);
        REQUIRE(resistances[Element::Feu] == (categorie == CategorieDeMonstre::CreatureMonstrueuse ? 40 : 0));
    }
}

TEST_CASE("deux categories de monstres ne se ressemblent ni par la couleur ni par la taille") {
    for (const CategorieDeMonstre a : s_categoriesDeMonstre) {
        for (const CategorieDeMonstre b : s_categoriesDeMonstre) {
            if (a != b) {
                INFO(static_cast<int>(a) << " et " << static_cast<int>(b));
                REQUIRE(definitionDeMonstre(a).couleur != definitionDeMonstre(b).couleur);
                REQUIRE(definitionDeMonstre(a).cote != definitionDeMonstre(b).cote);
            }
        }
    }
}

TEST_CASE("le cercle de chaque monstre ne touche pas les murs d'un chemin qui suit les centres des cases") {
    // Le chemin d'un monstre passe par les centres des cases : dans un couloir, il reste à une demi-case du mur de
    // sa voie, quelle que soit la largeur du couloir. Un cercle plus grand frotte le mur et se bloque aux coudes.
    const ParametresDonjon parametres;

    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        INFO("categorie " << static_cast<int>(categorie));
        REQUIRE(definitionDeMonstre(categorie).cote / 2.0f < parametres.tailleCase / 2.0f);
    }
}

TEST_CASE("la creature monstrueuse garde une marge de 4 px avec les murs d'un chemin par les centres des cases") {
    constexpr float marge = 4.0f;  // en pixels
    const ParametresDonjon parametres;

    REQUIRE(definitionDeMonstre(CategorieDeMonstre::CreatureMonstrueuse).cote / 2.0f + marge <=
            parametres.tailleCase / 2.0f);
}

namespace {
// Une composition utilisable : au moins un effectif, chacun avec 0 <= min <= max, et au moins un monstre possible.
void verifierComposition(const CompositionDeGroupe& composition) {
    REQUIRE_FALSE(composition.effectifs.empty());
    int maximum = 0;
    for (const EffectifDeCategorie& effectif : composition.effectifs) {
        REQUIRE(effectif.min >= 0);
        REQUIRE(effectif.min <= effectif.max);
        maximum += effectif.max;
    }
    REQUIRE(maximum >= 1);
}
}  // namespace

TEST_CASE("chaque composition de groupe est non vide et coherente") {
    const std::vector<CompositionDeGroupe> compositions = compositionsOrdinaires();

    REQUIRE_FALSE(compositions.empty());
    for (const CompositionDeGroupe& composition : compositions) {
        verifierComposition(composition);
        REQUIRE(composition.poids > 0);
    }
    verifierComposition(compositionDeLaCreature());
}

TEST_CASE("la composition de la creature est une creature seule") {
    const CompositionDeGroupe composition = compositionDeLaCreature();

    REQUIRE(composition.effectifs.size() == 1);
    REQUIRE(composition.effectifs.front().categorie == CategorieDeMonstre::CreatureMonstrueuse);
    REQUIRE(composition.effectifs.front().min == 1);
    REQUIRE(composition.effectifs.front().max == 1);
}

TEST_CASE("les parametres du generateur portent les compositions du jeu") {
    const ParametresDonjon parametres;

    REQUIRE(parametres.compositions.size() == compositionsOrdinaires().size());
    REQUIRE(parametres.compositionDeLaSalleLaPlusLoin.has_value());
}

TEST_CASE("la plage de valeurs de chaque affixe est positive et ordonnee") {
    for (const DefinitionDAffixe& affixe : catalogueAffixes()) {
        INFO(affixe.nom);
        REQUIRE(affixe.valeurs.min >= 1);
        REQUIRE(affixe.valeurs.min <= affixe.valeurs.max);
    }
}

TEST_CASE("chaque rarete a un poids positif a chaque niveau jusqu'a 20, et au moins un modele qui peut la porter") {
    for (const Rarete rarete : s_raretes) {
        INFO("rarete " << static_cast<int>(rarete));
        for (int niveau = 1; niveau <= 20; ++niveau) {
            INFO("niveau " << niveau);
            REQUIRE(poidsDeRarete(rarete, niveau) > 0);
        }
        bool possible = false;
        for (const DefinitionObjet& modele : catalogueObjets()) {
            possible = possible || peutPorterLaRarete(modele.id, rarete);
        }
        REQUIRE(possible);
    }
}

TEST_CASE("chaque affixe du catalogue peut etre porte par au moins un modele du catalogue d'objets") {
    for (const DefinitionDAffixe& affixe : catalogueAffixes()) {
        INFO(affixe.nom);
        bool porte = false;
        for (const DefinitionObjet& modele : catalogueObjets()) {
            porte = porte || affixePermis(affixe.id, modele.categorie);
        }
        REQUIRE(porte);
    }
}
