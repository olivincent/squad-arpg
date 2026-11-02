// ./src/monstres/Monstre.cpp
#include "monstres/Monstre.hpp"
#include "base/Cercle.hpp"
#include "combat/Autodefense.hpp"
#include "combat/Cible.hpp"
#include "combat/Controle.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/SourceDEffet.hpp"
#include "rendu/BarreDeVie.hpp"
#include "rendu/MarquesDEffets.hpp"
#include "monde/ChampDeVision.hpp"
#include "base/Eclaircir.hpp"
#include "monde/TileMap.hpp"
#include "rendu/TraceDeChemin.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <variant>

namespace {
constexpr std::uint8_t s_opaciteRetour = 128;  // sur 255 : environ 50 %, pour marquer le retour (débogage F3)
}  // namespace

Monstre::Monstre(sf::Vector2f centre, IdMonstre id, int groupe, const DefinitionDeMonstre& definition)
    : m_mobile(centre, {definition.cote, definition.cote}, definition.vitesse),
      m_couleur(definition.couleur),
      m_vitalite(definition.pointsDeVieMax,
                 {.armure = definition.armure, .statistiques = definition.statistiques}),
      m_id(id),
      m_groupe(groupe),
      m_attaque(definition.attaque),
      m_degats(definition.degats),
      m_rayonDetection(definition.rayonDetection),
      m_rayonTerritoire(definition.rayonTerritoire),
      m_zoneDeControle(definition.zoneDeControle),
      m_statistiques(definition.statistiques),
      m_chanceDeButin(definition.chanceDeButin),
      m_nombreDObjets(definition.nombreDObjets),
      m_experience(definition.experience),
      m_regenerationRetour(definition.regenerationRetour),
      m_etat(Etat::EnGarde),
      m_poste(centre),
      m_alerte(false),
      m_soinAccumule(0.0f),
      m_placeCibleEngagee(),
      m_etatDuRole(etatInitialDuRole(definition.role))
{
}

