// ./src/escouade/Specialite.cpp
#include "escouade/Specialite.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <utility>
#include "base/Eclaircir.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Degats.hpp"
#include "combat/Element.hpp"
#include "combat/IdSkill.hpp"
#include "combat/NatureDAttaque.hpp"
#include "combat/TypeDArme.hpp"
#include "combat/TypeSkill.hpp"
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
// Le bonus sacré de la frappe sacrée s'ajoute aux dégâts de l'arme. Calibré sur la masse de départ du paladin (15) :
// 15 + 45 = 60, le total d'avant l'étape 94 ; avec l'épée à deux mains (60), 105.
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

// Tous les montants des personnages (dégâts, soins) profitent de la Puissance, à plein et d'elle seule
// (la durée d'une provocation n'est pas un montant).
constexpr Coefficients s_puissanceSeule = Coefficients::seul(Caracteristique::Puissance, 1.0f);

// Les armes qui permettent les skills de frappe et de tir (modèle de *Titan Quest* : pas de restriction par classe).
const std::vector<TypeDArme> s_armesDeFrappePuissante = {TypeDArme::EpeeLongue, TypeDArme::Masse, TypeDArme::Dague,
                                                         TypeDArme::EpeeADeuxMains};
const std::vector<TypeDArme> s_armesDeFrappeSacree = {TypeDArme::EpeeLongue, TypeDArme::Masse, TypeDArme::EpeeADeuxMains};
const std::vector<TypeDArme> s_armesDeTir = {TypeDArme::Arc};

// Part de noir mélangée à la couleur de la classe, selon le nombre de fois où elle a déjà été prise.
constexpr std::array<float, 3> s_assombrissements = {0.0f, 0.30f, 0.55f};

// Équipement de départ : les mêlées portent une armure complète (9 en tout), un bouclier et leur arme, les tireurs
// seulement leur arme (leur attaque de base et leur zone de contrôle en viennent). Valeurs provisoires, à ajuster
// après test.
std::vector<ObjetDeDepart> equipementDeMelee(IdObjet arme) {
    return {{Emplacement::Tete, IdObjet::CasqueDeFer},
            {Emplacement::Torse, IdObjet::CuirasseDeMailles},
            {Emplacement::Jambes, IdObjet::JambieresDeFer},
            {Emplacement::MainDroite, arme},
            {Emplacement::MainGauche, IdObjet::BouclierRond}};
}

std::vector<ObjetDeDepart> equipementDeTireur(IdObjet arme) {
    return {{Emplacement::MainDroite, arme}};
}

std::vector<Specialite> construireLeCatalogue() {
    return {
        // Guerrier : devant, au contact.
        {IdSpecialite::Guerrier, "Warrior",
         {.couleur = s_couleurGuerrier,
          .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = s_porteeMelee,
                                             .puissance = s_degatsSkill, .rechargeMin = s_rechargeSkill,
                                             .rechargeMax = s_rechargeSkill, .tempsExecution = s_activationSkill, .coefficients = s_puissanceSeule,
                                             .nature = NatureDAttaque::Contact, .armesExigees = s_armesDeFrappePuissante})},
           {IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f,
                                         .puissance = s_dureeProvocation, .rechargeMin = s_rechargeProvocation,
                                         .rechargeMax = s_rechargeProvocation, .rayonZone = s_rayonProvocation,
                                         .tempsExecution = s_activationProvocation, .nature = NatureDAttaque::Contact})}},
          .equipementDeDepart = equipementDeMelee(IdObjet::EpeeLongue), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesGuerrier,
          .rang = Rang::Avant}},
        // Prêtre : derrière, soigne.
        {IdSpecialite::Pretre, "Priest",
         {.couleur = s_couleurPretre,
          .skills = {{IdSkill::Soin, Skill({.type = TypeSkill::Soin, .portee = s_porteeSoin, .puissance = s_puissanceSoin,
                                  .rechargeMin = s_rechargeSoin, .rechargeMax = s_rechargeSoin,
                                  .tempsExecution = s_activationSoin, .coefficients = s_puissanceSeule, .nature = NatureDAttaque::Contact})}},
          .equipementDeDepart = equipementDeTireur(IdObjet::Sceptre), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesPretre,
          .rang = Rang::Arriere}},
        // Mage : derrière, lance l'explosion.
        {IdSpecialite::Mage, "Mage",
         {.couleur = s_couleurMage,
          .skills = {{IdSkill::Explosion, Skill({.type = TypeSkill::Zone, .portee = s_porteeZone, .puissance = s_degatsZone,
                                       .rechargeMin = s_rechargeZone, .rechargeMax = s_rechargeZone,
                                       .rayonZone = s_rayonZone, .tempsExecution = s_activationZone, .coefficients = s_puissanceSeule,
                                       .nature = NatureDAttaque::Zone, .element = Element::Feu})}},
          .equipementDeDepart = equipementDeTireur(IdObjet::Baton), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesMage,
          .rang = Rang::Arriere}},
        // Paladin : devant, au contact, frappe et soigne.
        {IdSpecialite::Paladin, "Paladin",
         {.couleur = s_couleurPaladin,
          .skills = {{IdSkill::FrappeSacree, Skill({.type = TypeSkill::DegatsSurCible, .portee = s_porteeMelee,
                                          .puissance = 0, .rechargeMin = s_rechargeFrappeSacree,
                                          .rechargeMax = s_rechargeFrappeSacree,
                                          .tempsExecution = s_activationFrappeSacree, .coefficients = s_puissanceSeule,
                                          .nature = NatureDAttaque::Contact, .armesExigees = s_armesDeFrappeSacree,
                                          .bonusDeFrappe = degatsDe(Element::Sacre, s_bonusSacreFrappeSacree)})},
           {IdSkill::ImpositionDesMains, Skill({.type = TypeSkill::Soin, .portee = s_porteeMelee,
                                                .puissance = s_puissanceImposition,
                                                .rechargeMin = s_rechargeImposition,
                                                .rechargeMax = s_rechargeImposition,
                                                .tempsExecution = s_activationImposition, .coefficients = s_puissanceSeule, .nature = NatureDAttaque::Contact})}},
          .equipementDeDepart = equipementDeMelee(IdObjet::Masse), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesPaladin,
          .rang = Rang::Avant}},
        // Rôdeur : derrière, tire de loin et lance une pluie de flèches.
        {IdSpecialite::Rodeur, "Ranger",
         {.couleur = s_couleurRodeur,
          .skills = {{IdSkill::TirPercant, Skill({.type = TypeSkill::DegatsSurCible, .portee = s_porteeTirPercant,
                                        .puissance = s_degatsTirPercant, .rechargeMin = s_rechargeTirPercant,
                                        .rechargeMax = s_rechargeTirPercant, .tempsExecution = s_activationTirPercant, .coefficients = s_puissanceSeule,
                                        .nature = NatureDAttaque::Projectile, .armesExigees = s_armesDeTir})},
           {IdSkill::PluieDeFleches, Skill({.type = TypeSkill::Zone, .portee = s_porteePluie,
                                            .puissance = s_degatsPluie, .rechargeMin = s_rechargePluie,
                                            .rechargeMax = s_rechargePluie, .rayonZone = s_rayonPluie,
                                            .tempsExecution = s_activationPluie, .coefficients = s_puissanceSeule,
                                            .nature = NatureDAttaque::Zone, .armesExigees = s_armesDeTir})}},
          .equipementDeDepart = equipementDeTireur(IdObjet::Arc), .rayonVision = s_rayonVision,
          .caracteristiques = s_caracteristiquesRodeur,
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
