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
#include <variant>
#include <vector>

#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "combat/Cible.hpp"
#include "escouade/Escouade.hpp"
#include "interface/AffichageCaracteristiques.hpp"
#include "interface/AffichageStatistiques.hpp"
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
constexpr unsigned int s_tailleCaracteristique = 14;
constexpr unsigned int s_tailleNomDObjet = 12;
constexpr unsigned int s_tailleNomDEmplacement = 11;
constexpr unsigned int s_tailleSigle = 18;
constexpr unsigned int s_tailleEtiquetteDuSac = 16;
constexpr float s_margeDuTexte = 6.0f;  // entre le texte d'un emplacement et son bord
constexpr float s_hauteurEtiquetteDuSac = 24.0f;
constexpr unsigned int s_tailleInfobulle = 14;
constexpr float s_margeInfobulle = 8.0f;
const sf::Color s_couleurInfobulle(15, 15, 15);
const sf::Color s_couleurPanneau(40, 40, 40);
const sf::Color s_couleurCaseVide(25, 25, 25);
const sf::Color s_couleurCaseOccupee(70, 70, 70);
const sf::Color s_couleurContourVide(80, 80, 80);
const sf::Color s_couleurContourOccupe(200, 200, 200);
const sf::Color s_couleurTexteVide(120, 120, 120);
const sf::Color s_couleurMort(220, 80, 80);
const sf::Color s_couleurSurbrillance(255, 215, 0);
const sf::Color s_couleurInactif(230, 50, 50);     // cadre d'un objet porté dont le seuil n'est plus atteint, et lignes d'alerte de l'infobulle
constexpr float s_epaisseurInactif = 3.0f;
constexpr float s_epaisseurSurbrillance = 3.0f;
const sf::Color s_couleurAttenuation(40, 40, 40, 190);  // sur l'origine du glissement : le panneau, presque opaque

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

// Un emplacement : le nom de l'objet porté, encadré de rouge s'il est inactif, ou le nom de l'emplacement en petit
// sur fond grisé.
void dessinerEmplacement(sf::RenderTarget& rendu, const sf::Font& police, sf::FloatRect zone, Emplacement emplacement,
                         const std::optional<Objet>& objet, bool inactif) {
    if (objet.has_value()) {
        dessinerRectangle(rendu, zone, s_couleurCaseOccupee, s_couleurContourOccupe);
        dessinerTexteCentre(rendu, police, definitionObjet(objet->modele).nom, s_tailleNomDObjet, sf::Color::White, zone);
        if (inactif) {
            sf::RectangleShape cadre(zone.size);
            cadre.setPosition(zone.position);
            cadre.setFillColor(sf::Color::Transparent);
            cadre.setOutlineColor(s_couleurInactif);
            cadre.setOutlineThickness(-s_epaisseurInactif);  // vers l'intérieur : le cadre ne déborde pas sur le voisin
            rendu.draw(cadre);
        }
    } else {
        dessinerRectangle(rendu, zone, s_couleurCaseVide, s_couleurContourVide);
        dessinerTexteCentre(rendu, police, nomDeLEmplacement(emplacement), s_tailleNomDEmplacement, s_couleurTexteVide, zone);
    }
}

// Une ligne de texte dans la cellule du bloc des caractéristiques.
void dessinerCellule(sf::RenderTarget& rendu, const sf::Font& police, sf::FloatRect zone, const std::string& ligne) {
    sf::Text texteDeLaCellule = texte(police, ligne, s_tailleCaracteristique, sf::Color::White);
    texteDeLaCellule.setPosition(zone.position + sf::Vector2f{s_margeDuTexte, 0.0f});
    rendu.draw(texteDeLaCellule);
}

// Sous les emplacements d'un personnage : ses cinq caractéristiques effectives (avec le bonus de son
// équipement s'il y en a un) et ses points de vie, actuels sur maximum.
void dessinerLesCaracteristiques(sf::RenderTarget& rendu, const sf::Font& police, const Escouade& escouade,
                                 std::size_t place, sf::Vector2f tailleDeVue) {
    const Caracteristiques effectives = escouade.caracteristiquesDuPersonnage(place).value_or(Caracteristiques());
    const Caracteristiques bonus = escouade.equipementActifDuPersonnage(place).bonusTotal();
    for (std::size_t i = 0; i < s_nombreDeCaracteristiques; ++i) {
        const Caracteristique caracteristique = s_caracteristiques[i];
        dessinerCellule(rendu, police, rectangleDeLaCellule(place, i, tailleDeVue),
                        ligneDeCaracteristique(caracteristique, effectives.valeur(caracteristique), bonus.valeur(caracteristique)));
    }
    dessinerCellule(rendu, police, rectangleDeLaCellule(place, s_nombreDeCaracteristiques, tailleDeVue),
                    "HP " + std::to_string(escouade.pointsDeVieDuPersonnage(place).value_or(0)) + " / " +
                        std::to_string(escouade.pointsDeVieMaxDuPersonnage(place).value_or(0)));
}

