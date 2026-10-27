// ./src/objets/CatalogueObjets.cpp
#include "objets/CatalogueObjets.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <optional>
#include <utility>

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr int s_armureBouclier = 1;
constexpr int s_armureCollierDeProtection = 1;
constexpr int s_armureAnneauDeProtection = 1;
constexpr int s_bonusAnneauDePuissance = 3;    // Puissance
constexpr int s_bonusCollierDEndurance = 3;    // Endurance
constexpr int s_chanceDeBlocageBouclier = 20;       // en pour cent, au niveau 1
constexpr int s_montantBloqueBouclier = 8;          // points absorbés par blocage, au niveau 1 et à la qualité moyenne

// L'armure de base des pièces (niveau 1, qualité moyenne), par type : torse, jambes, tête, gants, bottes (valeurs provisoires, « à
// tester »). Chaque type bat le précédent sur chaque pièce ; un ensemble complet vaut 6, 11, 17 et 24.
constexpr std::array<int, 5> s_armureDeTissu = {2, 1, 1, 1, 1};
constexpr std::array<int, 5> s_armureDeLArmureLegere = {3, 2, 2, 2, 2};
constexpr std::array<int, 5> s_armureDeLArmureIntermediaire = {5, 3, 3, 3, 3};
constexpr std::array<int, 5> s_armureDeLArmureLourde = {7, 5, 4, 4, 4};

// Les malus d'esquive des pièces, en points de pourcentage (contre le contact comme contre les projectiles), fixes à tout niveau.
constexpr std::array<int, 5> s_aucunMalus = {0, 0, 0, 0, 0};
constexpr std::array<int, 5> s_malusIntermediaire = {1, 1, 0, 0, 0};
constexpr std::array<int, 5> s_malusLourd = {3, 2, 1, 1, 1};

// Le niveau d'apparition de chaque type dans le butin : les pièces plus lourdes n'arrivent qu'en descendant.
constexpr int s_apparitionDuTissu = 1;
constexpr int s_apparitionDuLeger = 1;
constexpr int s_apparitionDeLIntermediaire = 2;
constexpr int s_apparitionDuLourd = 3;

// Armes (valeurs provisoires, à tester). Une attaque de contact engage (zone de contrôle), un projectile non. Les durées sont
// en secondes, les portées en pixels, bord à bord.
constexpr float s_porteeDeContact = 20.0f;
constexpr float s_porteeDeLaLance = 35.0f;
constexpr float s_porteeDeProjectile = 120.0f;
constexpr float s_porteeDeLArbalete = 140.0f;
constexpr float s_executionUneMain = 0.15f;       // une arme à deux mains met le double de temps à frapper
constexpr float s_executionDeuxMains = 0.3f;

// Les mains nues, sans arme : elles ne sont pas un objet et ne dépendent ni d'un niveau ni d'une qualité.
constexpr int s_degatsMainsNues = 6;
constexpr float s_rechargeMainsNuesMin = 0.4f;
constexpr float s_rechargeMainsNuesMax = 0.6f;

// Les exigences d'équipement (valeurs provisoires, « à tester ») : le seuil de chaque exigence est
// `s_seuilAuNiveau1 + pente × (niveau − 1)` (`seuilsDeLObjet`). La pente de la caractéristique principale d'un objet, puis celle
// d'une caractéristique secondaire.
constexpr int s_penteSeuilPrincipale = 2;
constexpr int s_penteSeuilSecondaire = 1;

// Cette définition, avec ces exigences.
DefinitionObjet avecExigences(DefinitionObjet definition, std::vector<ExigenceDeCaracteristique> exigences) {
    definition.exigences = std::move(exigences);
    return definition;
}

// Une arme d'après son gabarit : le type, les mains (le temps d'exécution en découle), la référence de ses dégâts et l'écart de
// ses coups (en pour cent), son intervalle de recharge, sa nature et sa portée, sa pénétration.
DefinitionObjet uneArme(IdObjet id, std::string nom, TypeDArme type, int mains, int reference, int ecartPourCent, float rechargeMin,
                        float rechargeMax, NatureDAttaque nature, float portee, int penetration,
                        std::vector<ExigenceDeCaracteristique> exigences) {
    const DonneesDArme gabarit = {.type = type, .mains = mains, .degatsDeReference = reference, .ecartPourCent = ecartPourCent,
                                  .rechargeMin = rechargeMin, .rechargeMax = rechargeMax,
                                  .tempsExecution = mains == 2 ? s_executionDeuxMains : s_executionUneMain, .portee = portee,
                                  .nature = nature, .penetration = penetration};
    return avecExigences({id, std::move(nom), CategorieObjet::Arme, 0, {}, {}, gabarit}, std::move(exigences));
}

