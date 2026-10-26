// ./src/jeu/Partie.cpp
#include "jeu/Partie.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>
#include <variant>
#include "base/Cercle.hpp"
#include "combat/Coup.hpp"
#include "combat/Progression.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/Cible.hpp"
#include "monde/ObjetInteractif.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/GenerationDObjets.hpp"

namespace {
// Écart bord à bord (cercle d'un personnage, zone de la porte) à partir duquel la porte s'ouvre.
constexpr float s_porteeInteraction = 20.0f;
constexpr int s_idDeLEscouadeEntiere = 0;  // `idCible` d'un événement qui vise l'escouade et non l'un de ses personnages
const sf::Color s_couleurZoneDeControle(255, 0, 255, 180);  // débogage F3 : cercle fin des zones de contrôle
constexpr float s_epaisseurZoneDeControle = 1.0f;

// La classe de chaque membre de l'expédition, dans l'ordre.
std::vector<DefinitionPersonnage> classesDe(const Expedition& expedition) {
    std::vector<DefinitionPersonnage> classes;
    classes.reserve(expedition.membres.size());
    for (const MembreDExpedition& membre : expedition.membres) {
        classes.push_back(membre.definition);
    }
    return classes;
}
}

Partie::Partie(const Niveau& niveau, Expedition expedition, Generateur::result_type graine,
               const CatalogueDeMonstres& catalogue)
    : m_profondeur(expedition.profondeur),
      m_graineDExpedition(expedition.graine),
      m_classes(classesDe(expedition)),
      m_experience(expedition.experience),
      m_carte(niveau.plan, niveau.tailleCase),
      m_escouade(niveau.departEscouade, compositionDeLExpedition(expedition), expedition.sac, niveauPour(expedition.experience)),
      m_generateur(graine),
      m_vision(m_carte) {
    int prochainId = 1;
    for (const MonstreDuNiveau& monstre : niveau.monstres) {
        m_monstres.emplace_back(monstre.position, prochainId++, monstre.groupe,
                                renforcer(definitionDeMonstre(catalogue, monstre.categorie), m_profondeur));
    }
    int prochainIdObjet = 1;
    for (const PorteDuNiveau& porte : niveau.portes) {
        for (sf::Vector2i coordCase : porte.cases) {
            m_carte.placerPorte(coordCase);
        }
        m_objets.emplace_back(prochainIdObjet++, Porte{porte.cases}, niveau.tailleCase);
    }
    mettreAJourLaVision();  // la salle de départ est visible dès la première image
}

ResultatClic Partie::cliquer(sf::Vector2f point) {
    if (!m_escouade.estVivante()) {
        return ResultatClic::Ignore;
    }
    const auto it = std::find_if(m_monstres.begin(), m_monstres.end(), [&](const Monstre& m) {
        return m.estCiblable() && m_vision.estVisible(m.getCentre()) && m.contient(point);
    });
    if (it != m_monstres.end()) {
        m_escouade.verrouiller(it->getId());
        abandonnerLesVisees();
        return ResultatClic::Monstre;
    }
    abandonnerLesVisees();
    if (const ObjetAuSol* objet = objetAuSolSous(point)) {
        // Un ordre de ramassage : l'escouade marche vers l'objet comme vers le sol (le combat est rompu de la même façon) ;
        // `ramasserSiAPortee` le ramasse quand l'ancre arrive.
        m_escouade.deverrouiller();
        m_escouade.annulerAttaquesEngagees();
        m_objetAuSolVise = objet->id;
        m_deplacementOrdonne = objet->position;
        return ResultatClic::ObjetAuSol;
    }
    if (const std::optional<sf::Vector2i> escalier = m_carte.caseDeLEscalier();
        escalier.has_value() && m_carte.caseContenant(point) == *escalier && m_vision.etat(*escalier) != EtatCase::Inconnue) {
        // Un ordre de descente : l'escouade marche vers l'escalier comme vers le sol (le combat est rompu de la même façon) ;
        // `prendreLEscalierSiAPortee` la fait descendre quand l'ancre arrive. On peut descendre en plein combat.
        m_escouade.deverrouiller();
        m_escouade.annulerAttaquesEngagees();
        m_escalierVise = true;
        m_deplacementOrdonne = m_carte.centreDeCase(*escalier);
        return ResultatClic::Escalier;
    }
    if (const ObjetInteractif* objet = trouverObjetActif(point)) {
        if (m_escouade.unVivantPresDe(objet->zone, s_porteeInteraction)) {
            // Déjà à portée : ouverture sur place, sans ordre ni interaction mémorisée.
            m_escouade.deverrouiller();
            m_escouade.annulerAttaquesEngagees();
            m_deplacementOrdonne.reset();
            ouvrir(*objet);
            m_escouade.regarderVers(objet->zone.getCenter());
            return ResultatClic::Porte;
        }
        const std::optional<sf::Vector2f> approche = std::visit(
            Surcharges{[&](const Porte& porte) { return pointDApproche(porte, m_carte, m_escouade.getCentre()); }},
            objet->nature);
        if (approche.has_value()) {
            m_escouade.deverrouiller();
            m_escouade.annulerAttaquesEngagees();
            m_interactionEnCours = objet->id;
            m_deplacementOrdonne = approche;
            return ResultatClic::Porte;
        }
    }
    m_escouade.deverrouiller();
    m_escouade.annulerAttaquesEngagees();
    return ResultatClic::Sol;
}

