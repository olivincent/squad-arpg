// ./src/interface/EcranJeu.cpp
#include "interface/EcranJeu.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cstddef>
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
#include "combat/Progression.hpp"
#include "interface/AffichageNiveau.hpp"
#include "interface/AffichageObjetsAuSol.hpp"
#include "interface/AffichageProfondeur.hpp"
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

// L'étage où l'expédition est : la graine de l'étage (affichée dans la console, pour pouvoir
// rejouer à l'identique) dérive de celle de l'expédition et de la profondeur ; elle génère le niveau,
// puis la partie est construite avec la même graine : la même graine redonne la même carte et le
// même combat.
Partie partieDeLEtage(const Expedition& expedition) {
    const Generateur::result_type graine = graineDEtage(expedition.graine, expedition.profondeur);
    std::cout << "Seed: " << graine << '\n';
    Generateur generateurDeNiveau(graine);
    const Niveau niveau = genererNiveau(generateurDeNiveau, ParametresDonjon{});
    return Partie(niveau, expedition, graine);
}

// Une nouvelle expédition, au premier étage : tire sa graine et prend l'équipement de départ des classes choisies.
Partie nouvellePartie(std::random_device& tirage, const std::vector<IdSpecialite>& specialites) {
    return partieDeLEtage(nouvelleExpedition(composer(specialites), tirage()));
}

constexpr unsigned int s_tailleCompteur = 22;
constexpr float s_margeCompteur = 16.0f;
constexpr std::size_t s_objetsParF7 = 4;  // F7 (débogage) : combien d'objets tirés au hasard
constexpr float s_ecartDansLEnTete = 24.0f;            // entre « Depth » et « Level », en pixels
constexpr float s_largeurBarreDExperience = 110.0f;    // en pixels
constexpr float s_hauteurBarreDExperience = 8.0f;
const sf::Color s_couleurFondDeLaBarreDExperience(30, 30, 30, 200);
const sf::Color s_couleurDeLaBarreDExperience(120, 220, 255);  // le bleu clair de « Level up! »
const sf::Color s_couleurDesPointsADepenser(255, 215, 0);      // « Points to spend » : l'or de « Points: N » de l'inventaire

// Un texte de l'en-tête, contour noir pour rester lisible sur la carte.
sf::Text texteDeLEnTete(const sf::Font& police, const std::string& ligne, sf::Vector2f position) {
    sf::Text texte(police, ligne, s_tailleCompteur);
    texte.setFillColor(sf::Color::White);
    texte.setOutlineColor(sf::Color::Black);
    texte.setOutlineThickness(2.0f);
    texte.setPosition(position);
    return texte;
}

// La barre d'expérience : un fond sombre et un remplissage proportionnel à la progression vers le niveau suivant (de 0 à 1),
// centrée sur la hauteur de la ligne de texte qui la précède.
void dessinerLaBarreDExperience(sf::RenderTarget& rendu, sf::Vector2f coinHautGauche, float progression) {
    sf::RectangleShape fond({s_largeurBarreDExperience, s_hauteurBarreDExperience});
    fond.setPosition(coinHautGauche);
    fond.setFillColor(s_couleurFondDeLaBarreDExperience);
    fond.setOutlineColor(sf::Color::Black);
    fond.setOutlineThickness(1.0f);
    rendu.draw(fond);
    sf::RectangleShape remplissage({s_largeurBarreDExperience * std::clamp(progression, 0.0f, 1.0f), s_hauteurBarreDExperience});
    remplissage.setPosition(coinHautGauche);
    remplissage.setFillColor(s_couleurDeLaBarreDExperience);
    rendu.draw(remplissage);
}

