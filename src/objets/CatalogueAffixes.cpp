// ./src/objets/CatalogueAffixes.cpp
#include "objets/CatalogueAffixes.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>

namespace {
// Les catégories des suffixes de caractéristique : toutes.
const std::vector<CategorieObjet> s_toutesLesCategories = {
    CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Gants,   CategorieObjet::Bottes,
    CategorieObjet::Bouclier, CategorieObjet::Collier, CategorieObjet::Anneau, CategorieObjet::Arme};

// Les plages de valeurs (valeurs provisoires, « à tester »). Les pour cent sont des points de pourcentage.
constexpr PlageDeValeurs s_plageDegatsElementaires = {3, 8};  // feu et sacré, en points de dégâts
constexpr PlageDeValeurs s_plagePenetration = {10, 20};       // pour cent de l'armure ignorée
constexpr PlageDeValeurs s_plageArmure = {2, 5};              // points d'armure
constexpr PlageDeValeurs s_plageCaracteristique = {1, 3};     // points de caractéristique
constexpr PlageDeValeurs s_plageEsquive = {2, 4};             // pour cent
constexpr PlageDeValeurs s_plageBlocage = {3, 7};             // pour cent de chance

// De combien les bornes d'une plage grandissent à chaque niveau d'objet au-delà du premier, en part de la plage du catalogue
// (valeur provisoire, « à tester »).
constexpr double s_croissanceDesPlagesParNiveau = 0.25;

std::vector<DefinitionDAffixe> construireLeCatalogue() {
    return {
        {IdAffixe::Flamboyant, "Blazing", PositionDAffixe::Prefixe, EffetDAffixe::DegatsDeFeu, s_plageDegatsElementaires,
         {CategorieObjet::Arme}},
        {IdAffixe::Sanctifie, "Hallowed", PositionDAffixe::Prefixe, EffetDAffixe::DegatsSacres, s_plageDegatsElementaires,
         {CategorieObjet::Arme}},
        {IdAffixe::Acere, "Keen", PositionDAffixe::Prefixe, EffetDAffixe::PenetrationDArmure, s_plagePenetration,
         {CategorieObjet::Arme}},
        {IdAffixe::Renforce, "Reinforced", PositionDAffixe::Prefixe, EffetDAffixe::Armure, s_plageArmure,
         {CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Gants,
          CategorieObjet::Bottes, CategorieObjet::Bouclier}},
        {IdAffixe::DuBoeuf, "of the Ox", PositionDAffixe::Suffixe, EffetDAffixe::Puissance, s_plageCaracteristique,
         s_toutesLesCategories},
        {IdAffixe::DeLOurs, "of the Bear", PositionDAffixe::Suffixe, EffetDAffixe::Endurance, s_plageCaracteristique,
         s_toutesLesCategories},
        {IdAffixe::DuRenard, "of the Fox", PositionDAffixe::Suffixe, EffetDAffixe::Adresse, s_plageCaracteristique,
         s_toutesLesCategories},
        {IdAffixe::DeLaChouette, "of the Owl", PositionDAffixe::Suffixe, EffetDAffixe::Savoir, s_plageCaracteristique,
         s_toutesLesCategories},
        {IdAffixe::DuLion, "of the Lion", PositionDAffixe::Suffixe, EffetDAffixe::Volonte, s_plageCaracteristique,
         s_toutesLesCategories},
        {IdAffixe::DEsquive, "of Evasion", PositionDAffixe::Suffixe, EffetDAffixe::EsquiveContact, s_plageEsquive,
         {CategorieObjet::Bottes, CategorieObjet::Armure}},
        {IdAffixe::DeDeviation, "of Deflection", PositionDAffixe::Suffixe, EffetDAffixe::EsquiveProjectile, s_plageEsquive,
         {CategorieObjet::Casque, CategorieObjet::Bouclier}},
        {IdAffixe::DeBlocage, "of Blocking", PositionDAffixe::Suffixe, EffetDAffixe::ChanceDeBlocage, s_plageBlocage,
         {CategorieObjet::Bouclier}},
    };
}

// Ajoute l'apport d'un affixe. Le `switch` n'a pas de `default` : un effet ajouté sans règle fait avertir le compilateur.
void ajouter(ApportDesAffixes& apport, const Affixe& affixe) {
    const int valeur = affixe.valeur;
    switch (definitionAffixe(affixe.id).effet) {
        case EffetDAffixe::DegatsDeFeu: apport.degatsAjoutes[Element::Feu] += valeur; break;
        case EffetDAffixe::DegatsSacres: apport.degatsAjoutes[Element::Sacre] += valeur; break;
        case EffetDAffixe::PenetrationDArmure: apport.penetration += valeur; break;
        case EffetDAffixe::Armure: apport.armure += valeur; break;
        case EffetDAffixe::Puissance: apport.bonus += Caracteristiques::seule(Caracteristique::Puissance, valeur); break;
        case EffetDAffixe::Endurance: apport.bonus += Caracteristiques::seule(Caracteristique::Endurance, valeur); break;
        case EffetDAffixe::Adresse: apport.bonus += Caracteristiques::seule(Caracteristique::Adresse, valeur); break;
        case EffetDAffixe::Savoir: apport.bonus += Caracteristiques::seule(Caracteristique::Savoir, valeur); break;
        case EffetDAffixe::Volonte: apport.bonus += Caracteristiques::seule(Caracteristique::Volonte, valeur); break;
        case EffetDAffixe::EsquiveContact: apport.bonusDeCombat.esquiveContact += valeur; break;
        case EffetDAffixe::EsquiveProjectile: apport.bonusDeCombat.esquiveProjectile += valeur; break;
        case EffetDAffixe::ChanceDeBlocage: apport.bonusDeCombat.chanceDeBlocage += valeur; break;
    }
}
}  // namespace

const std::vector<DefinitionDAffixe>& catalogueAffixes() {
    static const std::vector<DefinitionDAffixe> s_catalogue = construireLeCatalogue();
    return s_catalogue;
}

const DefinitionDAffixe& definitionAffixe(IdAffixe id) {
    const DefinitionDAffixe& trouvee = catalogueAffixes()[static_cast<std::size_t>(id)];
    assert(trouvee.id == id);  // le catalogue suit l'ordre de IdAffixe
    return trouvee;
}

PlageDeValeurs plageAuNiveau(IdAffixe id, int niveau) {
    assert(niveau >= 1);
    const PlageDeValeurs& plage = definitionAffixe(id).valeurs;
    const double facteur = 1.0 + s_croissanceDesPlagesParNiveau * (niveau - 1);
    return {static_cast<int>(std::lround(plage.min * facteur)), static_cast<int>(std::lround(plage.max * facteur))};
}

bool affixePermis(IdAffixe id, CategorieObjet categorie) {
    const std::vector<CategorieObjet>& permises = definitionAffixe(id).categories;
    return std::ranges::find(permises, categorie) != permises.end();
}

std::vector<IdAffixe> affixesPermisPour(CategorieObjet categorie, PositionDAffixe position) {
    std::vector<IdAffixe> permis;
    for (const DefinitionDAffixe& definition : catalogueAffixes()) {
        if (definition.position == position && affixePermis(definition.id, categorie)) {
            permis.push_back(definition.id);
        }
    }
    return permis;
}

ApportDesAffixes apportDesAffixes(const Objet& objet) {
    ApportDesAffixes apport;
    if (objet.prefixe.has_value()) {
        ajouter(apport, *objet.prefixe);
    }
    if (objet.suffixe.has_value()) {
        ajouter(apport, *objet.suffixe);
    }
    return apport;
}
