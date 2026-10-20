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
constexpr float s_cote = 30.0f;      // côté du carré, en pixels
constexpr float s_vitesse = 160.0f;  // en pixels par seconde
constexpr int s_pointsDeVieMax = 100;
constexpr float s_executionAttaque = 0.3f;  // en secondes, immobile avant que le coup parte
constexpr std::uint8_t s_opaciteRetour = 128;  // sur 255 : environ 50 %, pour marquer le retour (débogage F3)
constexpr float s_regenerationRetour = 0.5f;  // fraction des points de vie maximum rendue par seconde en retour
}  // namespace

Monstre::Monstre(sf::Vector2f centre, int id, int armure, int groupe)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_vitalite(s_pointsDeVieMax, armure),
      m_id(id),
      m_groupe(groupe),
      m_attaque(TypeSkill::Degats, 15.0f, 10, 0.7f, 1.3f, 0.0f, s_executionAttaque),  // portée de 15 px, bord à bord
      m_rayonDetection(200.0f),
      m_rayonTerritoire(s_rayonTerritoire),
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
    sf::Color couleur = cibleEngagee().has_value() ? eclaircir(sf::Color::Red, s_eclaircissementExecution) : sf::Color::Red;
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

int Monstre::recevoirDegats(int quantite) {
    if (m_etat == Etat::Retour || !estVivant()) {
        return 0;
    }
    const int degats = m_vitalite.recevoirDegats(quantite);
    if (quantite > 0) {
        alerter();
    }
    return degats;
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
    return s_zoneDeControle;
}

int Monstre::getPuissanceAttaque() const {
    return m_attaque.getPuissance();
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
        m_soinAccumule += static_cast<float>(m_vitalite.getMaximum()) * s_regenerationRetour * dt;
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

int Monstre::avancerAttaque(float dt, Generateur& generateur) {
    if (!cibleEngagee().has_value() || !m_attaque.avancer(dt, generateur)) {
        return 0;
    }
    m_idCibleEngagee.reset();
    return m_attaque.getPuissance();
}

void Monstre::annulerAttaque() {
    m_attaque.annuler();
    m_idCibleEngagee.reset();
}
