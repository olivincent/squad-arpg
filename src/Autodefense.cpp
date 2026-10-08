// ./src/Autodefense.cpp
#include "Autodefense.hpp"

std::optional<int> choisirMonstreAFrapper(Cercle origine,
                                          const std::function<bool(Cercle)>& estAPortee,
                                          const std::vector<MonstreVu>& monstres,
                                          std::optional<int> idVerrouille) {
    std::optional<int> meilleur;
    float meilleurEcart = 0.0f;

    for (const MonstreVu& monstre : monstres) {
        if (!estAPortee(monstre.cercle)) {
            continue;
        }
        if (idVerrouille && monstre.id == *idVerrouille) {
            return monstre.id;
        }
        const float ecart = distanceBordABord(origine, monstre.cercle);
        if (!meilleur || ecart < meilleurEcart) {
            meilleur = monstre.id;
            meilleurEcart = ecart;
        }
    }
    return meilleur;
}

std::optional<Cercle> choisirAgresseurAViser(Cercle personnage,
                                             sf::Vector2f place,
                                             float laisse,
                                             const std::vector<MonstreVu>& agresseurs) {
    std::optional<Cercle> meilleur;
    float meilleurEcart = 0.0f;

    for (const MonstreVu& agresseur : agresseurs) {
        if ((agresseur.cercle.centre - place).length() > laisse) {
            continue;
        }
        const float ecart = distanceBordABord(personnage, agresseur.cercle);
        if (!meilleur || ecart < meilleurEcart) {
            meilleur = agresseur.cercle;
            meilleurEcart = ecart;
        }
    }
    return meilleur;
}
