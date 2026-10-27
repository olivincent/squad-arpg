// ./tests/objets/test_GenerationDObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/Aleatoire.hpp"
#include "objets/CatalogueAffixes.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/GenerationDObjets.hpp"

// Les objets tirés (étape 96) : les poids, la règle « ce modèle peut-il porter cette rareté » et `genererObjet`. Les tirages
// utilisent une graine fixe : un test qui échoue échoue toujours de la même façon.

TEST_CASE("poidsDeRarete : au niveau 1, commun 70, magique 25, rare 5") {
    REQUIRE(poidsDeRarete(Rarete::Commun, 1) == 70);
    REQUIRE(poidsDeRarete(Rarete::Magique, 1) == 25);
    REQUIRE(poidsDeRarete(Rarete::Rare, 1) == 5);
}

TEST_CASE("poidsDeRarete : au niveau 5, commun 50, magique 37, rare 13") {
    REQUIRE(poidsDeRarete(Rarete::Commun, 5) == 50);
    REQUIRE(poidsDeRarete(Rarete::Magique, 5) == 37);
    REQUIRE(poidsDeRarete(Rarete::Rare, 5) == 13);
}

TEST_CASE("poidsDeRarete : le commun recule de 5 par niveau jusqu'a son plancher de 20, le magique et le rare avancent sans limite") {
    REQUIRE(poidsDeRarete(Rarete::Commun, 10) == 25);
    REQUIRE(poidsDeRarete(Rarete::Commun, 11) == 20);  // le plancher est atteint
    REQUIRE(poidsDeRarete(Rarete::Commun, 12) == 20);  // et respecte : sans lui, 15
    REQUIRE(poidsDeRarete(Rarete::Commun, 20) == 20);  // sans lui, -25
    REQUIRE(poidsDeRarete(Rarete::Magique, 20) == 25 + 3 * 19);
    REQUIRE(poidsDeRarete(Rarete::Rare, 20) == 5 + 2 * 19);
}

TEST_CASE("peutPorterLaRarete : tous les modeles peuvent etre communs") {
    for (const DefinitionObjet& modele : catalogueObjets()) {
        INFO(modele.nom);
        REQUIRE(peutPorterLaRarete(modele.id, Rarete::Commun));
    }
}

TEST_CASE("peutPorterLaRarete : un bijou peut etre magique (un suffixe) mais jamais rare (aucun prefixe)") {
    for (const IdObjet bijou : {IdObjet::AnneauDeProtection, IdObjet::AnneauDePuissance, IdObjet::CollierDeProtection,
                                IdObjet::CollierDEndurance}) {
        INFO(definitionObjet(bijou).nom);
        REQUIRE(peutPorterLaRarete(bijou, Rarete::Magique));
        REQUIRE_FALSE(peutPorterLaRarete(bijou, Rarete::Rare));
    }
}

TEST_CASE("peutPorterLaRarete : les gants et les bottes peuvent etre rares (Reinforced), comme les armes et les armures") {
    for (const IdObjet modele : {IdObjet::GantsDeTissu, IdObjet::Chaussons, IdObjet::HeaumeIntegral, IdObjet::ArmureDePlaques,
                                 IdObjet::Bouclier, IdObjet::Arc, IdObjet::MasseAUneMain}) {
        INFO(definitionObjet(modele).nom);
        REQUIRE(peutPorterLaRarete(modele, Rarete::Magique));
        REQUIRE(peutPorterLaRarete(modele, Rarete::Rare));
    }
}

namespace {
constexpr unsigned int s_graine = 96;
constexpr int s_tirages = 10000;

// Les objets tirés d'un générateur de graine fixe.
std::vector<Objet> tirer(int nombre, unsigned int graine = s_graine, int niveau = 1) {
    Generateur generateur(graine);
    std::vector<Objet> objets;
    for (int i = 0; i < nombre; ++i) {
        objets.push_back(genererObjet(generateur, niveau));
    }
    return objets;
}

bool estUnBijou(IdObjet modele) {
    const CategorieObjet categorie = definitionObjet(modele).categorie;
    return categorie == CategorieObjet::Anneau || categorie == CategorieObjet::Collier;
}

bool estDesGantsOuDesBottes(IdObjet modele) {
    const CategorieObjet categorie = definitionObjet(modele).categorie;
    return categorie == CategorieObjet::Gants || categorie == CategorieObjet::Bottes;
}

// La valeur de cet affixe est-elle dans sa plage, à ce niveau ?
bool dansLaPlage(const Affixe& affixe, int niveau = 1) {
    const PlageDeValeurs plage = plageAuNiveau(affixe.id, niveau);
    return affixe.valeur >= plage.min && affixe.valeur <= plage.max;
}
}  // namespace

