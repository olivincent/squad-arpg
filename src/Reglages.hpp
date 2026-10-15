// ./src/Reglages.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <iosfwd>
#include <vector>

// Mode d'affichage de la fenêtre. Le plein écran utilise toujours la
// résolution du bureau ; la résolution des réglages ne sert qu'en mode fenêtré.
enum class ModeAffichage { Fenetre, PleinEcran };

// Une résolution de fenêtre, en pixels.
struct Resolution {
    unsigned largeur;
    unsigned hauteur;

    bool operator==(const Resolution&) const = default;
};

// Les réglages d'affichage, sauvegardés dans `reglages.ini`.
struct Reglages {
    unsigned largeur = 1200;
    unsigned hauteur = 800;
    ModeAffichage mode = ModeAffichage::Fenetre;
};

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
