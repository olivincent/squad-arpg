// ./src/objets/Objet.hpp
#pragma once
#include <optional>
#include "objets/Affixe.hpp"

// Les modèles d'objets du catalogue (`CatalogueObjets.hpp`).
enum class IdObjet {
    CasqueDeFer,
    CuirasseDeMailles,
    JambieresDeFer,
    BouclierRond,
    GantsDeCuir,
    BottesDeCuir,
    CasqueDeCuir,
    TuniqueMatelassee,
    CollierDeProtection,
    AnneauDeProtection,
    AnneauDePuissance,
    CollierDEndurance,
    // Les treize gabarits d'armes (étape 102), dans l'ordre de `TypeDArme`.
    Dague,
    EpeeAUneMain,
    HacheAUneMain,
    MasseAUneMain,
    Lance,
    EpeeADeuxMains,
    HacheADeuxMains,
    MasseADeuxMains,
    Baguette,
    Sceptre,
    Baton,
    Arc,
    Arbalete
};

// La qualité d'un objet va de 0 à `s_qualiteMaximale` (entier : l'égalité par valeur reste fiable) ; un objet écrit sans
// précision est de qualité moyenne.
inline constexpr int s_qualiteMaximale = 100;
inline constexpr int s_qualiteParDefaut = 50;

// Combien le niveau d'un objet fait grandir ce qui en dépend (les plages des affixes, les dégâts des armes) : le facteur
// vaut 1 au niveau 1 et grandit de `s_croissanceParNiveau` (« à tester ») à chaque niveau. L'unique endroit qui le dit.
inline constexpr double s_croissanceParNiveau = 0.25;

[[nodiscard]] constexpr double facteurDeNiveau(int niveau) {
    return 1.0 + s_croissanceParNiveau * (niveau - 1);
}

// Ce que la qualité fait grandir (dégâts d'une arme, armure, bonus, blocage) : le facteur va de `s_facteurDeQualiteMin` (qualité 0)
// à `s_facteurDeQualiteMin + s_amplitudeDeQualite` (qualité 100) et vaut 1 à la qualité moyenne (valeurs provisoires, « à tester »).
// Écrit une fois, partagé par les armes et les protections.
inline constexpr double s_facteurDeQualiteMin = 0.85;
inline constexpr double s_amplitudeDeQualite = 0.30;

[[nodiscard]] constexpr double facteurDeQualite(int qualite) {
    return s_facteurDeQualiteMin + s_amplitudeDeQualite * qualite / s_qualiteMaximale;
}

// Un objet : l'instance, distincte de son modèle. Petite valeur copiable, sans allocation ; un objet existe à un seul
// endroit à la fois (un emplacement ou une case du sac) et se déplace par valeur. Le type interdit un troisième
// affixe ; que les affixes soient aux bonnes positions, permis pour la catégorie du modèle et en nombre conforme à la
// rareté, et que le niveau et la qualité soient dans leurs bornes, sont des invariants que le type ne peut pas dire :
// `objetValide`. Le niveau (>= 1, la profondeur où l'objet est généré) et la qualité (0 à 100) sont les deux seules valeurs
// de l'objet qui ne viennent ni du modèle ni des affixes : tout le reste (dégâts d'une arme, seuils) est recalculé depuis
// le modèle, jamais stocké. L'égalité est par valeur (tous les champs) : deux objets du même modèle sont égaux s'ils ont
// la même rareté, les mêmes affixes, le même niveau et la même qualité.
struct Objet {  // NOLINT(cppcoreguidelines-pro-type-member-init): le modèle n'a pas de valeur par défaut exprès, un oubli est signalé par le compilateur
    IdObjet modele;
    Rarete rarete = Rarete::Commun;
    std::optional<Affixe> prefixe = {};
    std::optional<Affixe> suffixe = {};
    int niveau = 1;
    int qualite = s_qualiteParDefaut;

    bool operator==(const Objet&) const = default;
};
