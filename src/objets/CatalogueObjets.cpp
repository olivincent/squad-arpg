// ./src/objets/CatalogueObjets.cpp
#include "objets/CatalogueObjets.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

namespace {
// Valeurs provisoires, à ajuster après test.
constexpr int s_armureCasqueDeFer = 2;
constexpr int s_armureCuirasseDeMailles = 4;
constexpr int s_armureJambieresDeFer = 2;
constexpr int s_armureBouclierRond = 1;
constexpr int s_armureGantsDeCuir = 1;
constexpr int s_armureBottesDeCuir = 1;
constexpr int s_armureCasqueDeCuir = 1;
constexpr int s_armureTuniqueMatelassee = 2;
constexpr int s_armureCollierDeProtection = 1;
constexpr int s_armureAnneauDeProtection = 1;
constexpr int s_bonusAnneauDePuissance = 3;    // Puissance
constexpr int s_bonusCollierDEndurance = 3;    // Endurance
constexpr int s_esquiveBottesDeCuir = 5;       // en pour cent, contre le contact
constexpr int s_esquiveCasqueDeCuir = 5;       // en pour cent, contre les projectiles
constexpr int s_chanceDeBlocageBouclierRond = 30;       // en pour cent
constexpr int s_montantBloqueBouclierRond = 12;         // points absorbés par blocage

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

// Les dégâts moyens d'une arme : sa référence, fois le facteur de son niveau (`facteurDeNiveau`), fois un facteur de qualité qui
// va de `s_facteurDeQualiteMin` (qualité 0) à `s_facteurDeQualiteMin + s_amplitudeDeQualite` (qualité 100), donc 1 à la qualité
// moyenne (valeurs provisoires, « à tester »).
constexpr double s_facteurDeQualiteMin = 0.85;
constexpr double s_amplitudeDeQualite = 0.30;

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

std::vector<DefinitionObjet> construireLeCatalogue() {
    return {
        avecExigences({IdObjet::CasqueDeFer, "Iron Helm", CategorieObjet::Casque, s_armureCasqueDeFer},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::CuirasseDeMailles, "Chain Mail", CategorieObjet::Armure, s_armureCuirasseDeMailles},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::JambieresDeFer, "Iron Greaves", CategorieObjet::Jambieres, s_armureJambieresDeFer},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        avecExigences({IdObjet::BouclierRond, "Round Shield", CategorieObjet::Bouclier, s_armureBouclierRond, {}, {}, std::nullopt,
                       DonneesDeBouclier{.chanceDeBlocage = s_chanceDeBlocageBouclierRond, .montantBloque = s_montantBloqueBouclierRond}},
                      {{Caracteristique::Endurance, s_penteSeuilPrincipale}}),
        {IdObjet::GantsDeCuir, "Leather Gloves", CategorieObjet::Gants, s_armureGantsDeCuir},
        {IdObjet::BottesDeCuir, "Leather Boots", CategorieObjet::Bottes, s_armureBottesDeCuir, {},
         {.esquiveContact = s_esquiveBottesDeCuir}},
        {IdObjet::CasqueDeCuir, "Leather Cap", CategorieObjet::Casque, s_armureCasqueDeCuir, {},
         {.esquiveProjectile = s_esquiveCasqueDeCuir}},
        {IdObjet::TuniqueMatelassee, "Padded Tunic", CategorieObjet::Armure, s_armureTuniqueMatelassee},
        {IdObjet::CollierDeProtection, "Necklace of Protection", CategorieObjet::Collier,
         s_armureCollierDeProtection},
        {IdObjet::AnneauDeProtection, "Ring of Protection", CategorieObjet::Anneau,
         s_armureAnneauDeProtection},
        {IdObjet::AnneauDePuissance, "Ring of Might", CategorieObjet::Anneau, 0,
         Caracteristiques::seule(Caracteristique::Puissance, s_bonusAnneauDePuissance)},
        {IdObjet::CollierDEndurance, "Necklace of Constitution", CategorieObjet::Collier, 0,
         Caracteristiques::seule(Caracteristique::Endurance, s_bonusCollierDEndurance)},
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
    };
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

DescriptionDAttaque descriptionDeLArme(const Objet& arme) {
    const std::optional<DonneesDArme>& gabarit = definitionObjet(arme.modele).arme;
    assert(gabarit.has_value());
    const DonneesDArme& donnees = *gabarit;  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
    const double moyens = donnees.degatsDeReference * facteurDeNiveau(arme.niveau) *
                          (s_facteurDeQualiteMin + s_amplitudeDeQualite * arme.qualite / s_qualiteMaximale);
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
