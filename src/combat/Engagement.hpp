// ./src/combat/Engagement.hpp
#pragma once
#include <optional>
#include <vector>
#include "base/Cercle.hpp"

// Une entité telle que la règle des zones de contrôle la voit : son identifiant
// (monstre, ou place 1 à N d'un personnage), son cercle et sa zone de contrôle, en
// pixels, mesurée bord à bord. Une zone de 0 n'engage personne.
struct EntiteDeControle {
    int id;
    Cercle cercle;
    float zone;
};

// Les deux camps d'UN sens de la règle à un instant donné : ceux qui engagent
// (`attaquants`) et ceux qui peuvent être engagés (`cibles`). Pour l'autre sens,
// on échange les deux listes.
struct PhotoDesCamps {
    std::vector<EntiteDeControle> attaquants;
    std::vector<EntiteDeControle> cibles;
};

// Un coup gratuit : `idAttaquant` frappe `idCible`, qui vient de lui échapper.
struct CoupDOpportunite {
    int idAttaquant;
    int idCible;
};

// A engage B si la zone de A est > 0 et que l'écart bord à bord de A à B est <= cette
// zone. La relation n'est pas symétrique.
[[nodiscard]] bool engage(const EntiteDeControle& a, const EntiteDeControle& b);

// Coups gratuits d'une image, dans un sens (les attaquants de `avant` contre les cibles de
// `avant`), d'après les photos prises au début (`avant`) et à la fin (`apres`) de l'image.
// Une paire (A engage B) présente au début et absente à la fin s'est rompue. Si B a fui
// (il s'est écarté de A au moins autant que A s'est écarté de B, en projection sur la
// droite A→B du début), A frappe B, SAUF si A engage encore quelqu'un à la fin (la
// vérification se fait à la fin : deux fuyards simultanés ne s'exemptent pas). Rien si A
// ou B est absent de la photo de fin. Au plus un coup par attaquant : le fuyard le plus
// proche de lui à la fin (bord à bord ; égalité : le premier de `avant.cibles`). Les
// coups sortent dans l'ordre de `avant.attaquants`. Le résultat ne dépend d'aucune autre
// règle (recharge, ligne de vue, dégâts) : c'est à l'appelant de les appliquer.
[[nodiscard]] std::vector<CoupDOpportunite> coupsDOpportunite(const PhotoDesCamps& avant, const PhotoDesCamps& apres);

// Règle de départ du dégagement d'un personnage sans zone de contrôle : `personnage` est engagé
// par au moins un des `monstres` ET chacun de ces monstres engage aussi un AUTRE personnage de
// `personnages` (identifiant différent ; `personnage` peut figurer dans la liste, il ne compte pas).
// Un personnage qui a une zone (> 0) ne se dégage jamais. Renvoie l'identifiant du plus proche
// (bord à bord ; égalité : le premier de `monstres`) des monstres qui l'engagent, celui dont il
// va rejoindre la place d'attaque ; vide si la règle n'est pas remplie. Ne tient compte ni de
// l'ancre, ni des skills, ni d'un éventuel dégagement déjà en cours : c'est l'affaire de l'appelant.
[[nodiscard]] std::optional<int> monstreDontSeDegager(const EntiteDeControle& personnage,
                                                      const std::vector<EntiteDeControle>& personnages,
                                                      const std::vector<EntiteDeControle>& monstres);