// « Depth 2 / 5 », « Level N », sa barre d'expérience et, tant qu'un personnage a des points libres, « Points to spend » sur la
// première ligne, puis « Monsters: N » dessous, dans le coin haut gauche, en coordonnées de la fenêtre (à appeler avec la vue par
// défaut).
void dessinerEnTete(sf::RenderTarget& rendu, const sf::Font& police, const Partie& partie) {
    const float interligne = police.getLineSpacing(s_tailleCompteur);
    const sf::Text profondeur = texteDeLEnTete(police, ligneDeProfondeur(partie.getProfondeur()), {s_margeCompteur, s_margeCompteur});
    rendu.draw(profondeur);
    const float xNiveau = s_margeCompteur + profondeur.getGlobalBounds().size.x + s_ecartDansLEnTete;
    const sf::Text niveau = texteDeLEnTete(police, ligneDeNiveau(partie.getNiveau()), {xNiveau, s_margeCompteur});
    rendu.draw(niveau);
    const float xBarre = xNiveau + niveau.getGlobalBounds().size.x + s_ecartDansLEnTete / 2.0f;
    dessinerLaBarreDExperience(rendu, {xBarre, s_margeCompteur + (interligne - s_hauteurBarreDExperience) / 2.0f},
                               progressionVersLeNiveauSuivant(partie.getExperience()));
    if (partie.getEscouade().aDesPointsLibres()) {
        sf::Text points = texteDeLEnTete(police, ligneDePointsADepenser(), {xBarre + s_largeurBarreDExperience + s_ecartDansLEnTete, s_margeCompteur});
        points.setFillColor(s_couleurDesPointsADepenser);
        rendu.draw(points);
    }
    rendu.draw(texteDeLEnTete(police, "Monsters: " + std::to_string(partie.monstresRestants()), {s_margeCompteur, s_margeCompteur + interligne}));
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

// L'escouade a pris l'escalier : l'étage suivant, avec la même expédition (un étage plus bas). Une partie neuve, donc une
// carte neuve, des monstres neufs, les personnages à pleine vie ; l'interface de l'étage (visée, textes) repart de zéro. Les
// bascules de débogage et la barre de raccourcis (mêmes classes, mêmes skills) restent.
void EcranJeu::descendre() {
    m_partie = partieDeLEtage(etageSuivant(m_partie.expedition()));
    m_controleur = Controleur{};
    m_textes = TextesFlottants{};
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
                return Empiler{std::make_unique<EcranInventaire>(m_partie.getEscouade(), m_partie.getSol(), m_partie.getCarte(),
                                                                  std::move(noms))};
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
            [this](commande::AjouterDesObjetsAleatoires) -> Transition {
                // Un refus (pas quatre cases libres) est normal : rien n'est ajouté, il n'y a rien à dire.
                static_cast<void>(m_partie.ramasserDesObjetsAleatoires(s_objetsParF7));
                return Aucune{};
            },
            [this](commande::DescendreDUnEtage) -> Transition {
                m_partie.forcerLaDescente();  // la descente se fait à la fin de l'image (mettreAJour)
                return Aucune{};
            },
            [this](commande::GagnerUnNiveau) -> Transition {
                m_partie.gagnerUnNiveau();
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
    if (m_partie.doitDescendre()) {
        descendre();
    }
    // Sortie de la visée si le lanceur ne peut plus lancer (mort, défaite) :
    // ici, en fin d'image, donc avant les événements de la suivante.
    m_controleur.debutImage(m_partie);
    m_camera.setCenter(m_partie.getEscouade().getCentre());
}

void EcranJeu::dessiner(const Contexte& contexte) const {
    sf::RenderWindow& fenetre = contexte.fenetre;
    fenetre.setView(m_camera);
    m_partie.dessinerLaCarte(fenetre);
    dessinerObjetsAuSol(fenetre, m_partie.getSol().tous());  // au-dessus du sol, sous les monstres et l'escouade
    m_partie.dessinerLesEntites(fenetre, m_debogage.ancre, m_debogage.pointsVises, m_debogage.brouillard);
    m_controleur.dessinerApercu(m_partie, fenetre, fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre)));
    if (m_debogage.pointsVises) {
        m_partie.getEscouade().dessinerPointsVises(fenetre);
        m_partie.dessinerCheminsDesMonstres(fenetre);
        m_partie.dessinerZonesDeControle(fenetre);
    }
    m_textes.dessiner(fenetre, contexte.police);
    if (const ObjetAuSol* survole = m_partie.objetAuSolSous(fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre)))) {
        dessinerNomDObjetAuSol(fenetre, contexte.police, *survole);
    }

    // Interface fixe à l'écran : vue de l'interface, dessinée en dernier.
    fenetre.setView(vueDInterface(fenetre.getSize()));
    dessinerBarreDeSkills(fenetre, contexte.police,
                          iconesDeLaBarre(m_barre, m_partie.getEscouade(), contexte.reglages.touchesDesRaccourcis,
                                          m_controleur.getAdresseEnVisee()));
    dessinerEnTete(fenetre, contexte.police, m_partie);
    if (m_partie.estPerdue()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Defeat");
    } else if (m_partie.estGagnee()) {
        dessinerEcranDeFin(fenetre, contexte.police, "Victory");
    }
}
