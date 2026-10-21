// ./src/interface/PileDEcrans.cpp
#include "interface/PileDEcrans.hpp"

#include <cassert>

std::size_t premierEcranADessiner(const PileDEcrans& pile) {
    assert(!pile.empty());
    std::size_t indice = pile.size() - 1;
    while (indice > 0 && pile[indice]->laisseVoirDessous()) {
        --indice;
    }
    return indice;
}