// Sous les caractéristiques : ses statistiques de combat (capacités offensive et défensive).
void dessinerLesStatistiques(sf::RenderTarget& rendu, const sf::Font& police, const Escouade& escouade, std::size_t place,
                             sf::Vector2f tailleDeVue) {
    const std::vector<std::string> lignes = lignesDeStatistiques(escouade.statistiquesDeCombatDuPersonnage(place).value_or(StatistiquesDeCombat()));
    for (std::size_t i = 0; i < lignes.size(); ++i) {
        dessinerCellule(rendu, police, rectangleDeLaStatistique(place, i, tailleDeVue), lignes[i]);
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
    // L'armure qui a un effet : celle des objets actifs.
    sf::Text armure = texte(police, "Armor: " + std::to_string(escouade.equipementActifDuPersonnage(place).armureTotale()), s_tailleArmure, sf::Color::White);
    armure.setPosition(zoneArmure.position + sf::Vector2f{s_margeDuTexte, 0.0f});
    rendu.draw(armure);
    if (escouade.pointsDeVieDuPersonnage(place).value_or(0) == 0) {
        sf::Text mort = texte(police, "(dead)", s_tailleArmure, s_couleurMort);
        const sf::FloatRect limitesMort = mort.getLocalBounds();
        mort.setPosition({zoneArmure.position.x + zoneArmure.size.x - s_margeDuTexte - limitesMort.size.x - limitesMort.position.x,
                          zoneArmure.position.y});
        rendu.draw(mort);
    }

    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        const Emplacement emplacement = static_cast<Emplacement>(i);
        dessinerEmplacement(rendu, police, rectangleDeLEmplacement(place, emplacement, tailleDeVue), emplacement,
                            equipement.a(emplacement), escouade.objetPorteEstInactif(place, emplacement));
    }
    dessinerLesCaracteristiques(rendu, police, escouade, place, tailleDeVue);
    dessinerLesStatistiques(rendu, police, escouade, place, tailleDeVue);
}

// Le sac : une case par indice, vide, ou avec le sigle de son objet.
void dessinerLeSac(sf::RenderTarget& rendu, const sf::Font& police, const Inventaire& sac, sf::Vector2f tailleDeVue) {
    assert(sac.capacite() == s_casesDuSac);
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, tailleDeVue);
    sf::Text etiquette = texte(police, "Bag", s_tailleEtiquetteDuSac, sf::Color::White);
    etiquette.setPosition(premiere.position - sf::Vector2f{0.0f, s_hauteurEtiquetteDuSac});
    rendu.draw(etiquette);

    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        const sf::FloatRect zone = rectangleDeLaCaseDuSac(indice, tailleDeVue);
        const std::optional<Objet>& objet = sac.a(indice);
        if (objet.has_value()) {
            dessinerRectangle(rendu, zone, s_couleurCaseOccupee, s_couleurContourOccupe);
            dessinerTexteCentre(rendu, police, sigleDeLObjet(objet->modele), s_tailleSigle, sf::Color::White, zone);
        } else {
            dessinerRectangle(rendu, zone, s_couleurCaseVide, s_couleurContourVide);
        }
    }
}

// Le rectangle d'une position : un emplacement d'un personnage ou une case du sac.
sf::FloatRect rectangleDe(const PositionDObjet& position, sf::Vector2f tailleDeVue) {
    return std::visit(
        Surcharges{
            [&](const PositionPortee& portee) { return rectangleDeLEmplacement(portee.place, portee.emplacement, tailleDeVue); },
            [&](const PositionDansLeSac& dansLeSac) { return rectangleDeLaCaseDuSac(dansLeSac.caseDuSac, tailleDeVue); },
        },
        position);
}