TEST_CASE("genererObjet : 10000 tirages, tous valides") {
    for (const Objet& objet : tirer(s_tirages)) {
        INFO(nomDeLObjet(objet));
        REQUIRE(objetValide(objet));
    }
}

TEST_CASE("genererObjet : chaque rarete, chaque modele et chaque affixe apparaissent, chaque valeur reste dans sa plage") {
    std::set<Rarete> raretes;
    std::set<IdAffixe> affixes;
    for (const Objet& objet : tirer(s_tirages)) {
        raretes.insert(objet.rarete);
        for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
            if (affixe.has_value()) {
                affixes.insert(affixe->id);
                INFO(nomDeLObjet(objet));
                REQUIRE(dansLaPlage(*affixe));
            }
        }
    }
    REQUIRE(raretes.size() == s_raretes.size());
    REQUIRE(affixes.size() == catalogueAffixes().size());
}

TEST_CASE("genererObjet : a chaque niveau de 1 a 5, tous les modeles deja apparus sortent, et eux seuls") {
    for (int niveau = 1; niveau <= 5; ++niveau) {
        std::set<IdObjet> sortis;
        std::set<IdObjet> attendus;
        for (const Objet& objet : tirer(s_tirages, s_graine, niveau)) {
            sortis.insert(objet.modele);
        }
        for (const DefinitionObjet& definition : catalogueObjets()) {
            if (definition.niveauDApparition <= niveau) {
                attendus.insert(definition.id);
            }
        }
        INFO("niveau " << niveau);
        REQUIRE(sortis == attendus);
    }
}

TEST_CASE("genererObjet : les bornes de chaque plage sortent") {
    std::set<int> valeursDeFeu;
    for (const Objet& objet : tirer(s_tirages)) {
        for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
            if (affixe.has_value() && affixe->id == IdAffixe::Flamboyant) {
                valeursDeFeu.insert(affixe->valeur);
            }
        }
    }
    const PlageDeValeurs& plage = definitionAffixe(IdAffixe::Flamboyant).valeurs;
    REQUIRE(valeursDeFeu.size() == static_cast<std::size_t>(plage.max - plage.min + 1));
    REQUIRE(*valeursDeFeu.begin() == plage.min);
    REQUIRE(*valeursDeFeu.rbegin() == plage.max);
}

TEST_CASE("genererObjet : un anneau ou un collier n'est jamais rare, des gants et des bottes peuvent l'etre") {
    bool gantsOuBottesRares = false;
    for (const Objet& objet : tirer(s_tirages)) {
        if (estUnBijou(objet.modele)) {
            REQUIRE(objet.rarete != Rarete::Rare);
        }
        if (estDesGantsOuDesBottes(objet.modele) && objet.rarete == Rarete::Rare) {
            gantsOuBottesRares = true;
            REQUIRE(objet.prefixe->id == IdAffixe::Renforce);  // leur seul prefixe
        }
    }
    REQUIRE(gantsOuBottesRares);
}

namespace {
// Les proportions de raretés de 10 000 objets de ce niveau sont celles de `poidsDeRarete`, à deux points près. Sur 10 000
// tirages, l'écart type d'une proportion est d'au plus 0,5 point : deux points, c'est plus de quatre écarts types. La graine
// est fixe, donc la tolérance n'est pas là pour absorber le hasard : elle détecte un algorithme modifié (poids changés,
// rareté retirée après coup parce que le modèle ne convient pas…), qui déplacerait une proportion de plus de deux points.
void verifierLesProportions(int niveau) {
    constexpr double tolerance = 0.02;
    int poidsTotal = 0;
    for (const Rarete rarete : s_raretes) {
        poidsTotal += poidsDeRarete(rarete, niveau);
    }
    std::array<int, s_raretes.size()> compte = {};  // indexé par la valeur de Rarete
    for (const Objet& objet : tirer(s_tirages, s_graine, niveau)) {
        ++compte[static_cast<std::size_t>(objet.rarete)];
    }
    for (const Rarete rarete : s_raretes) {
        const double attendu = static_cast<double>(poidsDeRarete(rarete, niveau)) / poidsTotal;
        const double observe = static_cast<double>(compte[static_cast<std::size_t>(rarete)]) / s_tirages;
        INFO("niveau " << niveau << ", rarete " << static_cast<int>(rarete) << " : attendu " << attendu << ", observe " << observe);
        REQUIRE(std::abs(observe - attendu) <= tolerance);
    }
}
}  // namespace

