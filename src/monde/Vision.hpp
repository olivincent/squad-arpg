// ./src/monde/Vision.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "base/Cercle.hpp"

class TileMap;

enum class EtatCase { Inconnue, Exploree, Visible };

// Ce que le joueur sait de la carte : chaque case est inconnue, explorée (vue un jour)
// ou visible (vue en ce moment). La TileMap reste la vérité du monde ; la Vision n'en
// garde que la connaissance du joueur. Ne connaît ni Personnage, ni Escouade, ni Monstre :
// elle reçoit des sources de vision (centre et rayon, en pixels du monde).
class Vision {
public:
    // Dimensions copiées de la carte ; tout est inconnu.
    explicit Vision(const TileMap& carte);

    // Les cases visibles deviennent explorées, puis le champ de chaque source (centre et
    // rayon en pixels ; rayon en cases = rayon / taille de case, arrondi vers le bas) devient
    // visible. Une case explorée ne redevient jamais inconnue. `carte` a les dimensions de
    // celle de la construction.
    void mettreAJour(const TileMap& carte, const std::vector<Cercle>& sources);

    // Hors de la carte : Inconnue.
    [[nodiscard]] EtatCase etat(sf::Vector2i coordCase) const;
    // La case qui contient ce point (pixels du monde) est-elle visible ?
    [[nodiscard]] bool estVisible(sf::Vector2f point) const;

    // Voile noir opaque sur les cases inconnues, noir semi-transparent sur les explorées,
    // rien sur les visibles. À dessiner avec la vue du monde, après la carte et les entités.
    void dessiner(sf::RenderTarget& rendu) const;

private:
    int m_largeur;
    int m_hauteur;
    float m_tailleCase;
    std::vector<EtatCase> m_cases;  // largeur * hauteur cases, indice = y * largeur + x (comme TileMap)
};