DefinitionObjet uneArmeDeContact(IdObjet id, std::string nom, TypeDArme type, int mains, int reference, int ecartPourCent,
                                 float rechargeMin, float rechargeMax, float portee, int penetration,
                                 std::vector<ExigenceDeCaracteristique> exigences) {
    return uneArme(id, std::move(nom), type, mains, reference, ecartPourCent, rechargeMin, rechargeMax, NatureDAttaque::Contact,
                   portee, penetration, std::move(exigences));
}

DefinitionObjet uneArmeDeTir(IdObjet id, std::string nom, TypeDArme type, int mains, int reference, int ecartPourCent,
                             float rechargeMin, float rechargeMax, float portee, int penetration,
                             std::vector<ExigenceDeCaracteristique> exigences) {
    return uneArme(id, std::move(nom), type, mains, reference, ecartPourCent, rechargeMin, rechargeMax, NatureDAttaque::Projectile,
                   portee, penetration, std::move(exigences));
}

// Les cinq emplacements d'une série de pièces d'armure, dans l'ordre des identifiants de `IdObjet` : torse, jambes, tête, gants,
// bottes.
constexpr std::size_t s_piecesParType = 5;
constexpr std::array<CategorieObjet, s_piecesParType> s_categoriesDesPieces = {
    CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Casque, CategorieObjet::Gants, CategorieObjet::Bottes};

// Les exigences d'un type d'armure (le seuil de chacune : 6 au niveau 1, puis la pente par niveau) : le tissu aucune, le léger
// l'Adresse, l'intermédiaire l'Endurance et l'Adresse, le lourd l'Endurance.
std::vector<ExigenceDeCaracteristique> exigencesDuType(TypeDArmure type) {
    switch (type) {
        case TypeDArmure::Tissu: return {};
        case TypeDArmure::Leger: return {{Caracteristique::Adresse, s_penteSeuilPrincipale}};
        case TypeDArmure::Intermediaire:
            return {{Caracteristique::Endurance, s_penteSeuilSecondaire}, {Caracteristique::Adresse, s_penteSeuilSecondaire}};
        case TypeDArmure::Lourd: return {{Caracteristique::Endurance, s_penteSeuilPrincipale}};
    }
    return {};
}

// Les cinq pièces d'un type d'armure : leur armure de base (niveau 1, qualité moyenne), leur malus d'esquive (en points de
// pourcentage, contre le contact comme contre les projectiles ; fixe), le niveau d'apparition du type. Les identifiants se
// suivent dans `IdObjet` à partir du premier.
std::vector<DefinitionObjet> piecesDArmure(TypeDArmure type, IdObjet premier, const std::array<std::string, s_piecesParType>& noms,
                                          const std::array<int, s_piecesParType>& armure,
                                          const std::array<int, s_piecesParType>& malusDEsquive, int niveauDApparition) {
    std::vector<DefinitionObjet> pieces;
    for (std::size_t i = 0; i < s_piecesParType; ++i) {
        DefinitionObjet piece = {static_cast<IdObjet>(static_cast<std::size_t>(premier) + i), noms[i], s_categoriesDesPieces[i], armure[i]};
        piece.bonusDeCombat = {.esquiveContact = -malusDEsquive[i], .esquiveProjectile = -malusDEsquive[i]};
        piece.exigences = exigencesDuType(type);
        piece.niveauDApparition = niveauDApparition;
        piece.typeDArmure = type;
        pieces.push_back(std::move(piece));
    }
    return pieces;
}

