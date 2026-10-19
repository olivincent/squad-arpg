// ./src/Engagement.cpp
#include "Engagement.hpp"
#include <SFML/System/Vector2.hpp>
#include "Cercle.hpp"

namespace {
// L'entité de cet identifiant, nullptr si elle est absente.
const EntiteDeControle* trouver(const std::vector<EntiteDeControle>& entites, int id) {
    for (const EntiteDeControle& entite : entites) {
        if (entite.id == id) {
            return &entite;
        }
    }
    return nullptr;
}

bool engageQuelquUn(const EntiteDeControle& attaquant, const std::vector<EntiteDeControle>& cibles) {
    for (const EntiteDeControle& cible : cibles) {
        if (engage(attaquant, cible)) {
            return true;
        }
    }
    return false;
}

// Qui a rompu le contact : la cible, si elle s'est écartée de l'attaquant au moins autant que
// l'attaquant s'est écarté d'elle (déplacements projetés sur la droite attaquant → cible du début).
// Deux centres confondus n'ont pas de direction : personne n'a fui.
bool cibleAFui(const EntiteDeControle& attaquantAvant, const EntiteDeControle& attaquantApres,
               const EntiteDeControle& cibleAvant, const EntiteDeControle& cibleApres) {
    const sf::Vector2f direction = cibleAvant.cercle.centre - attaquantAvant.cercle.centre;
    if (direction.lengthSquared() == 0.0f) {
        return false;
    }
    const float ecartDeLaCible = (cibleApres.cercle.centre - cibleAvant.cercle.centre).dot(direction);
    const float ecartDeLAttaquant = (attaquantApres.cercle.centre - attaquantAvant.cercle.centre).dot(-direction);
    return ecartDeLaCible >= ecartDeLAttaquant;
}
}  // namespace

bool engage(const EntiteDeControle& a, const EntiteDeControle& b) {
    return a.zone > 0.0f && distanceBordABord(a.cercle, b.cercle) <= a.zone;
}

std::vector<CoupDOpportunite> coupsDOpportunite(const PhotoDesCamps& avant, const PhotoDesCamps& apres) {
    std::vector<CoupDOpportunite> coups;
    for (const EntiteDeControle& attaquantAvant : avant.attaquants) {
        const EntiteDeControle* attaquantApres = trouver(apres.attaquants, attaquantAvant.id);
        // Absent à la fin (mort, retiré) : rien. Encore au contact de quelqu'un à la fin : exempté.
        if (attaquantApres == nullptr || engageQuelquUn(*attaquantApres, apres.cibles)) {
            continue;
        }
        const EntiteDeControle* fuyardChoisi = nullptr;
        float ecartChoisi = 0.0f;
        for (const EntiteDeControle& cibleAvant : avant.cibles) {
            const EntiteDeControle* cibleApres = trouver(apres.cibles, cibleAvant.id);
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
