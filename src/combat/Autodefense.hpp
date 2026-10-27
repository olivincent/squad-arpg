// ./src/combat/Autodefense.hpp
#pragma once
#include <functional>
#include <optional>
#include <type_traits>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"

// Entité que frappe celui qui est en `origine` : l'entité préférée
// (verrouillée, ou proie) si elle est à sa portée, sinon la plus proche
// (bord à bord) parmi celles à sa portée (égalité : la première de la
// liste), sinon aucune. `estAPortee` dit si un cercle est à portée (celle de
// l'attaque ou celle d'un skill). Même règle pour les deux camps (d'où le gabarit : des monstres pour
// l'escouade, des personnages pour un monstre). Renvoie l'identifiant de l'entité.
template <class Identifiant>
[[nodiscard]] std::optional<Identifiant> choisirCibleAFrapper(Cercle origine,
                                                              const std::function<bool(Cercle)>& estAPortee,
                                                              const std::vector<EntiteVue<Identifiant>>& entites,
                                                              std::type_identity_t<std::optional<Identifiant>> idPrefere) {
    std::optional<Identifiant> meilleur;
    float meilleurEcart = 0.0f;

    for (const EntiteVue<Identifiant>& entite : entites) {
        if (!estAPortee(entite.cercle)) {
            continue;
        }
        if (idPrefere && entite.id == *idPrefere) {
            return entite.id;
        }
        const float ecart = distanceBordABord(origine, entite.cercle);
        if (!meilleur || ecart < meilleurEcart) {
            meilleur = entite.id;
            meilleurEcart = ecart;
        }
    }
    return meilleur;
}

// Agresseur vers lequel va un combattant (en `personnage`) dont la place en
// formation est `place` : le plus proche de lui (bord à bord ; égalité : le
// premier de la liste) parmi ceux dont le centre est à `laisse` ou moins de
// `place`, sinon aucun. Renvoie l'agresseur (identifiant et cercle).
[[nodiscard]] std::optional<MonstreVu> choisirAgresseurAViser(Cercle personnage,
                                                              sf::Vector2f place,
                                                              float laisse,
                                                              const std::vector<MonstreVu>& agresseurs);
