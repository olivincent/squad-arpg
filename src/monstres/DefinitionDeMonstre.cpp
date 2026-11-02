// ./src/monstres/DefinitionDeMonstre.cpp
#include "monstres/DefinitionDeMonstre.hpp"
#include <cassert>
#include <cstddef>
#include <variant>
#include "combat/Cible.hpp"
#include "combat/Degats.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/Element.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "combat/NatureDAttaque.hpp"

namespace {
// Ce que toutes les catégories partagent. Valeurs provisoires, à ajuster après test.
constexpr float s_rayonDetection = 200.0f;     // en pixels
constexpr float s_rayonTerritoire = 600.0f;    // en pixels
constexpr float s_zoneDeControle = 20.0f;      // en pixels, bord à bord
constexpr float s_regenerationRetour = 0.5f;   // fraction des points de vie maximum rendue par seconde en retour
constexpr float s_porteeAttaque = 15.0f;       // bord à bord
constexpr int s_resistanceAuFeuDeLaCreature = 40;  // en pour cent (« à tester »)

// Le tireur (étape 107.2, « à tester »).
constexpr float s_porteeDuTireur = 120.0f;         // bord à bord
constexpr float s_distanceDeContactDuTireur = 20.0f;  // en dessous, un personnage le fait reculer
constexpr float s_distanceDeReculDuTireur = 80.0f;
constexpr float s_delaiEntreReculsDuTireur = 8.0f;    // en secondes

// Le soigneur (étape 107.5, « à tester ») : son attaque est celle d'un tireur faible, son soin un skill de mode `Allie`.
constexpr float s_porteeDuSoigneur = 120.0f;       // de son attaque, bord à bord
constexpr int s_montantDuSoin = 75;                // points de vie rendus
constexpr float s_porteeDuSoin = 150.0f;           // bord à bord
constexpr float s_rechargeDuSoin = 6.0f;           // en secondes, fixe
constexpr float s_activationDuSoin = 0.8f;         // en secondes, immobile
constexpr int s_seuilDeBlessure = 75;              // en pour cent des points de vie maximum : un membre est blessé en dessous

// Le Ward du soigneur (étape 114, « à tester ») : un bouclier contre le non-physique sur un allié de son groupe, jamais sur lui-même. 70 points
// (23 % des 300 points de vie d'une infanterie), majorés par la profondeur comme les points de vie ; 8 s, recharge de 12 s (un buff ne dure jamais plus
// que sa recharge), à la portée du soin, avec la même activation.
constexpr int s_montantDuWard = 70;                // points absorbés
constexpr int s_dureeDuWard = 8;                   // en secondes
constexpr float s_rechargeDuWard = 12.0f;          // en secondes, fixe

// Le coup assommant de l'infanterie lourde (étape 113, « à tester ») : son attaque de base étourdit le personnage touché 1 s, au plus une fois
// toutes les 10 s.
constexpr float s_dureeDuCoupAssommant = 1.0f;     // en secondes, avant la résistance du personnage
constexpr float s_delaiEntreCoupsAssommants = 10.0f;  // en secondes

// Le renforcement par la profondeur : ce que chaque étage au-delà du premier ajoute (« à tester »).
constexpr int s_pourcentDePointsDeVieParEtage = 25;  // des points de vie maximum de la définition
constexpr int s_pourcentDeDegatsParEtage = 15;       // des dégâts de l'attaque de la définition
constexpr int s_capaciteParEtage = 10;               // points de capacité offensive et défensive
constexpr int s_pourcentDExperienceParEtage = 25;    // de l'expérience de la définition

// `valeur` augmentée de `pourcent` pour cent, arrondie au plus proche (la moitié vers le haut), en entiers : le résultat ne
// dépend pas des arrondis des flottants. `valeur` et `pourcent` ne sont pas négatifs.
constexpr int majoree(int valeur, int pourcent) {
    return (valeur * (100 + pourcent) + 50) / 100;
}

// Ce qui change d'une catégorie à l'autre : une ligne du tableau de `docs/classes-et-combat.md` (section 11).
struct Profil {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    sf::Color couleur;
    float rayon;              // du cercle de collision, en pixels (le carré a pour côté le double)
    float vitesse;            // en pixels par seconde
    int pointsDeVieMax;
    int armure;
    int degats;               // avant l'armure de la cible
    float rechargeMin;        // en secondes
    float rechargeMax;
    float execution;          // en secondes, immobile avant que le coup parte
    int capaciteOffensive;    // celle d'un personnage d'Adresse 10 : 100
    int capaciteDefensive;
    int esquive;              // en pour cent, contre le contact comme contre les projectiles
    Resistances resistances;  // en pour cent par élément : à quoi cette catégorie résiste, indépendamment de son corps
    int chanceDeButin;        // en pour cent (« à tester »)
    int nombreDObjets;        // laissés quand le butin tombe
    int experience;           // rapportée à l'escouade par sa mort (« à tester »)
    // Ce que toutes les catégories partagent, sauf le tireur : le monstre de mêlée, par défaut.
    float portee = s_porteeAttaque;            // bord à bord
    float zoneDeControle = s_zoneDeControle;   // en pixels, bord à bord (0 : n'engage personne)
    NatureDAttaque nature = NatureDAttaque::Contact;
    RoleDeMonstre role = RoleMelee{};
};

constexpr Profil s_nuee = {.couleur = {115, 165, 25}, .rayon = 9.0f, .vitesse = 190.0f, .pointsDeVieMax = 60,
                           .armure = 0, .degats = 10, .rechargeMin = 0.6f, .rechargeMax = 1.0f, .execution = 0.2f,
                           .capaciteOffensive = 90, .capaciteDefensive = 80, .esquive = 10, .resistances = {},
                           .chanceDeButin = 5, .nombreDObjets = 1, .experience = 3};
constexpr Profil s_infanterie = {.couleur = {255, 0, 0}, .rayon = 15.0f, .vitesse = 160.0f, .pointsDeVieMax = 300,
                                 .armure = 3, .degats = 30, .rechargeMin = 0.7f, .rechargeMax = 1.3f,
                                 .execution = 0.3f, .capaciteOffensive = 100, .capaciteDefensive = 100, .esquive = 5,
                                 .resistances = {}, .chanceDeButin = 20, .nombreDObjets = 1, .experience = 10};
constexpr Profil s_infanterieLourde = {.couleur = {110, 125, 140}, .rayon = 17.0f, .vitesse = 110.0f,
                                       .pointsDeVieMax = 450, .armure = 10, .degats = 45, .rechargeMin = 1.2f,
                                       .rechargeMax = 1.8f, .execution = 0.4f, .capaciteOffensive = 100,
                                       .capaciteDefensive = 120, .esquive = 0, .resistances = {},
                                       .chanceDeButin = 35, .nombreDObjets = 1, .experience = 25,
                                       .role = RoleAssommeur{.dureeDeLEtourdissement = s_dureeDuCoupAssommant,
                                                             .delaiEntreCoups = s_delaiEntreCoupsAssommants}};
// Rayon 16 et non 30 : un chemin suit les centres des cases, donc à une demi-case (20 px) du mur de sa voie, et un
// cercle de rayon 20 ou plus s'y bloque (mesuré : 19 passe, 20 non). 16 laisse 4 px de marge. À rétablir quand le
// chemin tiendra compte du rayon des monstres (`docs/monstre.md`).
constexpr Profil s_creatureMonstrueuse = {.couleur = {90, 30, 140}, .rayon = 16.0f, .vitesse = 140.0f,
                                          .pointsDeVieMax = 1500, .armure = 6, .degats = 70, .rechargeMin = 1.5f,
                                          .rechargeMax = 2.5f, .execution = 0.5f, .capaciteOffensive = 120,
                                          .capaciteDefensive = 110, .esquive = 0,
                                          .resistances = Resistances::seul(Element::Feu, s_resistanceAuFeuDeLaCreature),
                                          .chanceDeButin = 100, .nombreDObjets = 3, .experience = 100};

// Tire à distance (projectile, sans zone de contrôle, comme une arme à projectile chez un personnage) et recule quand on le colle.
constexpr Profil s_tireur = {.couleur = {190, 110, 40}, .rayon = 13.0f, .vitesse = 160.0f, .pointsDeVieMax = 200,
                             .armure = 1, .degats = 25, .rechargeMin = 1.2f, .rechargeMax = 1.8f, .execution = 0.4f,
                             .capaciteOffensive = 100, .capaciteDefensive = 90, .esquive = 10, .resistances = {},
                             .chanceDeButin = 20, .nombreDObjets = 1, .experience = 12,
                             .portee = s_porteeDuTireur, .zoneDeControle = 0.0f, .nature = NatureDAttaque::Projectile,
                             .role = RoleTireur{.distanceDeContact = s_distanceDeContactDuTireur,
                                                .distanceDeRecul = s_distanceDeReculDuTireur,
                                                .delaiEntreReculs = s_delaiEntreReculsDuTireur}};

// Soigne les membres blessés de son groupe (jamais lui-même) ; sans soin à faire, il tire faiblement de loin, sans reculer.
// `const` et non `constexpr` : la liste d'effets de son soin est un `std::vector` non vide, que le compilateur ne sait pas évaluer
// à la compilation. Il n'est lu que par `catalogueDeMonstres`, donc après le démarrage.
const Profil s_soigneur ={.couleur = {40, 160, 150}, .rayon = 12.0f, .vitesse = 150.0f, .pointsDeVieMax = 180,
                               .armure = 1, .degats = 12, .rechargeMin = 1.5f, .rechargeMax = 2.5f, .execution = 0.4f,
                               .capaciteOffensive = 90, .capaciteDefensive = 90, .esquive = 5, .resistances = {},
                               .chanceDeButin = 25, .nombreDObjets = 1, .experience = 15,
                               .portee = s_porteeDuSoigneur, .zoneDeControle = 0.0f, .nature = NatureDAttaque::Projectile,
                               .role = RoleSoigneur{.soin = {.mode = ModeDeCible::Allie,
                                                             .portee = s_porteeDuSoin,
                                                             .rechargeMin = s_rechargeDuSoin,
                                                             .rechargeMax = s_rechargeDuSoin,
                                                             .effets = {EffetSoin{.montant = s_montantDuSoin}},
                                                             .tempsExecution = s_activationDuSoin},
                                                    .seuilDeBlessure = s_seuilDeBlessure,
                                                    .ward = {.mode = ModeDeCible::Allie,
                                                             .portee = s_porteeDuSoin,
                                                             .rechargeMin = s_rechargeDuWard,
                                                             .rechargeMax = s_rechargeDuWard,
                                                             .effets = {EffetAbsorption{.famille = FamilleDAbsorption::Ward,
                                                                                        .montant = s_montantDuWard,
                                                                                        .duree = s_dureeDuWard}},
                                                             .tempsExecution = s_activationDuSoin}}};

DefinitionDeMonstre definitionDuProfil(const Profil& profil) {
    return {.cote = 2.0f * profil.rayon,
            .couleur = profil.couleur,
            .vitesse = profil.vitesse,
            .pointsDeVieMax = profil.pointsDeVieMax,
            .armure = profil.armure,
            .rayonDetection = s_rayonDetection,
            .rayonTerritoire = s_rayonTerritoire,
            .zoneDeControle = profil.zoneDeControle,
            .regenerationRetour = s_regenerationRetour,
            .attaque = {.mode = ModeDeCible::Monstre,
                        .portee = profil.portee,
                        .rechargeMin = profil.rechargeMin,
                        .rechargeMax = profil.rechargeMax,
                        .effets = {EffetDegats{.montant = 0, .nature = profil.nature}},
                        .tempsExecution = profil.execution},
            .degats = degatsPhysiques(profil.degats),
            .statistiques = {.capaciteOffensive = profil.capaciteOffensive,
                             .capaciteDefensive = profil.capaciteDefensive,
                             .esquiveContact = profil.esquive, .esquiveProjectile = profil.esquive,
                             .resistances = profil.resistances},
            .chanceDeButin = profil.chanceDeButin,
            .nombreDObjets = profil.nombreDObjets,
            .experience = profil.experience,
            .role = profil.role};
}
}  // namespace

