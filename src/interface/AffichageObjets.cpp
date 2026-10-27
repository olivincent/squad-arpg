// ./src/interface/AffichageObjets.cpp
#include "interface/AffichageObjets.hpp"

#include <cmath>
#include <optional>

#include "interface/AffichageCaracteristiques.hpp"
#include "interface/AffichageStatistiques.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {
// Des secondes : « 3 » pour 3.0, « 0.3 » pour 0.3 (une décimale au plus, point décimal).
std::string secondes(float valeur) {
    const float arrondi = std::round(valeur * 10.0f) / 10.0f;
    if (arrondi == std::floor(arrondi)) {
        return std::to_string(static_cast<int>(arrondi));
    }
    std::string texte = std::to_string(arrondi);  // « 0.300000 »
    texte.erase(texte.find('.') + 2);
    return texte;
}

// Les lignes d'une arme : l'attaque de base qu'elle donne (celle de CET objet, `descriptionDeLArme`) et, à deux mains,
// « Two-handed ».
void ajouterLesLignesDUneArme(std::vector<std::string>& lignes, const Objet& objet, const DonneesDArme& arme) {
    const DescriptionDAttaque attaque = descriptionDeLArme(objet);
    // Les bornes de la plage, avant la Puissance de qui la porte ; un seul nombre quand il n'y a pas d'écart.
    const int min = attaque.degats.min.total();
    const int max = attaque.degats.max.total();
    lignes.push_back("Damage: " + (min == max ? std::to_string(min) : std::to_string(min) + "–" + std::to_string(max)));
    lignes.push_back("Cooldown: " + (attaque.rechargeMin == attaque.rechargeMax
                                          ? secondes(attaque.rechargeMin)
                                          : secondes(attaque.rechargeMin) + " to " + secondes(attaque.rechargeMax)) + " s");
    lignes.push_back("Range: " + std::to_string(static_cast<int>(std::lround(attaque.portee))) + " px");
    // Le `switch` n'a pas de `default` : une nature ajoutée sans texte fait avertir le compilateur.
    switch (attaque.nature) {
        case NatureDAttaque::Contact: lignes.push_back("Melee"); break;
        case NatureDAttaque::Projectile: lignes.push_back("Projectile"); break;
        case NatureDAttaque::Zone: lignes.push_back("Area"); break;
    }
    if (attaque.penetration > 0) {
        lignes.push_back("Penetration: " + std::to_string(attaque.penetration) + " %");
    }
    if (arme.mains == 2) {
        lignes.push_back("Two-handed");
    }
}
}  // namespace

// Les trois `switch` n'ont pas de `default` : un emplacement, une catégorie ou un objet ajouté
// sans texte fait avertir le compilateur.

std::string nomDeLEmplacement(Emplacement emplacement) {
    switch (emplacement) {
        case Emplacement::Tete: return "Head";
        case Emplacement::Torse: return "Chest";
        case Emplacement::Jambes: return "Legs";
        case Emplacement::Gants: return "Hands";
        case Emplacement::Bottes: return "Feet";
        case Emplacement::MainDroite: return "Main Hand";
        case Emplacement::MainGauche: return "Off Hand";
        case Emplacement::Collier: return "Necklace";
        case Emplacement::Anneau1: return "Ring 1";
        case Emplacement::Anneau2: return "Ring 2";
    }
    return "";
}

std::string nomDeLaCategorie(CategorieObjet categorie) {
    switch (categorie) {
        case CategorieObjet::Casque: return "Helmet";
        case CategorieObjet::Armure: return "Body Armor";
        case CategorieObjet::Jambieres: return "Leggings";
        case CategorieObjet::Gants: return "Gloves";
        case CategorieObjet::Bottes: return "Boots";
        case CategorieObjet::Bouclier: return "Shield";
        case CategorieObjet::Collier: return "Necklace";
        case CategorieObjet::Anneau: return "Ring";
        case CategorieObjet::Arme: return "Weapon";
    }
    return "";
}

std::string sigleDeLObjet(IdObjet id) {
    switch (id) {
        case IdObjet::CasqueDeFer: return "IH";
        case IdObjet::CuirasseDeMailles: return "CM";
        case IdObjet::JambieresDeFer: return "IG";
        case IdObjet::BouclierRond: return "RS";
        case IdObjet::GantsDeCuir: return "LG";
        case IdObjet::BottesDeCuir: return "LB";
        case IdObjet::CasqueDeCuir: return "LC";
        case IdObjet::TuniqueMatelassee: return "PT";
        case IdObjet::CollierDeProtection: return "NP";
        case IdObjet::AnneauDeProtection: return "RP";
        case IdObjet::AnneauDePuissance: return "RM";
        case IdObjet::CollierDEndurance: return "NC";
        case IdObjet::Dague: return "DA";
        case IdObjet::EpeeAUneMain: return "SW";
        case IdObjet::HacheAUneMain: return "HA";
        case IdObjet::MasseAUneMain: return "MA";
        case IdObjet::Lance: return "SP";
        case IdObjet::EpeeADeuxMains: return "GS";
        case IdObjet::HacheADeuxMains: return "GA";
        case IdObjet::MasseADeuxMains: return "MU";
        case IdObjet::Baguette: return "WA";
        case IdObjet::Sceptre: return "SC";
        case IdObjet::Baton: return "ST";
        case IdObjet::Arc: return "BO";
        case IdObjet::Arbalete: return "CB";
    }
    return "";
}

