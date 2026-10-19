// ./src/interface/EcranOptions.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <vector>

#include "interface/Ecran.hpp"
#include "interface/ListeDeBoutons.hpp"
#include "interface/Reglages.hpp"

// L'écran des options d'affichage : le mode (un clic bascule), la résolution
// en mode fenêtré (flèches gauche et droite) et « Retour ». Chaque changement
// s'applique tout de suite : l'écran modifie sa copie des réglages et la
// renvoie dans `AppliquerReglages`.
class EcranOptions : public Ecran {
public:
    // `reglages` : les réglages en cours ; `resolutions` : celles que le
    // bureau permet (`resolutionsDisponibles`), dans l'ordre croissant.
    EcranOptions(const Reglages& reglages, std::vector<Resolution> resolutions);

    // Un clic gauche en `point` (coordonnées de la vue de l'interface), les
    // boutons étant centrés sur `centre`. Mode : bascule. Résolution : le tiers
    // gauche de la ligne passe à la résolution précédente, le tiers droit à la
    // suivante (rien aux deux bouts de la liste) ; ligne inactive en plein
    // écran. Retour : `Depiler`. Un clic ailleurs : rien.
    [[nodiscard]] Transition cliquer(sf::Vector2f point, sf::Vector2f centre);

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;

private:
    // Les libellés et l'état actif de chaque ligne, selon les réglages.
    [[nodiscard]] ListeDeBoutons creerBoutons() const;

    // Passe à la résolution voisine (`decalage` = -1 ou +1) ; `Aucune` au bout de la liste.
    [[nodiscard]] Transition changerResolution(int decalage);

    Reglages m_reglages;
    std::vector<Resolution> m_resolutions;
    std::size_t m_indiceResolution;  // position de la résolution choisie dans m_resolutions
    ListeDeBoutons m_boutons;
};
