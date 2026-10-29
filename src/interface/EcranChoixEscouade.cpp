// ./src/interface/EcranChoixEscouade.cpp
#include "interface/EcranChoixEscouade.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "escouade/Composition.hpp"
#include "interface/Contexte.hpp"
#include "interface/EcranJeu.hpp"
#include "escouade/Specialite.hpp"
#include "escouade/TailleDeLEscouade.hpp"
#include "interface/EntreeDInterface.hpp"

namespace {

constexpr std::size_t s_actionCommencer = 0;
constexpr std::size_t s_actionRetour = 1;

// Décalage de chaque colonne par rapport au centre de la vue.
const sf::Vector2f s_decalageDesClasses{-300.0f, 0.0f};
const sf::Vector2f s_decalageDesCases{300.0f, -80.0f};
const sf::Vector2f s_decalageDesActions{300.0f, 140.0f};
constexpr float s_hauteurDuTitre = -280.0f;  // par rapport au centre de la vue

constexpr unsigned int s_tailleTitre = 36;
constexpr unsigned int s_tailleLibelle = 28;
constexpr float s_largeurDeLaBande = 12.0f;  // bande de la couleur du personnage, à gauche d'une case
constexpr float s_epaisseurContour = 2.0f;
const sf::Color s_couleurFond(50, 50, 50);
const sf::Color s_couleurSurvol(95, 95, 95);
const sf::Color s_couleurCaseVide(30, 30, 30);
const sf::Color s_couleurTexteVide(110, 110, 110);

std::vector<std::string> nomsDesClasses() {
    std::vector<std::string> noms;
    for (const Specialite& specialite : catalogue()) {
        noms.push_back(specialite.nom);
    }
    return noms;
}

// Texte UTF-8 (accents, tiret) : sans fromUtf8, sf::String le lirait comme de l'ANSI.
sf::Text texteCentre(const sf::Font& police, const std::string& libelle, unsigned int taille, sf::Vector2f centre,
                     sf::Color couleur) {
    sf::Text texte(police, sf::String::fromUtf8(libelle.begin(), libelle.end()), taille);
    texte.setFillColor(couleur);
    const sf::FloatRect limites = texte.getLocalBounds();
    texte.setOrigin(limites.position + limites.size / 2.0f);
    texte.setPosition(centre);
    return texte;
}

// Les trois cases de sélection : « 1. Guerrier » ou « 1. — », avec la couleur que le personnage
// aura dans la partie (les doublons sont assombris) ; une case remplie survolée est plus claire.
void dessinerLesCases(sf::RenderTarget& rendu, const sf::Font& police, const SelectionDEscouade& selection,
                      sf::Vector2f centreDesColonne, std::optional<std::size_t> survolee) {
    const std::vector<DefinitionPersonnage> composition = composer(selection.liste());
    for (std::size_t i = 0; i < s_tailleDeLEscouade; ++i) {
        const sf::FloatRect zone = rectangleDeLaCase(i, centreDesColonne);
        const bool remplie = i < selection.liste().size();

        sf::RectangleShape fond(zone.size);
        fond.setPosition(zone.position);
        fond.setFillColor(!remplie ? s_couleurCaseVide : (survolee == i ? s_couleurSurvol : s_couleurFond));
        fond.setOutlineColor(remplie ? sf::Color::White : s_couleurTexteVide);
        fond.setOutlineThickness(s_epaisseurContour);
        rendu.draw(fond);

        const std::string numero = std::to_string(i + 1) + ". ";
        if (remplie) {
            sf::RectangleShape bande({s_largeurDeLaBande, zone.size.y});
            bande.setPosition(zone.position);
            bande.setFillColor(composition[i].couleur);
            rendu.draw(bande);
        }
        const std::string libelle = numero + (remplie ? specialite(selection.liste()[i]).nom : "—");
        rendu.draw(texteCentre(police, libelle, s_tailleLibelle, zone.position + zone.size / 2.0f,
                               remplie ? sf::Color::White : s_couleurTexteVide));
    }
}

}  // namespace

sf::Vector2f centreDesClasses(sf::Vector2f centre) {
    return centre + s_decalageDesClasses;
}

sf::Vector2f centreDesCases(sf::Vector2f centre) {
    return centre + s_decalageDesCases;
}

sf::Vector2f centreDesActions(sf::Vector2f centre) {
    return centre + s_decalageDesActions;
}

EcranChoixEscouade::EcranChoixEscouade() : m_classes(nomsDesClasses()), m_actions({"Start", "Back"}) {
    actualiserBoutons();
}

void EcranChoixEscouade::actualiserBoutons() {
    m_actions.definirActif(s_actionCommencer, m_selection.estComplete());
}

const SelectionDEscouade& EcranChoixEscouade::getSelection() const {
    return m_selection;
}

Transition EcranChoixEscouade::cliquer(sf::Vector2f point, sf::Vector2f centre) {
    if (const std::optional<std::size_t> classe = m_classes.boutonSous(point, centreDesClasses(centre))) {
        m_selection.ajouter(catalogue()[*classe].id);
        actualiserBoutons();
        return Aucune{};
    }
    if (const std::optional<std::size_t> indice = caseSous(point, centreDesCases(centre));
        indice.has_value() && *indice < m_selection.liste().size()) {
        m_selection.retirer(*indice);
        actualiserBoutons();
        return Aucune{};
    }
    if (const std::optional<std::size_t> action = m_actions.boutonSous(point, centreDesActions(centre))) {
        if (*action == s_actionCommencer) {
            return Empiler{std::make_unique<EcranJeu>(m_selection.liste())};
        }
        if (*action == s_actionRetour) {
            return Depiler{};
        }
    }
    return Aucune{};
}

Transition EcranChoixEscouade::traiterEvenement(const sf::Event& evenement, const Contexte& contexte) {
    const std::optional<PointDInterface> clic = clicGaucheDInterface(evenement, contexte);
    if (!clic.has_value()) {
        return Aucune{};
    }
    return cliquer(clic->point, clic->centre);
}

void EcranChoixEscouade::mettreAJour(float, const Contexte&) {}

void EcranChoixEscouade::dessiner(const Contexte& contexte) const {
    sf::RenderWindow& fenetre = contexte.fenetre;
    const PointDInterface pointeur = preparerLInterface(contexte);
    const sf::Vector2f centre = pointeur.centre;
    const sf::Vector2f curseur = pointeur.point;

    const std::string titre = "Choose " + std::to_string(s_tailleDeLEscouade) + " classes";
    fenetre.draw(texteCentre(contexte.police, titre, s_tailleTitre, centre + sf::Vector2f{0.0f, s_hauteurDuTitre}, sf::Color::White));

    m_classes.dessiner(fenetre, contexte.police, centreDesClasses(centre), m_classes.boutonSous(curseur, centreDesClasses(centre)));
    dessinerLesCases(fenetre, contexte.police, m_selection, centreDesCases(centre), caseSous(curseur, centreDesCases(centre)));
    m_actions.dessiner(fenetre, contexte.police, centreDesActions(centre), m_actions.boutonSous(curseur, centreDesActions(centre)));
}
