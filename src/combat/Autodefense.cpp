// ./src/combat/Autodefense.cpp
#include "combat/Autodefense.hpp"

std::optional<int> choisirCibleAFrapper(Cercle origine,
                                          const std::function<bool(Cercle)>& estAPortee,
                                          const std::vector<EntiteVue>& entites,
                                          std::optional<int> idPrefere) {
    std::optional<int> meilleur;
    float meilleurEcart = 0.0f;

    for (const EntiteVue& entite : entites) {
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

std::optional<EntiteVue> choisirAgresseurAViser(Cercle personnage,
                                             sf::Vector2f place,
                                             float laisse,
                                             const std::vector<EntiteVue>& agresseurs) {
    std::optional<EntiteVue> meilleur;
    float meilleurEcart = 0.0f;

    for (const EntiteVue& agresseur : agresseurs) {
        if ((agresseur.cercle.centre - place).length() > laisse) {
            continue;
        }
        const float ecart = distanceBordABord(personnage, agresseur.cercle);
        if (!meilleur || ecart < meilleurEcart) {
            meilleur = agresseur;
            meilleurEcart = ecart;
        }
    }
    return meilleur;
}
