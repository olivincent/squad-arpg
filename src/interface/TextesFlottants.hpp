// ./src/interface/TextesFlottants.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include "combat/EvenementDeCombat.hpp"

// Un texte qui monte et s'efface au-dessus de sa cible. L'événement porte une COPIE de la
// position de la cible : le texte survit à un monstre tué par ce coup.
struct TexteFlottant {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    EvenementDeCombat evenement;
    float age;          // en secondes, depuis son ajout
    int rang;           // 0 pour le premier texte d'une cible dans une image, puis 1, 2… (empilement)
    std::size_t image;  // numéro de l'image où il a été ajouté (voir TextesFlottants::mettreAJour)
};

// Fonctions pures de l'âge : l'animation se calcule, rien d'autre n'est stocké.
// Hauteur au-dessus du centre de la cible, en pixels : 20 au départ, 30 de plus sur toute la durée de vie.
[[nodiscard]] float hauteurDuTexte(float age);
// Opacité de 0 à 1 : pleine pendant la première moitié de la vie, puis linéaire jusqu'à 0.
[[nodiscard]] float opaciteDuTexte(float age);
// Position du centre du texte : sous la hauteur de son âge, relevée de 14 px par rang d'empilement.
[[nodiscard]] sf::Vector2f positionDuTexte(const TexteFlottant& texte);
// Dégâts : « 12 » (suivi de « ! » pour un coup gratuit) ; soin : « +12 » ; l'un et l'autre suivis de
// « Critique ! » pour un critique (« 12! Critique ! ») ; skill lancé : son nom ; provoqué : « Provoqué » ; esquive :
// « Esquivé » ; blocage : « Bloqué » (total) ou « Bloqué −40 » (partiel, signe moins U+2212) (UTF-8).
[[nodiscard]] std::string libelleDuTexte(const EvenementDeCombat& evenement);
// Dégâts critiques jaunes (même pour un coup gratuit), coup gratuit orange, soin vert (critique ou non), dégâts sur un
// monstre blancs, sur un personnage rouges, nom de skill jaune doré, « Provoqué » rouge orangé, « Esquivé » gris clair,
// « Bloqué » gris.
[[nodiscard]] sf::Color couleurDuTexte(const EvenementDeCombat& evenement);
// 18 px pour un critique (dégâts ou soin), 20 px pour un coup gratuit, 14 px pour un nom de skill, « Provoqué »,
// « Esquivé » ou « Bloqué », 16 px sinon.
[[nodiscard]] unsigned int tailleDuTexte(const EvenementDeCombat& evenement);

// Les textes à l'écran. Logique pure (sans fenêtre) ; seul `dessiner` touche à SFML. Possédé par
// EcranJeu, remis à zéro à chaque nouvelle partie. Aucun hasard : l'empilement est déterministe.
class TextesFlottants {
public:
    static constexpr float s_dureeDeVie = 0.8f;

    // Un texte par événement. Les textes ajoutés dans la même image (entre deux `mettreAJour`) pour la
    // même cible (campCible et idCible) s'empilent : le k-ième est relevé de k fois 14 px.
    void ajouter(const EvenementDeCombat& evenement);
    // Fait vieillir tous les textes de `dt`, retire ceux qui ont atteint `s_dureeDeVie`, et ouvre
    // l'image suivante pour l'empilement.
    void mettreAJour(float dt);
    [[nodiscard]] const std::vector<TexteFlottant>& actifs() const;
    // À appeler avec la vue du monde.
    void dessiner(sf::RenderTarget& rendu, const sf::Font& police) const;

private:
    std::vector<TexteFlottant> m_textes;
    std::size_t m_image = 0;
};
