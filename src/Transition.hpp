// ./src/Transition.hpp
#pragma once

#include <memory>
#include <variant>

#include "Reglages.hpp"

class Ecran;

// Ce qu'un écran demande à l'Application, une fois son appel terminé : un
// écran ne modifie jamais la pile lui-même (il pourrait se détruire pendant
// l'exécution de sa propre fonction).
struct Aucune {};

struct Empiler {
    std::unique_ptr<Ecran> ecran;  // le nouvel écran, qui passe au-dessus
};

struct Depiler {};

struct Quitter {};

struct AppliquerReglages {
    Reglages reglages;  // les nouveaux réglages d'affichage, à appliquer tout de suite
};

// Un type seulement déplaçable (à cause du std::unique_ptr). Les utilisateurs
// qui détruisent une Transition incluent Ecran.hpp : le std::unique_ptr a
// besoin du type complet pour libérer l'écran.
using Transition = std::variant<Aucune, Empiler, Depiler, Quitter, AppliquerReglages>;