void Monstre::dessiner(sf::RenderTarget& rendu, bool estCible, bool marquerLeRetour) const {
    const sf::Vector2f centre = m_mobile.getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();

    sf::RectangleShape forme(taille);
    forme.setPosition(centre - taille / 2.0f);
    const bool enExecution = cibleEngagee().has_value() || cibleDuSoinEngage().has_value();  // un coup ou un soin
    sf::Color couleur = enExecution ? eclaircir(m_couleur, s_eclaircissementExecution) : m_couleur;
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
    dessinerAbsorption(rendu, positionBarreDeVie(centre, taille), taille.x, m_vitalite.getRatio(),
                       m_vitalite.getRatioDAbsorption(FamilleDAbsorption::Aegis), m_vitalite.getRatioDAbsorption(FamilleDAbsorption::Ward));
    dessinerMarquesDEffets(rendu, positionMarquesDEffets(centre, taille), m_vitalite.getEffets().pilesDesBonus(),
                           m_vitalite.getEffets().pilesDesMalus(), m_vitalite.getEffets().nombreDeControles());
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

IdMonstre Monstre::getId() const {
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
    appliquerLeControle();
    // Une attaque sans dégâts mais qui pose des effets (un contrôle) est hostile aussi : elle alerte.
    if (attaque.degats.total() > 0 || !attaque.effets.empty()) {
        alerter();
    }
    return resultat;
}

void Monstre::appliquerLeControle() {
    if (!peutAttaquer(m_vitalite.getEffets())) {
        annulerAttaque();
    }
    if (!peutUtiliserUnSkill(m_vitalite.getEffets())) {
        annulerSoin();
    }
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

int Monstre::getPointsDeVieMax() const {
    return m_vitalite.getMaximum();
}

const EffetsActifs& Monstre::getEffets() const {
    return m_vitalite.getEffets();
}

std::vector<Tick> Monstre::prendreLesTicks() {
    return m_vitalite.prendreLesTicks();
}

std::vector<ControleApplique> Monstre::prendreLesControles() {
    return m_vitalite.prendreLesControles();
}

float Monstre::getZoneDeControle() const {
    return m_zoneDeControle;
}

Attaque Monstre::getAttaque() const {
    // Les dégâts d'un monstre sont ceux de sa définition ; sa nature est celle du seul effet qu'il ait, des dégâts propres.
    return {.degats = m_degats,
            .capaciteOffensive = m_statistiques.capaciteOffensive,
            .nature = std::get<EffetDegats>(m_attaque.getEffet()).nature};
}

const StatistiquesDeCombat& Monstre::getStatistiques() const {
    return m_statistiques;
}

int Monstre::getChanceDeButin() const {
    return m_chanceDeButin;
}

int Monstre::getNombreDObjets() const {
    return m_nombreDObjets;
}

int Monstre::getExperience() const {
    return m_experience;
}

// Détecte la proie si elle est dans le rayon de détection, dans le territoire
// et en vue (`ligneDeVue`, en cases). Seule la détection exige la vue :
// une fois en poursuite, le monstre ne perd pas sa proie en la perdant de vue.
bool Monstre::proieDetectee(const std::optional<PersonnageVu>& proie, const TileMap& carte) const {
    if (!proie) {
        return false;
    }
    const sf::Vector2f centreProie = proie->cercle.centre;
    const float distanceMonstre = (centreProie - m_mobile.getCentre()).length();
    const float distancePoste = (centreProie - m_poste).length();
    return distanceMonstre <= m_rayonDetection && distancePoste <= m_rayonTerritoire
        && ligneDeVue(carte, m_mobile.getCentre(), centreProie);
}

std::optional<PersonnageVu> Monstre::cibleDeFrappe(const std::optional<PersonnageVu>& proie,
                                                  const std::vector<PersonnageVu>& candidats,
                                                  const TileMap& carte) const {
    const std::optional<Place> id = choisirCibleAFrapper(
        getCercle(), [&](Cercle c) { return peutAtteindre(c, carte); }, candidats,
        proie ? std::optional<Place>(proie->id) : std::nullopt);
    if (!id) {
        return std::nullopt;
    }
    // choisirCibleAFrapper ne choisit que parmi les candidats.
    const PersonnageVu* cible = trouverEntite(candidats, *id);
    assert(cible != nullptr);
    return *cible;
}

void Monstre::faireAvancerLeRole(float dt) {
    std::visit(Surcharges{[](EtatMelee&) {},
                          [dt](EtatTireur& tireur) { tireur.delaiRestant = std::max(0.0f, tireur.delaiRestant - dt); },
                          [dt](EtatSoigneur& soigneur) { soigneur.soin.mettreAJour(dt); },
                          [dt](EtatAssommeur& assommeur) { assommeur.delaiRestant = std::max(0.0f, assommeur.delaiRestant - dt); }},
               m_etatDuRole);
}

bool Monstre::agirSelonLeRole(const std::vector<PersonnageVu>& candidats, const std::vector<AllieVu>& allies,
                              const TileMap& carte, const std::vector<Cercle>& aContourner) {
    return std::visit(Surcharges{[](EtatMelee&) { return false; },
                                 [&](EtatTireur& tireur) { return reculerSiAuContact(tireur, candidats, carte, aContourner); },
                                 [&](EtatSoigneur& soigneur) { return allerSoigner(soigneur, allies, carte, aContourner); },
                                 [](EtatAssommeur&) { return false; }},
                      m_etatDuRole);
}

bool Monstre::peutSoigner(const EtatSoigneur& soigneur, const AllieVu& allie, const TileMap& carte) const {
    return soigneur.soin.estAPortee(getCercle(), allie.cercle) && ligneDeVue(carte, m_mobile.getCentre(), allie.cercle.centre);
}

bool Monstre::allerSoigner(EtatSoigneur& soigneur, const std::vector<AllieVu>& allies, const TileMap& carte,
                           const std::vector<Cercle>& aContourner) {
    // En Disruption (ou étourdi), le soin est interdit : le soigneur se comporte comme son attaque le décrit.
    if (soigneur.soin.getPhase() != PhaseSkill::Pret || !peutUtiliserUnSkill(m_vitalite.getEffets())) {
        return false;
    }
    const std::optional<AllieVu> blesse = choisirLeBlesse(allies, soigneur.role.seuilDeBlessure);
    if (!blesse) {
        return false;
    }
    if (peutSoigner(soigneur, *blesse, carte)) {
        m_mobile.arreter();  // immobile, pour que `engagerSoin` réussisse
    } else {
        m_mobile.allerVers(blesse->cercle.centre, carte, aContourner);
    }
    return true;
}

bool Monstre::engagerSoin(const std::vector<AllieVu>& allies, const TileMap& carte) {
    EtatSoigneur* soigneur = std::get_if<EtatSoigneur>(&m_etatDuRole);
    if (soigneur == nullptr || m_etat != Etat::Poursuite || cibleEngagee().has_value() || m_mobile.estEnMouvement() ||
        !peutUtiliserUnSkill(m_vitalite.getEffets())) {
        return false;
    }
    const std::optional<AllieVu> blesse = choisirLeBlesse(allies, soigneur->role.seuilDeBlessure);
    if (!blesse || !peutSoigner(*soigneur, *blesse, carte) || !soigneur->soin.commencer()) {
        return false;
    }
    soigneur->cibleDuSoin = blesse->id;
    return true;
}

std::optional<IdMonstre> Monstre::cibleDuSoinEngage() const {
    const EtatSoigneur* soigneur = std::get_if<EtatSoigneur>(&m_etatDuRole);
    if (soigneur == nullptr || soigneur->soin.getPhase() != PhaseSkill::EnExecution) {
        return std::nullopt;
    }
    return soigneur->cibleDuSoin;
}

std::optional<SoinDeMonstre> Monstre::avancerSoin(float dt, Generateur& generateur) {
    EtatSoigneur* soigneur = std::get_if<EtatSoigneur>(&m_etatDuRole);
    const std::optional<IdMonstre> cible = cibleDuSoinEngage();
    if (soigneur == nullptr || !cible.has_value() || !soigneur->soin.avancer(dt, generateur)) {
        return std::nullopt;
    }
    soigneur->cibleDuSoin.reset();
    // Le seul effet du soin d'un monstre est un `EffetSoin` (le constructeur de `Skill` vérifie le mode `Allie`).
    return SoinDeMonstre{.cible = *cible, .montant = std::get<EffetSoin>(soigneur->soin.getEffet()).montant};
}

void Monstre::annulerSoin() {
    if (EtatSoigneur* soigneur = std::get_if<EtatSoigneur>(&m_etatDuRole)) {
        soigneur->soin.annuler();
        soigneur->cibleDuSoin.reset();
    }
}

int Monstre::recevoirSoin(int montant) {
    if (!estCiblable() || !estVivant()) {
        return 0;
    }
    return m_vitalite.soigner(montant);
}

bool Monstre::reculerSiAuContact(EtatTireur& tireur, const std::vector<PersonnageVu>& candidats, const TileMap& carte,
                                 const std::vector<Cercle>& aContourner) {
    if (tireur.enRecul) {
        if (m_mobile.directionVoulue() && !m_mobile.estBloque()) {
            return true;
        }
        tireur.enRecul = false;  // arrivé ou bloqué : il reprend son comportement normal
        return false;
    }
    if (tireur.delaiRestant > 0.0f) {
        return false;
    }

    const PersonnageVu* auContact = nullptr;
    float ecartAuContact = 0.0f;
    for (const PersonnageVu& candidat : candidats) {
        const float ecart = distanceBordABord(getCercle(), candidat.cercle);
        if (ecart <= tireur.role.distanceDeContact && (auContact == nullptr || ecart < ecartAuContact)) {
            auContact = &candidat;
            ecartAuContact = ecart;
        }
    }
    if (auContact == nullptr) {
        return false;
    }

    const sf::Vector2f eloignement = m_mobile.getCentre() - auContact->cercle.centre;
    if (eloignement.lengthSquared() == 0.0f) {
        return false;  // centres confondus : pas de direction opposée
    }
    const sf::Vector2f destination = m_mobile.getCentre() + eloignement.normalized() * tireur.role.distanceDeRecul;
    // Un recul impossible n'a pas lieu et ne consomme pas le délai : destination dans un mur, derrière un coin, aucun chemin.
    if (!ligneDeVue(carte, m_mobile.getCentre(), destination)) {
        return false;
    }
    m_mobile.arreter();
    m_mobile.allerVers(destination, carte, aContourner);
    if (!m_mobile.directionVoulue()) {
        return false;
    }
    tireur.enRecul = true;
    tireur.delaiRestant = tireur.role.delaiEntreReculs;
    return true;
}

void Monstre::mettreAJour(float dt, std::optional<PersonnageVu> proie, const std::vector<PersonnageVu>& candidats,
                          const std::vector<AllieVu>& allies, const TileMap& carte, const std::vector<Cercle>& obstacles,
                          const std::vector<Cercle>& aContourner) {
    m_attaque.mettreAJour(dt * facteurDAttaque(m_vitalite.getEffets()));  // l'entrave ralentit l'attaque de base, pas le soin
    m_vitalite.mettreAJour(dt);
    if (!estVivant()) {
        return;  // mort d'un tick : `Partie` le retire dans l'image
    }
    faireAvancerLeRole(dt);

    // Étourdi, le monstre ne fait rien : ni déplacement, ni coup, ni soin (`appliquerLeControle` a déjà annulé l'activation en cours).
    // Ses recharges et son rôle ont continué à s'écouler plus haut ; son état (garde, poursuite, retour) reprend à la fin de l'effet.
    if (!peutBouger(m_vitalite.getEffets())) {
        m_mobile.arreter();
        return;
    }

    // Un coup (ou un soin) en exécution : le monstre reste sur place, ni poursuite ni
    // retour (l'état et le coup reçu sont pris en compte après le coup).
    if (cibleEngagee().has_value() || cibleDuSoinEngage().has_value()) {
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
        m_vitalite.viderLesEffets();  // invulnérable en rentrant : un effet dans le temps ne le tuerait pas, et ne reste pas
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

    // Un monstre en recul (tireur) ou en marche vers un blessé (soigneur) a déjà son ordre : il ne poursuit pas sa proie.
    if (m_etat == Etat::Poursuite && !agirSelonLeRole(candidats, allies, carte, aContourner)) {
        if (cibleDeFrappe(proie, candidats, carte)) {
            m_mobile.arreter();
        } else {
            // Sans proie, la poursuite a basculé en retour juste au-dessus.
            assert(proie.has_value());
            m_mobile.allerVers(proie->cercle.centre, carte, aContourner);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
        }
    }

    m_mobile.mettreAJour(dt * facteurDeDeplacement(m_vitalite.getEffets()), carte, obstacles);  // le ralentissement
}

bool Monstre::peutAtteindre(Cercle cible, const TileMap& carte) const {
    return m_attaque.estAPortee(getCercle(), cible) && ligneDeVue(carte, m_mobile.getCentre(), cible.centre);
}

bool Monstre::engagerAttaque(std::optional<PersonnageVu> proie, const std::vector<PersonnageVu>& candidats, const TileMap& carte) {
    if (m_etat == Etat::Retour) {
        return false;
    }
    const std::optional<PersonnageVu> cible = cibleDeFrappe(proie, candidats, carte);
    if (!cible || !peutAttaquer(m_vitalite.getEffets()) || cibleDuSoinEngage().has_value() || m_mobile.estEnMouvement() || !m_attaque.commencer()) {
        return false;
    }
    m_placeCibleEngagee = cible->id;
    return true;
}

std::optional<Place> Monstre::cibleEngagee() const {
    if (m_attaque.getPhase() != PhaseSkill::EnExecution) {
        return std::nullopt;
    }
    return m_placeCibleEngagee;
}

std::optional<Attaque> Monstre::avancerAttaque(float dt, Generateur& generateur) {
    if (!cibleEngagee().has_value() || !m_attaque.avancer(dt * facteurDAttaque(m_vitalite.getEffets()), generateur)) {
        return std::nullopt;
    }
    m_placeCibleEngagee.reset();
    Attaque attaque = getAttaque();
    poserLeCoupAssommantSiPret(attaque);
    return attaque;
}

void Monstre::poserLeCoupAssommantSiPret(Attaque& attaque) {
    EtatAssommeur* assommeur = std::get_if<EtatAssommeur>(&m_etatDuRole);
    // La Disruption l'empêche comme un skill ; elle ne peut pas coïncider avec un étourdissement, qui annule le coup avant qu'il parte. Le délai
    // part avec le coup, qu'il touche ou non : le monstre ne sait pas ce que sa cible en fait (esquive).
    if (assommeur == nullptr || assommeur->delaiRestant > 0.0f || !peutUtiliserUnSkill(m_vitalite.getEffets())) {
        return;
    }
    attaque.effets.push_back({.source = IdCapacite::CoupAssommant,
                              .rang = 0,
                              .dureeRestante = assommeur->role.dureeDeLEtourdissement,
                              .sorte = Controle{.etat = EtatInflige::Etourdissement}});
    assommeur->delaiRestant = assommeur->role.delaiEntreCoups;
}

void Monstre::annulerAttaque() {
    m_attaque.annuler();
    m_placeCibleEngagee.reset();
}
