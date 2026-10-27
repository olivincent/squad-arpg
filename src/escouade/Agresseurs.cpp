// ./src/escouade/Agresseurs.cpp
#include "escouade/Agresseurs.hpp"
#include <algorithm>

namespace {
constexpr float s_dureeMemoire = 3.0f;  // en secondes, sans nouveau coup du même monstre
}  // namespace

void Agresseurs::enregistrer(IdMonstre id) {
    const auto connu = std::find_if(m_agresseurs.begin(), m_agresseurs.end(), [&](const Agresseur& a) {
        return a.id == id;
    });
    if (connu != m_agresseurs.end()) {
        connu->tempsRestant = s_dureeMemoire;
    } else {
        m_agresseurs.push_back({id, s_dureeMemoire});
    }
}

void Agresseurs::mettreAJour(float dt) {
    for (Agresseur& agresseur : m_agresseurs) {
        agresseur.tempsRestant -= dt;
    }
    std::erase_if(m_agresseurs, [](const Agresseur& a) { return a.tempsRestant <= 0.0f; });
}

void Agresseurs::retirerMonstresAbsents(const std::vector<MonstreVu>& monstres) {
    std::erase_if(m_agresseurs, [&](const Agresseur& a) { return trouverEntite(monstres, a.id) == nullptr; });
}

std::vector<MonstreVu> Agresseurs::vusParmi(const std::vector<MonstreVu>& monstres) const {
    std::vector<MonstreVu> vus;
    vus.reserve(m_agresseurs.size());
    for (const Agresseur& agresseur : m_agresseurs) {
        if (const MonstreVu* vu = trouverEntite(monstres, agresseur.id)) {
            vus.push_back(*vu);
        }
    }
    return vus;
}

const std::vector<Agresseurs::Agresseur>& Agresseurs::liste() const {
    return m_agresseurs;
}
