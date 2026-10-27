// ./src/combat/EntiteVue.hpp
#pragma once
#include <algorithm>
#include <vector>
#include "base/Cercle.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/Place.hpp"

// Une entité telle que l'autre camp la voit : son identifiant et sa forme. L'identifiant est celui d'un monstre
// (`MonstreVu`) ou la place d'un personnage (`PersonnageVu`) : un seul gabarit parce que les règles qui lisent ces
// vues (`trouverEntite`, `choisirCibleAFrapper`) sont les mêmes pour les deux camps, deux types parce qu'on ne
// confond plus un monstre et un personnage.
template <class Identifiant>
struct EntiteVue {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    Identifiant id;
    Cercle cercle;
};
using MonstreVu = EntiteVue<IdMonstre>;
using PersonnageVu = EntiteVue<Place>;

// L'entité portant cet identifiant, nullptr si elle n'est pas dans la liste.
// Le pointeur reste valide tant que la liste n'est pas modifiée.
template <class Identifiant>
[[nodiscard]] const EntiteVue<Identifiant>* trouverEntite(const std::vector<EntiteVue<Identifiant>>& entites, Identifiant id) {
    const auto it = std::ranges::find(entites, id, &EntiteVue<Identifiant>::id);
    return it == entites.end() ? nullptr : &*it;
}
