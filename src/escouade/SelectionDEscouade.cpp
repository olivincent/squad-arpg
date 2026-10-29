// ./src/escouade/SelectionDEscouade.cpp
#include "escouade/SelectionDEscouade.hpp"
#include <cassert>

#include "escouade/TailleDeLEscouade.hpp"

namespace {
// Mêmes dimensions que les boutons de `ListeDeBoutons`, pour que les colonnes s'alignent.
constexpr float s_largeurCase = 520.0f;
constexpr float s_hauteurCase = 60.0f;
constexpr float s_ecartEntreCases = 20.0f;
}  // namespace

void SelectionDEscouade::ajouter(IdSpecialite id) {
    if (!estComplete()) {
        m_specialites.push_back(id);
    }
}

void SelectionDEscouade::retirer(std::size_t indice) {
    assert(indice < m_specialites.size());
    m_specialites.erase(m_specialites.begin() + static_cast<std::ptrdiff_t>(indice));
}

bool SelectionDEscouade::estComplete() const {
    return m_specialites.size() == s_tailleDeLEscouade;
}

const std::vector<IdSpecialite>& SelectionDEscouade::liste() const {
    return m_specialites;
}

sf::FloatRect rectangleDeLaCase(std::size_t indice, sf::Vector2f centre) {
    assert(indice < s_tailleDeLEscouade);
    const float nombre = static_cast<float>(s_tailleDeLEscouade);
    const float hauteurTotale = nombre * s_hauteurCase + (nombre - 1.0f) * s_ecartEntreCases;
    const float haut = centre.y - hauteurTotale / 2.0f + static_cast<float>(indice) * (s_hauteurCase + s_ecartEntreCases);
    return sf::FloatRect({centre.x - s_largeurCase / 2.0f, haut}, {s_largeurCase, s_hauteurCase});
}

std::optional<std::size_t> caseSous(sf::Vector2f point, sf::Vector2f centre) {
    for (std::size_t i = 0; i < s_tailleDeLEscouade; ++i) {
        if (rectangleDeLaCase(i, centre).contains(point)) {
            return i;
        }
    }
    return std::nullopt;
}
