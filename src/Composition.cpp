// ./src/Composition.cpp
#include "Composition.hpp"
#include <cstddef>
#include "IdSkill.hpp"
#include "TypeSkill.hpp"

namespace {
const sf::Color s_couleurGuerrier = sf::Color::Red;
const sf::Color s_couleurPretre(255, 240, 150);  // jaune pâle
const sf::Color s_couleurMage(160, 80, 220);     // violet
constexpr float s_porteeMelee = 20.0f;    // bord à bord
constexpr float s_porteeTireur = 120.0f;  // bord à bord
constexpr int s_armureMelee = 50;
constexpr int s_armureTireur = 0;
constexpr float s_rayonVision = 320.0f;  // en pixels (8 cases), le même pour tous
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

constexpr float s_decalageDeRang = 25.0f;     // distance de chaque rang à l'ancre, selon x
constexpr float s_ecartDansLeRang = 50.0f;    // entre deux personnages d'un même rang, selon y

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
}  // namespace

std::vector<DefinitionPersonnage> compositionParDefaut() {
    return {
        // Guerrier : devant, au contact.
        {s_couleurGuerrier, attaqueDeBaseDeMelee(),
         {{IdSkill::FrappePuissante, Skill(TypeSkill::Degats, s_porteeMelee, s_degatsSkill, s_rechargeSkill, s_rechargeSkill, 0.0f, s_activationSkill)},
          {IdSkill::Provocation, Skill(TypeSkill::Provocation, 0.0f, s_dureeProvocation, s_rechargeProvocation, s_rechargeProvocation, s_rayonProvocation, s_activationProvocation)}},
         s_armureMelee, s_rayonVision, Rang::Avant},
        // Prêtre : derrière, soigne.
        {s_couleurPretre, attaqueDeBaseDeTireur(),
         {{IdSkill::Soin, Skill(TypeSkill::Soin, s_porteeSoin, s_puissanceSoin, s_rechargeSoin, s_rechargeSoin, 0.0f, s_activationSoin)}},
         s_armureTireur, s_rayonVision, Rang::Arriere},
        // Mage : derrière, lance l'explosion.
        {s_couleurMage, attaqueDeBaseDeTireur(),
         {{IdSkill::Explosion, Skill(TypeSkill::Zone, s_porteeZone, s_degatsZone, s_rechargeZone, s_rechargeZone, s_rayonZone, s_activationZone)}},
         s_armureTireur, s_rayonVision, Rang::Arriere},
    };
}

std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs) {
    std::size_t nombreAvant = 0;
    for (const Rang rang : rangs) {
        if (rang == Rang::Avant) {
            ++nombreAvant;
        }
    }
    const std::size_t nombreArriere = rangs.size() - nombreAvant;

    std::vector<sf::Vector2f> decalages;
    std::size_t rangAvant = 0;    // indice du prochain dans son rang
    std::size_t rangArriere = 0;
    for (const Rang rang : rangs) {
        const bool devant = rang == Rang::Avant;
        const std::size_t k = devant ? rangAvant++ : rangArriere++;
        const std::size_t n = devant ? nombreAvant : nombreArriere;
        const float y = (static_cast<float>(k) - static_cast<float>(n - 1) / 2.0f) * s_ecartDansLeRang;
        decalages.push_back({devant ? s_decalageDeRang : -s_decalageDeRang, y});
    }
    return decalages;
}
