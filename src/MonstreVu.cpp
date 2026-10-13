// ./src/MonstreVu.cpp
#include "MonstreVu.hpp"
#include <algorithm>

const MonstreVu* trouverMonstre(const std::vector<MonstreVu>& monstres, int id) {
    const auto it = std::find_if(monstres.begin(), monstres.end(), [&](const MonstreVu& m) {
        return m.id == id;
    });
    return it == monstres.end() ? nullptr : &*it;
}