std::string ligneDeSeuil(const SeuilDeCaracteristique& seuil) {
    return "Requires " + nomDeLaCaracteristique(seuil.caracteristique) + " " + std::to_string(seuil.valeur);
}

std::string ligneDObjetInactif() {
    return "Inactive: requirement not met";
}

sf::Color couleurDeLaRarete(Rarete rarete) {
    switch (rarete) {
        case Rarete::Commun: return sf::Color::White;
        case Rarete::Magique: return sf::Color(240, 220, 70);
        case Rarete::Rare: return sf::Color(90, 220, 90);
    }
    return sf::Color::White;
}

std::string ligneDAffixe(const Affixe& affixe) {
    const int valeur = affixe.valeur;
    const std::string plus = "+" + std::to_string(valeur);
    // Le `switch` n'a pas de `default` : un effet ajouté sans texte fait avertir le compilateur.
    switch (definitionAffixe(affixe.id).effet) {
        case EffetDAffixe::DegatsDeFeu: return "Fire Damage: " + plus;
        case EffetDAffixe::DegatsSacres: return "Holy Damage: " + plus;
        case EffetDAffixe::PenetrationDArmure: return "Penetration: " + plus + " %";
        case EffetDAffixe::Armure: return "Armor: " + plus;
        case EffetDAffixe::Puissance: return nomDeLaCaracteristique(Caracteristique::Puissance) + ": " + plus;
        case EffetDAffixe::Endurance: return nomDeLaCaracteristique(Caracteristique::Endurance) + ": " + plus;
        case EffetDAffixe::Adresse: return nomDeLaCaracteristique(Caracteristique::Adresse) + ": " + plus;
        case EffetDAffixe::Savoir: return nomDeLaCaracteristique(Caracteristique::Savoir) + ": " + plus;
        case EffetDAffixe::Volonte: return nomDeLaCaracteristique(Caracteristique::Volonte) + ": " + plus;
        case EffetDAffixe::EsquiveContact: return lignesDeBonusDeCombat({.esquiveContact = valeur}).front();
        case EffetDAffixe::EsquiveProjectile: return lignesDeBonusDeCombat({.esquiveProjectile = valeur}).front();
        case EffetDAffixe::ChanceDeBlocage: return "Block: " + plus + " %";
    }
    return "";
}

std::vector<std::string> lignesDeLInfobulle(const Objet& objet) {
    const DefinitionObjet& definition = definitionObjet(objet.modele);
    // La qualité n'est pas affichée : le joueur la voit dans ce qu'elle change (les dégâts d'une arme).
    std::vector<std::string> lignes = {nomDeLObjet(objet), nomDeLaCategorie(definition.categorie),
                                       "Item Level " + std::to_string(objet.niveau)};
    // Les effets de base de CET objet (son niveau et sa qualité compris), jamais ceux du modèle.
    const EffetsDeBase effets = effetsDeBase(objet);
    if (effets.armure != 0) {
        lignes.push_back("Armor: +" + std::to_string(effets.armure));
    }
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (const int bonus = effets.bonus.valeur(caracteristique); bonus != 0) {
            lignes.push_back(nomDeLaCaracteristique(caracteristique) + ": " + (bonus > 0 ? "+" : "") + std::to_string(bonus));
        }
    }
    for (const std::string& ligne : lignesDeBonusDeCombat(effets.bonusDeCombat)) {
        lignes.push_back(ligne);
    }
    if (definition.bouclier.has_value()) {
        lignes.push_back("Block: " + std::to_string(effets.bonusDeCombat.chanceDeBlocage) + " %, " +
                         std::to_string(effets.bonusDeCombat.montantBloque) + " points");
    }
    if (definition.arme.has_value()) {
        ajouterLesLignesDUneArme(lignes, objet, *definition.arme);
    }
    for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
        if (affixe.has_value()) {
            lignes.push_back(ligneDAffixe(*affixe));
        }
    }
    for (const SeuilDeCaracteristique& seuil : seuilsDeLObjet(objet)) {
        lignes.push_back(ligneDeSeuil(seuil));
    }
    return lignes;
}
