// ./src/objets/DeplacementDObjets.cpp
#include "objets/DeplacementDObjets.hpp"
#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"

namespace {
// `std::visit` sans `Surcharges` (qui est dans `combat/`, et `objets/` ne dépend que de `base/` et de ce qu'il lui
// faut de `combat/`) : un `if constexpr` sur le type de la position.
Equipement& equipementDe(EtatDesObjets& etat, const PositionPortee& portee) {
    assert(portee.place >= 1 && portee.place <= etat.equipements.size());
    return etat.equipements[portee.place - 1];
}

const Equipement& equipementDe(const EtatDesObjets& etat, const PositionPortee& portee) {
    assert(portee.place >= 1 && portee.place <= etat.equipements.size());
    return etat.equipements[portee.place - 1];
}

std::optional<Objet> objetA(const EtatDesObjets& etat, const PositionDObjet& position) {
    return std::visit(
        [&](const auto& precise) -> std::optional<Objet> {
            if constexpr (std::is_same_v<std::decay_t<decltype(precise)>, PositionDansLeSac>) {
                return etat.sac.a(precise.caseDuSac);
            } else {
                return equipementDe(etat, precise).a(precise.emplacement);
            }
        },
        position);
}

// Vide cette position et rend son objet.
std::optional<Objet> retirerA(EtatDesObjets& etat, const PositionDObjet& position) {
    return std::visit(
        [&](const auto& precise) -> std::optional<Objet> {
            if constexpr (std::is_same_v<std::decay_t<decltype(precise)>, PositionDansLeSac>) {
                return etat.sac.prendre(precise.caseDuSac);
            } else {
                return equipementDe(etat, precise).retirer(precise.emplacement);
            }
        },
        position);
}

// Pose l'objet sur une position qu'on vient de vider (`retirerA`).
void poserA(EtatDesObjets& etat, const PositionDObjet& position, Objet objet) {
    [[maybe_unused]] const std::optional<Objet> ancien = std::visit(
        [&](const auto& precise) -> std::optional<Objet> {
            if constexpr (std::is_same_v<std::decay_t<decltype(precise)>, PositionDansLeSac>) {
                return etat.sac.poser(precise.caseDuSac, objet);
            } else {
                return equipementDe(etat, precise).poser(precise.emplacement, objet);
            }
        },
        position);
    assert(!ancien.has_value());
}

// La règle des deux mains pour l'objet qui vient d'être posé à cette position : s'il ne laisse pas de place à l'objet de
// l'autre main, celui-ci va dans la première case libre du sac. Faux, sans rien ranger, si le sac est plein.
bool libererLaMainOpposee(EtatDesObjets& etat, const PositionDObjet& position) {
    const PositionPortee* portee = std::get_if<PositionPortee>(&position);
    if (portee == nullptr) {
        return true;
    }
    Equipement& equipement = equipementDe(etat, *portee);
    const std::optional<Objet> pose = equipement.a(portee->emplacement);
    if (!pose.has_value()) {
        return true;
    }
    // Les deux mains : une arme à deux mains en main droite chasse la main gauche, un bouclier en main gauche chasse
    // une arme à deux mains.
    const std::optional<Objet> droite = equipement.a(Emplacement::MainDroite);
    std::optional<Emplacement> aRanger;
    if (portee->emplacement == Emplacement::MainDroite && estADeuxMains(pose->id) &&
        equipement.a(Emplacement::MainGauche).has_value()) {
        aRanger = Emplacement::MainGauche;
    } else if (portee->emplacement == Emplacement::MainGauche && droite.has_value() && estADeuxMains(droite->id)) {
        aRanger = Emplacement::MainDroite;
    }
    if (!aRanger.has_value()) {
        return true;
    }
    const std::optional<Objet> chasse = equipement.retirer(*aRanger);
    assert(chasse.has_value());
    return etat.sac.ajouter(*chasse);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
}

// Le personnage qui reçoit l'objet posé à cette position atteint-il ses seuils, sans compter cet objet ? Toujours
// vrai vers une case du sac.
bool seuilsRespectes(const EtatDesObjets& etat, const PositionDObjet& position) {
    const PositionPortee* portee = std::get_if<PositionPortee>(&position);
    if (portee == nullptr) {
        return true;
    }
    Equipement sansLui = equipementDe(etat, *portee);
    const std::optional<Objet> pose = sansLui.retirer(portee->emplacement);
    if (!pose.has_value()) {
        return true;
    }
    assert(portee->place <= etat.bases.size());
    return seuilsAtteints(pose->id, caracteristiquesEffectives(etat.bases[portee->place - 1], sansLui));
}
}  // namespace

bool deplacerLesObjets(EtatDesObjets& etat, const PositionDObjet& de, const PositionDObjet& vers) {
    const std::optional<Objet> objetDe = objetA(etat, de);
    const std::optional<Objet> objetVers = objetA(etat, vers);
    if (!deplacementPermis(de, objetDe, vers, objetVers)) {
        return false;
    }
    if (de == vers) {
        return true;
    }
    // Sur une copie : un sac plein au moment de ranger une main refuse tout, état intact.
    EtatDesObjets essai = etat;
    // En croix : les deux objets sont sortis avant d'être posés, donc aucun n'existe à deux endroits.
    [[maybe_unused]] const std::optional<Objet> sortiDe = retirerA(essai, de);
    [[maybe_unused]] const std::optional<Objet> sortiVers = retirerA(essai, vers);
    assert(sortiDe.has_value());
    poserA(essai, vers, *sortiDe);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (la règle de base a refusé les sources vides)
    if (sortiVers.has_value()) {
        poserA(essai, de, *sortiVers);
    }
    if (!libererLaMainOpposee(essai, vers) || (sortiVers.has_value() && !libererLaMainOpposee(essai, de))) {
        return false;
    }
    if (!seuilsRespectes(essai, vers) || (sortiVers.has_value() && !seuilsRespectes(essai, de))) {
        return false;
    }
    etat = std::move(essai);
    return true;
}

bool deplacementPossible(const EtatDesObjets& etat, const PositionDObjet& de, const PositionDObjet& vers) {
    EtatDesObjets copie = etat;
    return deplacerLesObjets(copie, de, vers);
}
