// ./src/combat/Contributions.cpp
#include "combat/Contributions.hpp"
#include <array>
#include "combat/Energie.hpp"
#include "combat/StatistiquesDeCombat.hpp"

namespace {
// Une ligne de la table : la caractéristique, la statistique qu'elle alimente, et le poids de chaque point au-dessus de la
// référence (négatif en dessous).
struct Contribution {
    Caracteristique caracteristique;
    StatistiqueAlimentee statistique;
    float poids;
};

constexpr float s_pvParPoint = static_cast<float>(s_pvParPointDEndurance);
constexpr float s_energieParPoint = static_cast<float>(s_energieParPointDeVolonte);
constexpr float s_capaciteParPoint = static_cast<float>(s_capaciteParPointDAdresse);

// La table de contributions (`docs/classes-et-combat.md`, section 13). Provisoire, à tester.
constexpr std::array s_contributions = {
    Contribution{Caracteristique::Endurance, StatistiqueAlimentee::PointsDeVieMax, s_pvParPoint},
    Contribution{Caracteristique::Endurance, StatistiqueAlimentee::ResistanceEtourdissement, s_resistanceAuControleParPoint},
    Contribution{Caracteristique::Endurance, StatistiqueAlimentee::ResistanceRalentissement, s_resistanceAuControleParPoint},
    Contribution{Caracteristique::Adresse, StatistiqueAlimentee::CapaciteOffensive, s_capaciteParPoint},
    Contribution{Caracteristique::Adresse, StatistiqueAlimentee::CapaciteDefensive, s_capaciteParPoint},
    Contribution{Caracteristique::Volonte, StatistiqueAlimentee::EnergieMax, s_energieParPoint},
    Contribution{Caracteristique::Volonte, StatistiqueAlimentee::ResistanceCharme, s_resistanceAuControleParPoint},
    Contribution{Caracteristique::Volonte, StatistiqueAlimentee::ResistancePeur, s_resistanceAuControleParPoint},
    Contribution{Caracteristique::Volonte, StatistiqueAlimentee::ResistanceDisruption, s_resistanceAuControleParPoint},
};

// Indexée par `EtatDeControle`, dans l'ordre de l'énumération.
constexpr std::array<StatistiqueAlimentee, s_nombreDEtatsDeControle> s_resistances = {
    StatistiqueAlimentee::ResistanceEtourdissement, StatistiqueAlimentee::ResistanceRalentissement,
    StatistiqueAlimentee::ResistanceCharme, StatistiqueAlimentee::ResistancePeur, StatistiqueAlimentee::ResistanceDisruption};
}  // namespace

StatistiqueAlimentee resistanceAuControle(EtatDeControle etat) {
    return s_resistances[static_cast<std::size_t>(etat)];
}

float apportDesCaracteristiques(StatistiqueAlimentee statistique, const Caracteristiques& caracteristiques) {
    float apport = 0.0f;
    for (const Contribution& contribution : s_contributions) {
        if (contribution.statistique == statistique) {
            apport += contribution.poids * static_cast<float>(caracteristiques.valeur(contribution.caracteristique) - s_valeurDeReference);
        }
    }
    return apport;
}
