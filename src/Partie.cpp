// ./src/Partie.cpp
#include "Partie.hpp"
#include <algorithm>
#include "Cercle.hpp"
#include "Coup.hpp"
#include "MonstreVu.hpp"

Partie::Partie(const Niveau& niveau)
    : m_carte(niveau.plan, niveau.tailleCase), m_escouade(niveau.departEscouade) {
    int prochainId = 1;
    for (const sf::Vector2f position : niveau.positionsMonstres) {
        m_monstres.emplace_back(position, prochainId++);
    }
}

ResultatClic Partie::cliquer(sf::Vector2f point) {
    if (!m_escouade.estVivante()) {
        return ResultatClic::Ignore;
    }
    const auto it = std::find_if(m_monstres.begin(), m_monstres.end(), [&](const Monstre& m) {
        return m.contient(point);
    });
    if (it != m_monstres.end()) {
        m_escouade.verrouiller(it->getId());
        return ResultatClic::Monstre;
    }
    m_escouade.deverrouiller();
    return ResultatClic::Sol;
}

void Partie::demanderSkill(std::size_t place) {
    m_skillDemande = place;
}

void Partie::lancerSkillDeZone(std::size_t place, sf::Vector2f point) {
    m_lancerDeZoneDemande = LancerDeZone{place, point};
}

void Partie::ordonnerDeplacement(sf::Vector2f point) {
    m_deplacementOrdonne = point;
}

Monstre* Partie::trouverMonstre(int id) {
    const auto it = std::find_if(m_monstres.begin(), m_monstres.end(), [&](const Monstre& m) {
        return m.getId() == id;
    });
    return it == m_monstres.end() ? nullptr : &*it;
}

void Partie::appliquer(const std::vector<Coup>& coups) {
    for (const Coup& coup : coups) {
        if (Monstre* touche = trouverMonstre(coup.idMonstre)) {
            touche->recevoirDegats(coup.degats);
        }
    }
}

void Partie::resoudreLesCoups() {
    // Monstre verrouillé : vide s'il n'y en a pas, ou s'il a disparu.
    std::optional<int> idVerrouille = m_escouade.getCibleVerrouillee();
    if (idVerrouille.has_value() && trouverMonstre(*idVerrouille) == nullptr) {
        m_escouade.deverrouiller();
        idVerrouille.reset();
    }

    std::vector<MonstreVu> monstresVus;
    for (const Monstre& m : m_monstres) {
        monstresVus.push_back({m.getId(), m.getCercle()});
    }

    // Lancer du skill de zone : réussi ou refusé, la demande est consommée.
    if (m_lancerDeZoneDemande.has_value()) {
        appliquer(m_escouade.declencherSkillDeZone(m_lancerDeZoneDemande->place, m_lancerDeZoneDemande->point, monstresVus, m_carte));
    }

    // Le skill passe avant l'attaque de base : s'il part, il relance
    // la recharge de base de ce personnage, qui ne frappe donc pas une
    // seconde fois dans la même image. Un soin ne rend aucun dégât.
    if (m_skillDemande.has_value()) {
        appliquer(m_escouade.declencherSkill(*m_skillDemande, monstresVus, m_carte));
    }

    // Les coups sont demandés à chaque image, verrou ou non : chaque
    // personnage vivant frappe de lui-même un monstre à sa portée.
    if (m_escouade.estVivante()) {
        appliquer(m_escouade.attaquer(monstresVus, m_carte));
        if (idVerrouille.has_value()) {
            const Monstre* verrouille = trouverMonstre(*idVerrouille);
            if (verrouille != nullptr && !verrouille->estVivant()) {
                m_escouade.deverrouiller();
            }
        }
    }

    std::erase_if(m_monstres, [](const Monstre& m) { return !m.estVivant(); });
}

void Partie::faireTournerLesMonstres(float dt) {
    // Les personnages vivants (à leur position à jour) sont des obstacles pour tous les monstres.
    const std::vector<Cercle> cerclesPersonnages = m_escouade.cerclesDesVivants();

    for (Monstre& m : m_monstres) {
        // Les autres monstres, à leur position courante, sont aussi des obstacles.
        std::vector<Cercle> obstacles = cerclesPersonnages;
        for (const Monstre& autre : m_monstres) {
            if (autre.getId() != m.getId()) {
                obstacles.push_back(autre.getCercle());
            }
        }
        const auto cible = m_escouade.proieDuMonstre(m.getId(), m.getCentre());
        m.mettreAJour(dt, cible, m_carte, obstacles);
        const sf::Vector2f origine = m.getCentre();
        if (cible) {
            m_escouade.recevoirDegats(m.getId(), origine, m.frapper(*cible));
        }
    }
}

void Partie::mettreAJour(float dt) {
    resoudreLesCoups();

    if (m_deplacementOrdonne.has_value() && m_escouade.estVivante()) {
        m_escouade.allerVers(*m_deplacementOrdonne, m_carte);
    }

    // Les monstres (à leur position de l'image précédente) sont des obstacles pour les personnages.
    std::vector<Cercle> cerclesMonstres;
    for (const Monstre& m : m_monstres) {
        cerclesMonstres.push_back(m.getCercle());
    }
    m_escouade.mettreAJour(dt, m_carte, cerclesMonstres);
    faireTournerLesMonstres(dt);

    m_lancerDeZoneDemande.reset();
    m_skillDemande.reset();
    m_deplacementOrdonne.reset();
}

void Partie::dessiner(sf::RenderTarget& rendu) const {
    m_carte.dessiner(rendu);
    for (const Monstre& m : m_monstres) {
        m.dessiner(rendu, m_escouade.getCibleVerrouillee() == m.getId());
    }
    m_escouade.dessiner(rendu);
}

const Escouade& Partie::getEscouade() const {
    return m_escouade;
}

const std::vector<Monstre>& Partie::getMonstres() const {
    return m_monstres;
}
