// ./src/Agresseurs.cpp
#include "Agresseurs.hpp"
#include <algorithm>

namespace {
constexpr float s_dureeMemoire = 3.0f;  // en secondes, sans nouveau coup du même monstre
}  // namespace

void Agresseurs::enregistrer(int id, sf::Vector2f origine) {
    const auto connu = std::find_if(m_agresseurs.begin(), m_agresseurs.end(), [&](const Agresseur& a) {
        return a.monstre.id == id;
    });
    if (connu != m_agresseurs.end()) {
        connu->monstre.cercle.centre = origine;
        connu->tempsRestant = s_dureeMemoire;
    } else {
        m_agresseurs.push_back({{id, {origine, 0.0f}}, s_dureeMemoire});
    }
}

void Agresseurs::mettreAJour(float dt) {
    for (Agresseur& agresseur : m_agresseurs) {
        agresseur.tempsRestant -= dt;
    }
    std::erase_if(m_agresseurs, [](const Agresseur& a) { return a.tempsRestant <= 0.0f; });
}

void Agresseurs::actualiser(const std::vector<MonstreVu>& monstres) {
    std::erase_if(m_agresseurs, [&](const Agresseur& a) { return trouverMonstre(monstres, a.monstre.id) == nullptr; });
    for (Agresseur& agresseur : m_agresseurs) {
        if (const MonstreVu* vu = trouverMonstre(monstres, agresseur.monstre.id)) {
            agresseur.monstre.cercle = vu->cercle;
        }
    }
}

std::vector<MonstreVu> Agresseurs::monstresVus() const {
    std::vector<MonstreVu> vus;
    vus.reserve(m_agresseurs.size());
    for (const Agresseur& agresseur : m_agresseurs) {
        vus.push_back(agresseur.monstre);
    }
    return vus;
}

const std::vector<Agresseurs::Agresseur>& Agresseurs::liste() const {
    return m_agresseurs;
}
