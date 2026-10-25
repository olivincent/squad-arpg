// ./src/monstres/Monstre.cpp
#include "monstres/Monstre.hpp"
#include "combat/Autodefense.hpp"
#include "rendu/BarreDeVie.hpp"
#include "monde/ChampDeVision.hpp"
#include "base/Eclaircir.hpp"
#include "monde/TileMap.hpp"
#include "rendu/TraceDeChemin.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cassert>
#include <cstdint>

namespace {
constexpr std::uint8_t s_opaciteRetour = 128;  // sur 255 : environ 50 %, pour marquer le retour (débogage F3)
}  // namespace

Monstre::Monstre(sf::Vector2f centre, int id, int groupe, const DefinitionDeMonstre& definition)
    : m_mobile(centre, {definition.cote, definition.cote}, definition.vitesse),
      m_couleur(definition.couleur),
      m_vitalite(definition.pointsDeVieMax,
                 {.armure = definition.armure, .statistiques = definition.statistiques}),
      m_id(id),
      m_groupe(groupe),
      m_attaque(definition.attaque),
      m_rayonDetection(definition.rayonDetection),
      m_rayonTerritoire(definition.rayonTerritoire),
      m_zoneDeControle(definition.zoneDeControle),
      m_statistiques(definition.statistiques),
      m_regenerationRetour(definition.regenerationRetour),
      m_etat(Etat::EnGarde),
      m_poste(centre),
      m_alerte(false),
      m_soinAccumule(0.0f),
      m_idCibleEngagee()
{
}

void Monstre::dessiner(sf::RenderTarget& rendu, bool estCible, bool marquerLeRetour) const {
    const sf::Vector2f centre = m_mobile.getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();

    sf::RectangleShape forme(taille);
    forme.setPosition(centre - taille / 2.0f);
    sf::Color couleur = cibleEngagee().has_value() ? eclaircir(m_couleur, s_eclaircissementExecution) : m_couleur;
    if (marquerLeRetour && m_etat == Etat::Retour) {
        couleur.a = s_opaciteRetour;
    }
    forme.setFillColor(couleur);
    if (estCible) {
        forme.setOutlineColor(sf::Color::Yellow);
        forme.setOutlineThickness(-3.0f);
    }
    rendu.draw(forme);

    dessinerBarreDeVie(rendu, positionBarreDeVie(centre, taille), taille.x, m_vitalite.getRatio());
}

void Monstre::dessinerChemin(sf::RenderTarget& rendu) const {
    dessinerTraceDeChemin(rendu, m_mobile.getCentre(), m_mobile.cheminRestant(), sf::Color::Red);
}

sf::Vector2f Monstre::getCentre() const {
    return m_mobile.getCentre();
}

Cercle Monstre::getCercle() const {
    return m_mobile.getCercle();
}

int Monstre::getId() const {
    return m_id;
}

int Monstre::getGroupe() const {
    return m_groupe;
}

bool Monstre::contient(sf::Vector2f point) const {
    const sf::Vector2f centre = m_mobile.getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();
    return sf::FloatRect(centre - taille / 2.0f, taille).contains(point);
}

ResultatDuCoup Monstre::recevoirDegats(const Attaque& attaque, Generateur& generateur) {
    if (m_etat == Etat::Retour || !estVivant()) {
        return {.montant = 0, .critique = false};
    }
    const ResultatDuCoup resultat = m_vitalite.recevoirDegats(attaque, generateur);
    if (attaque.degats.total() > 0) {
        alerter();
    }
    return resultat;
}

void Monstre::alerter() {
    if (m_etat == Etat::Retour) {
        return;
    }
    m_alerte = true;
}

bool Monstre::estEnPoursuite() const {
    return m_etat == Etat::Poursuite;
}

bool Monstre::estCiblable() const {
    return m_etat != Etat::Retour;
}

bool Monstre::estVivant() const {
    return m_vitalite.estVivant();
}

int Monstre::getPointsDeVie() const {
    return m_vitalite.getValeur();
}

float Monstre::getZoneDeControle() const {
    return m_zoneDeControle;
}

Attaque Monstre::getAttaque() const {
    return {.degats = degatsPhysiques(m_attaque.getPuissance()),
            .capaciteOffensive = m_statistiques.capaciteOffensive,
            .nature = m_attaque.getNature()};
}

const StatistiquesDeCombat& Monstre::getStatistiques() const {
    return m_statistiques;
}

// Détecte la proie si elle est dans le rayon de détection, dans le territoire
// et en vue (`ligneDeVue`, en cases). Seule la détection exige la vue :
// une fois en poursuite, le monstre ne perd pas sa proie en la perdant de vue.
bool Monstre::proieDetectee(const std::optional<EntiteVue>& proie, const TileMap& carte) const {
    if (!proie) {
        return false;
    }
    const sf::Vector2f centreProie = proie->cercle.centre;
    const float distanceMonstre = (centreProie - m_mobile.getCentre()).length();
    const float distancePoste = (centreProie - m_poste).length();
    return distanceMonstre <= m_rayonDetection && distancePoste <= m_rayonTerritoire
        && ligneDeVue(carte, m_mobile.getCentre(), centreProie);
}

