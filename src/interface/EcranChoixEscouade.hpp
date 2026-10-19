// ./src/interface/EcranChoixEscouade.hpp
#pragma once

#include <SFML/System/Vector2.hpp>

#include "interface/Ecran.hpp"
#include "interface/ListeDeBoutons.hpp"
#include "escouade/SelectionDEscouade.hpp"

// Centres des trois colonnes de l'écran, pour une vue d'interface centrée sur `centre` :
// les classes à gauche ; à droite, les cases de sélection, puis « Commencer » et « Retour ».
// Le dessin, le clic et les tests s'en servent tous.
[[nodiscard]] sf::Vector2f centreDesClasses(sf::Vector2f centre);
[[nodiscard]] sf::Vector2f centreDesCases(sf::Vector2f centre);
[[nodiscard]] sf::Vector2f centreDesActions(sf::Vector2f centre);

// Le choix des classes de l'escouade, entre le menu et la partie : un bouton par classe du
// catalogue (un clic l'ajoute à la sélection), trois cases de sélection (un clic sur une case
// remplie la retire), « Commencer » (actif quand la sélection est complète) et « Retour ».
// L'écran possède la sélection : elle survit à une partie, qui est empilée par-dessus.
class EcranChoixEscouade : public Ecran {
public:
    EcranChoixEscouade();

    // Un clic gauche en `point` (coordonnées de la vue de l'interface), la vue étant centrée sur
    // `centre`. Classe : l'ajoute à la sélection (sans effet si elle est complète). Case remplie :
    // la retire. Commencer : `Empiler` l'écran de jeu avec la sélection. Retour : `Depiler`. Un clic
    // ailleurs, ou sur une case vide : rien.
    [[nodiscard]] Transition cliquer(sf::Vector2f point, sf::Vector2f centre);

    [[nodiscard]] const SelectionDEscouade& getSelection() const;

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;

private:
    // « Commencer » n'est actif que si la sélection est complète.
    void actualiserBoutons();

    SelectionDEscouade m_selection;
    ListeDeBoutons m_classes;  // un bouton par entrée du catalogue, dans son ordre
    ListeDeBoutons m_actions;  // « Commencer », « Retour »
};
