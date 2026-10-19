// ./src/EcranJeu.cpp
#include "EcranJeu.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>
#include <iostream>
#include <optional>
#include <string>

#include "AdresseSkill.hpp"
#include "Aleatoire.hpp"
#include "BarreDeRaccourcis.hpp"
#include "BarreDeSkills.hpp"
#include "Composition.hpp"
#include "ClicInterface.hpp"
#include "Contexte.hpp"
#include "Controleur.hpp"
#include "EcranDeFin.hpp"
#include "GenerateurDonjon.hpp"
#include "Niveau.hpp"
#include "Reglages.hpp"
#include "TailleDeVue.hpp"

namespace {

// Une nouvelle carte : tire une graine (affichée dans la console, pour pouvoir
// rejouer à l'identique), génère le niveau avec elle, puis construit la
// partie avec la même graine : la même graine redonne la même carte et le
// même combat.
Partie nouvellePartie(std::random_device& tirage) {
    const Generateur::result_type graine = tirage();
    std::cout << "Graine : " << graine << '\n';
    Generateur generateurDeNiveau(graine);
    const Niveau niveau = genererNiveau(generateurDeNiveau, ParametresDonjon{});
    return Partie(niveau, compositionParDefaut(), graine);
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

EcranJeu::EcranJeu()
    : m_partie(nouvellePartie(m_tirageDeGraine)), m_barre(barreParDefaut(m_partie.getEscouade().adressesDesSkills())) {}

void EcranJeu::ajusterCamera(const Contexte& contexte) {
    m_camera.setSize(tailleDeVue(contexte.fenetre.getSize(), s_hauteurDeVue));
}

// Réaffectation : la Partie garde son adresse, le Controleur repart
// sans visée ni déplacement continu et la barre retrouve sa disposition par défaut.
void EcranJeu::nouvelleCarte() {
    m_partie = nouvellePartie(m_tirageDeGraine);
    m_controleur = Controleur{};
    m_barre = barreParDefaut(m_partie.getEscouade().adressesDesSkills());
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
        if (data->scancode == sf::Keyboard::Scan::Escape) {
            m_controleur.appuiEchap();
        } else if (data->scancode == sf::Keyboard::Scan::R && finDePartie) {
            return Depiler{};  // retour au menu
        } else if (data->scancode == sf::Keyboard::Scan::F5) {
            // Touche de débogage : à tout moment, même effet que R.
            nouvelleCarte();
        } else if (data->scancode == sf::Keyboard::Scan::F3) {
            m_debogage.pointsVises = !m_debogage.pointsVises;  // touche de débogage : points visés des personnages
        } else if (data->scancode == sf::Keyboard::Scan::F4) {
            m_debogage.ancre = !m_debogage.ancre;  // touche de débogage : carré vert de l'ancre
        } else if (data->scancode == sf::Keyboard::Scan::F6) {
            m_debogage.brouillard = !m_debogage.brouillard;  // touche de débogage : brouillard de guerre (visuel seulement)
        } else if (!finDePartie) {
            // Sur l'écran de fin, seul R compte : les raccourcis sont ignorés.
            const std::optional<std::size_t> emplacement = emplacementPourTouche(contexte.reglages.touchesDesRaccourcis, data->scancode);
            if (emplacement.has_value()) {
                m_controleur.activerEmplacement(m_partie, m_barre, *emplacement);
            }
        }
    }
    return Aucune{};
}

void EcranJeu::mettreAJour(float dt, const Contexte& contexte) {
    sf::RenderWindow& fenetre = contexte.fenetre;
    ajusterCamera(contexte);
    fenetre.setView(m_camera);
    const bool boutonTenu = fenetre.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    m_controleur.mettreAJourDeplacement(m_partie, boutonTenu, fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre)));
    m_partie.mettreAJour(dt);
    // Sortie de la visée si le lanceur ne peut plus lancer (mort, défaite) :
    // ici, en fin d'image, donc avant les événements de la suivante.
    m_controleur.debutImage(m_partie);
    m_camera.setCenter(m_partie.getEscouade().getCentre());
}

void EcranJeu::dessiner(const Contexte& contexte) const {
    sf::RenderWindow& fenetre = contexte.fenetre;
    fenetre.setView(m_camera);
    fenetre.clear();
    m_partie.dessiner(fenetre, m_debogage.ancre, m_debogage.pointsVises, m_debogage.brouillard);
    m_controleur.dessinerApercu(m_partie, fenetre, fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre)));
    if (m_debogage.pointsVises) {
        m_partie.getEscouade().dessinerPointsVises(fenetre);
        m_partie.dessinerCheminsDesMonstres(fenetre);
    }

    // Interface fixe à l'écran : vue de l'interface, dessinée en dernier.
    fenetre.setView(vueDInterface(fenetre.getSize()));
    // Une icône par emplacement de la barre, construite à partir de son adresse.
    std::array<IconeDeRaccourci, s_emplacementsPrincipaux> icones;
    for (std::size_t i = 0; i < icones.size(); ++i) {
        icones[i].touche = sf::Keyboard::getDescription(contexte.reglages.touchesDesRaccourcis[i]);
        const std::optional<AdresseSkill> adresse = m_barre.adresse(i);
        if (!adresse.has_value()) {
            continue;
        }
        icones[i].etat = m_partie.getEscouade().etatDuSkill(*adresse);
        icones[i].couleurDuLanceur = m_partie.getEscouade().couleurDuPersonnage(adresse->place).value_or(sf::Color::White);
        icones[i].enVisee = adresse == m_controleur.getAdresseEnVisee();
    }
    dessinerBarreDeSkills(fenetre, contexte.police, icones);
    dessinerCompteurDeMonstres(fenetre, contexte.police, m_partie.monstresRestants());
    if (m_partie.estPerdue()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Défaite");
    } else if (m_partie.estGagnee()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Victoire");
    }
}
