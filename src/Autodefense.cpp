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
