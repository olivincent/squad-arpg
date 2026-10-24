// ./src/objets/Equipement.cpp
#include "objets/Equipement.hpp"
#include <cassert>
#include <cstddef>
#include <utility>
#include "objets/CatalogueObjets.hpp"

Equipement::Equipement(const std::vector<ObjetDeDepart>& objetsDeDepart) {
    for (const ObjetDeDepart& entree : objetsDeDepart) {
        [[maybe_unused]] const std::optional<Objet> ancien = poser(entree.emplacement, {entree.objet});
        assert(!ancien.has_value());
    }
}

const std::optional<Objet>& Equipement::a(Emplacement emplacement) const {
    return m_objets[static_cast<std::size_t>(emplacement)];
}

std::optional<Objet> Equipement::poser(Emplacement emplacement, Objet objet) {
    assert(peutAller(definitionObjet(objet.id).categorie, emplacement));
    std::optional<Objet>& place = m_objets[static_cast<std::size_t>(emplacement)];
    return std::exchange(place, objet);
}

std::optional<Objet> Equipement::retirer(Emplacement emplacement) {
    return std::exchange(m_objets[static_cast<std::size_t>(emplacement)], std::nullopt);
}

int Equipement::armureTotale() const {
    int total = 0;
    for (const std::optional<Objet>& objet : m_objets) {
        if (objet) {
            total += definitionObjet(objet->id).armure;
        }
    }
    return total;
}

Caracteristiques Equipement::bonusTotal() const {
    Caracteristiques total;
    for (const std::optional<Objet>& objet : m_objets) {
        if (objet) {
            total += definitionObjet(objet->id).bonus;
        }
    }
    return total;
}
