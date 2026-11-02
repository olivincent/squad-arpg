// ./src/escouade/Specialite.cpp
#include "escouade/Specialite.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <utility>
#include "base/Eclaircir.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Degats.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/Element.hpp"
#include "combat/Energie.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "combat/IdSkill.hpp"
#include "combat/ModeDeCible.hpp"
#include "combat/NatureDAttaque.hpp"
#include "combat/TypeDArme.hpp"
#include "escouade/TailleDeLEscouade.hpp"
#include "objets/Equipement.hpp"

namespace {
const sf::Color s_couleurGuerrier = sf::Color::Red;
const sf::Color s_couleurPretre(255, 240, 150);  // jaune pâle
const sf::Color s_couleurMage(160, 80, 220);     // violet
const sf::Color s_couleurPaladin(255, 140, 0);   // orange
const sf::Color s_couleurRodeur(50, 170, 70);    // vert
constexpr float s_porteeMelee = 20.0f;    // bord à bord
constexpr float s_rayonVision = 320.0f;  // en pixels (8 cases), le même pour tous
constexpr int s_degatsSkill = 75;
constexpr float s_rechargeSkill = 5.0f;   // en secondes
constexpr float s_activationSkill = 0.3f; // en secondes, immobile avant que l'effet parte
// Bénédiction du prêtre (étape 111, « à tester ») : un buff d'armure pour toute l'escouade. Un sort : sa durée profite de l'Intellect, mais
// jamais au-delà de sa recharge (`Personnage::effetsActifsDuSkill`) ; 10 s de base avec +100 % au plafond font exactement les 20 s de la recharge.
constexpr int s_bonusDArmureBenediction = 20;       // en pour cent de l'armure
constexpr int s_dureeBenediction = 10;              // en secondes, de base
constexpr float s_rechargeBenediction = 20.0f;      // en secondes
constexpr float s_activationBenediction = 0.5f;     // en secondes, immobile avant que l'effet parte
constexpr int s_malusDArmureFrappePuissante = -25;       // en pour cent de l'armure de la cible (« à tester »)
constexpr int s_dureeMalusDArmureFrappePuissante = 8;    // en secondes
// Renew du prêtre (étape 112, « à tester ») : un soin dans le temps sur l'allié le plus blessé, sur la Volonté comme le soin. 11 points par seconde
// pendant 8 s, soit 88 points : du même ordre que les 90 du soin. Recharge de 10 s (plus que sa durée de 8 s : pas de chevauchement), activation
// plus courte que celle du soin, coût d'un sort (règle du soin, `coutDUnSort`). Un coefficient de durée de 0, comme tout effet dans le temps.
constexpr int s_soinRenouvellement = 11;           // par seconde
constexpr int s_dureeRenouvellement = 8;           // en secondes
constexpr float s_rechargeRenouvellement = 10.0f;  // en secondes
constexpr float s_activationRenouvellement = 0.4f; // en secondes
// Aegis du paladin (étape 114, « à tester ») : un bouclier d'absorption du physique sur l'allié le plus blessé (mode `Allie`, comme le soin),
// sur la Volonté comme les soins du paladin. 60 points au niveau 1 (environ 20 % des points de vie de départ), 8 s de base, recharge de 15 s. Un
// buff : sa durée profite du bonus de durée de l'Intellect (coefficient 1) mais ne dépasse jamais la recharge (`Personnage::effetsActifsDuSkill`) ;
// 8 s de base avec +100 % au plafond font 16 s, ramenées à 15. Portée, activation et règle du coût de Soin.
constexpr int s_montantAegis = 60;                // points absorbés
constexpr int s_dureeAegis = 8;                   // en secondes, de base
constexpr float s_rechargeAegis = 15.0f;          // en secondes
constexpr float s_porteeSoin = 40.0f;     // bord à bord
constexpr int s_puissanceSoin = 90;
constexpr float s_rechargeSoin = 8.0f;    // en secondes
constexpr float s_activationSoin = 0.6f;  // en secondes
constexpr float s_rayonProvocation = 150.0f;  // rayon de zone, depuis le centre du provocateur
constexpr int s_dureeProvocation = 4;         // en secondes (la puissance de ce skill)
constexpr float s_rechargeProvocation = 10.0f;  // en secondes
constexpr float s_activationProvocation = 0.2f;  // en secondes
constexpr float s_porteeZone = 250.0f;        // portée de lancement du skill de zone, bord à bord
constexpr int s_degatsZone = 45;
constexpr float s_rechargeZone = 8.0f;        // en secondes
constexpr float s_activationZone = 0.8f;      // en secondes
constexpr float s_rayonZone = 60.0f;          // rayon de zone, depuis le point visé
// Brûlure de l'explosion (étape 112, « à tester ») : du feu dans le temps sur chaque monstre touché, sur le Savoir du mage, une seule pile
// (lancer l'explosion de nouveau rafraîchit). 8 points par seconde pendant 4 s, soit 32 points (71 % des 45 de l'explosion). Coefficient de
// durée de 0, comme tout effet dans le temps : son montant gagne déjà par le Savoir.
constexpr int s_brulureExplosion = 8;         // par seconde
constexpr int s_dureeBrulure = 4;             // en secondes
// Contresort (Counterspell) du mage (étape 113, « à tester ») : un sort sans dégâts qui met un monstre en Disruption, donc l'empêche de lancer
// ses capacités (le soin du soigneur, par exemple) pendant 2 s de base, majorées de moitié du bonus de durée de l'Intellect. Même portée que l'explosion,
// activation courte (c'est une interruption), recharge de 10 s : son coût de sort (53, règle de `coutDUnSort`) laisse le mage lancer aussi
// une explosion (43) avec sa réserve de départ (106).
constexpr float s_porteeContresort = 250.0f;          // bord à bord
constexpr float s_dureeDeDisruptionContresort = 2.0f; // en secondes, de base
constexpr float s_coefficientDeDureeContresort = 0.5f;
constexpr float s_rechargeContresort = 10.0f;         // en secondes
constexpr float s_activationContresort = 0.3f;        // en secondes

// Le coût en énergie des skills (étape 108), déduit d'une règle de calibrage et non choisi un par un : à partir d'une réserve pleine
// de niveau 1, un personnage qui lance un skill à chaque fin de recharge, avec la régénération de base, tombe à sec en `T` secondes.
// Avec la recharge `r` du skill, il dépense `coût / r` points par seconde et en regagne `s_regenerationDEnergieDeBase` :
//   coût / r − régénération = réserve / T, donc coût = r × (régénération + réserve / T),
// arrondi au plus proche. `T` vaut 30 s pour un sort (l'explosion du mage, le soin du prêtre) et 120 s pour une technique (tout autre
// skill de classe). Cette distinction ne sert qu'au calibrage : ce n'est pas un concept du jeu, un skill ne la porte pas. « À tester ».
constexpr float s_secondesAvantLeVideDUnSort = 30.0f;
constexpr float s_secondesAvantLeVideDUneTechnique = 120.0f;
constexpr int coutCalibre(float recharge, float secondesAvantLeVide) {
    return static_cast<int>(recharge * (s_regenerationDEnergieDeBase + static_cast<float>(s_energieDeBase) / secondesAvantLeVide) + 0.5f);
}
constexpr int coutDUnSort(float recharge) {
    return coutCalibre(recharge, s_secondesAvantLeVideDUnSort);
}
constexpr int coutDUneTechnique(float recharge) {
    return coutCalibre(recharge, s_secondesAvantLeVideDUneTechnique);
}

// Paladin et rôdeur : valeurs provisoires, à ajuster après test.
// Le bonus sacré de la frappe sacrée s'ajoute aux dégâts de l'arme. Calibré à l'étape 94 sur une masse de départ de 15 (15 + 45 =
// 60) ; depuis l'étape 102, la masse de départ du paladin fait 26 en moyenne (71 en tout) : à retoucher en jouant.
constexpr int s_bonusSacreFrappeSacree = 45;
constexpr float s_rechargeFrappeSacree = 5.0f;    // en secondes
constexpr float s_activationFrappeSacree = 0.3f;  // en secondes
// Étourdissement de la frappe sacrée (étape 113, « à tester ») : si le coup touche, le monstre ne fait plus rien pendant 1,5 s (avant sa résistance
// et le rendement décroissant). Coefficient de durée de 0 : une frappe, la durée ne profite pas de l'Intellect.
constexpr float s_dureeEtourdissementFrappeSacree = 1.5f;  // en secondes
constexpr int s_puissanceImposition = 75;
constexpr float s_rechargeImposition = 10.0f;     // en secondes
constexpr float s_activationImposition = 0.5f;    // en secondes
constexpr float s_porteeTirPercant = 200.0f;      // bord à bord
constexpr int s_degatsTirPercant = 75;
constexpr float s_rechargeTirPercant = 6.0f;      // en secondes
constexpr float s_activationTirPercant = 0.5f;    // en secondes
// Saignement du tir perçant (étape 112, « à tester ») : du physique dans le temps, qui ignore l'armure et suit la Puissance du tireur. Une pile
// vaut 8 points par seconde pendant 6 s, soit 48 points (64 % des 75 du tir) ; jusqu'à 3 piles, donc 24 par seconde au mieux. Un effet dans le temps
// a un coefficient de durée de 0 : son montant gagne déjà par la Puissance, la durée ne gagnerait pas deux fois.
constexpr int s_saignementTirPercant = 8;         // par seconde et par pile
constexpr int s_dureeSaignement = 6;              // en secondes
constexpr int s_pilesDeSaignement = 3;
constexpr float s_porteePluie = 320.0f;           // portée de lancement, bord à bord
constexpr int s_degatsPluie = 36;
constexpr float s_rechargePluie = 8.0f;           // en secondes
constexpr float s_activationPluie = 0.8f;         // en secondes
constexpr float s_rayonPluie = 45.0f;             // rayon de zone, depuis le point visé
// Ralentissement et entrave de la pluie de flèches (étape 113, « à tester ») : chaque monstre touché avance 30 % moins vite ET frappe 20 % moins
// vite pendant 3 s. Deux effets actifs d'un même skill sur la même cible, distingués par leur rang. Coefficient de durée de 0 : une technique,
// la durée ne profite pas de l'Intellect. Une zone ne s'esquive pas : tout monstre touché est affecté, sauf immunité (rendement décroissant).
constexpr int s_ralentissementPluie = 30;         // en pour cent de la vitesse de déplacement
constexpr int s_entravePluie = 20;                // en pour cent de la vitesse d'attaque de base
constexpr float s_dureeDeControlePluie = 3.0f;    // en secondes

// Caractéristiques de base de chaque classe (somme 50, provisoires, à tester) : Puissance, Endurance,
// Adresse, Savoir, Volonté. Les points de vie ne viennent que de l'Endurance (`pointsDeVieMax` : base commune
// plus `s_pvParPointDEndurance` par point) : guerrier 324, paladin 318, prêtre et rôdeur 300, mage 288.
constexpr Caracteristiques s_caracteristiquesGuerrier({12, 14, 10, 6, 8});
constexpr Caracteristiques s_caracteristiquesPaladin({11, 13, 8, 7, 11});
constexpr Caracteristiques s_caracteristiquesPretre({8, 10, 8, 12, 12});
constexpr Caracteristiques s_caracteristiquesMage({6, 8, 8, 16, 12});
constexpr Caracteristiques s_caracteristiquesRodeur({10, 10, 14, 8, 8});

// La paire de caractéristiques qui grandissent avec le niveau de chaque classe (la première au niveau 2, la seconde au niveau 3,
// en alternance ; « à tester »). Le Savoir et la Volonté ont des effets depuis l'étape 110 : le mage grandit en Savoir puis en Volonté,
// le prêtre en Volonté puis en Savoir, le paladin en Puissance puis en Volonté (ses soins).
constexpr PaireDeCroissance s_croissanceGuerrier = {Caracteristique::Puissance, Caracteristique::Endurance};
constexpr PaireDeCroissance s_croissancePaladin = {Caracteristique::Puissance, Caracteristique::Volonte};
constexpr PaireDeCroissance s_croissancePretre = {Caracteristique::Volonte, Caracteristique::Savoir};
constexpr PaireDeCroissance s_croissanceMage = {Caracteristique::Savoir, Caracteristique::Volonte};
constexpr PaireDeCroissance s_croissanceRodeur = {Caracteristique::Adresse, Caracteristique::Puissance};

// La convention des coefficients d'un skill (étape 110) : une caractéristique à plein et elle seule, selon ce que fait le skill.
// La Puissance pour ce qui frappe avec l'arme (frappes, tirs), le Savoir pour un sort de dégâts, la Volonté pour un soin ou une
// protection. Une provocation n'a aucun coefficient : sa durée n'est pas un montant. L'aire (`coefficientDAire`) suit la même idée : 1 pour un
// sort de zone (l'explosion), 0 pour une zone qui n'en est pas un (la pluie de flèches, la provocation). La durée (`coefficientDeDuree`) vaut
// 0 pour tout effet dans le temps qui gagne déjà par son montant, pour ne pas gagner deux fois.
constexpr Coefficients s_puissanceSeule = Coefficients::seul(Caracteristique::Puissance, 1.0f);
constexpr Coefficients s_savoirSeul = Coefficients::seul(Caracteristique::Savoir, 1.0f);
constexpr Coefficients s_volonteSeule = Coefficients::seul(Caracteristique::Volonte, 1.0f);

// Les armes qui permettent les skills de frappe et de tir (modèle de *Titan Quest* : pas de restriction par classe). Les skills
// de frappe (frappe puissante, frappe sacrée) veulent une arme de contact, quelle qu'elle soit ; les skills de tir (tir perçant,
// pluie de flèches) un arc ou une arbalète.
const std::vector<TypeDArme> s_armesDeContact = {TypeDArme::Dague,          TypeDArme::EpeeAUneMain,    TypeDArme::HacheAUneMain,
                                                 TypeDArme::MasseAUneMain,  TypeDArme::Lance,           TypeDArme::EpeeADeuxMains,
                                                 TypeDArme::HacheADeuxMains, TypeDArme::MasseADeuxMains};
const std::vector<TypeDArme> s_armesDeTir = {TypeDArme::Arc, TypeDArme::Arbalete};

// Part de noir mélangée à la couleur de la classe, selon le nombre de fois où elle a déjà été prise : une classe peut remplir toute
// l'escouade, donc une nuance par place.
constexpr std::array<float, 4> s_assombrissements = {0.0f, 0.20f, 0.40f, 0.60f};
static_assert(s_assombrissements.size() >= s_tailleDeLEscouade);

// Équipement de départ (niveau 1, qualité moyenne, communs) : une seule pièce d'armure, le torse (un justaucorps de cuir pour le
// guerrier, le paladin et le rôdeur, une robe pour le prêtre et le mage), l'arme, un bouclier pour les mêlées, une relique sacrée
// pour le prêtre. Valeurs
// provisoires, à ajuster après test.
std::vector<ObjetDeDepart> equipementDeMelee(IdObjet arme) {
    return {{Emplacement::Torse, IdObjet::Justaucorps},
            {Emplacement::MainDroite, arme},
            {Emplacement::MainGauche, IdObjet::Bouclier}};
}

std::vector<ObjetDeDepart> equipementDeTireur(IdObjet arme, IdObjet torse) {
    return {{Emplacement::Torse, torse}, {Emplacement::MainDroite, arme}};
}

// Le prêtre tient en plus une relique sacrée en main gauche.
std::vector<ObjetDeDepart> equipementDuPretre(IdObjet arme) {
    std::vector<ObjetDeDepart> equipement = equipementDeTireur(arme, IdObjet::Robe);
    equipement.push_back({Emplacement::MainGauche, IdObjet::ReliqueSacree});
    return equipement;
}

std::vector<Specialite> construireLeCatalogue() {
    return {
        // Guerrier : devant, au contact.
        {IdSpecialite::Guerrier, "Warrior",
         {.couleur = s_couleurGuerrier,
          .skills = {{IdSkill::FrappePuissante, Skill({.mode = ModeDeCible::Monstre,
                                                       .portee = s_porteeMelee,
                                                       .rechargeMin = s_rechargeSkill,
                                                       .rechargeMax = s_rechargeSkill,
                                                       .effets = {EffetDegats{.montant = s_degatsSkill, .nature = NatureDAttaque::Contact, .coefficients = s_puissanceSeule},
                                                                  // Si le coup touche : l'armure de la cible baisse un moment. Une technique, pas un sort : la durée ne profite pas de l'Intellect.
                                                                  EffetModificateur{.statistique = StatistiqueModifiable::Armure,
                                                                                    .pourcentage = s_malusDArmureFrappePuissante,
                                                                                    .duree = s_dureeMalusDArmureFrappePuissante}},
                                                       .tempsExecution = s_activationSkill,
                                                       .armesExigees = s_armesDeContact,
                                                       .cout = coutDUneTechnique(s_rechargeSkill)})},
           {IdSkill::Provocation, Skill({.mode = ModeDeCible::Soi,
                                         .portee = 0.0f,
                                         .rechargeMin = s_rechargeProvocation,
                                         .rechargeMax = s_rechargeProvocation,
                                         .effets = {EffetProvocation{.duree = s_dureeProvocation}},
                                         .rayonZone = s_rayonProvocation,
                                         .tempsExecution = s_activationProvocation,
                                         .cout = coutDUneTechnique(s_rechargeProvocation)})}},
          .equipementDeDepart = equipementDeMelee(IdObjet::EpeeAUneMain), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesGuerrier, .croissance = s_croissanceGuerrier,
          .rang = Rang::Avant}},
        // Prêtre : derrière, soigne.
        {IdSpecialite::Pretre, "Priest",
         {.couleur = s_couleurPretre,
          .skills = {{IdSkill::Soin, Skill({.mode = ModeDeCible::Allie,
                                            .portee = s_porteeSoin,
                                            .rechargeMin = s_rechargeSoin,
                                            .rechargeMax = s_rechargeSoin,
                                            .effets = {EffetSoin{.montant = s_puissanceSoin, .coefficients = s_volonteSeule}},
                                            .tempsExecution = s_activationSoin,
                                            .cout = coutDUnSort(s_rechargeSoin)})},
           // Le coût suit la règle des techniques (57), pas celle des sorts (107, plus que la réserve de départ) : une recharge de 20 s
           // ne se règle pas sur une réserve de 30 s.
           {IdSkill::Benediction, Skill({.mode = ModeDeCible::Escouade,
                                         .portee = 0.0f,
                                         .rechargeMin = s_rechargeBenediction,
                                         .rechargeMax = s_rechargeBenediction,
                                         .effets = {EffetModificateur{.statistique = StatistiqueModifiable::Armure,
                                                                      .pourcentage = s_bonusDArmureBenediction,
                                                                      .duree = s_dureeBenediction,
                                                                      .coefficientDeDuree = 1.0f}},
                                         .tempsExecution = s_activationBenediction,
                                         .cout = coutDUneTechnique(s_rechargeBenediction)})},
           // Un soin dans le temps sur l'allié le plus blessé (mode `Allie`, comme le soin) : rien tout de suite, des ticks ensuite.
           {IdSkill::Renouvellement, Skill({.mode = ModeDeCible::Allie,
                                            .portee = s_porteeSoin,
                                            .rechargeMin = s_rechargeRenouvellement,
                                            .rechargeMax = s_rechargeRenouvellement,
                                            .effets = {EffetSoinDansLeTemps{.montant = s_soinRenouvellement,
                                                                            .duree = s_dureeRenouvellement,
                                                                            .coefficients = s_volonteSeule}},
                                            .tempsExecution = s_activationRenouvellement,
                                            .cout = coutDUnSort(s_rechargeRenouvellement)})}},
          .equipementDeDepart = equipementDuPretre(IdObjet::Sceptre), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesPretre, .croissance = s_croissancePretre,
          .rang = Rang::Arriere}},
        // Mage : derrière, lance l'explosion.
        {IdSpecialite::Mage, "Mage",
         {.couleur = s_couleurMage,
          .skills = {{IdSkill::Explosion, Skill({.mode = ModeDeCible::Point,
                                                 .portee = s_porteeZone,
                                                 .rechargeMin = s_rechargeZone,
                                                 .rechargeMax = s_rechargeZone,
                                                 .effets = {EffetDegats{.montant = s_degatsZone, .nature = NatureDAttaque::Zone, .element = Element::Feu, .coefficients = s_savoirSeul},
                                                            // Chaque monstre que la zone touche brûle un moment.
                                                            EffetDegatsDansLeTemps{.montant = s_brulureExplosion,
                                                                                   .duree = s_dureeBrulure,
                                                                                   .element = Element::Feu,
                                                                                   .coefficients = s_savoirSeul}},
                                                 .rayonZone = s_rayonZone,
                                                 .coefficientDAire = 1.0f,  // un sort : son aire profite de l'Intellect
                                                 .tempsExecution = s_activationZone,
                                                 .cout = coutDUnSort(s_rechargeZone)})},
           // Un sort sans dégâts sur un monstre : il ne peut plus lancer de skill pendant un moment.
           {IdSkill::Contresort, Skill({.mode = ModeDeCible::Monstre,
                                        .portee = s_porteeContresort,
                                        .rechargeMin = s_rechargeContresort,
                                        .rechargeMax = s_rechargeContresort,
                                        .effets = {EffetControle{.etat = EtatInflige::Disruption,
                                                                 .duree = s_dureeDeDisruptionContresort,
                                                                 .coefficientDeDuree = s_coefficientDeDureeContresort}},
                                        .tempsExecution = s_activationContresort,
                                        .cout = coutDUnSort(s_rechargeContresort)})}},
          .equipementDeDepart = equipementDeTireur(IdObjet::Baton, IdObjet::Robe), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesMage, .croissance = s_croissanceMage,
          .rang = Rang::Arriere}},
        // Paladin : devant, au contact, frappe et soigne.
        {IdSpecialite::Paladin, "Paladin",
         {.couleur = s_couleurPaladin,
          .skills = {{IdSkill::FrappeSacree, Skill({.mode = ModeDeCible::Monstre,
                                                    .portee = s_porteeMelee,
                                                    .rechargeMin = s_rechargeFrappeSacree,
                                                    .rechargeMax = s_rechargeFrappeSacree,
                                                    .effets = {EffetFrappe{.bonus = degatsDe(Element::Sacre, s_bonusSacreFrappeSacree), .coefficients = s_puissanceSeule},
                                                               // Si le coup touche : le monstre est étourdi un moment.
                                                               EffetControle{.etat = EtatInflige::Etourdissement, .duree = s_dureeEtourdissementFrappeSacree}},
                                                    .tempsExecution = s_activationFrappeSacree,
                                                    .armesExigees = s_armesDeContact,
                                                    .cout = coutDUneTechnique(s_rechargeFrappeSacree)})},
           {IdSkill::ImpositionDesMains, Skill({.mode = ModeDeCible::Allie,
                                                .portee = s_porteeMelee,
                                                .rechargeMin = s_rechargeImposition,
                                                .rechargeMax = s_rechargeImposition,
                                                .effets = {EffetSoin{.montant = s_puissanceImposition, .coefficients = s_volonteSeule}},
                                                .tempsExecution = s_activationImposition,
                                                .cout = coutDUneTechnique(s_rechargeImposition)})},
           // Un bouclier contre le physique sur l'allié le plus blessé : la règle du coût d'un sort, comme le soin dont il reprend la portée et l'activation.
           {IdSkill::Aegis, Skill({.mode = ModeDeCible::Allie,
                                   .portee = s_porteeSoin,
                                   .rechargeMin = s_rechargeAegis,
                                   .rechargeMax = s_rechargeAegis,
                                   .effets = {EffetAbsorption{.famille = FamilleDAbsorption::Aegis,
                                                              .montant = s_montantAegis,
                                                              .duree = s_dureeAegis,
                                                              .coefficients = s_volonteSeule,
                                                              .coefficientDeDuree = 1.0f}},
                                   .tempsExecution = s_activationSoin,
                                   .cout = coutDUnSort(s_rechargeAegis)})}},
          .equipementDeDepart = equipementDeMelee(IdObjet::MasseAUneMain), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesPaladin, .croissance = s_croissancePaladin,
          .rang = Rang::Avant}},
        // Rôdeur : derrière, tire de loin et lance une pluie de flèches.
        {IdSpecialite::Rodeur, "Ranger",
         {.couleur = s_couleurRodeur,
          .skills = {{IdSkill::TirPercant, Skill({.mode = ModeDeCible::Monstre,
                                                  .portee = s_porteeTirPercant,
                                                  .rechargeMin = s_rechargeTirPercant,
                                                  .rechargeMax = s_rechargeTirPercant,
                                                  .effets = {EffetDegats{.montant = s_degatsTirPercant, .nature = NatureDAttaque::Projectile, .coefficients = s_puissanceSeule},
                                                             // Si le tir touche : le monstre saigne, chaque tir de plus ajoute une pile.
                                                             EffetDegatsDansLeTemps{.montant = s_saignementTirPercant,
                                                                                    .duree = s_dureeSaignement,
                                                                                    .element = Element::Physique,
                                                                                    .pilesMax = s_pilesDeSaignement,
                                                                                    .coefficients = s_puissanceSeule}},
                                                  .tempsExecution = s_activationTirPercant,
                                                  .armesExigees = s_armesDeTir,
                                                  .cout = coutDUneTechnique(s_rechargeTirPercant)})},
           {IdSkill::PluieDeFleches, Skill({.mode = ModeDeCible::Point,
                                            .portee = s_porteePluie,
                                            .rechargeMin = s_rechargePluie,
                                            .rechargeMax = s_rechargePluie,
                                            .effets = {EffetDegats{.montant = s_degatsPluie, .nature = NatureDAttaque::Zone, .coefficients = s_puissanceSeule},
                                                       // Chaque monstre touché est ralenti et entravé un moment.
                                                       EffetControle{.etat = EtatInflige::Ralentissement,
                                                                     .duree = s_dureeDeControlePluie,
                                                                     .pourcentage = s_ralentissementPluie},
                                                       EffetControle{.etat = EtatInflige::Entrave,
                                                                     .duree = s_dureeDeControlePluie,
                                                                     .pourcentage = s_entravePluie}},
                                            .rayonZone = s_rayonPluie,
                                            .tempsExecution = s_activationPluie,
                                            .armesExigees = s_armesDeTir,
                                            .cout = coutDUneTechnique(s_rechargePluie)})}},
          .equipementDeDepart = equipementDeTireur(IdObjet::Arc, IdObjet::Justaucorps), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesRodeur, .croissance = s_croissanceRodeur,
          .rang = Rang::Arriere}},
    };
}
}  // namespace

const std::vector<Specialite>& catalogue() {
    static const std::vector<Specialite> s_catalogue = construireLeCatalogue();
    return s_catalogue;
}

const Specialite& specialite(IdSpecialite id) {
    const Specialite& trouvee = catalogue()[static_cast<std::size_t>(id)];
    assert(trouvee.id == id);  // le catalogue suit l'ordre de IdSpecialite
    return trouvee;
}

std::vector<DefinitionPersonnage> composer(const std::vector<IdSpecialite>& identifiants) {
    std::vector<DefinitionPersonnage> composition;
    std::vector<std::size_t> dejaPrises(catalogue().size(), 0);
    for (const IdSpecialite id : identifiants) {
        std::size_t& occurrences = dejaPrises[static_cast<std::size_t>(id)];
        assert(occurrences < s_assombrissements.size());
        DefinitionPersonnage definition = specialite(id).definition;
        definition.couleur = assombrir(definition.couleur, s_assombrissements[occurrences]);
        composition.push_back(std::move(definition));
        ++occurrences;
    }
    return composition;
}
