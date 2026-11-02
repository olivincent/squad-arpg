// ./src/combat/Contributions.hpp
#pragma once
#include "combat/Caracteristiques.hpp"
#include "combat/EtatDeControle.hpp"

// Les statistiques que les caractéristiques alimentent par une contribution intrinsèque (toujours active ; étape 110).
enum class StatistiqueAlimentee {
    PointsDeVieMax,
    EnergieMax,
    CapaciteOffensive,
    CapaciteDefensive,
    ResistanceEtourdissement,
    ResistanceRalentissement,
    ResistanceCharme,
    ResistancePeur,
    ResistanceDisruption,
    BonusDAire,    // en fraction de l'aire (0,25 : +25 %), avant le plafond (`ValeursEffectives.hpp`)
    BonusDeDuree   // en fraction de la durée, avant le plafond
};

// La statistique qu'est la résistance à cet état de contrôle.
[[nodiscard]] StatistiqueAlimentee resistanceAuControle(EtatDeControle etat);

// Ce que les caractéristiques ajoutent à cette statistique : la somme, sur les lignes de la table de contributions, de
//   poids × (caractéristique − `s_valeurDeReference`).
// Une seule forme de calcul ; une statistique peut avoir plusieurs lignes (plusieurs sources) sans que le calcul change. La base de
// la statistique, elle, reste dans la règle de chaque statistique (`pointsDeVieMax`, `energieMax`, `statistiquesDuPersonnage`). Non
// arrondi : l'arrondi est celui de la règle qui l'appelle. Sous la référence, l'apport est négatif.
[[nodiscard]] float apportDesCaracteristiques(StatistiqueAlimentee statistique, const Caracteristiques& caracteristiques);
