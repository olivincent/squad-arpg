// ./src/combat/FamilleDAbsorption.cpp
#include "combat/FamilleDAbsorption.hpp"
#include <algorithm>
#include <cassert>

FamilleDAbsorption familleQuiAbsorbe(Element element) {
    return element == Element::Physique ? FamilleDAbsorption::Aegis : FamilleDAbsorption::Ward;
}

FamilleDAbsorption familleQuiImmunise(EtatDeControle etat) {
    switch (etat) {
        case EtatDeControle::Etourdissement:
        case EtatDeControle::Ralentissement:
            return FamilleDAbsorption::Aegis;
        case EtatDeControle::Charme:
        case EtatDeControle::Peur:
        case EtatDeControle::Disruption:
        case EtatDeControle::Nombre:
            return FamilleDAbsorption::Ward;
    }
    return FamilleDAbsorption::Ward;
}

Degats portionsAbsorbees(const Degats& portions, const PointsDAbsorption& points) {
    int restantAegis = points.aegis;
    int restantWard = points.ward;
    Degats absorbees;
    for (const Element element : s_elements) {
        assert(portions[element] >= 0);
        int& restant = familleQuiAbsorbe(element) == FamilleDAbsorption::Aegis ? restantAegis : restantWard;
        absorbees[element] = std::min(portions[element], restant);
        restant -= absorbees[element];
    }
    return absorbees;
}
