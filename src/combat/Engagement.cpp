// ./src/combat/Engagement.cpp
#include "combat/Engagement.hpp"
#include <SFML/System/Vector2.hpp>
#include "base/Cercle.hpp"

namespace {
// L'entité de cet identifiant, nullptr si elle est absente.
template <class Identifiant>
const EntiteDeControle<Identifiant>* trouver(const std::vector<EntiteDeControle<Identifiant>>& entites, Identifiant id) {
    for (const EntiteDeControle<Identifiant>& entite : entites) {
        if (entite.id == id) {
            return &entite;
        }
    }
    return nullptr;
}

template <class IdA, class IdC>
bool engageQuelquUn(const EntiteDeControle<IdA>& attaquant, const std::vector<EntiteDeControle<IdC>>& cibles) {
    for (const EntiteDeControle<IdC>& cible : cibles) {
        if (engage(attaquant, cible)) {
            return true;
        }
    }
    return false;
}

// Qui a rompu le contact : la cible, si elle s'est écartée de l'attaquant au moins autant que
// l'attaquant s'est écarté d'elle (déplacements projetés sur la droite attaquant → cible du début).
// Deux centres confondus n'ont pas de direction : personne n'a fui.
template <class IdA, class IdC>
bool cibleAFui(const EntiteDeControle<IdA>& attaquantAvant, const EntiteDeControle<IdA>& attaquantApres,
               const EntiteDeControle<IdC>& cibleAvant, const EntiteDeControle<IdC>& cibleApres) {
    const sf::Vector2f direction = cibleAvant.cercle.centre - attaquantAvant.cercle.centre;
    if (direction.lengthSquared() == 0.0f) {
        return false;
    }
    const float ecartDeLaCible = (cibleApres.cercle.centre - cibleAvant.cercle.centre).dot(direction);
    const float ecartDeLAttaquant = (attaquantApres.cercle.centre - attaquantAvant.cercle.centre).dot(-direction);
    return ecartDeLaCible >= ecartDeLAttaquant;
}

// Le monstre engage-t-il un personnage de place différente de `placeExclue` ?
bool engageUnAutre(const MonstreDeControle& monstre, const std::vector<PersonnageDeControle>& personnages, Place placeExclue) {
    for (const PersonnageDeControle& personnage : personnages) {
        if (personnage.id != placeExclue && engage(monstre, personnage)) {
            return true;
        }
    }
    return false;
}
}  // namespace

template <class IdAttaquant, class IdCible>
std::vector<CoupDOpportunite<IdAttaquant, IdCible>> coupsDOpportunite(const PhotoDesCamps<IdAttaquant, IdCible>& avant,
                                                                      const PhotoDesCamps<IdAttaquant, IdCible>& apres) {
    std::vector<CoupDOpportunite<IdAttaquant, IdCible>> coups;
    for (const EntiteDeControle<IdAttaquant>& attaquantAvant : avant.attaquants) {
        const EntiteDeControle<IdAttaquant>* attaquantApres = trouver(apres.attaquants, attaquantAvant.id);
        // Absent à la fin (mort, retiré) : rien. Encore au contact de quelqu'un à la fin : exempté.
        if (attaquantApres == nullptr || engageQuelquUn(*attaquantApres, apres.cibles)) {
            continue;
        }
        const EntiteDeControle<IdCible>* fuyardChoisi = nullptr;
        float ecartChoisi = 0.0f;
        for (const EntiteDeControle<IdCible>& cibleAvant : avant.cibles) {
            const EntiteDeControle<IdCible>* cibleApres = trouver(apres.cibles, cibleAvant.id);
            // Seule une paire engagée au début, dont la cible existe encore, a pu se rompre ;
            // l'attaquant n'engage plus personne à la fin, donc elle est bien rompue.
            if (cibleApres == nullptr || !engage(attaquantAvant, cibleAvant) ||
                !cibleAFui(attaquantAvant, *attaquantApres, cibleAvant, *cibleApres)) {
                continue;
            }
            const float ecart = distanceBordABord(attaquantApres->cercle, cibleApres->cercle);
            if (fuyardChoisi == nullptr || ecart < ecartChoisi) {
                fuyardChoisi = &cibleAvant;
                ecartChoisi = ecart;
            }
        }
        if (fuyardChoisi != nullptr) {
            coups.push_back({attaquantAvant.id, fuyardChoisi->id});
        }
    }
    return coups;
}

// Les deux sens de la règle : un monstre contre un personnage, et un personnage contre un monstre.
template std::vector<CoupDOpportunite<IdMonstre, Place>> coupsDOpportunite(const PhotoDesCamps<IdMonstre, Place>&,
                                                                          const PhotoDesCamps<IdMonstre, Place>&);
template std::vector<CoupDOpportunite<Place, IdMonstre>> coupsDOpportunite(const PhotoDesCamps<Place, IdMonstre>&,
                                                                          const PhotoDesCamps<Place, IdMonstre>&);

std::optional<IdMonstre> monstreDontSeDegager(const PersonnageDeControle& personnage,
                                              const std::vector<PersonnageDeControle>& personnages,
                                              const std::vector<MonstreDeControle>& monstres) {
    if (personnage.zone > 0.0f) {
        return std::nullopt;
    }
    const MonstreDeControle* plusProche = nullptr;
    float ecartChoisi = 0.0f;
    for (const MonstreDeControle& monstre : monstres) {
        if (!engage(monstre, personnage)) {
            continue;
        }
        // Un seul monstre qui l'engage sans qu'un autre personnage soit dans sa zone suffit à rester.
        if (!engageUnAutre(monstre, personnages, personnage.id)) {
            return std::nullopt;
        }
        const float ecart = distanceBordABord(monstre.cercle, personnage.cercle);
        if (plusProche == nullptr || ecart < ecartChoisi) {
            plusProche = &monstre;
            ecartChoisi = ecart;
        }
    }
    if (plusProche == nullptr) {
        return std::nullopt;
    }
    return plusProche->id;
}