void Partie::demanderSkill(AdresseSkill adresse) {
    m_skillDemande = adresse;
}

void Partie::lancerSkillDeZone(AdresseSkill adresse, sf::Vector2f point) {
    m_lancerDeZoneDemande = LancerDeZone{adresse, point};
}

void Partie::ordonnerDeplacement(sf::Vector2f point) {
    abandonnerLesVisees();
    m_deplacementOrdonne = point;
}

void Partie::abandonnerLesVisees() {
    m_interactionEnCours.reset();
    m_objetAuSolVise.reset();
    m_escalierVise = false;
}

Monstre* Partie::trouverMonstre(int id) {
    const auto it = std::find_if(m_monstres.begin(), m_monstres.end(), [&](const Monstre& m) {
        return m.getId() == id;
    });
    return it == m_monstres.end() ? nullptr : &*it;
}

const ObjetInteractif* Partie::trouverObjetActif(sf::Vector2f point) const {
    for (const ObjetInteractif& objet : m_objets) {
        const bool fermee = std::visit(
            Surcharges{[&](const Porte& porte) { return m_carte.getTile(porte.cases.front().x, porte.cases.front().y) == Tile::PorteFermee; }},
            objet.nature);
        // Une porte que le joueur n'a jamais vue (aucune de ses cases) n'est pas cliquable.
        const bool connue = std::visit(
            Surcharges{[&](const Porte& porte) {
                return std::ranges::any_of(porte.cases, [&](sf::Vector2i coordCase) { return m_vision.etat(coordCase) != EtatCase::Inconnue; });
            }},
            objet.nature);
        if (fermee && connue && objet.zone.contains(point)) {
            return &objet;
        }
    }
    return nullptr;
}

void Partie::mettreAJourLaVision() {
    m_vision.mettreAJour(m_carte, m_escouade.cerclesDeVision());
}

const ObjetInteractif* Partie::objetEnCours() const {
    if (!m_interactionEnCours.has_value()) {
        return nullptr;
    }
    const auto it = std::ranges::find(m_objets, *m_interactionEnCours, &ObjetInteractif::id);
    return it == m_objets.end() ? nullptr : &*it;
}

void Partie::ouvrir(const ObjetInteractif& objet) {
    std::visit(Surcharges{[&](const Porte& porte) {
                   for (sf::Vector2i coordCase : porte.cases) {
                       m_carte.ouvrirPorte(coordCase);
                   }
               }},
               objet.nature);
}

void Partie::interagirSiAPortee() {
    const ObjetInteractif* objet = objetEnCours();
    if (objet != nullptr && m_escouade.unVivantPresDe(objet->zone, s_porteeInteraction)) {
        ouvrir(*objet);
        m_interactionEnCours.reset();
    }
}

