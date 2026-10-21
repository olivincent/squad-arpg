// ./src/interface/EcranJeu.cpp
#include "interface/EcranJeu.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>
#include <memory>
#include <string>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "combat/AdresseSkill.hpp"
#include "base/Aleatoire.hpp"
#include "interface/BarreDeRaccourcis.hpp"
#include "interface/BarreDeSkills.hpp"
#include "escouade/Specialite.hpp"
#include "interface/ClicInterface.hpp"
#include "combat/Cible.hpp"
#include "interface/CommandeDeTouche.hpp"
#include "interface/Contexte.hpp"
#include "interface/Controleur.hpp"
#include "interface/EcranDeFin.hpp"
#include "interface/EcranInventaire.hpp"
#include "monde/GenerateurDonjon.hpp"
#include "monde/Niveau.hpp"
#include "interface/Reglages.hpp"
#include "interface/TailleDeVue.hpp"

namespace {

// Une nouvelle carte : tire une graine (affichée dans la console, pour pouvoir
// rejouer à l'identique), génère le niveau avec elle, puis construit la
// partie avec la même graine : la même graine redonne la même carte et le
// même combat.
Partie nouvellePartie(std::random_device& tirage, const std::vector<IdSpecialite>& specialites) {
    const Generateur::result_type graine = tirage();
    std::cout << "Graine : " << graine << '\n';
    Generateur generateurDeNiveau(graine);
    const Niveau niveau = genererNiveau(generateurDeNiveau, ParametresDonjon{});
    return Partie(niveau, composer(specialites), graine);
}

constexpr unsigned int s_tailleCompteur = 22;
constexpr float s_margeCompteur = 16.0f;

// « Monstres : N » dans le coin haut gauche, en coordonnées de la fenêtre
// (à appeler avec la vue par défaut). Contour noir pour rester lisible sur la carte.
void dessinerCompteurDeMonstres(sf::RenderTarget& rendu, const sf::Font& police, int restants) {
    sf::Text texte(police, "Monstres : " + std::to_string(restants), s_tailleCompteur);
    texte.setFillColor(sf::Color::White);
    texte.setOutlineColor(sf::Color::Black);
    texte.setOutlineThickness(2.0f);
    texte.setPosition({s_margeCompteur, s_margeCompteur});
    rendu.draw(texte);
}

}  // namespace

EcranJeu::EcranJeu(std::vector<IdSpecialite> specialites)
    : m_specialites(std::move(specialites)),
      m_partie(nouvellePartie(m_tirageDeGraine, m_specialites)),
      m_barre(barreParDefaut(m_partie.getEscouade().adressesDesSkills())) {}

void EcranJeu::ajusterCamera(const Contexte& contexte) {
    m_camera.setSize(tailleDeVue(contexte.fenetre.getSize(), s_hauteurDeVue));
}

// Réaffectation : la Partie garde son adresse, le Controleur repart
// sans visée ni déplacement continu et la barre retrouve sa disposition par défaut.
void EcranJeu::nouvelleCarte() {
    m_partie = nouvellePartie(m_tirageDeGraine, m_specialites);
    m_controleur = Controleur{};
    m_barre = barreParDefaut(m_partie.getEscouade().adressesDesSkills());
    m_textes = TextesFlottants{};
    m_debogage = OptionsDeDebogage{};
}

Transition EcranJeu::traiterEvenement(const sf::Event& evenement, const Contexte& contexte) {
    sf::RenderWindow& fenetre = contexte.fenetre;
    // L'image précédente a fini sur la vue de l'interface : les clics
    // doivent être convertis avec la caméra du monde.
    ajusterCamera(contexte);
    fenetre.setView(m_camera);
    if (const auto* data = evenement.getIf<sf::Event::MouseButtonPressed>()) {
        if (data->button == sf::Mouse::Button::Left) {
            // L'interface d'abord : le pixel est testé avec la vue d'interface avant tout
            // traitement dans le monde. Un clic sur la barre est consommé ; sur l'écran de fin,
            // les raccourcis sont ignorés comme au clavier.
            const sf::View vueInterface = vueDInterface(fenetre.getSize());
            const std::optional<ClicInterface> clic =
                clicSurInterface(fenetre.mapPixelToCoords(data->position, vueInterface), vueInterface.getSize());
            if (!clic.has_value()) {
                m_controleur.appuiGauche(m_partie, fenetre.mapPixelToCoords(data->position));
            } else if (!m_partie.estPerdue() && !m_partie.estGagnee()) {
                m_controleur.appuiInterface(m_partie, m_barre, *clic);
            }
        } else if (data->button == sf::Mouse::Button::Right) {
            m_controleur.appuiDroit();
        }
    }
    if (const auto* data = evenement.getIf<sf::Event::KeyPressed>()) {
        // Scancode : la touche physique, quelle que soit la disposition du clavier
        // (AZERTY : la touche « Q » du réglage par défaut est étiquetée « A »).
        const bool finDePartie = m_partie.estPerdue() || m_partie.estGagnee();
        return executer(commandePourTouche(data->scancode, finDePartie, contexte.reglages.touchesDesRaccourcis));
    }
    return Aucune{};
}

