// ./src/Partie.cpp
#include "Partie.hpp"
#include <algorithm>
#include "Cercle.hpp"
#include "Coup.hpp"
#include "EntiteVue.hpp"

Partie::Partie(const Niveau& niveau, Generateur::result_type graine)
    : m_carte(niveau.plan, niveau.tailleCase), m_escouade(niveau.departEscouade), m_generateur(graine) {
    int prochainId = 1;
    for (const MonstreDuNiveau& monstre : niveau.monstres) {
        m_monstres.emplace_back(monstre.position, prochainId++, niveau.armureMonstres, monstre.groupe);
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
    m_escouade.annulerAttaquesEngagees();
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

void Partie::resoudreLesCoups(float dt) {
    // Monstre verrouillé : vide s'il n'y en a pas, ou s'il a disparu.
    std::optional<int> idVerrouille = m_escouade.getCibleVerrouillee();
    if (idVerrouille.has_value() && trouverMonstre(*idVerrouille) == nullptr) {
        m_escouade.deverrouiller();
        idVerrouille.reset();
    }

    std::vector<EntiteVue> monstresVus;
    for (const Monstre& m : m_monstres) {
        monstresVus.push_back({m.getId(), m.getCercle()});
    }

    // Lancer du skill de zone : réussi ou refusé, la demande est consommée.
    if (m_lancerDeZoneDemande.has_value()) {
        appliquer(m_escouade.declencherSkillDeZone(m_lancerDeZoneDemande->place, m_lancerDeZoneDemande->point, monstresVus, m_carte, m_generateur));
    }

    // Le skill passe avant l'attaque de base : s'il part, il relance
    // la recharge de base de ce personnage, qui ne frappe donc pas une
    // seconde fois dans la même image. Un soin ne rend aucun dégât.
    if (m_skillDemande.has_value()) {
        appliquer(m_escouade.declencherSkill(*m_skillDemande, monstresVus, m_carte, m_generateur));
    }

    // Les coups sont demandés à chaque image, verrou ou non : chaque
    // personnage vivant frappe de lui-même un monstre à sa portée.
    if (m_escouade.estVivante()) {
        appliquer(m_escouade.attaquer(dt, monstresVus, m_carte, m_generateur));
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

    // Le statut de chaque monstre avant leur mise à jour, pour repérer ceux
    // qui passent en poursuite pendant celle-ci.
    std::vector<bool> enPoursuiteAvant;
    for (const Monstre& m : m_monstres) {
        enPoursuiteAvant.push_back(m.estEnPoursuite());
    }

    for (Monstre& m : m_monstres) {
        // Les autres monstres, à leur position courante, sont aussi des obstacles.
        // Ils sont aussi à contourner par le chemin, contrairement aux personnages.
        std::vector<Cercle> obstacles = cerclesPersonnages;
        std::vector<Cercle> autresMonstres;
        for (const Monstre& autre : m_monstres) {
            if (autre.getId() != m.getId()) {
                autresMonstres.push_back(autre.getCercle());
            }
        }
        obstacles.insert(obstacles.end(), autresMonstres.begin(), autresMonstres.end());
        const std::optional<EntiteVue> proie = m_escouade.proieDuMonstre(m.getId(), m.getCentre());
        const std::vector<EntiteVue> candidats = m_escouade.candidatsDeFrappe(m.getId());
        m.mettreAJour(dt, proie, candidats, m_carte, obstacles, autresMonstres);
        faireFrapperLeMonstre(m, proie, candidats, dt);
    }

    alerterLesGroupes(enPoursuiteAvant);
}

// Les groupes à alerter sont tous collectés avant le premier `alerter()` : un
// monstre alerté ici ne relance pas d'alerte dans la même image, le groupe est
// un ensemble fermé. L'alerté commence à bouger à l'image suivante.
void Partie::alerterLesGroupes(const std::vector<bool>& enPoursuiteAvant) {
    std::vector<int> groupesAAlerter;
    for (std::size_t i = 0; i < m_monstres.size(); ++i) {
        if (!enPoursuiteAvant[i] && m_monstres[i].estEnPoursuite()) {
            groupesAAlerter.push_back(m_monstres[i].getGroupe());
        }
    }
    for (Monstre& m : m_monstres) {
        if (std::ranges::find(groupesAAlerter, m.getGroupe()) != groupesAAlerter.end()) {
            m.alerter();
        }
    }
}

// Un coup engagé part toujours à la fin de son exécution, sur le personnage
// visé à l'engagement (même si une provocation a changé la proie entre-temps).
// Seule la mort de ce personnage l'annule. Sinon, un monstre immobile avec une
// cible de frappe (proie ou autre candidat) engage un coup.
void Partie::faireFrapperLeMonstre(Monstre& monstre, std::optional<EntiteVue> proie,
                                   const std::vector<EntiteVue>& candidats, float dt) {
    if (const std::optional<int> place = monstre.cibleEngagee()) {
        const std::size_t placeVisee = static_cast<std::size_t>(*place);
        if (!m_escouade.placeEstVivante(placeVisee)) {
            monstre.annulerAttaque();
        } else if (const int degats = monstre.avancerAttaque(dt, m_generateur); degats > 0) {
            m_escouade.recevoirCoup(monstre.getId(), placeVisee, monstre.getCentre(), degats);
        }
        return;
    }
    monstre.engagerAttaque(proie, candidats, m_carte);
}

bool Partie::estPerdue() const {
    return !m_escouade.estVivante();
}

bool Partie::estGagnee() const {
    return m_monstres.empty() && !estPerdue();
}

int Partie::monstresRestants() const {
    return static_cast<int>(m_monstres.size());
}

void Partie::mettreAJour(float dt) {
    if (estPerdue() || estGagnee()) {
        return;
    }
    resoudreLesCoups(dt);

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

void Partie::dessiner(sf::RenderTarget& rendu, bool afficherAncre) const {
    m_carte.dessiner(rendu);
    for (const Monstre& m : m_monstres) {
        m.dessiner(rendu, m_escouade.getCibleVerrouillee() == m.getId());
    }
    m_escouade.dessiner(rendu, afficherAncre);
}

void Partie::dessinerCheminsDesMonstres(sf::RenderTarget& rendu) const {
    for (const Monstre& m : m_monstres) {
        m.dessinerChemin(rendu);
    }
}

const Escouade& Partie::getEscouade() const {
    return m_escouade;
}

const std::vector<Monstre>& Partie::getMonstres() const {
    return m_monstres;
}
