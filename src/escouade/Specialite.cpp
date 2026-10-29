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

// Paladin et rôdeur : valeurs provisoires, à ajuster après test.
// Le bonus sacré de la frappe sacrée s'ajoute aux dégâts de l'arme. Calibré à l'étape 94 sur une masse de départ de 15 (15 + 45 =
// 60) ; depuis l'étape 102, la masse de départ du paladin fait 26 en moyenne (71 en tout) : à retoucher en jouant.
constexpr int s_bonusSacreFrappeSacree = 45;
constexpr float s_rechargeFrappeSacree = 5.0f;    // en secondes
constexpr float s_activationFrappeSacree = 0.3f;  // en secondes
constexpr int s_puissanceImposition = 75;
constexpr float s_rechargeImposition = 10.0f;     // en secondes
constexpr float s_activationImposition = 0.5f;    // en secondes
constexpr float s_porteeTirPercant = 200.0f;      // bord à bord
constexpr int s_degatsTirPercant = 75;
constexpr float s_rechargeTirPercant = 6.0f;      // en secondes
constexpr float s_activationTirPercant = 0.5f;    // en secondes
constexpr float s_porteePluie = 320.0f;           // portée de lancement, bord à bord
constexpr int s_degatsPluie = 36;
constexpr float s_rechargePluie = 8.0f;           // en secondes
constexpr float s_activationPluie = 0.8f;         // en secondes
constexpr float s_rayonPluie = 45.0f;             // rayon de zone, depuis le point visé

// Caractéristiques de base de chaque classe (somme 50, provisoires, à tester) : Puissance, Endurance,
// Adresse, Savoir, Volonté. Les points de vie ne viennent que de l'Endurance (`pointsDeVieMax` : base commune
// plus `s_pvParPointDEndurance` par point) : guerrier 324, paladin 318, prêtre et rôdeur 300, mage 288.
constexpr Caracteristiques s_caracteristiquesGuerrier({12, 14, 10, 6, 8});
constexpr Caracteristiques s_caracteristiquesPaladin({11, 13, 8, 7, 11});
constexpr Caracteristiques s_caracteristiquesPretre({10, 10, 8, 10, 12});
constexpr Caracteristiques s_caracteristiquesMage({10, 8, 8, 14, 10});
constexpr Caracteristiques s_caracteristiquesRodeur({10, 10, 14, 8, 8});

// La paire de caractéristiques qui grandissent avec le niveau de chaque classe (la première au niveau 2, la seconde au niveau 3,
// en alternance ; « à tester »). Le Savoir et la Volonté n'ont encore aucun effet : les paires du prêtre et du mage deviendront
// justes quand ces caractéristiques serviront.
constexpr PaireDeCroissance s_croissanceGuerrier = {Caracteristique::Puissance, Caracteristique::Endurance};
constexpr PaireDeCroissance s_croissancePaladin = {Caracteristique::Puissance, Caracteristique::Endurance};
constexpr PaireDeCroissance s_croissancePretre = {Caracteristique::Puissance, Caracteristique::Volonte};
constexpr PaireDeCroissance s_croissanceMage = {Caracteristique::Puissance, Caracteristique::Savoir};
constexpr PaireDeCroissance s_croissanceRodeur = {Caracteristique::Adresse, Caracteristique::Puissance};

// Tous les montants des personnages (dégâts, soins) profitent de la Puissance, à plein et d'elle seule
// (la durée d'une provocation n'est pas un montant).
constexpr Coefficients s_puissanceSeule = Coefficients::seul(Caracteristique::Puissance, 1.0f);

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
                                                       .effet = EffetDegats{.montant = s_degatsSkill, .nature = NatureDAttaque::Contact, .coefficients = s_puissanceSeule},
                                                       .tempsExecution = s_activationSkill,
                                                       .armesExigees = s_armesDeContact})},
           {IdSkill::Provocation, Skill({.mode = ModeDeCible::Soi,
                                         .portee = 0.0f,
                                         .rechargeMin = s_rechargeProvocation,
                                         .rechargeMax = s_rechargeProvocation,
                                         .effet = EffetProvocation{.duree = s_dureeProvocation},
                                         .rayonZone = s_rayonProvocation,
                                         .tempsExecution = s_activationProvocation})}},
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
                                            .effet = EffetSoin{.montant = s_puissanceSoin, .coefficients = s_puissanceSeule},
                                            .tempsExecution = s_activationSoin})}},
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
                                                 .effet = EffetDegats{.montant = s_degatsZone, .nature = NatureDAttaque::Zone, .element = Element::Feu, .coefficients = s_puissanceSeule},
                                                 .rayonZone = s_rayonZone,
                                                 .tempsExecution = s_activationZone})}},
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
                                                    .effet = EffetFrappe{.bonus = degatsDe(Element::Sacre, s_bonusSacreFrappeSacree), .coefficients = s_puissanceSeule},
                                                    .tempsExecution = s_activationFrappeSacree,
                                                    .armesExigees = s_armesDeContact})},
           {IdSkill::ImpositionDesMains, Skill({.mode = ModeDeCible::Allie,
                                                .portee = s_porteeMelee,
                                                .rechargeMin = s_rechargeImposition,
                                                .rechargeMax = s_rechargeImposition,
                                                .effet = EffetSoin{.montant = s_puissanceImposition, .coefficients = s_puissanceSeule},
                                                .tempsExecution = s_activationImposition})}},
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
                                                  .effet = EffetDegats{.montant = s_degatsTirPercant, .nature = NatureDAttaque::Projectile, .coefficients = s_puissanceSeule},
                                                  .tempsExecution = s_activationTirPercant,
                                                  .armesExigees = s_armesDeTir})},
           {IdSkill::PluieDeFleches, Skill({.mode = ModeDeCible::Point,
                                            .portee = s_porteePluie,
                                            .rechargeMin = s_rechargePluie,
                                            .rechargeMax = s_rechargePluie,
                                            .effet = EffetDegats{.montant = s_degatsPluie, .nature = NatureDAttaque::Zone, .coefficients = s_puissanceSeule},
                                            .rayonZone = s_rayonPluie,
                                            .tempsExecution = s_activationPluie,
                                            .armesExigees = s_armesDeTir})}},
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