void Partie::ramasserSiAPortee() {
    if (!m_objetAuSolVise.has_value()) {
        return;
    }
    const ObjetAuSol* objet = m_sol.trouver(*m_objetAuSolVise);
    if (objet == nullptr) {
        m_objetAuSolVise.reset();  // l'objet n'existe plus : la visée est abandonnée
        return;
    }
    const sf::Vector2f ancre = m_escouade.getCentre();
    const sf::Vector2f ecart = objet->position - ancre;
    if (std::hypot(ecart.x, ecart.y) >= s_distanceDeRamassage) {
        return;
    }
    // À portée, l'ordre est terminé, que l'objet entre dans le sac ou non : un sac plein le laisse au sol, sans qu'on
    // insiste.
    m_objetAuSolVise.reset();
    if (m_escouade.ramasser(objet->objet)) {
        [[maybe_unused]] const bool retire = m_sol.retirer(objet->id);
        assert(retire);  // l'objet vient d'être trouvé
    } else {
        signaler(Camp::Escouade, s_idDeLEscouadeEntiere, ancre, SacPlein{});
    }
}

void Partie::prendreLEscalierSiAPortee() {
    if (!m_escalierVise) {
        return;
    }
    const std::optional<sf::Vector2i> escalier = m_carte.caseDeLEscalier();
    assert(escalier.has_value());  // la visée vient d'un clic sur l'escalier
    const sf::Vector2f ecart = m_carte.centreDeCase(*escalier) - m_escouade.getCentre();  // NOLINT(bugprone-unchecked-optional-access): l'assert ci-dessus
    if (std::hypot(ecart.x, ecart.y) < s_distanceDeDescente) {
        m_escalierVise = false;
        m_escalierPris = true;
    }
}

std::vector<EntiteVue> Partie::entitesCiblables() const {
    std::vector<EntiteVue> entites;
    for (const Monstre& m : m_monstres) {
        if (m.estCiblable()) {
            entites.push_back({m.getId(), m.getCercle()});
        }
    }
    return entites;
}

std::vector<EntiteVue> Partie::entitesPhysiques() const {
    std::vector<EntiteVue> entites;
    for (const Monstre& m : m_monstres) {
        entites.push_back({m.getId(), m.getCercle()});
    }
    return entites;
}

Partie::CampsDeControle Partie::photographierLesCamps() const {
    CampsDeControle camps;
    camps.personnages = m_escouade.entitesDeControle();
    for (const Monstre& m : m_monstres) {
        if (m.estCiblable()) {
            camps.monstres.push_back({m.getId(), m.getCercle(), m.getZoneDeControle()});
        }
    }
    return camps;
}

void Partie::appliquerLesCoupsDOpportunite(const CampsDeControle& avant) {
    const CampsDeControle apres = photographierLesCamps();
    const std::vector<CoupDOpportunite> surLesPersonnages =
        coupsDOpportunite({avant.monstres, avant.personnages}, {apres.monstres, apres.personnages});
    const std::vector<CoupDOpportunite> surLesMonstres =
        coupsDOpportunite({avant.personnages, avant.monstres}, {apres.personnages, apres.monstres});

    // Monstre qui frappe un personnage : comme un coup normal (le monstre devient un agresseur).
    for (const CoupDOpportunite& coup : surLesPersonnages) {
        if (const Monstre* attaquant = trouverMonstre(coup.idAttaquant)) {
            infligerAUnPersonnage(attaquant->getId(), static_cast<std::size_t>(coup.idCible),
                                  attaquant->getAttaque(), true);
        }
    }
    // Personnage qui frappe un monstre : un Coup comme ceux d'attaquer(), sans toucher à aucune recharge.
    std::vector<Coup> coupsSurLesMonstres;
    for (const CoupDOpportunite& coup : surLesMonstres) {
        if (const std::optional<Attaque> attaque = m_escouade.attaqueDeBase(static_cast<std::size_t>(coup.idAttaquant))) {
            coupsSurLesMonstres.push_back({coup.idCible, *attaque});
        }
    }
    appliquer(coupsSurLesMonstres, true);

    const std::optional<int> idVerrouille = m_escouade.getCibleVerrouillee();
    if (idVerrouille.has_value()) {
        const Monstre* verrouille = trouverMonstre(*idVerrouille);
        if (verrouille != nullptr && !verrouille->estVivant()) {
            m_escouade.deverrouiller();
        }
    }
    retirerLesMorts();
}

