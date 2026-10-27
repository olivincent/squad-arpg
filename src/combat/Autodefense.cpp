// ./src/combat/Autodefense.cpp
#include "combat/Autodefense.hpp"

std::optional<MonstreVu> choisirAgresseurAViser(Cercle personnage,
                                                sf::Vector2f place,
                                                float laisse,
                                                const std::vector<MonstreVu>& agresseurs) {
    std::optional<MonstreVu> meilleur;
    float meilleurEcart = 0.0f;

    for (const MonstreVu& agresseur : agresseurs) {
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
