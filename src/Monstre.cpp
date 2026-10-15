// ./src/Monstre.cpp
#include "Monstre.hpp"
#include "Autodefense.hpp"
#include "BarreDeVie.hpp"
#include "Defense.hpp"
#include "Eclaircir.hpp"
#include "TileMap.hpp"
#include "TraceDeChemin.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace {
constexpr float s_cote = 30.0f;      // côté du carré, en pixels
constexpr float s_vitesse = 120.0f;  // en pixels par seconde
constexpr int s_pointsDeVieMax = 100;
constexpr float s_eclaircissementExecution = 0.5f;  // part de blanc pendant l'exécution d'un coup
constexpr float s_executionAttaque = 0.3f;  // en secondes, immobile avant que le coup parte
}  // namespace

Monstre::Monstre(sf::Vector2f centre, int id, int armure, int groupe)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_pointsDeVie(s_pointsDeVieMax),
      m_armure(armure),
      m_id(id),
      m_groupe(groupe),
      m_attaque(TypeSkill::Degats, 15.0f, 10, 0.7f, 1.3f, 0.0f, s_executionAttaque),  // portée de 15 px, bord à bord
      m_rayonDetection(200.0f),
      m_rayonTerritoire(400.0f),
      m_etat(Etat::EnGarde),
      m_poste(centre),
      m_alerte(false),
      m_idCibleEngagee()
{
}

void Monstre::dessiner(sf::RenderTarget& rendu, bool estCible) const {
    const sf::Vector2f centre = m_mobile.getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();

    sf::RectangleShape forme(taille);
    forme.setPosition(centre - taille / 2.0f);
    forme.setFillColor(cibleEngagee().has_value() ? eclaircir(sf::Color::Red, s_eclaircissementExecution) : sf::Color::Red);
    if (estCible) {
        forme.setOutlineColor(sf::Color::Yellow);
        forme.setOutlineThickness(-3.0f);
    }
    rendu.draw(forme);

    dessinerBarreDeVie(rendu, positionBarreDeVie(centre, taille), taille.x, m_pointsDeVie.getRatio());
}

void Monstre::dessinerChemin(sf::RenderTarget& rendu) const {
    dessinerTraceDeChemin(rendu, m_mobile.getCentre(), m_mobile.cheminRestant(), sf::Color::Red);
}

sf::Vector2f Monstre::getCentre() const {
    return m_mobile.getCentre();
}

Cercle Monstre::getCercle() const {
    return {m_mobile.getCentre(), m_mobile.getTaille().x / 2.0f};
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

void Monstre::recevoirDegats(int quantite) {
    m_pointsDeVie.subirDegats(appliquerArmure(quantite, m_armure));
    if (quantite > 0) {
        alerter();
    }
}

void Monstre::alerter() {
    m_alerte = true;
}

bool Monstre::estEnPoursuite() const {
    return m_etat == Etat::Poursuite;
}

bool Monstre::estVivant() const {
    return m_pointsDeVie.estVivant();
}

int Monstre::getPointsDeVie() const {
    return m_pointsDeVie.getValeur();
}

// Détecte la proie si elle est dans le rayon de détection, dans le territoire
// et en vue (ligne de vue centre à centre). Seule la détection exige la vue :
// une fois en poursuite, le monstre ne perd pas sa proie en la perdant de vue.
bool Monstre::proieDetectee(const std::optional<EntiteVue>& proie, const TileMap& carte) const {
    if (!proie) {
        return false;
    }
    const sf::Vector2f centreProie = proie->cercle.centre;
    const float distanceMonstre = (centreProie - m_mobile.getCentre()).length();
    const float distancePoste = (centreProie - m_poste).length();
    return distanceMonstre <= m_rayonDetection && distancePoste <= m_rayonTerritoire
        && carte.ligneDeVue(m_mobile.getCentre(), centreProie);
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
    return *trouverEntite(candidats, *id);
}

void Monstre::mettreAJour(float dt, std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats,
                          const TileMap& carte, const std::vector<Cercle>& obstacles) {
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
            if (proieDetectee(proie, carte) || alerte) {
                m_etat = Etat::Poursuite;
            } else if (!m_mobile.directionVoulue()) {
                m_etat = Etat::EnGarde;
            }
            break;
    }

    if (m_etat == Etat::Retour && etatPrecedent != Etat::Retour) {
        m_mobile.arreter();
        m_mobile.allerVers(m_poste, carte);
    }

    if (m_etat == Etat::Poursuite) {
        if (cibleDeFrappe(proie, candidats, carte)) {
            m_mobile.arreter();
        } else {
            m_mobile.allerVers(proie->cercle.centre, carte);
        }
    }

    m_mobile.mettreAJour(dt, carte, obstacles);
}

bool Monstre::peutAtteindre(Cercle cible, const TileMap& carte) const {
    return m_attaque.estAPortee(getCercle(), cible) && carte.ligneDeVue(m_mobile.getCentre(), cible.centre);
}

bool Monstre::engagerAttaque(std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte) {
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
