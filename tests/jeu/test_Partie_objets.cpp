// ./tests/jeu/test_Partie_objets.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Aleatoire.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/Partie.hpp"
#include "objets/GenerationDObjets.hpp"
#include "objets/CatalogueObjets.hpp"

// Les objets tirés au hasard par la touche F7 (étape 96.4) : `Partie::ramasserDesObjetsAleatoires`.
namespace {
constexpr std::size_t s_objetsParF7 = 4;

Partie partieDuSac() {
    return unePartie(niveauDegage({{400.0f, 400.0f}}), s_graineDeTest);
}

// Remplit le sac d'objets communs jusqu'à ne laisser que `libres` cases libres.
void remplirLeSac(Partie& partie, std::size_t libres) {
    while (partie.getEscouade().sac().casesLibres() > libres) {
        REQUIRE(partie.getEscouade().ramasser(Objet{IdObjet::GantsDeTissu}));
    }
}

// Une partie du sac à cette profondeur.
Partie partieDuSacALaProfondeur(int profondeur) {
    Expedition expedition = expeditionDeTest();
    expedition.profondeur = profondeur;
    return Partie(niveauDegage({{400.0f, 400.0f}}), std::move(expedition), s_graineDeTest, catalogueDeTest(monstreSansHasard()));
}

// Les objets du sac, dans l'ordre des cases, sans les cases vides.
std::vector<Objet> objetsDuSac(const Partie& partie) {
    std::vector<Objet> objets;
    for (std::size_t i = 0; i < partie.getEscouade().sac().capacite(); ++i) {
        if (partie.getEscouade().sac().a(i).has_value()) {
            objets.push_back(*partie.getEscouade().sac().a(i));
        }
    }
    return objets;
}

std::vector<std::optional<Objet>> contenuDuSac(const Partie& partie) {
    std::vector<std::optional<Objet>> contenu;
    for (std::size_t i = 0; i < partie.getEscouade().sac().capacite(); ++i) {
        contenu.push_back(partie.getEscouade().sac().a(i));
    }
    return contenu;
}
}  // namespace

TEST_CASE("ramasserDesObjetsAleatoires : quatre objets de plus dans le sac") {
    Partie partie = partieDuSac();
    const std::size_t libresAvant = partie.getEscouade().sac().casesLibres();
    const std::vector<std::optional<Objet>> avant = contenuDuSac(partie);

    REQUIRE(partie.ramasserDesObjetsAleatoires(s_objetsParF7));

    REQUIRE(partie.getEscouade().sac().casesLibres() == libresAvant - s_objetsParF7);
    const std::vector<std::optional<Objet>> apres = contenuDuSac(partie);
    std::size_t nouveaux = 0;
    for (std::size_t i = 0; i < apres.size(); ++i) {
        if (!avant[i].has_value() && apres[i].has_value()) {
            ++nouveaux;
            REQUIRE(objetValide(*apres[i]));
        } else {
            REQUIRE(apres[i] == avant[i]);  // rien d'autre n'a bougé
        }
    }
    REQUIRE(nouveaux == s_objetsParF7);
}

TEST_CASE("ramasserDesObjetsAleatoires : le sac juste assez libre accepte, il se remplit") {
    Partie partie = partieDuSac();
    remplirLeSac(partie, s_objetsParF7);

    REQUIRE(partie.ramasserDesObjetsAleatoires(s_objetsParF7));

    REQUIRE(partie.getEscouade().sac().casesLibres() == 0);
}

TEST_CASE("ramasserDesObjetsAleatoires : sans assez de cases libres, rien n'est ajoute") {
    Partie partie = partieDuSac();
    remplirLeSac(partie, s_objetsParF7 - 1);
    const std::vector<std::optional<Objet>> avant = contenuDuSac(partie);

    REQUIRE_FALSE(partie.ramasserDesObjetsAleatoires(s_objetsParF7));

    REQUIRE(contenuDuSac(partie) == avant);
}

TEST_CASE("ramasserDesObjetsAleatoires : un refus ne consomme aucun tirage") {
    // Deux parties de même graine, au sac identique. L'une essaie d'abord de ramasser quatre objets (refusée), puis trois ;
    // l'autre ramasse directement trois. Si le refus avait tiré un objet, les trois objets de la première seraient
    // d'autres objets que ceux de la seconde.
    Partie avecRefus = partieDuSac();
    Partie sansRefus = partieDuSac();
    remplirLeSac(avecRefus, s_objetsParF7 - 1);
    remplirLeSac(sansRefus, s_objetsParF7 - 1);

    REQUIRE_FALSE(avecRefus.ramasserDesObjetsAleatoires(s_objetsParF7));
    REQUIRE(avecRefus.ramasserDesObjetsAleatoires(s_objetsParF7 - 1));
    REQUIRE(sansRefus.ramasserDesObjetsAleatoires(s_objetsParF7 - 1));

    REQUIRE(contenuDuSac(avecRefus) == contenuDuSac(sansRefus));
}

TEST_CASE("ramasserDesObjetsAleatoires : le hasard est celui de la partie, la graine decide des objets") {
    Partie a = unePartie(niveauDegage({{400.0f, 400.0f}}), 1);
    Partie memeGraine = unePartie(niveauDegage({{400.0f, 400.0f}}), 1);
    Partie autreGraine = unePartie(niveauDegage({{400.0f, 400.0f}}), 2);

    REQUIRE(a.ramasserDesObjetsAleatoires(s_objetsParF7));
    REQUIRE(memeGraine.ramasserDesObjetsAleatoires(s_objetsParF7));
    REQUIRE(autreGraine.ramasserDesObjetsAleatoires(s_objetsParF7));

    REQUIRE(contenuDuSac(a) == contenuDuSac(memeGraine));
    REQUIRE_FALSE(contenuDuSac(a) == contenuDuSac(autreGraine));
}

TEST_CASE("ramasserDesObjetsAleatoires : au 1er etage, les objets sont ceux de genererObjet au niveau 1") {
    Partie partie = partieDuSacALaProfondeur(1);
    Generateur temoin(s_graineDeTest);  // la partie n'a tiré aucun nombre avant F7

    REQUIRE(partie.ramasserDesObjetsAleatoires(s_objetsParF7));

    std::vector<Objet> attendus;
    for (std::size_t i = 0; i < s_objetsParF7; ++i) {
        attendus.push_back(genererObjet(temoin, 1));
    }
    REQUIRE(objetsDuSac(partie) == attendus);
}

TEST_CASE("ramasserDesObjetsAleatoires : a l'etage 5, le generateur recoit la profondeur comme niveau") {
    Partie partie = partieDuSacALaProfondeur(5);
    Generateur temoin(s_graineDeTest);

    REQUIRE(partie.ramasserDesObjetsAleatoires(s_objetsParF7));

    std::vector<Objet> attendus;
    for (std::size_t i = 0; i < s_objetsParF7; ++i) {
        attendus.push_back(genererObjet(temoin, 5));
    }
    REQUIRE(objetsDuSac(partie) == attendus);
}