void Partie::retirerLesMorts() {
    for (const Monstre& monstre : m_monstres) {
        if (monstre.estVivant()) {
            continue;
        }
        m_experience += monstre.getExperience();  // à l'escouade, quel que soit le tueur, même si des personnages sont morts
        for (const Objet& objet : genererButin(m_generateur, monstre.getChanceDeButin(), monstre.getNombreDObjets(), 1)) {
            m_sol.poser(objet, monstre.getCentre(), m_carte);
        }
    }
    std::erase_if(m_monstres, [](const Monstre& m) { return !m.estVivant(); });
    monterSiLExperienceLeDonne();
}

void Partie::monterSiLExperienceLeDonne() {
    const int niveau = niveauPour(m_experience);
    if (niveau <= m_escouade.getNiveau()) {
        return;
    }
    // Un événement par niveau gagné, puis tous les niveaux d'un coup : la base d'un personnage est une fonction de son niveau.
    for (int atteint = m_escouade.getNiveau() + 1; atteint <= niveau; ++atteint) {
        signaler(Camp::Escouade, s_idDeLEscouadeEntiere, m_escouade.getCentre(), NiveauAtteint{atteint});
    }
    m_escouade.monterAuNiveau(niveau);
}

int Partie::getExperience() const {
    return m_experience;
}

int Partie::getNiveau() const {
    return m_escouade.getNiveau();
}

void Partie::gagnerUnNiveau() {
    if (estPerdue()) {
        return;
    }
    m_experience += experienceManquante(m_experience);
    monterSiLExperienceLeDonne();
}

void Partie::appliquer(const std::vector<Coup>& coups, bool opportunite) {
    for (const Coup& coup : coups) {
        if (Monstre* touche = trouverMonstre(coup.idMonstre)) {
            const ResultatDuCoup resultat = touche->recevoirDegats(coup.attaque, m_generateur);
            signalerLeCoup(Camp::Monstres, touche->getId(), touche->getCentre(), resultat, opportunite);
        }
    }
}

void Partie::infligerAUnPersonnage(int idMonstre, std::size_t place, const Attaque& attaque, bool opportunite) {
    const ResultatDuCoup resultat = m_escouade.recevoirCoup(idMonstre, place, attaque, m_generateur);
    if (const std::optional<sf::Vector2f> centre = m_escouade.centreDuPersonnage(place)) {
        signalerLeCoup(Camp::Escouade, static_cast<int>(place), *centre, resultat, opportunite);
    }
}

void Partie::signalerLesFaits() {
    for (const Escouade::FaitDEscouade& fait : m_escouade.prendreFaits()) {
        std::visit(Surcharges{[&](const Escouade::SoinApplique& soin) {
                       signaler(Camp::Escouade, static_cast<int>(soin.place), soin.position, Soin{soin.montant, soin.critique});
                   },
                   [&](const Escouade::SkillPart& part) {
                       signaler(Camp::Escouade, static_cast<int>(part.place), part.position, SkillLance{part.skill});
                   },
                   [&](const Escouade::MonstreProvoque& provoque) {
                       // Un monstre disparu dans l'image n'a plus de position : pas de texte.
                       if (const Monstre* monstre = trouverMonstre(provoque.idMonstre)) {
                           signaler(Camp::Monstres, monstre->getId(), monstre->getCentre(), Provoque{});
                       }
                   }},
                   fait);
    }
}

void Partie::signalerLeCoup(Camp campCible, int idCible, sf::Vector2f position, const ResultatDuCoup& resultat,
                            bool opportunite) {
    for (const DetailEvenement& detail : detailsDuCoup(resultat, opportunite)) {
        signaler(campCible, idCible, position, detail);
    }
}

