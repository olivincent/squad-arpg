// ./src/interface/Controleur.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include "combat/AdresseSkill.hpp"
#include "interface/ClicInterface.hpp"

class BarreDeRaccourcis;
class Partie;

// L'état d'interface : le mode visée d'un skill de zone et le déplacement
// continu. Traduit les actions du joueur (déjà dépouillées des événements
// SFML par main.cpp) en demandes à la Partie. Tous les points sont en
// pixels du monde. Type valeur : la Partie est reçue à chaque appel, sans
// référence membre, pour que le Controleur reste réaffectable.
class Controleur {
public:
    // À appeler au début de chaque image : sort de la visée si le lanceur
    // ne peut plus lancer (mort, défaite).
    void debutImage(const Partie& partie);
    // Clic gauche : en visée, lance le sort au point ; sinon engage un
    // combat (monstre) ou arme le déplacement continu (sol).
    void appuiGauche(Partie& partie, sf::Vector2f point);
    // Clic gauche tombé sur l'interface (règle « l'interface d'abord » : il n'atteint jamais
    // appuiGauche, donc ni verrou, ni porte, ni déplacement, ni point d'impact de visée). Désarme le
    // déplacement continu, puis active l'emplacement cliqué comme le ferait sa touche.
    void appuiInterface(Partie& partie, const BarreDeRaccourcis& barre, ClicInterface clic);
    // Clic droit : annule la visée.
    void appuiDroit();
    // Active l'emplacement `emplacement` de la barre (touche ou clic : même action). Vide : rien.
    // Skill de mode `Point` : entre en visée de cette adresse (escouade vivante, skill prêt ; remplace
    // la visée d'un autre skill à point) ou en sort si cette adresse est déjà en visée. Autre mode :
    // demandé à la partie, la visée en cours continue. `assert(emplacement < barre.taille())`.
    void activerEmplacement(Partie& partie, const BarreDeRaccourcis& barre, std::size_t emplacement);
    void appuiEchap();
    // Annule la visée et désarme le déplacement continu : à l'ouverture d'un écran par-dessus la partie,
    // dont le relâchement du bouton ne parviendrait pas jusqu'ici.
    void interrompre();
    // `boutonTenu` : bouton gauche enfoncé ET fenêtre avec le focus.
    void mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur);

    // Cercle de zone sous le curseur et cercle de portée du lanceur ; rien hors visée.
    void dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const;
    [[nodiscard]] std::optional<AdresseSkill> getAdresseEnVisee() const;

private:
    bool m_deplacementContinu = false;
    // Vide = mode normal, sinon le skill de zone en cours de visée.
    std::optional<AdresseSkill> m_adresseEnVisee;
};
