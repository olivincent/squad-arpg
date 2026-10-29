// ./src/interface/EcranInventaire.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "combat/Caracteristiques.hpp"
#include "combat/Place.hpp"
#include "escouade/TailleDeLEscouade.hpp"
#include "interface/DispositionInventaire.hpp"
#include "interface/Ecran.hpp"
#include "objets/PositionDObjet.hpp"

class Escouade;
class ObjetsAuSol;
class TileMap;

// L'inventaire, ouvert par-dessus la partie (touche I) : l'équipement des personnages et le sac
// de l'escouade, où l'on déplace les objets par glisser-déposer. Il laisse voir l'écran de jeu
// dessous, figé : seul l'écran du dessus est mis à jour. I ferme l'écran ; Échap annule un
// glissement en cours, sinon ferme l'écran.
//
// Le glissement ne touche pas au modèle : on retient seulement la position d'origine, et tout se joue
// au relâchement, par `Escouade::deplacerObjet`, ou `jeterUnObjet` (`jeu/ObjetsAuSol`) si l'objet est lâché hors du panneau de
// l'inventaire. Les points sont en coordonnées de la vue d'interface.
//
// Les points libres (étape 100) : chaque personnage qui en a voit « Points: N » et, à côté de chacune de ses caractéristiques,
// un « + » (grisé quand N vaut 0) et, si elle a des points en attente, un « − ». Ces points en attente sont un BROUILLON,
// un état de l'écran : l'`Escouade` n'en sait rien jusqu'à « Apply », qui les dépense pour de bon
// (`Escouade::depenserDesPoints`, définitif) ; « Reset » et la fermeture de l'écran les effacent sans rien appliquer.
// Le brouillon ne touche que la ligne de la caractéristique : points de vie, capacités et armure changent à « Apply ».
class EcranInventaire : public Ecran {
public:
    // Ce que l'écran utilise, et rien d'autre de la partie : `escouade`, dont il lit et déplace les objets, `sol`, où il jette
    // ceux qu'on lâche hors du panneau, et `carte`, qui dit où ils tombent près de l'ancre. Ils appartiennent à la `Partie` de
    // l'écran de jeu, qui reste sous celui-ci dans la pile et leur survit donc (invariant de la pile, `docs/ecrans.md`).
    // `nomsDesClasses` : le nom de la classe de chaque place, dans l'ordre des places (`s_tailleDeLEscouade` noms).
    EcranInventaire(Escouade& escouade, ObjetsAuSol& sol, const TileMap& carte, std::vector<std::string> nomsDesClasses);

    // Une touche pressée : I dépile l'écran ; Échap annule le glissement en cours, ou dépile s'il n'y en
    // a pas ; les autres ne font rien. Fermer l'écran efface le brouillon des points libres, sans rien appliquer. Sépare la logique de l'événement SFML, comme `cliquer` pour les menus.
    [[nodiscard]] Transition appuyer(sf::Keyboard::Scancode touche);

    // Appui du bouton gauche en `point` : sur un bouton des points libres, le fait agir (et ne commence aucun glissement) ;
    // sinon commence un glissement si un objet s'y trouve, sinon rien.
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

    // Le brouillon des points libres du personnage de cette place (de 1 à `s_tailleDeLEscouade`, `assert`) : ce qui est en attente
    // dans chaque caractéristique, et ce qui reste à répartir (ses points libres moins ce qui est en attente).
    [[nodiscard]] const Caracteristiques& pointsEnAttente(Place place) const;
    [[nodiscard]] int pointsRestants(Place place) const;
    // Met un point de plus en attente dans cette caractéristique ; sans effet s'il ne reste aucun point à répartir.
    void ajouterUnPoint(Place place, Caracteristique caracteristique);
    // En retire un : seulement un point en attente, jamais un point déjà dépensé ; sans effet s'il n'y en a pas.
    void retirerUnPoint(Place place, Caracteristique caracteristique);
    // Dépense pour de bon ce qui est en attente chez ce personnage (`Escouade::depenserDesPoints`) puis vide son brouillon ;
    // sans effet s'il est vide.
    void appliquer(Place place);
    // Efface le brouillon de ce personnage sans rien appliquer.
    void reinitialiser(Place place);

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;
    [[nodiscard]] bool laisseVoirDessous() const override;

private:
    Escouade& m_escouade;
    ObjetsAuSol& m_sol;
    const TileMap& m_carte;
    std::vector<std::string> m_nomsDesClasses;
    std::optional<PositionDObjet> m_origine;
    std::array<Caracteristiques, s_tailleDeLEscouade> m_enAttente{};  // le brouillon, par place (dans l'ordre des places)
    // Le bouton qu'on vient de presser agit.
    void cliquerSurUnBouton(const BoutonDePoints& bouton);
};
