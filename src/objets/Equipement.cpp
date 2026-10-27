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
    assert(objetValide(objet));
    assert(peutAller(definitionObjet(objet.modele).categorie, emplacement));
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
            total += effetsDeBase(*objet).armure + apportDesAffixes(*objet).armure;
        }
    }
    return total;
}

StatistiquesDeCombat Equipement::bonusDeCombatTotal() const {
    StatistiquesDeCombat total;
    for (const std::optional<Objet>& objet : m_objets) {
        if (objet) {
            total += effetsDeBase(*objet).bonusDeCombat;
            total += apportDesAffixes(*objet).bonusDeCombat;
        }
    }
    return total;
}

DescriptionDAttaque Equipement::attaqueDeBase() const {
    const std::optional<Objet>& main = a(Emplacement::MainDroite);
    if (main.has_value()) {
        const std::optional<DonneesDArme>& arme = definitionObjet(main->modele).arme;
        if (arme.has_value()) {
            // L'attaque de l'arme, avec ce que ses affixes y ajoutent : du feu ou du sacré en portions de plus (sans écart : au
            // minimum comme au maximum), de la pénétration en plus de celle de l'arme (le plafond s'applique avec la défense).
            DescriptionDAttaque attaque = descriptionDeLArme(*main);
            const ApportDesAffixes apport = apportDesAffixes(*main);
            for (const Element element : s_elements) {
                attaque.degats.min[element] += apport.degatsAjoutes[element];
                attaque.degats.max[element] += apport.degatsAjoutes[element];
            }
            attaque.penetration += apport.penetration;
            return attaque;
        }
    }
    return mainsNues();
}

std::optional<TypeDArme> Equipement::typeDArme() const {
    const std::optional<Objet>& main = a(Emplacement::MainDroite);
    if (main.has_value()) {
        const std::optional<DonneesDArme>& arme = definitionObjet(main->modele).arme;
        if (arme.has_value()) {
            return arme->type;
        }
    }
    return std::nullopt;
}

Equipement Equipement::actifPour(const Caracteristiques& base) const {
    std::vector<std::optional<ExigencesEtBonus>> portes(m_objets.size());
    for (std::size_t i = 0; i < m_objets.size(); ++i) {
        const std::optional<Objet>& objet = m_objets[i];
        if (objet.has_value()) {
            // Les seuils viennent du modèle et du niveau de l'objet (`seuilsDeLObjet`) ; le bonus compte celui de base et celui des
            // affixes (un suffixe de caractéristique peut rendre actif un autre objet).
            portes[i] = ExigencesEtBonus{seuilsDeLObjet(*objet), effetsDeBase(*objet).bonus + apportDesAffixes(*objet).bonus};
        }
    }
    const std::vector<bool> actifs = objetsActifs(base, portes);
    Equipement actif = *this;
    for (std::size_t i = 0; i < m_objets.size(); ++i) {
        if (!actifs[i]) {
            actif.m_objets[i].reset();
        }
    }
    return actif;
}

Caracteristiques caracteristiquesEffectives(const Caracteristiques& base, const Equipement& equipement) {
    return base + equipement.actifPour(base).bonusTotal();
}

Caracteristiques Equipement::bonusTotal() const {
    Caracteristiques total;
    for (const std::optional<Objet>& objet : m_objets) {
        if (objet) {
            total += effetsDeBase(*objet).bonus;
            total += apportDesAffixes(*objet).bonus;
        }
    }
    return total;
}