void Partie::signaler(Camp campCible, int idCible, sf::Vector2f position, DetailEvenement detail) {
    const bool montantNul = std::visit(Surcharges{[](const DegatsSubis& degats) { return degats.montant <= 0; },
                                                  [](const Soin& soin) { return soin.montant <= 0; },
                                                  [](const SkillLance&) { return false; },
                                                  [](const Provoque&) { return false; },
                                                  [](const Esquive&) { return false; },
                                                  [](const Bloque&) { return false; },
                                                  [](const SacPlein&) { return false; },
                                                  [](const NiveauAtteint&) { return false; }},
                                       detail);
    if (montantNul) {
        return;
    }
    m_evenements.push_back({campCible, idCible, position, detail});
}

std::vector<EvenementDeCombat> Partie::prendreEvenements() {
    return std::exchange(m_evenements, {});
}

void Partie::resoudreLesCoups(float dt) {
    // Monstre verrouillé : vide s'il n'y en a pas, s'il a disparu ou s'il rentre à son poste.
    std::optional<int> idVerrouille = m_escouade.getCibleVerrouillee();
    if (idVerrouille.has_value()) {
        const Monstre* verrouille = trouverMonstre(*idVerrouille);
        if (verrouille == nullptr || !verrouille->estCiblable()) {
            m_escouade.deverrouiller();
            idVerrouille.reset();
        }
    }

    const std::vector<EntiteVue> monstresVus = entitesCiblables();

    // Lancer du skill de zone : réussi ou refusé, la demande est consommée.
    if (m_lancerDeZoneDemande.has_value()) {
        appliquer(m_escouade.declencherSkillDeZone(m_lancerDeZoneDemande->adresse, m_lancerDeZoneDemande->point, monstresVus, m_carte, m_generateur));
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

    // Soins du skill demandé et des skills qui partent dans attaquer().
    signalerLesFaits();

    retirerLesMorts();
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
        } else if (const std::optional<Attaque> attaque = monstre.avancerAttaque(dt, m_generateur)) {
            infligerAUnPersonnage(monstre.getId(), placeVisee, *attaque, false);
        }
        return;
    }
    // Tentative à chaque image : l'échec est normal (en marche, en recharge, rien à frapper…).
    static_cast<void>(monstre.engagerAttaque(proie, candidats, m_carte));
}

bool Partie::doitDescendre() const {
    return m_escalierPris && !estPerdue() && m_profondeur < s_profondeurMaximale;
}

void Partie::forcerLaDescente() {
    if (!estPerdue()) {
        m_escalierVise = false;
        m_escalierPris = true;
    }
}

bool Partie::estPerdue() const {
    return !m_escouade.estVivante();
}

bool Partie::estGagnee() const {
    return m_escalierPris && !estPerdue() && m_profondeur >= s_profondeurMaximale;
}

int Partie::getProfondeur() const {
    return m_profondeur;
}

int Partie::monstresRestants() const {
    return static_cast<int>(m_monstres.size());
}

void Partie::mettreAJour(float dt) {
    if (estPerdue() || m_escalierPris) {
        return;
    }
    resoudreLesCoups(dt);

    if (m_deplacementOrdonne.has_value() && m_escouade.estVivante()) {
        m_escouade.allerVers(*m_deplacementOrdonne, m_carte);
        // Approche d'un objet : allerVers a effacé le point à regarder, on le redonne après lui.
        if (const ObjetInteractif* objet = objetEnCours()) {
            m_escouade.regarderVers(objet->zone.getCenter());
        }
    }

    // Tous les monstres (à leur position de l'image précédente), même ceux qui rentrent à leur
    // poste, sont des obstacles pour les personnages ; seuls les ciblables comptent pour le verrou
    // et la portée.
    const CampsDeControle avant = photographierLesCamps();
    m_escouade.mettreAJour(dt, m_carte, {.physiques = entitesPhysiques(),
                                         .ciblables = entitesCiblables(),
                                         .deControle = avant.monstres});
    interagirSiAPortee();
    ramasserSiAPortee();
    prendreLEscalierSiAPortee();
    faireTournerLesMonstres(dt);
    appliquerLesCoupsDOpportunite(avant);
    // En dernier, pour que ce que le joueur voit corresponde aux positions de la fin de l'image.
    mettreAJourLaVision();

    m_lancerDeZoneDemande.reset();
    m_skillDemande.reset();
    m_deplacementOrdonne.reset();
}

