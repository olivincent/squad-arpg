// ./src/objets/DonneesDeBouclier.hpp
#pragma once

// Les données d'un bouclier, portées par une `DefinitionObjet` : la chance de blocage et le montant absorbé sont des
// propriétés du bouclier (et non des bonus de statistiques), données **au niveau 1** : `effetsDeBase` les fait grandir avec le
// niveau de l'objet (la chance d'un point par niveau, le montant avec le niveau et la qualité). La récupération qui suit un
// blocage est commune à tous (`s_recuperationDuBlocage`). Un bouclier se porte en main gauche.
struct DonneesDeBouclier {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int chanceAuNiveau1;  // en pour cent
    int montantAuNiveau1; // points absorbés par un blocage, sans dépasser le coup, à la qualité moyenne

    bool operator==(const DonneesDeBouclier&) const = default;
};
