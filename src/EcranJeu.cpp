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
#include "BarreDeSkills.hpp"
#include "Composition.hpp"
#include "Contexte.hpp"
#include "Controleur.hpp"
#include "EcranDeFin.hpp"
#include "EtatSkill.hpp"
#include "GenerateurDonjon.hpp"
#include "Niveau.hpp"
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

EcranJeu::EcranJeu() : m_partie(nouvellePartie(m_tirageDeGraine)) {}

void EcranJeu::ajusterCamera(const Contexte& contexte) {
    m_camera.setSize(tailleDeVue(contexte.fenetre.getSize(), s_hauteurDeVue));
}

// Réaffectation : la Partie garde son adresse, et le Controleur repart
// sans visée ni déplacement continu.
void EcranJeu::nouvelleCarte() {
    m_partie = nouvellePartie(m_tirageDeGraine);
    m_controleur = Controleur{};
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
            m_controleur.appuiGauche(m_partie, fenetre.mapPixelToCoords(data->position));
        } else if (data->button == sf::Mouse::Button::Right) {
            m_controleur.appuiDroit();
        }
    }
    if (const auto* data = evenement.getIf<sf::Event::KeyPressed>()) {
        // Scancode : la touche physique « 1 » de la rangée du haut,
        // quelle que soit la disposition du clavier (AZERTY : « & »).
        // Touche N : skill à l'adresse de la touche (table du Controleur).
        if (data->scancode == sf::Keyboard::Scan::Num1) {
            m_controleur.appuiTouche(m_partie, 1);
        } else if (data->scancode == sf::Keyboard::Scan::Num2) {
            m_controleur.appuiTouche(m_partie, 2);
        } else if (data->scancode == sf::Keyboard::Scan::Num3) {
            m_controleur.appuiTouche(m_partie, 3);
        } else if (data->scancode == sf::Keyboard::Scan::Num4) {
            m_controleur.appuiTouche(m_partie, 4);
        } else if (data->scancode == sf::Keyboard::Scan::Escape) {
            m_controleur.appuiEchap();
        } else if (data->scancode == sf::Keyboard::Scan::R && (m_partie.estPerdue() || m_partie.estGagnee())) {
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
    // Une icône par touche, construite à partir de l'adresse de la touche.
    std::array<std::optional<EtatSkill>, Controleur::s_nombreDeTouches> etatsDesSkills;
    std::optional<std::size_t> toucheEnVisee;
    for (std::size_t i = 0; i < etatsDesSkills.size(); ++i) {
        const std::optional<AdresseSkill> adresse = Controleur::adresseDeLaTouche(i + 1);
        if (!adresse.has_value()) {
            continue;
        }
        etatsDesSkills[i] = m_partie.getEscouade().etatDuSkill(*adresse);
        if (adresse == m_controleur.getAdresseEnVisee()) {
            toucheEnVisee = i + 1;
        }
    }
    dessinerBarreDeSkills(fenetre, contexte.police, etatsDesSkills, toucheEnVisee);
    dessinerCompteurDeMonstres(fenetre, contexte.police, m_partie.monstresRestants());
    if (m_partie.estPerdue()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Défaite");
    } else if (m_partie.estGagnee()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Victoire");
    }
}
