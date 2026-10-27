// ./src/objets/Affixe.hpp
#pragma once
#include <array>

// La rareté d'un objet : elle dit combien d'affixes il porte (`objetValide`). Commun : aucun ; magique : exactement un,
// préfixe ou suffixe ; rare : un préfixe et un suffixe.
enum class Rarete { Commun, Magique, Rare };

// Toutes les raretés, de la plus commune à la plus rare : pour les parcourir (tirage, tests de données).
inline constexpr std::array s_raretes = {Rarete::Commun, Rarete::Magique, Rarete::Rare};

// Les affixes du catalogue (`CatalogueAffixes.hpp`) : un modificateur qui s'ajoute à un objet. Noms de travail, en
// français ; le texte affiché est dans le catalogue (`docs/glossaire.md`).
enum class IdAffixe {
    // Préfixes
    Flamboyant,
    Sanctifie,
    Acere,
    Renforce,
    // Suffixes
    DuBoeuf,
    DeLOurs,
    DuRenard,
    DeLaChouette,
    DuLion,
    DEsquive,
    DeDeviation,
    DeBlocage
};

// Le penchant d'un gabarit pour un affixe : dans le tirage, le poids de cet affixe est multiplié par `multiplicateur` (au moins
// `s_multiplicateurMinimalDAffinite`) sur les objets de ce gabarit. Un affixe que la catégorie de l'objet ne permet pas n'est
// jamais tiré, affinité ou non : l'affinité ne fait que pondérer parmi les affixes permis.
struct AffiniteDAffixe {
    IdAffixe affixe;
    int multiplicateur;

    bool operator==(const AffiniteDAffixe&) const = default;
};

inline constexpr int s_multiplicateurMinimalDAffinite = 2;

// L'affixe d'un objet : son identifiant et la valeur de CET objet. La valeur est gardée dans l'objet, pas lue dans le
// catalogue : rééquilibrer un affixe ne change pas les objets déjà trouvés.
struct Affixe {
    IdAffixe id;
    int valeur;

    bool operator==(const Affixe&) const = default;
};
