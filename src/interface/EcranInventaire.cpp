// ./src/interface/EcranInventaire.cpp
#include "interface/EcranInventaire.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "escouade/Escouade.hpp"
#include "interface/AffichageObjets.hpp"
#include "interface/Contexte.hpp"
#include "interface/DispositionInventaire.hpp"
#include "interface/EntreeDInterface.hpp"
#include "objets/CatalogueObjets.hpp"

namespace {

constexpr unsigned char s_opaciteVoile = 170;
constexpr float s_margePanneau = 40.0f;  // entre le panneau et le bord de la vue
constexpr float s_epaisseurContour = 2.0f;
constexpr unsigned int s_tailleTitre = 36;
constexpr float s_hauteurTitre = 20.0f;  // du haut du panneau
constexpr unsigned int s_tailleEnTete = 18;
constexpr unsigned int s_tailleArmure = 16;
constexpr unsigned int s_tailleNomDObjet = 12;
constexpr unsigned int s_tailleNomDEmplacement = 11;
constexpr unsigned int s_tailleSigle = 18;
constexpr unsigned int s_tailleEtiquetteDuSac = 16;
constexpr float s_margeDuTexte = 6.0f;  // entre le texte d'un emplacement et son bord
constexpr float s_hauteurEtiquetteDuSac = 24.0f;
const sf::Color s_couleurPanneau(40, 40, 40);
const sf::Color s_couleurCaseVide(25, 25, 25);
const sf::Color s_couleurCaseOccupee(70, 70, 70);
const sf::Color s_couleurContourVide(80, 80, 80);
const sf::Color s_couleurContourOccupe(200, 200, 200);
const sf::Color s_couleurTexteVide(120, 120, 120);
const sf::Color s_couleurMort(220, 80, 80);

// Un texte UTF-8 (accents) : sans fromUtf8, sf::String le lirait comme de l'ANSI.
sf::Text texte(const sf::Font& police, const std::string& libelle, unsigned int taille, sf::Color couleur) {
    sf::Text objet(police, sf::String::fromUtf8(libelle.begin(), libelle.end()), taille);
    objet.setFillColor(couleur);
    return objet;
}

// `libelle` centré dans `zone`, rétréci s'il dépasse sa largeur.
void dessinerTexteCentre(sf::RenderTarget& rendu, const sf::Font& police, const std::string& libelle, unsigned int taille,
                         sf::Color couleur, sf::FloatRect zone) {
    sf::Text objet = texte(police, libelle, taille, couleur);
    const sf::FloatRect limites = objet.getLocalBounds();
    const float largeurDisponible = zone.size.x - 2.0f * s_margeDuTexte;
    if (limites.size.x > largeurDisponible) {
        objet.setScale({largeurDisponible / limites.size.x, largeurDisponible / limites.size.x});
    }
    objet.setOrigin(limites.position + limites.size / 2.0f);
    objet.setPosition(zone.position + zone.size / 2.0f);
    rendu.draw(objet);
}

void dessinerRectangle(sf::RenderTarget& rendu, sf::FloatRect zone, sf::Color fond, sf::Color contour) {
    sf::RectangleShape forme(zone.size);
    forme.setPosition(zone.position);
    forme.setFillColor(fond);
    forme.setOutlineColor(contour);
    forme.setOutlineThickness(1.0f);
    rendu.draw(forme);
}

// Un emplacement : le nom de l'objet porté, ou le nom de l'emplacement en petit sur fond grisé.
void dessinerEmplacement(sf::RenderTarget& rendu, const sf::Font& police, sf::FloatRect zone, Emplacement emplacement,
                         const std::optional<Objet>& objet) {
    if (objet.has_value()) {
        dessinerRectangle(rendu, zone, s_couleurCaseOccupee, s_couleurContourOccupe);
        dessinerTexteCentre(rendu, police, definitionObjet(objet->id).nom, s_tailleNomDObjet, sf::Color::White, zone);
    } else {
        dessinerRectangle(rendu, zone, s_couleurCaseVide, s_couleurContourVide);
        dessinerTexteCentre(rendu, police, nomDeLEmplacement(emplacement), s_tailleNomDEmplacement, s_couleurTexteVide, zone);
    }
}

// La colonne d'un personnage : son en-tête (classe, couleur du personnage), son armure et ses 10 emplacements.
void dessinerColonne(sf::RenderTarget& rendu, const sf::Font& police, const Escouade& escouade, std::size_t place,
                     const std::string& nomDeLaClasse, sf::Vector2f tailleDeVue) {
    const sf::FloatRect enTete = rectangleDeLEnTete(place, tailleDeVue);
    dessinerRectangle(rendu, enTete, escouade.couleurDuPersonnage(place).value_or(sf::Color(90, 90, 90)), sf::Color::Black);
    sf::Text classe = texte(police, nomDeLaClasse, s_tailleEnTete, sf::Color::White);
    classe.setOutlineColor(sf::Color::Black);
    classe.setOutlineThickness(1.5f);
    const sf::FloatRect limites = classe.getLocalBounds();
    classe.setOrigin(limites.position + limites.size / 2.0f);
    classe.setPosition(enTete.position + enTete.size / 2.0f);
    rendu.draw(classe);

    const Equipement& equipement = escouade.equipementDuPersonnage(place);
    const sf::FloatRect zoneArmure = rectangleDeLArmure(place, tailleDeVue);
    sf::Text armure = texte(police, "Armure : " + std::to_string(equipement.armureTotale()), s_tailleArmure, sf::Color::White);
    armure.setPosition(zoneArmure.position + sf::Vector2f{s_margeDuTexte, 0.0f});
    rendu.draw(armure);
    if (escouade.pointsDeVieDuPersonnage(place).value_or(0) == 0) {
        sf::Text mort = texte(police, "(mort)", s_tailleArmure, s_couleurMort);
        const sf::FloatRect limitesMort = mort.getLocalBounds();
        mort.setPosition({zoneArmure.position.x + zoneArmure.size.x - s_margeDuTexte - limitesMort.size.x - limitesMort.position.x,
                          zoneArmure.position.y});
        rendu.draw(mort);
    }

    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        const Emplacement emplacement = static_cast<Emplacement>(i);
        dessinerEmplacement(rendu, police, rectangleDeLEmplacement(place, emplacement, tailleDeVue), emplacement,
                            equipement.a(emplacement));
    }
}

