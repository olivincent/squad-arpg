// ./src/combat/Jauge.hpp
#pragma once

// Une valeur entière plafonnée : 0 <= valeur <= maximum. La règle commune des points de vie (`PointsDeVie`) et de l'énergie
// (`Reserve`), qui y ajoutent chacune la leur. Commence pleine. Ne dépend d'aucun autre module du projet.
class Jauge {
public:
    // Précondition : `maximum` > 0.
    explicit Jauge(int maximum);
    // Rend ce qui a réellement été ajouté (plafonné au maximum). Précondition : `montant` >= 0.
    int ajouter(int montant);
    // Rend ce qui a réellement été retiré (plafonné à ce qu'il y avait). Précondition : `montant` >= 0.
    int retirer(int montant);
    // Change le maximum et la valeur de la même différence, sans sortir de [`plancher`, nouveau maximum] : monter le maximum
    // ajoute d'autant, le baisser retire d'autant. Préconditions : `nouveauMaximum` > 0, 0 <= `plancher` <= `nouveauMaximum`.
    void changerMaximum(int nouveauMaximum, int plancher = 0);
    // Change le maximum sans toucher à la valeur, sauf pour la plafonner. Précondition : `nouveauMaximum` > 0.
    void definirMaximum(int nouveauMaximum);
    [[nodiscard]] int getValeur() const;
    [[nodiscard]] int getMaximum() const;
    [[nodiscard]] float getRatio() const;

private:
    int m_valeur;
    int m_maximum;
};