// Un contour doré autour d'une destination valide du glissement.
void dessinerSurbrillance(sf::RenderTarget& rendu, sf::FloatRect zone) {
    sf::RectangleShape forme(zone.size);
    forme.setPosition(zone.position);
    forme.setFillColor(sf::Color::Transparent);
    forme.setOutlineColor(s_couleurSurbrillance);
    forme.setOutlineThickness(s_epaisseurSurbrillance);
    rendu.draw(forme);
}

// Contour sur chaque position, autre que l'origine, où l'objet saisi peut être déposé (emplacements
// des personnages et cases du sac) : la même requête que le relâchement, donc ce qui brille est permis.
void dessinerLesDestinations(sf::RenderTarget& rendu, const Escouade& escouade, const PositionDObjet& origine,
                             sf::Vector2f tailleDeVue) {
    const auto surligner = [&](const PositionDObjet& position) {
        if (position != origine && escouade.peutDeplacerObjet(origine, position)) {
            dessinerSurbrillance(rendu, rectangleDe(position, tailleDeVue));
        }
    };
    for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
        for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
            surligner(PositionPortee{place, static_cast<Emplacement>(i)});
        }
    }
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        surligner(PositionDansLeSac{indice});
    }
}

// L'objet saisi, centré sur le curseur : un carré de la taille d'une case avec le sigle.
void dessinerObjetSaisi(sf::RenderTarget& rendu, const sf::Font& police, const Objet& objet, sf::Vector2f curseur,
                        sf::Vector2f tailleDeVue) {
    const sf::Vector2f taille = rectangleDeLaCaseDuSac(0, tailleDeVue).size;
    const sf::FloatRect zone{curseur - taille / 2.0f, taille};
    dessinerRectangle(rendu, zone, s_couleurCaseOccupee, sf::Color::White);
    dessinerTexteCentre(rendu, police, sigleDeLObjet(objet.modele), s_tailleSigle, sf::Color::White, zone);
}

// L'infobulle d'un objet près du curseur, gardée dans la vue : un cadre sombre, une ligne par texte. Les seuils de
// `manques` (ceux que le personnage visé n'atteint pas) et, si l'objet est `inactif`, une dernière ligne « Inactif :
// seuil non atteint », sont en rouge.
void dessinerInfobulle(sf::RenderTarget& rendu, const sf::Font& police, const Objet& objet, sf::Vector2f curseur,
                       sf::Vector2f tailleDeVue, const std::vector<SeuilDeCaracteristique>& manques, bool inactif) {
    std::vector<sf::Text> lignes;
    float largeur = 0.0f;
    std::vector<std::string> textes = lignesDeLInfobulle(objet.modele);
    if (inactif) {
        textes.push_back(ligneDObjetInactif());
    }
    for (const std::string& ligne : textes) {
        const bool enRouge = ligne == ligneDObjetInactif() ||
                             std::ranges::any_of(manques, [&](const SeuilDeCaracteristique& seuil) { return ligne == ligneDeSeuil(seuil); });
        lignes.push_back(texte(police, ligne, s_tailleInfobulle, enRouge ? s_couleurInactif : sf::Color::White));
        largeur = std::max(largeur, lignes.back().getLocalBounds().size.x);
    }
    const float hauteurDeLigne = police.getLineSpacing(s_tailleInfobulle);
    const sf::Vector2f taille{largeur + 2.0f * s_margeInfobulle, static_cast<float>(lignes.size()) * hauteurDeLigne + 2.0f * s_margeInfobulle};
    const sf::FloatRect zone = placerInfobulle(curseur, taille, tailleDeVue);

    dessinerRectangle(rendu, zone, s_couleurInfobulle, sf::Color::White);
    for (std::size_t i = 0; i < lignes.size(); ++i) {
        lignes[i].setPosition(zone.position + sf::Vector2f{s_margeInfobulle, s_margeInfobulle + static_cast<float>(i) * hauteurDeLigne});
        rendu.draw(lignes[i]);
    }
}

}  // namespace

EcranInventaire::EcranInventaire(Escouade& escouade, std::vector<std::string> nomsDesClasses)
    : m_escouade(escouade), m_nomsDesClasses(std::move(nomsDesClasses)) {
    assert(m_nomsDesClasses.size() == s_nombreDePlaces);
}

Transition EcranInventaire::appuyer(sf::Keyboard::Scancode touche) {
    if (touche == sf::Keyboard::Scan::Escape && m_origine.has_value()) {
        annulerLeGlissement();
        return Aucune{};
    }
    if (touche == sf::Keyboard::Scan::I || touche == sf::Keyboard::Scan::Escape) {
        return Depiler{};
    }
    return Aucune{};
}