std::vector<DefinitionObjet> construireLeCatalogue() {
    std::vector<DefinitionObjet> catalogue;
    const auto ajouter = [&catalogue](std::vector<DefinitionObjet> definitions) {
        catalogue.insert(catalogue.end(), std::make_move_iterator(definitions.begin()), std::make_move_iterator(definitions.end()));
    };
    // Les vingt pièces d'armure (étape 103.3). Le type bat le précédent sur CHAQUE pièce ; aucune pièce n'a de bonus de
    // caractéristique ; seules les pièces intermédiaires et lourdes ont un malus d'esquive. Torse, jambes, tête, gants, bottes.
    ajouter(piecesDArmure(TypeDArmure::Tissu, IdObjet::Robe, {"Robe", "Cloth Leggings", "Hood", "Cloth Gloves", "Slippers"},
                          s_armureDeTissu, s_aucunMalus, s_apparitionDuTissu));
    ajouter(piecesDArmure(TypeDArmure::Leger, IdObjet::Justaucorps,
                          {"Leather Jerkin", "Leather Leggings", "Leather Cap", "Leather Gloves", "Leather Boots"}, s_armureDeLArmureLegere,
                          s_aucunMalus, s_apparitionDuLeger));
    ajouter(piecesDArmure(TypeDArmure::Intermediaire, IdObjet::CotteDeMailles,
                          {"Chain Mail", "Chain Leggings", "Chain Coif", "Chain Gloves", "Chain Boots"}, s_armureDeLArmureIntermediaire,
                          s_malusIntermediaire, s_apparitionDeLIntermediaire));
    ajouter(piecesDArmure(TypeDArmure::Lourd, IdObjet::ArmureDePlaques,
                          {"Plate Armor", "Plate Greaves", "Great Helm", "Gauntlets", "Sabatons"}, s_armureDeLArmureLourde, s_malusLourd,
                          s_apparitionDuLourd));
    ajouter({
        avecExigences({IdObjet::Bouclier, "Shield", CategorieObjet::Bouclier, s_armureBouclier, {}, {}, std::nullopt,
                       DonneesDeBouclier{.chanceAuNiveau1 = s_chanceDeBlocageBouclier, .montantAuNiveau1 = s_montantBloqueBouclier}},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        {IdObjet::CollierDeProtection, "Necklace of Protection", CategorieObjet::Collier,
         s_armureCollierDeProtection},
        {IdObjet::AnneauDeProtection, "Ring of Protection", CategorieObjet::Anneau,
         s_armureAnneauDeProtection},
        {IdObjet::AnneauDePuissance, "Ring of Might", CategorieObjet::Anneau, 0,
         Caracteristiques::seule(Caracteristique::Puissance, s_bonusAnneauDePuissance)},
        {IdObjet::CollierDEndurance, "Necklace of Constitution", CategorieObjet::Collier, 0,
         Caracteristiques::seule(Caracteristique::Endurance, s_bonusCollierDEndurance)},
    });
    ajouter({
        // Les treize gabarits d'armes (étape 102.4), dans l'ordre de `IdObjet`. Chacun fixe l'identité de l'arme ; ses dégâts
        // dépendent de l'objet (`descriptionDeLArme`). Les dégâts/s visés sont 23 à une main et 30 à deux mains : la référence
        // est ces dégâts/s fois (exécution + recharge moyenne).
        //                id,                  nom,          type,                    mains, réf., écart, recharge,  portée, pénétr., exigences
        uneArmeDeContact(IdObjet::Dague,           "Dagger",     TypeDArme::Dague,           1, 10, 10, 0.20f, 0.50f, s_porteeDeContact, 15,
                         {{Caracteristique::Adresse, s_penteSeuilPrincipale}}),
        uneArmeDeContact(IdObjet::EpeeAUneMain,    "Sword",      TypeDArme::EpeeAUneMain,    1, 15, 15, 0.30f, 0.70f, s_porteeDeContact, 0,
                         {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        uneArmeDeContact(IdObjet::HacheAUneMain,   "Hand Axe",   TypeDArme::HacheAUneMain,   1, 20, 25, 0.40f, 1.00f, s_porteeDeContact, 0,
                         {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        uneArmeDeContact(IdObjet::MasseAUneMain,   "Mace",       TypeDArme::MasseAUneMain,   1, 26, 40, 0.60f, 1.40f, s_porteeDeContact, 0,
                         {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        uneArmeDeContact(IdObjet::Lance,           "Spear",      TypeDArme::Lance,           1, 28, 25, 0.70f, 1.45f, s_porteeDeLaLance, 30,
                         {{Caracteristique::Adresse, s_penteSeuilPrincipale}, {Caracteristique::Puissance, s_penteSeuilSecondaire}}),
        uneArmeDeContact(IdObjet::EpeeADeuxMains,  "Greatsword", TypeDArme::EpeeADeuxMains,  2, 35, 15, 0.50f, 1.20f, s_porteeDeContact, 0,
                         {{Caracteristique::Puissance, s_penteSeuilPrincipale}, {Caracteristique::Adresse, s_penteSeuilSecondaire}}),
        uneArmeDeContact(IdObjet::HacheADeuxMains, "Great Axe",  TypeDArme::HacheADeuxMains, 2, 48, 25, 0.80f, 1.80f, s_porteeDeContact, 0,
                         {{Caracteristique::Puissance, s_penteSeuilPrincipale}}),
        uneArmeDeContact(IdObjet::MasseADeuxMains, "Maul",       TypeDArme::MasseADeuxMains, 2, 48, 40, 0.80f, 1.80f, s_porteeDeContact, 0,
                         {{Caracteristique::Puissance, s_penteSeuilPrincipale}, {Caracteristique::Endurance, s_penteSeuilSecondaire}}),
        uneArmeDeTir(IdObjet::Baguette,            "Wand",       TypeDArme::Baguette,        1, 15, 15, 0.30f, 0.70f, s_porteeDeProjectile, 0,
                     {{Caracteristique::Savoir, s_penteSeuilPrincipale}}),
        uneArmeDeTir(IdObjet::Sceptre,             "Scepter",    TypeDArme::Sceptre,         1, 20, 15, 0.40f, 1.00f, s_porteeDeProjectile, 0,
                     {{Caracteristique::Volonte, s_penteSeuilPrincipale}}),
        uneArmeDeTir(IdObjet::Baton,               "Staff",      TypeDArme::Baton,           2, 30, 25, 0.55f, 0.85f, s_porteeDeProjectile, 0,
                     {{Caracteristique::Savoir, s_penteSeuilPrincipale}}),
        uneArmeDeTir(IdObjet::Arc,                 "Bow",        TypeDArme::Arc,             2, 23, 25, 0.35f, 0.55f, s_porteeDeProjectile, 15,
                     {{Caracteristique::Adresse, s_penteSeuilPrincipale}}),
        uneArmeDeTir(IdObjet::Arbalete,            "Crossbow",   TypeDArme::Arbalete,        2, 30, 15, 0.55f, 0.85f, s_porteeDeLArbalete, 30,
                     {{Caracteristique::Adresse, s_penteSeuilPrincipale}, {Caracteristique::Puissance, s_penteSeuilSecondaire}}),
    });
    return catalogue;
}
}  // namespace

const std::vector<DefinitionObjet>& catalogueObjets() {
    static const std::vector<DefinitionObjet> s_catalogue = construireLeCatalogue();
    return s_catalogue;
}

const DefinitionObjet& definitionObjet(IdObjet id) {
    const DefinitionObjet& trouvee = catalogueObjets()[static_cast<std::size_t>(id)];
    assert(trouvee.id == id);  // le catalogue suit l'ordre de IdObjet
    return trouvee;
}

const DescriptionDAttaque& mainsNues() {
    static const DescriptionDAttaque s_mainsNues = {.degats = plageFixe(degatsPhysiques(s_degatsMainsNues)),
                                                    .rechargeMin = s_rechargeMainsNuesMin, .rechargeMax = s_rechargeMainsNuesMax,
                                                    .tempsExecution = s_executionUneMain, .portee = s_porteeDeContact,
                                                    .nature = NatureDAttaque::Contact, .penetration = 0};
    return s_mainsNues;
}

EffetsDeBase effetsDeBase(const Objet& objet) {
    const DefinitionObjet& definition = definitionObjet(objet.modele);
    const double facteur = facteurDeNiveau(objet.niveau) * facteurDeQualite(objet.qualite);
    const auto grandi = [facteur](int base) { return static_cast<int>(std::lround(base * facteur)); };

    EffetsDeBase effets;
    effets.armure = grandi(definition.armure);
    for (const Caracteristique caracteristique : s_caracteristiques) {
        effets.bonus += Caracteristiques::seule(caracteristique, grandi(definition.bonus.valeur(caracteristique)));
    }
    effets.bonusDeCombat = definition.bonusDeCombat;  // les esquives et les résistances restent celles du modèle
    effets.bonusDeCombat.capaciteOffensive = grandi(definition.bonusDeCombat.capaciteOffensive);
    effets.bonusDeCombat.capaciteDefensive = grandi(definition.bonusDeCombat.capaciteDefensive);
    if (definition.bouclier.has_value()) {
        effets.bonusDeCombat.chanceDeBlocage += definition.bouclier->chanceAuNiveau1 + s_gainDeBlocageParNiveau * (objet.niveau - 1);
        effets.bonusDeCombat.montantBloque += grandi(definition.bouclier->montantAuNiveau1);
    }
    return effets;
}

DescriptionDAttaque descriptionDeLArme(const Objet& arme) {
    const std::optional<DonneesDArme>& gabarit = definitionObjet(arme.modele).arme;
    assert(gabarit.has_value());
    const DonneesDArme& donnees = *gabarit;  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
    const double moyens = donnees.degatsDeReference * facteurDeNiveau(arme.niveau) * facteurDeQualite(arme.qualite);
    // Un seul arrondi par borne, au plus proche, et au moins 1 point.
    const auto borne = [&](int pourCent) { return std::max(1, static_cast<int>(std::lround(moyens * pourCent / 100.0))); };
    return {.degats = {degatsPhysiques(borne(100 - donnees.ecartPourCent)), degatsPhysiques(borne(100 + donnees.ecartPourCent))},
            .rechargeMin = donnees.rechargeMin, .rechargeMax = donnees.rechargeMax, .tempsExecution = donnees.tempsExecution,
            .portee = donnees.portee, .nature = donnees.nature, .penetration = donnees.penetration};
}

bool objetValide(const Objet& objet) {
    if (objet.niveau < 1 || objet.qualite < 0 || objet.qualite > s_qualiteMaximale) {
        return false;
    }
    const CategorieObjet categorie = definitionObjet(objet.modele).categorie;
    const auto affixeValide = [&](const std::optional<Affixe>& affixe, PositionDAffixe position) {
        return !affixe.has_value() || (affixe->valeur > 0 && definitionAffixe(affixe->id).position == position &&
                                       affixePermis(affixe->id, categorie));
    };
    if (!affixeValide(objet.prefixe, PositionDAffixe::Prefixe) || !affixeValide(objet.suffixe, PositionDAffixe::Suffixe)) {
        return false;
    }
    switch (objet.rarete) {
        case Rarete::Commun: return !objet.prefixe.has_value() && !objet.suffixe.has_value();
        case Rarete::Magique: return objet.prefixe.has_value() != objet.suffixe.has_value();
        case Rarete::Rare: return objet.prefixe.has_value() && objet.suffixe.has_value();
    }
    return false;
}

std::string nomDeLObjet(const Objet& objet) {
    std::string nom = definitionObjet(objet.modele).nom;
    if (objet.prefixe.has_value()) {
        nom = definitionAffixe(objet.prefixe->id).nom + " " + nom;
    }
    if (objet.suffixe.has_value()) {
        nom += " " + definitionAffixe(objet.suffixe->id).nom;
    }
    return nom;
}

std::vector<SeuilDeCaracteristique> seuilsDeLObjet(const Objet& objet) {
    std::vector<SeuilDeCaracteristique> seuils;
    for (const ExigenceDeCaracteristique& exigence : definitionObjet(objet.modele).exigences) {
        seuils.push_back({exigence.caracteristique, s_seuilAuNiveau1 + exigence.pente * (objet.niveau - 1)});
    }
    return seuils;
}

bool seuilsAtteints(const Objet& objet, const Caracteristiques& caracteristiques) {
    for (const SeuilDeCaracteristique& seuil : seuilsDeLObjet(objet)) {
        if (caracteristiques.valeur(seuil.caracteristique) < seuil.valeur) {
            return false;
        }
    }
    return true;
}

bool estADeuxMains(IdObjet id) {
    const std::optional<DonneesDArme>& arme = definitionObjet(id).arme;
    return arme.has_value() && arme->mains == 2;
}