const CatalogueDeMonstres& catalogueDeMonstres() {
    static const CatalogueDeMonstres catalogue = {definitionDuProfil(s_nuee), definitionDuProfil(s_infanterie),
                                                  definitionDuProfil(s_infanterieLourde),
                                                  definitionDuProfil(s_creatureMonstrueuse), definitionDuProfil(s_tireur),
                                                  definitionDuProfil(s_soigneur)};
    return catalogue;
}

const DefinitionDeMonstre& definitionDeMonstre(const CatalogueDeMonstres& catalogue, CategorieDeMonstre categorie) {
    return catalogue[static_cast<std::size_t>(categorie)];
}

const DefinitionDeMonstre& definitionDeMonstre(CategorieDeMonstre categorie) {
    return definitionDeMonstre(catalogueDeMonstres(), categorie);
}

DefinitionDeMonstre renforcer(const DefinitionDeMonstre& definition, int profondeur) {
    assert(profondeur >= 1);
    const int etagesEnPlus = profondeur - 1;
    DefinitionDeMonstre renforcee = definition;
    renforcee.pointsDeVieMax = majoree(definition.pointsDeVieMax, s_pourcentDePointsDeVieParEtage * etagesEnPlus);
    for (const Element element : s_elements) {
        renforcee.degats[element] = majoree(definition.degats[element], s_pourcentDeDegatsParEtage * etagesEnPlus);
    }
    renforcee.statistiques.capaciteOffensive += s_capaciteParEtage * etagesEnPlus;
    renforcee.statistiques.capaciteDefensive += s_capaciteParEtage * etagesEnPlus;
    renforcee.experience = majoree(definition.experience, s_pourcentDExperienceParEtage * etagesEnPlus);
    // Le soin d'un soigneur est majoré du même pourcentage que les dégâts ; les autres rôles n'ont rien à majorer (l'étourdissement d'un
    // assommeur ne grandit pas avec la profondeur : le contrôle n'est pas un montant).
    std::visit(Surcharges{[](RoleMelee&) {},
                          [](RoleTireur&) {},
                          [](RoleAssommeur&) {},
                          [etagesEnPlus](RoleSoigneur& soigneur) {
                              EffetSoin& soin = std::get<EffetSoin>(soigneur.soin.effets.front());
                              soin.montant = majoree(soin.montant, s_pourcentDeDegatsParEtage * etagesEnPlus);
                              // Le Ward est majoré comme les points de vie (c'est de la vie en plus), non comme les dégâts.
                              EffetAbsorption& ward = std::get<EffetAbsorption>(soigneur.ward.effets.front());
                              ward.montant = majoree(ward.montant, s_pourcentDePointsDeVieParEtage * etagesEnPlus);
                          }},
               renforcee.role);
    return renforcee;
}
