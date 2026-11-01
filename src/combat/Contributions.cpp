// ./src/combat/Contributions.cpp
#include "combat/Contributions.hpp"
#include <array>
#include "combat/StatistiquesDeCombat.hpp"

namespace {
// Une ligne de la table : la caractéristique, la statistique qu'elle alimente, et le poids de chaque point au-dessus de la
// référence (négatif en dessous).
struct Contribution {
    Caracteristique caracteristique;
    StatistiqueAlimentee statistique;
    float poids;
};

// La table de contributions (`docs/classes-et-combat.md`, section 13). Provisoire, à tester.
constexpr std::array s_contributions = {
    Contribution{Caracteristique::Endurance, StatistiqueAlimentee::PointsDeVieMax, static_cast<float>(s_pvParPointDEndurance)},
    Contribution{Caracteristique::Adresse, StatistiqueAlimentee::CapaciteOffensive, static_cast<float>(s_capaciteParPointDAdresse)},
    Contribution{Caracteristique::Adresse, StatistiqueAlimentee::CapaciteDefensive, static_cast<float>(s_capaciteParPointDAdresse)},
};
}  // namespace

float apportDesCaracteristiques(StatistiqueAlimentee statistique, const Caracteristiques& caracteristiques) {
    float apport = 0.0f;
    for (const Contribution& contribution : s_contributions) {
        if (contribution.statistique == statistique) {
            apport += contribution.poids * static_cast<float>(caracteristiques.valeur(contribution.caracteristique) - s_valeurDeReference);
        }
    }
    return apport;
}
