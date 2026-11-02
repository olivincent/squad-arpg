// ./src/combat/RendementsDeControle.cpp
#include "combat/RendementsDeControle.hpp"

float RendementsDeControle::facteurSuivant(EtatInflige etat) const {
    const std::size_t deja = static_cast<std::size_t>(applications(etat));
    return deja < s_rendementsDesApplications.size() ? s_rendementsDesApplications[deja] : 0.0f;
}

float RendementsDeControle::appliquer(EtatInflige etat) {
    const float facteur = facteurSuivant(etat);
    if (facteur > 0.0f) {
        Compteur& compteur = m_compteurs[static_cast<std::size_t>(etat)];
        ++compteur.applications;
        compteur.depuisLaDerniere = 0.0f;
    }
    return facteur;
}

int RendementsDeControle::applications(EtatInflige etat) const {
    return m_compteurs[static_cast<std::size_t>(etat)].applications;
}

void RendementsDeControle::mettreAJour(float dt) {
    for (Compteur& compteur : m_compteurs) {
        if (compteur.applications == 0) {
            continue;
        }
        compteur.depuisLaDerniere += dt;
        if (compteur.depuisLaDerniere >= s_remiseAZeroDuRendement) {
            compteur = Compteur{};
        }
    }
}

void RendementsDeControle::vider() {
    m_compteurs = {};
}
