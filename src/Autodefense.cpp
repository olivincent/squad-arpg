// ./src/Autodefense.cpp
#include "Autodefense.hpp"
#include "Cercle.hpp"
#include "Personnage.hpp"

std::optional<int> choisirMonstreAFrapper(const Personnage& personnage,
                                          const std::vector<MonstreVu>& monstres,
                                          std::optional<int> idVerrouille) {
    std::optional<int> meilleur;
    float meilleurEcart = 0.0f;

    for (const MonstreVu& monstre : monstres) {
        if (!personnage.estAPortee(monstre.cercle)) {
            continue;
        }
        if (idVerrouille && monstre.id == *idVerrouille) {
            return monstre.id;
        }
        const float ecart = distanceBordABord(personnage.getCercle(), monstre.cercle);
        if (!meilleur || ecart < meilleurEcart) {
            meilleur = monstre.id;
            meilleurEcart = ecart;
        }
    }
    return meilleur;
}
