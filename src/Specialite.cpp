// ./src/Specialite.cpp
#include "Specialite.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <utility>
#include "Eclaircir.hpp"
#include "IdSkill.hpp"
#include "TypeSkill.hpp"

namespace {
const sf::Color s_couleurGuerrier = sf::Color::Red;
const sf::Color s_couleurPretre(255, 240, 150);  // jaune pâle
const sf::Color s_couleurMage(160, 80, 220);     // violet
const sf::Color s_couleurPaladin(255, 140, 0);   // orange
const sf::Color s_couleurRodeur(50, 170, 70);    // vert
constexpr float s_porteeMelee = 20.0f;    // bord à bord
constexpr float s_porteeTireur = 120.0f;  // bord à bord
constexpr int s_armureMelee = 50;
constexpr int s_armureTireur = 0;
constexpr float s_rayonVision = 320.0f;  // en pixels (8 cases), le même pour tous
constexpr float s_zoneDeControleMelee = 20.0f;   // bord à bord : les mêlées engagent ce qui les touche presque
constexpr float s_zoneDeControleTireur = 0.0f;   // les tireurs n'engagent personne (mais peuvent être engagés)
constexpr int s_degatsAttaqueDeBase = 5;
constexpr float s_executionAttaqueDeBase = 0.15f;  // en secondes, immobile avant que le coup parte
constexpr float s_rechargeMeleeMin = 0.3f;   // attaque de base des mêlées, en secondes
constexpr float s_rechargeMeleeMax = 0.7f;
constexpr float s_rechargeTireurMin = 0.4f;  // attaque de base des tireurs, en secondes
constexpr float s_rechargeTireurMax = 0.6f;
constexpr int s_degatsSkill = 25;
constexpr float s_rechargeSkill = 5.0f;   // en secondes
constexpr float s_activationSkill = 0.3f; // en secondes, immobile avant que l'effet parte
constexpr float s_porteeSoin = 40.0f;     // bord à bord
constexpr int s_puissanceSoin = 30;
constexpr float s_rechargeSoin = 8.0f;    // en secondes
constexpr float s_activationSoin = 0.6f;  // en secondes
constexpr float s_rayonProvocation = 150.0f;  // rayon de zone, depuis le centre du provocateur
constexpr int s_dureeProvocation = 4;         // en secondes (la puissance de ce skill)
constexpr float s_rechargeProvocation = 10.0f;  // en secondes
constexpr float s_activationProvocation = 0.2f;  // en secondes
constexpr float s_porteeZone = 250.0f;        // portée de lancement du skill de zone, bord à bord
constexpr int s_degatsZone = 15;
constexpr float s_rechargeZone = 8.0f;        // en secondes
constexpr float s_activationZone = 0.8f;      // en secondes
constexpr float s_rayonZone = 60.0f;          // rayon de zone, depuis le point visé

// Paladin et rôdeur : valeurs provisoires, à ajuster après test.
constexpr int s_degatsFrappeSacree = 20;
constexpr float s_rechargeFrappeSacree = 5.0f;    // en secondes
constexpr float s_activationFrappeSacree = 0.3f;  // en secondes
constexpr int s_puissanceImposition = 25;
constexpr float s_rechargeImposition = 10.0f;     // en secondes
constexpr float s_activationImposition = 0.5f;    // en secondes
constexpr float s_porteeTirPercant = 200.0f;      // bord à bord
constexpr int s_degatsTirPercant = 25;
constexpr float s_rechargeTirPercant = 6.0f;      // en secondes
constexpr float s_activationTirPercant = 0.5f;    // en secondes
constexpr float s_porteePluie = 320.0f;           // portée de lancement, bord à bord
constexpr int s_degatsPluie = 12;
constexpr float s_rechargePluie = 8.0f;           // en secondes
constexpr float s_activationPluie = 0.8f;         // en secondes
constexpr float s_rayonPluie = 45.0f;             // rayon de zone, depuis le point visé

// Part de noir mélangée à la couleur de la classe, selon le nombre de fois où elle a déjà été prise.
constexpr std::array<float, 3> s_assombrissements = {0.0f, 0.30f, 0.55f};

// Attaque de base d'un personnage, mêlée ou tireur : la portée et l'intervalle de recharge changent.
Skill attaqueDeBase(float portee, float rechargeMin, float rechargeMax) {
    return Skill(TypeSkill::Degats, portee, s_degatsAttaqueDeBase, rechargeMin, rechargeMax, 0.0f,
                 s_executionAttaqueDeBase);
}

Skill attaqueDeBaseDeMelee() {
    return attaqueDeBase(s_porteeMelee, s_rechargeMeleeMin, s_rechargeMeleeMax);
}

Skill attaqueDeBaseDeTireur() {
    return attaqueDeBase(s_porteeTireur, s_rechargeTireurMin, s_rechargeTireurMax);
}

std::vector<Specialite> construireLeCatalogue() {
    return {
        // Guerrier : devant, au contact.
        {IdSpecialite::Guerrier, "Guerrier",
         {s_couleurGuerrier, attaqueDeBaseDeMelee(),
          {{IdSkill::FrappePuissante, Skill(TypeSkill::Degats, s_porteeMelee, s_degatsSkill, s_rechargeSkill, s_rechargeSkill, 0.0f, s_activationSkill)},
           {IdSkill::Provocation, Skill(TypeSkill::Provocation, 0.0f, s_dureeProvocation, s_rechargeProvocation, s_rechargeProvocation, s_rayonProvocation, s_activationProvocation)}},
          s_armureMelee, s_rayonVision, s_zoneDeControleMelee, Rang::Avant}},
        // Prêtre : derrière, soigne.
        {IdSpecialite::Pretre, "Prêtre",
         {s_couleurPretre, attaqueDeBaseDeTireur(),
          {{IdSkill::Soin, Skill(TypeSkill::Soin, s_porteeSoin, s_puissanceSoin, s_rechargeSoin, s_rechargeSoin, 0.0f, s_activationSoin)}},
          s_armureTireur, s_rayonVision, s_zoneDeControleTireur, Rang::Arriere}},
        // Mage : derrière, lance l'explosion.
        {IdSpecialite::Mage, "Mage",
         {s_couleurMage, attaqueDeBaseDeTireur(),
          {{IdSkill::Explosion, Skill(TypeSkill::Zone, s_porteeZone, s_degatsZone, s_rechargeZone, s_rechargeZone, s_rayonZone, s_activationZone)}},
          s_armureTireur, s_rayonVision, s_zoneDeControleTireur, Rang::Arriere}},
        // Paladin : devant, au contact, frappe et soigne.
        {IdSpecialite::Paladin, "Paladin",
         {s_couleurPaladin, attaqueDeBaseDeMelee(),
          {{IdSkill::FrappeSacree, Skill(TypeSkill::Degats, s_porteeMelee, s_degatsFrappeSacree, s_rechargeFrappeSacree, s_rechargeFrappeSacree, 0.0f, s_activationFrappeSacree)},
           {IdSkill::ImpositionDesMains, Skill(TypeSkill::Soin, s_porteeMelee, s_puissanceImposition, s_rechargeImposition, s_rechargeImposition, 0.0f, s_activationImposition)}},
          s_armureMelee, s_rayonVision, s_zoneDeControleMelee, Rang::Avant}},
        // Rôdeur : derrière, tire de loin et lance une pluie de flèches.
        {IdSpecialite::Rodeur, "Rôdeur",
         {s_couleurRodeur, attaqueDeBaseDeTireur(),
          {{IdSkill::TirPercant, Skill(TypeSkill::Degats, s_porteeTirPercant, s_degatsTirPercant, s_rechargeTirPercant, s_rechargeTirPercant, 0.0f, s_activationTirPercant)},
           {IdSkill::PluieDeFleches, Skill(TypeSkill::Zone, s_porteePluie, s_degatsPluie, s_rechargePluie, s_rechargePluie, s_rayonPluie, s_activationPluie)}},
          s_armureTireur, s_rayonVision, s_zoneDeControleTireur, Rang::Arriere}},
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
