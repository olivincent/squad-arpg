// ./src/escouade/Formation.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "base/Cercle.hpp"

class TileMap;

// La formation de l'escouade : un décalage par personnage autour du centre de l'ancre, une
// orientation qui tourne progressivement vers l'orientation voulue, et la place de chaque
// personnage (théorique, puis repliée près des murs, écartée des autres places et sortie des
// monstres). Ne connaît ni Personnage ni Escouade : elle reçoit le centre de l'ancre, sa direction
// de marche, les rayons des personnages, la carte et les cercles des monstres.
class Formation {
public:
    // `decalages` : un par personnage, pour une formation orientée vers +x. Orientation de départ 0 ;
    // places de départ : les places théoriques autour de `centre`, sans repli.
    // `vitesseRotation` en degrés par seconde.
    Formation(std::vector<sf::Vector2f> decalages, sf::Vector2f centre, float vitesseRotation);
    // Une fois l'ancre arrêtée, la formation se tourne vers ce point (voir mettreAJour).
    void regarderVers(sf::Vector2f point);
    // Efface le point à regarder (un nouvel ordre à l'ancre).
    void oublierPointARegarder();
    // L'orientation voulue devient la direction de marche de l'ancre ; à l'arrêt (vide), la direction
    // du point à regarder s'il y en a un et qu'il n'est pas sur `centre` ; sinon elle ne change pas.
    // L'orientation tourne ensuite vers l'orientation voulue (par le plus court côté, à vitesse
    // bornée), puis les places sont calculées autour de `centre`.
    // `rayons` : un par personnage, dans l'ordre des décalages.
    void mettreAJour(float dt, sf::Vector2f centre, std::optional<sf::Vector2f> directionDeMarche,
                     const std::vector<float>& rayons, const TileMap& carte, const std::vector<Cercle>& monstres);
    // Place de chaque personnage, dans l'ordre des décalages, calculée par le dernier mettreAJour.
    [[nodiscard]] const std::vector<sf::Vector2f>& getPlaces() const;

private:
    [[nodiscard]] sf::Vector2f placeTheorique(std::size_t indice, sf::Vector2f centre) const;
    void choisirOrientation(std::optional<sf::Vector2f> directionDeMarche, sf::Vector2f centre);
    void tourner(float dt);
    void calculerPlaces(sf::Vector2f centre, const std::vector<float>& rayons, const TileMap& carte,
                        const std::vector<Cercle>& monstres);

    std::vector<sf::Vector2f> m_decalages;        // pour une formation orientée vers +x
    sf::Angle m_orientation;                      // 0 = orientée vers +x
    sf::Angle m_orientationVoulue;                // vers laquelle m_orientation tourne
    std::optional<sf::Vector2f> m_pointARegarder; // orientation voulue à l'arrêt (voir regarderVers)
    float m_vitesseRotation;                      // en degrés par seconde
    std::vector<sf::Vector2f> m_places;           // place de chaque personnage, repliée si besoin
};