// Le sac : une case par indice, vide, ou avec le sigle de son objet.
void dessinerLeSac(sf::RenderTarget& rendu, const sf::Font& police, const Inventaire& sac, sf::Vector2f tailleDeVue) {
    assert(sac.capacite() == s_casesDuSac);
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, tailleDeVue);
    sf::Text etiquette = texte(police, "Sac", s_tailleEtiquetteDuSac, sf::Color::White);
    etiquette.setPosition(premiere.position - sf::Vector2f{0.0f, s_hauteurEtiquetteDuSac});
    rendu.draw(etiquette);

    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        const sf::FloatRect zone = rectangleDeLaCaseDuSac(indice, tailleDeVue);
        const std::optional<Objet>& objet = sac.a(indice);
        if (objet.has_value()) {
            dessinerRectangle(rendu, zone, s_couleurCaseOccupee, s_couleurContourOccupe);
            dessinerTexteCentre(rendu, police, sigleDeLObjet(objet->id), s_tailleSigle, sf::Color::White, zone);
        } else {
            dessinerRectangle(rendu, zone, s_couleurCaseVide, s_couleurContourVide);
        }
    }
}

}  // namespace

EcranInventaire::EcranInventaire(const Escouade& escouade, std::vector<std::string> nomsDesClasses)
    : m_escouade(escouade), m_nomsDesClasses(std::move(nomsDesClasses)) {
    assert(m_nomsDesClasses.size() == s_nombreDePlaces);
}

Transition EcranInventaire::appuyer(sf::Keyboard::Scancode touche) const {
    if (touche == sf::Keyboard::Scan::I || touche == sf::Keyboard::Scan::Escape) {
        return Depiler{};
    }
    return Aucune{};
}

Transition EcranInventaire::traiterEvenement(const sf::Event& evenement, const Contexte&) {
    if (const auto* data = evenement.getIf<sf::Event::KeyPressed>()) {
        return appuyer(data->scancode);
    }
    return Aucune{};
}

void EcranInventaire::mettreAJour(float, const Contexte&) {}

bool EcranInventaire::laisseVoirDessous() const {
    return true;
}

void EcranInventaire::dessiner(const Contexte& contexte) const {
    const PointDInterface vue = preparerLInterface(contexte);
    sf::RenderWindow& fenetre = contexte.fenetre;
    const sf::Vector2f tailleVue = vue.centre * 2.0f;

    sf::RectangleShape voile(tailleVue);
    voile.setFillColor(sf::Color(0, 0, 0, s_opaciteVoile));
    fenetre.draw(voile);

    sf::RectangleShape panneau(tailleVue - sf::Vector2f{2.0f * s_margePanneau, 2.0f * s_margePanneau});
    panneau.setPosition({s_margePanneau, s_margePanneau});
    panneau.setFillColor(s_couleurPanneau);
    panneau.setOutlineColor(sf::Color::White);
    panneau.setOutlineThickness(s_epaisseurContour);
    fenetre.draw(panneau);

    sf::Text titre = texte(contexte.police, "Inventaire", s_tailleTitre, sf::Color::White);
    const sf::FloatRect limites = titre.getLocalBounds();
    titre.setOrigin({limites.position.x + limites.size.x / 2.0f, limites.position.y});
    titre.setPosition({vue.centre.x, s_margePanneau + s_hauteurTitre});
    fenetre.draw(titre);

    for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
        dessinerColonne(fenetre, contexte.police, m_escouade, place, m_nomsDesClasses[place - 1], tailleVue);
    }
    dessinerLeSac(fenetre, contexte.police, m_escouade.sac(), tailleVue);
}
