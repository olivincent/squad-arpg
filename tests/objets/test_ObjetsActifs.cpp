// ./tests/objets/test_ObjetsActifs.cpp
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <vector>

#include "combat/Caracteristiques.hpp"
#include "objets/ObjetsActifs.hpp"

// Le calcul des objets actifs (étape 92.2), sur des objets inventés : le catalogue n'a aucun objet qui donne un
// bonus et exige un seuil à la fois, donc aucune chaîne de dépendances. Les objets du catalogue sont testés avec
// `Equipement` et `Personnage`.
namespace {
using Portes = std::vector<std::optional<ExigencesEtBonus>>;

// Un objet qui exige ce seuil et donne ce bonus de Puissance.
ExigencesEtBonus exigeEtDonne(Caracteristique exigee, int valeur, int bonusDePuissance) {
    return {{{exigee, valeur}}, Caracteristiques::seule(Caracteristique::Puissance, bonusDePuissance)};
}

ExigencesEtBonus donneSeulement(int bonusDePuissance) {
    return {{}, Caracteristiques::seule(Caracteristique::Puissance, bonusDePuissance)};
}

const Caracteristiques s_base({10, 10, 0, 0, 0});
}  // namespace

TEST_CASE("objetsActifs : sans seuil, tout objet porte est actif, un emplacement vide ne l'est pas") {
    const Portes portes = {donneSeulement(3), std::nullopt, ExigencesEtBonus{{}, Caracteristiques()}};

    REQUIRE(objetsActifs(s_base, portes) == std::vector<bool>{true, false, true});
}

TEST_CASE("objetsActifs : un seuil non atteint rend l'objet inactif, un seuil atteint exactement le laisse actif") {
    const Portes portes = {exigeEtDonne(Caracteristique::Endurance, 11, 0), exigeEtDonne(Caracteristique::Endurance, 10, 0)};

    REQUIRE(objetsActifs(s_base, portes) == std::vector<bool>{false, true});
}

TEST_CASE("objetsActifs : le bonus d'un autre objet actif aide a atteindre un seuil") {
    const Portes portes = {donneSeulement(3), exigeEtDonne(Caracteristique::Puissance, 13, 0)};

    REQUIRE(objetsActifs(s_base, portes) == std::vector<bool>{true, true});
}

TEST_CASE("objetsActifs : un objet ne compte pas pour son propre seuil") {
    // Il donne +5 de Puissance mais en exige 12 : sa base est de 10, il ne se soutient pas lui-même.
    const Portes portes = {exigeEtDonne(Caracteristique::Puissance, 12, 5)};

    REQUIRE(objetsActifs(s_base, portes) == std::vector<bool>{false});
}

TEST_CASE("objetsActifs : la chaine, desactiver un objet en desactive un autre") {
    // A donne +3 de Puissance mais exige 12 d'Endurance (la base en a 10) : inactif. B exige 13 de Puissance, que
    // seul le bonus de A lui donnait : inactif à son tour.
    const Portes chaine = {exigeEtDonne(Caracteristique::Endurance, 12, 3), exigeEtDonne(Caracteristique::Puissance, 13, 0)};
    REQUIRE(objetsActifs(s_base, chaine) == std::vector<bool>{false, false});

    // Avec l'Endurance qu'il faut, A est actif, et B avec lui.
    REQUIRE(objetsActifs(Caracteristiques({10, 12, 0, 0, 0}), chaine) == std::vector<bool>{true, true});
}

TEST_CASE("objetsActifs : la chaine se propage sur plusieurs maillons, quel que soit l'ordre des emplacements") {
    // C exige B, qui exige A, qui exige 12 d'Endurance : tous inactifs ; dans les deux ordres.
    const ExigencesEtBonus a = exigeEtDonne(Caracteristique::Endurance, 12, 3);
    const ExigencesEtBonus b = exigeEtDonne(Caracteristique::Puissance, 13, 3);
    const ExigencesEtBonus c = exigeEtDonne(Caracteristique::Puissance, 16, 0);

    REQUIRE(objetsActifs(s_base, Portes{a, b, c}) == std::vector<bool>{false, false, false});
    REQUIRE(objetsActifs(s_base, Portes{c, b, a}) == std::vector<bool>{false, false, false});
    REQUIRE(objetsActifs(Caracteristiques({10, 12, 0, 0, 0}), Portes{c, b, a}) == std::vector<bool>{true, true, true});
}

TEST_CASE("objetsActifs : la remise en service quand le seuil revient") {
    const ExigencesEtBonus anneau = donneSeulement(3);
    const ExigencesEtBonus lourd = exigeEtDonne(Caracteristique::Puissance, 13, 0);

    REQUIRE(objetsActifs(s_base, Portes{lourd}) == std::vector<bool>{false});             // sans l'anneau
    REQUIRE(objetsActifs(s_base, Portes{lourd, anneau}) == std::vector<bool>{true, true});  // l'anneau revient
}

TEST_CASE("objetsActifs : deux objets qui se soutiennent l'un l'autre restent actifs, la liste ne fait que diminuer") {
    // Chacun voit le bonus de l'autre : le calcul part de tous les objets et ne retire que ce qui manque.
    const Portes portes = {exigeEtDonne(Caracteristique::Puissance, 13, 3), exigeEtDonne(Caracteristique::Puissance, 13, 3)};

    REQUIRE(objetsActifs(s_base, portes) == std::vector<bool>{true, true});
}
