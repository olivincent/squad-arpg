// ./src/escouade/Separation.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

// Ce que la séparation douce sait d'un personnage. Une énumération plutôt que deux booléens
// (« pousse », « poussable ») : l'état « mort mais poussable » n'est pas représentable.
enum class EtatDeSeparation {
    Absent,  // mort : il ne pousse pas et n'est pas poussé
    Fixe,    // coup en exécution ou skill en activation : il pousse mais n'est pas poussé
    Libre,   // il pousse et peut être poussé
};

// Un personnage tel que la séparation le voit : son centre et son état (par défaut absent : un
// membre dont on n'a pas donné l'état ne pousse personne et n'est pas poussé).
struct MembreASeparer {
    sf::Vector2f centre;
    EtatDeSeparation etat = EtatDeSeparation::Absent;
};

// Séparation douce : chaque paire de membres présents dont les centres sont à moins de
// `ecartMinimal` est écartée le long de la droite de leurs centres, chacun de la moitié du manque ;
// un membre fixe n'est pas poussé et l'autre prend tout le manque (deux fixes : rien). Centres
// confondus : axe des x, le plus petit indice vers la gauche. Toutes les paires sont jugées sur
// les centres reçus (calcul simultané), et les poussées d'un membre s'additionnent, puis sa
// poussée totale est plafonnée à `vitesseMax` × `dt`. Une poussée par membre, dans le même
// ordre (nulle pour un absent ou un fixe). Pure : l'appelant applique les poussées.
[[nodiscard]] std::vector<sf::Vector2f> pousseesDeSeparation(const std::vector<MembreASeparer>& membres, float ecartMinimal,
                                                             float vitesseMax, float dt);