void Partie::dessinerLaCarte(sf::RenderTarget& rendu) const {
    m_carte.dessiner(rendu);
}

void Partie::dessinerLesEntites(sf::RenderTarget& rendu, bool afficherAncre, bool marquerLeRetour, bool brouillard) const {
    for (const Monstre& m : m_monstres) {
        // Un monstre hors de vue n'est pas dessiné du tout (ni carré, ni barre de vie, ni contour).
        if (!brouillard || m_vision.estVisible(m.getCentre())) {
            m.dessiner(rendu, m_escouade.getCibleVerrouillee() == m.getId(), marquerLeRetour);
        }
    }
    m_escouade.dessiner(rendu, afficherAncre);
    if (brouillard) {
        m_vision.dessiner(rendu);
    }
}

void Partie::dessinerCheminsDesMonstres(sf::RenderTarget& rendu) const {
    for (const Monstre& m : m_monstres) {
        m.dessinerChemin(rendu);
    }
}

void Partie::dessinerZonesDeControle(sf::RenderTarget& rendu) const {
    const CampsDeControle camps = photographierLesCamps();
    for (const std::vector<EntiteDeControle>* camp : {&camps.personnages, &camps.monstres}) {
        for (const EntiteDeControle& entite : *camp) {
            if (entite.zone <= 0.0f) {
                continue;
            }
            const float rayon = entite.cercle.rayon + entite.zone;
            sf::CircleShape cercle(rayon);
            cercle.setOrigin({rayon, rayon});
            cercle.setPosition(entite.cercle.centre);
            cercle.setFillColor(sf::Color::Transparent);
            cercle.setOutlineColor(s_couleurZoneDeControle);
            cercle.setOutlineThickness(s_epaisseurZoneDeControle);
            rendu.draw(cercle);
        }
    }
}

bool Partie::estVisible(sf::Vector2f point) const {
    return m_vision.estVisible(point);
}

Expedition Partie::expedition() const {
    Expedition expedition{.profondeur = m_profondeur, .graine = m_graineDExpedition, .membres = {}, .sac = m_escouade.sac(),
                          .experience = m_experience};
    for (std::size_t i = 0; i < m_classes.size(); ++i) {
        const std::size_t place = i + 1;
        expedition.membres.push_back({.definition = m_classes[i],
                                      .equipement = m_escouade.equipementDuPersonnage(place),
                                      .pointsDepenses = m_escouade.pointsDepensesDuPersonnage(place).value()});
    }
    return expedition;
}

bool Partie::ramasserDesObjetsAleatoires(std::size_t nombre) {
    // La vérification vient avant le tirage : un refus ne consomme aucun nombre aléatoire.
    if (m_escouade.sac().casesLibres() < nombre) {
        return false;
    }
    std::vector<Objet> objets;
    objets.reserve(nombre);
    for (std::size_t i = 0; i < nombre; ++i) {
        objets.push_back(genererObjet(m_generateur, 1));
    }
    [[maybe_unused]] const bool ok = m_escouade.ramasserTout(objets);
    assert(ok);  // il y avait assez de cases libres
    return true;
}

const Escouade& Partie::getEscouade() const {
    return m_escouade;
}

Escouade& Partie::getEscouade() {
    return m_escouade;
}

const std::vector<Monstre>& Partie::getMonstres() const {
    return m_monstres;
}

const TileMap& Partie::getCarte() const {
    return m_carte;
}

const ObjetsAuSol& Partie::getSol() const {
    return m_sol;
}

ObjetsAuSol& Partie::getSol() {
    return m_sol;
}

const ObjetAuSol* Partie::objetAuSolSous(sf::Vector2f point) const {
    return m_sol.sous(point, m_carte, m_vision);
}
