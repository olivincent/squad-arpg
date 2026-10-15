// ./src/EntiteVue.cpp
#include "EntiteVue.hpp"
#include <algorithm>

const EntiteVue* trouverEntite(const std::vector<EntiteVue>& entites, int id) {
    const auto it = std::find_if(entites.begin(), entites.end(), [&](const EntiteVue& e) {
        return e.id == id;
    });
    return it == entites.end() ? nullptr : &*it;
}
