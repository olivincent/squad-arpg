// ./src/objets/Inventaire.cpp
#include "objets/Inventaire.hpp"
#include <algorithm>
#include <cassert>
#include <utility>

Inventaire::Inventaire(std::size_t capacite) : m_cases(capacite) {}

std::size_t Inventaire::capacite() const {
    return m_cases.size();
}

const std::optional<Objet>& Inventaire::a(std::size_t caseDuSac) const {
    assert(caseDuSac < m_cases.size());
    return m_cases[caseDuSac];
}

std::optional<Objet> Inventaire::prendre(std::size_t caseDuSac) {
    assert(caseDuSac < m_cases.size());
    return std::exchange(m_cases[caseDuSac], std::nullopt);
}

std::optional<Objet> Inventaire::poser(std::size_t caseDuSac, Objet objet) {
    assert(caseDuSac < m_cases.size());
    return std::exchange(m_cases[caseDuSac], objet);
}

bool Inventaire::ajouter(Objet objet) {
    const auto libre = std::ranges::find_if(m_cases, [](const std::optional<Objet>& c) { return !c; });
    if (libre == m_cases.end()) {
        return false;
    }
    *libre = objet;
    return true;
}
