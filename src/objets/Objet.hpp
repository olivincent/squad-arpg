// ./src/objets/Objet.hpp
#pragma once

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
    EpeeLongue,
    Masse,
    Dague,
    EpeeADeuxMains,
    Sceptre,
    Baton,
    Arc
};

// Un objet : l'instance, distincte de son modèle. Petite valeur copiable ; un objet existe à un
// seul endroit à la fois (un emplacement ou une case du sac) et se déplace par valeur.
struct Objet {
    IdObjet id;

    bool operator==(const Objet&) const = default;
};