std::optional<EntiteVue> Monstre::cibleDeFrappe(const std::optional<EntiteVue>& proie,
                                                const std::vector<EntiteVue>& candidats,
                                                const TileMap& carte) const {
    const std::optional<int> id = choisirCibleAFrapper(
        getCercle(), [&](Cercle c) { return peutAtteindre(c, carte); }, candidats,
        proie ? std::optional<int>(proie->id) : std::nullopt);
    if (!id) {
        return std::nullopt;
    }
    // choisirCibleAFrapper ne choisit que parmi les candidats.
    const EntiteVue* cible = trouverEntite(candidats, *id);
    assert(cible != nullptr);
    return *cible;
}

void Monstre::mettreAJour(float dt, std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats,
                          const TileMap& carte, const std::vector<Cercle>& obstacles,
                          const std::vector<Cercle>& aContourner) {
    m_attaque.mettreAJour(dt);
    m_vitalite.mettreAJour(dt);

    // Un coup en exécution : le monstre reste sur place, ni poursuite ni
    // retour (l'état et le coup reçu sont pris en compte après le coup).
    if (cibleEngagee().has_value()) {
        m_mobile.arreter();
        m_mobile.mettreAJour(dt, carte, obstacles);
        return;
    }

    const Etat etatPrecedent = m_etat;

    // Une alerte (un coup reçu, par exemple) réveille le monstre s'il a une
    // proie et se trouve dans son territoire ; elle ne sert qu'une fois.
    const bool alerte = m_alerte && proie
        && (m_mobile.getCentre() - m_poste).length() <= m_rayonTerritoire;
    m_alerte = false;

    switch (m_etat) {
        case Etat::EnGarde:
            if (proieDetectee(proie, carte) || alerte) {
                m_etat = Etat::Poursuite;
            }
            break;
        case Etat::Poursuite:
            if (!proie || (m_mobile.getCentre() - m_poste).length() > m_rayonTerritoire) {
                m_etat = Etat::Retour;
            }
            break;
        case Etat::Retour:
            // Le monstre ne réagit à rien jusqu'à son arrivée au poste (hystérésis).
            if (!m_mobile.directionVoulue()) {
                m_etat = Etat::EnGarde;
            }
            break;
    }

    if (m_etat == Etat::Retour && etatPrecedent != Etat::Retour) {
        m_mobile.arreter();
        m_mobile.allerVers(m_poste, carte, aContourner);
    } else if (m_etat == Etat::Retour && m_mobile.estBloque()) {
        // Bloqué en rentrant : le chemin choisi à l'entrée en retour ignorait l'obstacle qui gêne
        // maintenant. allerVers remet le compteur à zéro : au plus un recalcul toutes les 0,3 s.
        m_mobile.allerVers(m_poste, carte, aContourner);
    }

    if (m_etat == Etat::Retour) {
        // Seule la partie entière est soignée : un entier à 0,83 point par image serait tronqué à 0.
        m_soinAccumule += static_cast<float>(m_vitalite.getMaximum()) * m_regenerationRetour * dt;
        const int entier = static_cast<int>(m_soinAccumule);
        m_vitalite.soigner(entier);
        m_soinAccumule -= static_cast<float>(entier);
        if (m_vitalite.getValeur() == m_vitalite.getMaximum()) {
            m_soinAccumule = 0.0f;
        }
    } else {
        m_soinAccumule = 0.0f;
    }

    if (m_etat == Etat::Poursuite) {
        if (cibleDeFrappe(proie, candidats, carte)) {
            m_mobile.arreter();
        } else {
            // Sans proie, la poursuite a basculé en retour juste au-dessus.
            assert(proie.has_value());
            m_mobile.allerVers(proie->cercle.centre, carte, aContourner);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
        }
    }

    m_mobile.mettreAJour(dt, carte, obstacles);
}

bool Monstre::peutAtteindre(Cercle cible, const TileMap& carte) const {
    return m_attaque.estAPortee(getCercle(), cible) && ligneDeVue(carte, m_mobile.getCentre(), cible.centre);
}

bool Monstre::engagerAttaque(std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte) {
    if (m_etat == Etat::Retour) {
        return false;
    }
    const std::optional<EntiteVue> cible = cibleDeFrappe(proie, candidats, carte);
    if (!cible || m_mobile.estEnMouvement() || !m_attaque.commencer()) {
        return false;
    }
    m_idCibleEngagee = cible->id;
    return true;
}

std::optional<int> Monstre::cibleEngagee() const {
    if (m_attaque.getPhase() != PhaseSkill::EnExecution) {
        return std::nullopt;
    }
    return m_idCibleEngagee;
}

std::optional<Attaque> Monstre::avancerAttaque(float dt, Generateur& generateur) {
    if (!cibleEngagee().has_value() || !m_attaque.avancer(dt, generateur)) {
        return std::nullopt;
    }
    m_idCibleEngagee.reset();
    return getAttaque();
}

void Monstre::annulerAttaque() {
    m_attaque.annuler();
    m_idCibleEngagee.reset();
}
