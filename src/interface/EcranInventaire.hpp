// ./src/interface/EcranInventaire.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <string>
#include <vector>

#include "interface/Ecran.hpp"
#include "objets/PositionDObjet.hpp"

class Escouade;
class Partie;

// L'inventaire, ouvert par-dessus la partie (touche I) : l'équipement des personnages et le sac
// de l'escouade, où l'on déplace les objets par glisser-déposer. Il laisse voir l'écran de jeu
// dessous, figé : seul l'écran du dessus est mis à jour. I ferme l'écran ; Échap annule un
// glissement en cours, sinon ferme l'écran.
//
// Le glissement ne touche pas au modèle : on retient seulement la position d'origine, et tout se joue
// au relâchement, par `Escouade::deplacerObjet`, ou `Partie::jeterUnObjet` si l'objet est lâché hors du panneau de
// l'inventaire. Les points sont en coordonnées de la vue d'interface.
class EcranInventaire : public Ecran {
public:
    // `partie` : celle de l'écran de jeu, qui reste sous celui-ci dans la pile et lui survit
    // donc (invariant de la pile, `docs/ecrans.md`) ; l'écran lit et déplace les objets de son escouade, et jette ceux
    // qu'on lâche hors du panneau sur son sol. `nomsDesClasses` : le nom de la classe de
    // chaque place, dans l'ordre des places (`s_nombreDePlaces` noms).
    EcranInventaire(Partie& partie, std::vector<std::string> nomsDesClasses);

    // Une touche pressée : I dépile l'écran ; Échap annule le glissement en cours, ou dépile s'il n'y en
    // a pas ; les autres ne font rien. Sépare la logique de l'événement SFML, comme `cliquer` pour les menus.
    [[nodiscard]] Transition appuyer(sf::Keyboard::Scancode touche);

    // Appui du bouton gauche en `point` : commence un glissement si un objet s'y trouve, sinon rien.
    void saisir(sf::Vector2f point, sf::Vector2f tailleDeVue);
    // Relâchement du bouton gauche en `point` : tente de déplacer l'objet saisi vers ce qu'il y a sous
    // le point (rien ne bouge si le déplacement est refusé ou si le point est dans un écart du panneau) ; hors du panneau
    // (`rectangleDuPanneau`), l'objet est jeté au sol près de l'ancre, qu'il vienne du sac ou d'un emplacement porté. Le
    // glissement se termine dans tous les cas ; sans glissement, ne fait rien.
    void relacher(sf::Vector2f point, sf::Vector2f tailleDeVue);
    // Abandonne le glissement sans rien déplacer (Échap, perte du focus).
    void annulerLeGlissement();
    // L'origine du glissement en cours, s'il y en a un.
    [[nodiscard]] const std::optional<PositionDObjet>& origineDuGlissement() const;

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;
    [[nodiscard]] bool laisseVoirDessous() const override;

private:
    Partie& m_partie;
    Escouade& m_escouade;  // celle de m_partie
    std::vector<std::string> m_nomsDesClasses;
    std::optional<PositionDObjet> m_origine;
};
