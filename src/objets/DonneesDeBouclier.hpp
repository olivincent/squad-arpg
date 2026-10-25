// ./src/objets/DonneesDeBouclier.hpp
#pragma once

// Les données d'un bouclier, portées par une `DefinitionObjet` : la chance de blocage et le montant absorbé sont des
// propriétés du bouclier (et non des bonus de statistiques). La récupération qui suit un blocage est commune à tous
// (`s_recuperationDuBlocage`). Un bouclier se porte en main gauche.
struct DonneesDeBouclier {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int chanceDeBlocage;  // en pour cent
    int montantBloque;    // points absorbés par un blocage, sans dépasser le coup

    bool operator==(const DonneesDeBouclier&) const = default;
};
