// ./src/escouade/Provocations.cpp
#include "escouade/Provocations.hpp"
#include <algorithm>

void Provocations::provoquer(IdMonstre id, Place place, float duree) {
    const auto connu = std::find_if(m_provocations.begin(), m_provocations.end(), [&](const Provocation& p) {
        return p.id == id;
    });
    if (connu != m_provocations.end()) {
        connu->place = place;
        connu->tempsRestant = duree;
    } else {
        m_provocations.push_back({id, place, duree});
    }
}

void Provocations::mettreAJour(float dt) {
    for (Provocation& provocation : m_provocations) {
        provocation.tempsRestant -= dt;
    }
    std::erase_if(m_provocations, [](const Provocation& p) { return p.tempsRestant <= 0.0f; });
}

void Provocations::retirerMonstresAbsents(const std::vector<MonstreVu>& monstres) {
    std::erase_if(m_provocations, [&](const Provocation& p) { return trouverEntite(monstres, p.id) == nullptr; });
}

void Provocations::retirerProvocateur(Place place) {
    std::erase_if(m_provocations, [&](const Provocation& p) { return p.place == place; });
}

std::optional<Place> Provocations::provocateurDe(IdMonstre id) const {
    const auto connu = std::find_if(m_provocations.begin(), m_provocations.end(), [&](const Provocation& p) {
        return p.id == id;
    });
    if (connu == m_provocations.end()) {
        return std::nullopt;
    }
    return connu->place;
}

const std::vector<Provocations::Provocation>& Provocations::liste() const {
    return m_provocations;
}
