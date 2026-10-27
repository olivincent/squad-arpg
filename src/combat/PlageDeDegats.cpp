// ./src/combat/PlageDeDegats.cpp
#include "combat/PlageDeDegats.hpp"
#include <cassert>

Degats tirerLesDegats(Generateur& generateur, const PlageDeDegats& plage) {
    Degats tires;
    for (const Element element : s_elements) {
        assert(plage.min[element] <= plage.max[element]);
        tires[element] = plage.min[element] == plage.max[element] ? plage.min[element]
                                                                  : tirerEntre(generateur, plage.min[element], plage.max[element]);
    }
    return tires;
}
