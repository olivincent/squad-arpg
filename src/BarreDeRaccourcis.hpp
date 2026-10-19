// ./src/BarreDeRaccourcis.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "AdresseSkill.hpp"

// Nombre d'emplacements de la barre principale (une rangée de 4 par personnage, 3 rangées).
inline constexpr std::size_t s_emplacementsPrincipaux = 12;

// Une barre de raccourcis : des emplacements numérotés, chacun vide ou pointant vers un skill
// d'un personnage (une AdresseSkill). Pur modèle : ni touche, ni position à l'écran (voir
// emplacementPourTouche et rectangleDeLEmplacement). L'emplacement i reste l'emplacement i
// quelles que soient sa touche et sa place à l'écran. Aucune limite par personnage ; une même
// adresse peut être placée sur plusieurs emplacements.
class BarreDeRaccourcis {
public:
    explicit BarreDeRaccourcis(std::size_t taille);

    [[nodiscard]] std::size_t taille() const;
    // Vide si l'emplacement est vide ; `assert(emplacement < taille())`.
    [[nodiscard]] std::optional<AdresseSkill> adresse(std::size_t emplacement) const;
    // Remplace le contenu de l'emplacement ; `assert(emplacement < taille())`.
    void placer(std::size_t emplacement, AdresseSkill adresse);
    // Vide l'emplacement ; `assert(emplacement < taille())`.
    void vider(std::size_t emplacement);

private:
    std::vector<std::optional<AdresseSkill>> m_emplacements;
};

// Disposition par défaut de la barre principale (`s_emplacementsPrincipaux` emplacements) : une
// rangée de 4 emplacements par personnage, dans l'ordre des places, ses skills dans l'ordre
// reçu (rangée de la place 1 : emplacements 0 à 3, place 2 : 4 à 7…). `adresses` est ordonnée
// par place puis par liste du personnage (Escouade::adressesDesSkills). Un skill au-delà du
// quatrième d'un personnage, ou d'une place sans rangée, n'est pas placé.
[[nodiscard]] BarreDeRaccourcis barreParDefaut(const std::vector<AdresseSkill>& adresses);

// Disposition à l'écran de la barre principale, calculée à partir de la taille de la vue
// d'interface seulement (le modèle ne sait rien de sa place) : une grille de 4 colonnes sur
// 3 rangées, icônes de 48 px espacées de 4 px, centrée en bas de la vue avec 10 px de marge.
// L'emplacement 0 est en haut à gauche, les rangées se lisent de haut en bas (la rangée du
// haut est celle de la place 1). `assert(emplacement < s_emplacementsPrincipaux)`.
[[nodiscard]] sf::FloatRect rectangleDeLEmplacement(std::size_t emplacement, sf::Vector2f tailleVue);

// Emplacement dont l'icône contient ce point (en coordonnées de la vue d'interface) ; vide
// dans un écart entre icônes ou hors de la barre. Construite sur rectangleDeLEmplacement :
// une seule source de vérité pour le dessin et pour le clic.
[[nodiscard]] std::optional<std::size_t> emplacementSous(sf::Vector2f point, sf::Vector2f tailleVue);
