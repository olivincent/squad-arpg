// ./src/objets/Affixe.hpp
#pragma once

// La rareté d'un objet : elle dit combien d'affixes il porte (`objetValide`). Commun : aucun ; magique : exactement un,
// préfixe ou suffixe ; rare : un préfixe et un suffixe.
enum class Rarete { Commun, Magique, Rare };

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

// L'affixe d'un objet : son identifiant et la valeur de CET objet. La valeur est gardée dans l'objet, pas lue dans le
// catalogue : rééquilibrer un affixe ne change pas les objets déjà trouvés.
struct Affixe {
    IdAffixe id;
    int valeur;

    bool operator==(const Affixe&) const = default;
};