TEST_CASE("genererObjet : les proportions de raretes sont celles des poids, a deux points pres") {
    verifierLesProportions(1);
}

TEST_CASE("genererObjet : au niveau 5, les proportions sont 50 / 37 / 13, a deux points pres") {
    verifierLesProportions(5);
}

TEST_CASE("genererObjet : a un niveau eleve, le commun garde son plancher (20 sur 145) et les proportions suivent") {
    verifierLesProportions(20);
}

TEST_CASE("familleDe : les armes, l'armure, la main gauche et les bijoux") {
    REQUIRE(familleDe(CategorieObjet::Arme) == FamilleDeButin::Arme);
    for (const CategorieObjet categorie : {CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres,
                                           CategorieObjet::Gants, CategorieObjet::Bottes}) {
        REQUIRE(familleDe(categorie) == FamilleDeButin::Armure);
    }
    REQUIRE(familleDe(CategorieObjet::Bouclier) == FamilleDeButin::MainGauche);
    REQUIRE(familleDe(CategorieObjet::Relique) == FamilleDeButin::MainGauche);
    REQUIRE(familleDe(CategorieObjet::Collier) == FamilleDeButin::Bijou);
    REQUIRE(familleDe(CategorieObjet::Anneau) == FamilleDeButin::Bijou);
}

TEST_CASE("poidsDeFamille : arme 35, armure 35, main gauche 15, bijou 15") {
    REQUIRE(poidsDeFamille(FamilleDeButin::Arme) == 35);
    REQUIRE(poidsDeFamille(FamilleDeButin::Armure) == 35);
    REQUIRE(poidsDeFamille(FamilleDeButin::MainGauche) == 15);
    REQUIRE(poidsDeFamille(FamilleDeButin::Bijou) == 15);
}

namespace {
// Les proportions de familles des objets de cette rareté, tirés à ce niveau, sont celles de `poidsDeFamille` parmi les familles
// éligibles (qui ont au moins un modèle apparu à ce niveau et capable de porter la rareté), à trois points près. La graine est
// fixe : la tolérance détecte un algorithme modifié (famille tirée après le modèle, poids changés…).
void verifierLesFamilles(Rarete rarete, int niveau) {
    constexpr int tirages = 20000;
    constexpr double tolerance = 0.03;
    std::array<bool, s_famillesDeButin.size()> eligible = {};
    for (const DefinitionObjet& definition : catalogueObjets()) {
        if (definition.niveauDApparition <= niveau && peutPorterLaRarete(definition.id, rarete)) {
            eligible[static_cast<std::size_t>(familleDe(definition.categorie))] = true;
        }
    }
    int poidsTotal = 0;
    for (const FamilleDeButin famille : s_famillesDeButin) {
        poidsTotal += eligible[static_cast<std::size_t>(famille)] ? poidsDeFamille(famille) : 0;
    }
    std::array<int, s_famillesDeButin.size()> compte = {};
    int total = 0;
    for (const Objet& objet : tirer(tirages, s_graine, niveau)) {
        if (objet.rarete == rarete) {
            ++compte[static_cast<std::size_t>(familleDe(definitionObjet(objet.modele).categorie))];
            ++total;
        }
    }
    REQUIRE(total > 500);
    for (const FamilleDeButin famille : s_famillesDeButin) {
        const std::size_t indice = static_cast<std::size_t>(famille);
        const double attendu = eligible[indice] ? static_cast<double>(poidsDeFamille(famille)) / poidsTotal : 0.0;
        const double observe = static_cast<double>(compte[indice]) / total;
        INFO("niveau " << niveau << ", rarete " << static_cast<int>(rarete) << ", famille " << indice << " : attendu " << attendu
                       << ", observe " << observe);
        REQUIRE(std::abs(observe - attendu) <= tolerance);
    }
}
}  // namespace