void EcranInventaire::saisir(sf::Vector2f point, sf::Vector2f tailleDeVue) {
    const std::optional<PositionDObjet> position = elementSous(point, tailleDeVue);
    if (position.has_value() && m_escouade.objetA(*position).has_value()) {
        m_origine = position;
    }
}

void EcranInventaire::relacher(sf::Vector2f point, sf::Vector2f tailleDeVue) {
    if (!m_origine.has_value()) {
        return;
    }
    const std::optional<PositionDObjet> cible = elementSous(point, tailleDeVue);
    if (cible.has_value()) {
        // Un déplacement refusé est normal (emplacement incompatible) : rien ne bouge, il n'y a rien à dire.
        static_cast<void>(m_escouade.deplacerObjet(*m_origine, *cible));
    }
    m_origine.reset();
}

void EcranInventaire::annulerLeGlissement() {
    m_origine.reset();
}

const std::optional<PositionDObjet>& EcranInventaire::origineDuGlissement() const {
    return m_origine;
}

Transition EcranInventaire::traiterEvenement(const sf::Event& evenement, const Contexte& contexte) {
    if (const auto* data = evenement.getIf<sf::Event::KeyPressed>()) {
        return appuyer(data->scancode);
    }
    if (const std::optional<PointDInterface> clic = clicGaucheDInterface(evenement, contexte)) {
        saisir(clic->point, clic->centre * 2.0f);
    } else if (const std::optional<PointDInterface> relachement = relachementGaucheDInterface(evenement, contexte)) {
        relacher(relachement->point, relachement->centre * 2.0f);
    } else if (evenement.is<sf::Event::FocusLost>()) {
        // Le relâchement hors de la fenêtre n'arrive pas : sans cela l'objet resterait collé au curseur.
        annulerLeGlissement();
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

    sf::Text titre = texte(contexte.police, "Inventory", s_tailleTitre, sf::Color::White);
    const sf::FloatRect limites = titre.getLocalBounds();
    titre.setOrigin({limites.position.x + limites.size.x / 2.0f, limites.position.y});
    titre.setPosition({vue.centre.x, s_margePanneau + s_hauteurTitre});
    fenetre.draw(titre);

    for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
        dessinerColonne(fenetre, contexte.police, m_escouade, place, m_nomsDesClasses[place - 1], tailleVue);
    }
    dessinerLeSac(fenetre, contexte.police, m_escouade.sac(), tailleVue);

    // Dernières, par-dessus tout le reste.
    if (m_origine.has_value()) {
        dessinerLesDestinations(fenetre, m_escouade, *m_origine, tailleVue);
        // L'origine atténuée (l'objet n'a pas bougé : il est « en main »), l'objet saisi sous le curseur.
        const sf::FloatRect origine = rectangleDe(*m_origine, tailleVue);
        sf::RectangleShape voileDOrigine(origine.size);
        voileDOrigine.setPosition(origine.position);
        voileDOrigine.setFillColor(s_couleurAttenuation);
        fenetre.draw(voileDOrigine);
        if (const std::optional<Objet> saisi = m_escouade.objetA(*m_origine)) {
            dessinerObjetSaisi(fenetre, contexte.police, *saisi, vue.point, tailleVue);
            // Sur l'emplacement d'un personnage : les exigences de l'objet saisi, celles que ce personnage n'atteint
            // pas en rouge (étape 92.3).
            const std::optional<PositionDObjet> visee = elementSous(vue.point, tailleVue);
            if (visee.has_value() && std::holds_alternative<PositionPortee>(*visee) && !definitionObjet(saisi->modele).seuils.empty()) {
                dessinerInfobulle(fenetre, contexte.police, *saisi, vue.point, tailleVue, m_escouade.seuilsManques(*m_origine, *visee), false);
            }
        }
    } else if (const std::optional<PositionDObjet> survolee = elementSous(vue.point, tailleVue)) {
        if (const std::optional<Objet> objet = m_escouade.objetA(*survolee)) {
            const PositionPortee* portee = std::get_if<PositionPortee>(&*survolee);
            const bool inactif = portee != nullptr && m_escouade.objetPorteEstInactif(portee->place, portee->emplacement);
            dessinerInfobulle(fenetre, contexte.police, *objet, vue.point, tailleVue, {}, inactif);
        }
    }
}
