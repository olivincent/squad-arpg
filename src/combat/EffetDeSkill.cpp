// ./src/combat/EffetDeSkill.cpp
#include "combat/EffetDeSkill.hpp"
#include "combat/Cible.hpp"

CampTouche campTouche(const EffetDeSkill& effet) {
    return std::visit(Surcharges{[](const EffetDegats&) { return CampTouche::Monstres; },
                                 [](const EffetFrappe&) { return CampTouche::Monstres; },
                                 [](const EffetSoin&) { return CampTouche::Allies; },
                                 [](const EffetProvocation&) { return CampTouche::Monstres; }},
                      effet);
}