TEST_CASE("genererObjet : au niveau 1, les objets communs sont 35 / 35 / 15 / 15 % par famille") {
    verifierLesFamilles(Rarete::Commun, 1);
}

TEST_CASE("genererObjet : au niveau 5, les memes proportions de familles malgre les pieces intermediaires et lourdes debloquees") {
    verifierLesFamilles(Rarete::Commun, 5);
}

TEST_CASE("genererObjet : les objets magiques gardent les quatre familles, a tout niveau") {
    verifierLesFamilles(Rarete::Magique, 1);
    verifierLesFamilles(Rarete::Magique, 5);
}

TEST_CASE("genererObjet : un objet rare n'est jamais un bijou ni une relique, la main gauche est alors un bouclier") {
    bool unBouclierRare = false;
    for (const Objet& objet : tirer(s_tirages, s_graine, 5)) {
        if (objet.rarete != Rarete::Rare) {
            continue;
        }
        const CategorieObjet categorie = definitionObjet(objet.modele).categorie;
        INFO(nomDeLObjet(objet));
        REQUIRE(familleDe(categorie) != FamilleDeButin::Bijou);
        REQUIRE(categorie != CategorieObjet::Relique);
        unBouclierRare = unBouclierRare || categorie == CategorieObjet::Bouclier;
    }
    REQUIRE(unBouclierRare);
    verifierLesFamilles(Rarete::Rare, 5);  // les proportions suivent les familles éligibles : 35 / 35 / 15 sur 85
}

TEST_CASE("genererObjet : la meme graine donne la meme suite d'objets, une autre graine une autre suite") {
    REQUIRE(tirer(200) == tirer(200));
    REQUIRE_FALSE(tirer(200) == tirer(200, s_graine + 1));
}

namespace {
// Un objet en une ligne : modèle, rareté, niveau, qualité, puis préfixe et suffixe éventuels (identifiant et valeur).
std::string decrire(const Objet& objet) {
    std::string texte = std::to_string(static_cast<int>(objet.modele)) + "/" + std::to_string(static_cast<int>(objet.rarete)) +
                        " n" + std::to_string(objet.niveau) + " q" + std::to_string(objet.qualite);
    for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
        texte += affixe.has_value() ? " " + std::to_string(static_cast<int>(affixe->id)) + "=" + std::to_string(affixe->valeur) : " -";
    }
    return texte;
}
}  // namespace

TEST_CASE("genererObjet : la suite d'objets de la graine 96 au niveau 1 est epinglee (qualite tiree apres le modele, vingt pieces d'armure et treize gabarits d'armes)") {
    // Épinglée avec la bibliothèque standard de MinGW (`std::uniform_int_distribution` n'est pas défini par la norme), à
    // réépingler si elle change ou si l'ordre des tirages change (rareté, famille, modèle, qualité, affixes : étape 104) :
    // modèle (`IdObjet`) / rareté (`Rarete`), niveau, qualité, préfixe, suffixe (`IdAffixe` = valeur).
    const std::vector<std::string> attendu = {
        "9/0 n1 q90 - -",      // bottes de cuir, commun
        "38/1 n1 q70 - 6=2",   // arc, magique, of the Fox
        "0/0 n1 q46 - -",      // robe, commun
        "34/0 n1 q72 - -", "31/0 n1 q93 - -", "25/0 n1 q67 - -", "30/0 n1 q54 - -", "27/0 n1 q89 - -",
    };
    std::vector<std::string> obtenu;
    for (const Objet& objet : tirer(static_cast<int>(attendu.size()))) {
        obtenu.push_back(decrire(objet));
    }
    REQUIRE(obtenu == attendu);

    // Le nombre de tirages : après 200 objets, le prochain nombre du générateur est toujours le même.
    Generateur generateur(s_graine);
    for (int i = 0; i < 200; ++i) {
        static_cast<void>(genererObjet(generateur, 1));
    }
    REQUIRE(generateur() == 259422132U);
}

