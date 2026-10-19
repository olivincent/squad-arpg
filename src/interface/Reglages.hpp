// ./src/interface/Reglages.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <cstddef>
#include <iosfwd>
#include <optional>
#include <vector>

#include "interface/EmplacementsPrincipaux.hpp"

// Mode d'affichage de la fenêtre. Le plein écran utilise toujours la
// résolution du bureau ; la résolution des réglages ne sert qu'en mode fenêtré.
enum class ModeAffichage { Fenetre, PleinEcran };

// Une résolution de fenêtre, en pixels.
struct Resolution {
    unsigned largeur;
    unsigned hauteur;

    bool operator==(const Resolution&) const = default;
};

// La touche (physique : scancode) de chaque emplacement de la barre principale ; l'indice est
// l'emplacement.
using TouchesDesRaccourcis = std::array<sf::Keyboard::Scancode, s_emplacementsPrincipaux>;

// Les réglages. L'affichage est sauvegardé dans `reglages.ini` ; les touches des raccourcis ne
// sont pas encore lues ni écrites (elles le seront avec l'écran de configuration des touches).
struct Reglages {
    unsigned largeur = 1200;
    unsigned hauteur = 800;
    ModeAffichage mode = ModeAffichage::Fenetre;
    // Par défaut la grille Q W E R / A S D F / Z X C V, comme la disposition à l'écran.
    TouchesDesRaccourcis touchesDesRaccourcis = {
        sf::Keyboard::Scan::Q, sf::Keyboard::Scan::W, sf::Keyboard::Scan::E, sf::Keyboard::Scan::R,
        sf::Keyboard::Scan::A, sf::Keyboard::Scan::S, sf::Keyboard::Scan::D, sf::Keyboard::Scan::F,
        sf::Keyboard::Scan::Z, sf::Keyboard::Scan::X, sf::Keyboard::Scan::C, sf::Keyboard::Scan::V,
    };
};

// L'emplacement de cette touche dans la table, vide si la table ne l'utilise pas.
[[nodiscard]] std::optional<std::size_t> emplacementPourTouche(const TouchesDesRaccourcis& touches, sf::Keyboard::Scancode scancode);

// Les résolutions proposées en mode fenêtré, dans l'ordre croissant
// (1200 × 800 est la résolution par défaut).
[[nodiscard]] const std::vector<Resolution>& resolutionsProposees();

// Celles des résolutions proposées qui tiennent dans le bureau.
[[nodiscard]] std::vector<Resolution> resolutionsDisponibles(sf::Vector2u tailleBureau);

// Lit des réglages au format `cle=valeur` (une clé par ligne : `largeur`,
// `hauteur`, `mode` = `fenetre` ou `plein_ecran`). Ne lève jamais d'exception :
// clé inconnue ou ligne mal formée ignorée, valeur invalide = valeur par
// défaut, résolution hors de la liste proposée = 1200 × 800.
[[nodiscard]] Reglages lireReglages(std::istream& flux);

// Écrit les réglages dans le format lu par `lireReglages`.
void ecrireReglages(std::ostream& flux, const Reglages& reglages);
