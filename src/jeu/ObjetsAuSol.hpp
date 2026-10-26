// ./src/jeu/ObjetsAuSol.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "jeu/ObjetAuSol.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"

class Escouade;
class TileMap;
class Vision;

// Les objets posés au sol d'un étage, dans l'ordre où ils sont tombés. Possédés par la `Partie`, donc par le niveau : une
// nouvelle partie les efface. C'est ici que naît l'identifiant d'un objet au sol (un entier jamais réutilisé dans cet
// ensemble) et que se décide où il tombe ; la carte et la vision sont passées à chaque appel qui en a besoin, jamais gardées
// (une `Partie` se déplace, une référence resterait pendante).
class ObjetsAuSol {
public:
    // Pose cet objet à la place libre la plus proche de `origine` (`positionPour`), avec un identifiant neuf.
    void poser(const Objet& objet, sf::Vector2f origine, const TileMap& carte);
    // Retire l'objet de cet identifiant ; faux, sans rien changer, s'il n'y en a pas.
    [[nodiscard]] bool retirer(int id);
    // L'objet de cet identifiant, nullptr s'il n'y en a pas. Le pointeur ne survit pas à un `poser` ni à un `retirer`.
    [[nodiscard]] const ObjetAuSol* trouver(int id) const;
    // L'objet que ce point (pixels du monde) désigne : le plus proche à moins de `s_rayonDeSurvolDObjet`, nullptr s'il n'y
    // en a pas. Un objet est comme le terrain : sur une case jamais vue (inconnue), il est caché, donc introuvable ; dans
    // une case explorée, même hors de vue, il se désigne.
    [[nodiscard]] const ObjetAuSol* sous(sf::Vector2f point, const TileMap& carte, const Vision& vision) const;
    // Où poser un objet de plus, à partir de `origine` : le centre de la case de `origine` si aucun objet n'y est, sinon celui
    // de la case voisine la plus proche qui est praticable (ni mur ni porte fermée) et sans objet. Les voisines sont
    // essayées par couronnes de cases autour de l'origine (jusqu'à trois cases), les plus proches d'abord ; si toutes sont
    // prises ou bloquées, l'objet se pose sur la case de l'origine, avec celui qui y est. `origine` est un point praticable
    // (un monstre, l'ancre de l'escouade). Aucun hasard : même résultat pour le même état.
    [[nodiscard]] sf::Vector2f positionPour(const TileMap& carte, sf::Vector2f origine) const;
    // Tous les objets, dans l'ordre où ils sont tombés.
    [[nodiscard]] const std::vector<ObjetAuSol>& tous() const;

private:
    std::vector<ObjetAuSol> m_objets;
    int m_prochainId = 1;  // jamais réutilisé
};

// Jette l'objet de cette position (case du sac ou emplacement d'un personnage, `Escouade::retirerObjet`) : il est retiré de
// l'escouade, qui recalcule ce que son retrait change, et posé au sol près de l'ancre avec un identifiant neuf. Un objet n'est
// jamais perdu ni dupliqué : il quitte l'escouade et arrive au sol dans le même appel. Faux, sans rien changer, si la position
// est vide.
[[nodiscard]] bool jeterUnObjet(Escouade& escouade, ObjetsAuSol& sol, const TileMap& carte, const PositionDObjet& position);