TEST_CASE("genererObjet : l'objet retient le niveau recu et sa qualite va de 0 a 100, les deux bornes sortent") {
    for (const int niveau : {1, 5}) {
        std::set<int> qualites;
        for (const Objet& objet : tirer(s_tirages, s_graine, niveau)) {
            REQUIRE(objet.niveau == niveau);
            REQUIRE(objet.qualite >= 0);
            REQUIRE(objet.qualite <= 100);
            qualites.insert(objet.qualite);
        }
        REQUIRE(*qualites.begin() == 0);
        REQUIRE(*qualites.rbegin() == 100);
    }
}

TEST_CASE("genererObjet : aucun modele n'est tire sous son niveau d'apparition, a chaque niveau de 1 a 5") {
    for (int niveau = 1; niveau <= 5; ++niveau) {
        for (const Objet& objet : tirer(s_tirages, s_graine, niveau)) {
            INFO("niveau " << niveau << ", " << definitionObjet(objet.modele).nom);
            REQUIRE(definitionObjet(objet.modele).niveauDApparition <= niveau);
        }
    }
}

TEST_CASE("genererObjet : au niveau 5, 10000 tirages restent valides, chaque valeur est dans la plage agrandie, certaines depassent l'ancien maximum") {
    bool depasseLAncienMaximum = false;
    for (const Objet& objet : tirer(s_tirages, s_graine, 5)) {
        INFO(nomDeLObjet(objet));
        REQUIRE(objetValide(objet));
        for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
            if (affixe.has_value()) {
                REQUIRE(dansLaPlage(*affixe, 5));
                depasseLAncienMaximum = depasseLAncienMaximum || affixe->valeur > definitionAffixe(affixe->id).valeurs.max;
            }
        }
    }
    REQUIRE(depasseLAncienMaximum);
}

TEST_CASE("genererObjet : au niveau 5, les bornes de la plage agrandie sortent (Blazing de 6 a 16)") {
    std::set<int> valeursDeFeu;
    for (const Objet& objet : tirer(s_tirages, s_graine, 5)) {
        for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
            if (affixe.has_value() && affixe->id == IdAffixe::Flamboyant) {
                valeursDeFeu.insert(affixe->valeur);
            }
        }
    }
    REQUIRE(*valeursDeFeu.begin() == 6);
    REQUIRE(*valeursDeFeu.rbegin() == 16);
}

TEST_CASE("genererButin : une chance nulle ne donne rien et ne consomme aucun tirage") {
    Generateur generateur(s_graine);
    const Generateur avant = generateur;

    REQUIRE(genererButin(generateur, 0, 3, 1).empty());

    REQUIRE(generateur == avant);
}

TEST_CASE("genererButin : une chance de 100 donne tous les objets, valides, sans tirage pour la chance") {
    Generateur generateur(s_graine);
    Generateur temoin(s_graine);

    const std::vector<Objet> butin = genererButin(generateur, 100, 3, 1);

    REQUIRE(butin.size() == 3);
    for (const Objet& objet : butin) {
        REQUIRE(objetValide(objet));
        REQUIRE(objet == genererObjet(temoin, 1));  // les objets sont ceux que `genererObjet` tire d'emblée
    }
    REQUIRE(generateur == temoin);
}

TEST_CASE("genererButin : une chance entre deux donne soit rien, soit tous les objets, dans la proportion voulue") {
    Generateur generateur(s_graine);
    int avecButin = 0;
    for (int i = 0; i < s_tirages; ++i) {
        const std::vector<Objet> butin = genererButin(generateur, 20, 2, 1);
        REQUIRE((butin.empty() || butin.size() == 2));
        if (!butin.empty()) {
            ++avecButin;
        }
    }
    // 20 % de 10000 : 2000, à 3 écarts types (environ 120) près.
    REQUIRE(avecButin > 1800);
    REQUIRE(avecButin < 2200);
}

TEST_CASE("genererButin : la meme graine donne le meme butin") {
    Generateur a(s_graine);
    Generateur b(s_graine);

    REQUIRE(genererButin(a, 50, 3, 1) == genererButin(b, 50, 3, 1));
}

TEST_CASE("genererButin : les objets sont du niveau recu") {
    Generateur generateur(s_graine);
    Generateur temoin(s_graine);

    const std::vector<Objet> butin = genererButin(generateur, 100, 3, 5);

    REQUIRE(butin.size() == 3);
    for (const Objet& objet : butin) {
        REQUIRE(objet == genererObjet(temoin, 5));
    }
}