Transition EcranJeu::executer(const CommandeDeTouche& commande) {
    return std::visit(
        Surcharges{
            [](commande::Rien) -> Transition { return Aucune{}; },
            [this](commande::Annuler) -> Transition {
                m_controleur.appuiEchap();
                return Aucune{};
            },
            [this](commande::OuvrirInventaire) -> Transition {
                // Le relâchement du bouton sera reçu par l'inventaire, pas par le Controleur.
                m_controleur.interrompre();
                std::vector<std::string> noms;
                noms.reserve(m_specialites.size());
                for (const IdSpecialite id : m_specialites) {
                    noms.push_back(specialite(id).nom);
                }
                return Empiler{std::make_unique<EcranInventaire>(m_partie.getEscouade(), std::move(noms))};
            },
            [](commande::RetourAuChoix) -> Transition { return Depiler{}; },  // retour à l'écran de choix
            [this](commande::NouvelleCarte) -> Transition {
                nouvelleCarte();
                return Aucune{};
            },
            [this](commande::BasculerPointsVises) -> Transition {
                m_debogage.pointsVises = !m_debogage.pointsVises;
                return Aucune{};
            },
            [this](commande::BasculerAncre) -> Transition {
                m_debogage.ancre = !m_debogage.ancre;
                return Aucune{};
            },
            [this](commande::BasculerBrouillard) -> Transition {
                m_debogage.brouillard = !m_debogage.brouillard;
                return Aucune{};
            },
            [this](commande::ActiverEmplacement activer) -> Transition {
                m_controleur.activerEmplacement(m_partie, m_barre, activer.emplacement);
                return Aucune{};
            },
        },
        commande);
}

void EcranJeu::mettreAJour(float dt, const Contexte& contexte) {
    sf::RenderWindow& fenetre = contexte.fenetre;
    ajusterCamera(contexte);
    fenetre.setView(m_camera);
    const bool boutonTenu = fenetre.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    m_controleur.mettreAJourDeplacement(m_partie, boutonTenu, fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre)));
    m_partie.mettreAJour(dt);
    // Les événements de l'image deviennent des textes ; dans le brouillard, on ne montre rien (F6 : tout).
    // Les textes continuent leur animation après une défaite ou une victoire : le monde est figé, pas eux.
    for (const EvenementDeCombat& evenement : m_partie.prendreEvenements()) {
        if (!m_debogage.brouillard || m_partie.estVisible(evenement.position)) {
            m_textes.ajouter(evenement);
        }
    }
    m_textes.mettreAJour(dt);
    // Sortie de la visée si le lanceur ne peut plus lancer (mort, défaite) :
    // ici, en fin d'image, donc avant les événements de la suivante.
    m_controleur.debutImage(m_partie);
    m_camera.setCenter(m_partie.getEscouade().getCentre());
}

void EcranJeu::dessiner(const Contexte& contexte) const {
    sf::RenderWindow& fenetre = contexte.fenetre;
    fenetre.setView(m_camera);
    m_partie.dessiner(fenetre, m_debogage.ancre, m_debogage.pointsVises, m_debogage.brouillard);
    m_controleur.dessinerApercu(m_partie, fenetre, fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre)));
    if (m_debogage.pointsVises) {
        m_partie.getEscouade().dessinerPointsVises(fenetre);
        m_partie.dessinerCheminsDesMonstres(fenetre);
        m_partie.dessinerZonesDeControle(fenetre);
    }
    m_textes.dessiner(fenetre, contexte.police);

    // Interface fixe à l'écran : vue de l'interface, dessinée en dernier.
    fenetre.setView(vueDInterface(fenetre.getSize()));
    dessinerBarreDeSkills(fenetre, contexte.police,
                          iconesDeLaBarre(m_barre, m_partie.getEscouade(), contexte.reglages.touchesDesRaccourcis,
                                          m_controleur.getAdresseEnVisee()));
    dessinerCompteurDeMonstres(fenetre, contexte.police, m_partie.monstresRestants());
    if (m_partie.estPerdue()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Défaite");
    } else if (m_partie.estGagnee()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Victoire");
    }
}
