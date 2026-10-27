// ./tests/objets/test_Emplacement.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

#include "objets/Emplacement.hpp"

namespace {
const std::vector<Emplacement> s_tousLesEmplacements = {
    Emplacement::Tete,       Emplacement::Torse,      Emplacement::Jambes,  Emplacement::Gants,
    Emplacement::Bottes,     Emplacement::MainDroite, Emplacement::MainGauche, Emplacement::Collier,
    Emplacement::Anneau1,    Emplacement::Anneau2};

// Les emplacements acceptés par une catégorie, comptés dans tous les emplacements.
std::size_t nombreDEmplacements(CategorieObjet categorie) {
    std::size_t nombre = 0;
    for (const Emplacement emplacement : s_tousLesEmplacements) {
        if (peutAller(categorie, emplacement)) {
            ++nombre;
        }
    }
    return nombre;
}
}  // namespace

TEST_CASE("emplacements : il y en a s_nombreEmplacements, dans l'ordre de l'enum") {
    REQUIRE(s_tousLesEmplacements.size() == s_nombreEmplacements);
    REQUIRE(static_cast<std::size_t>(Emplacement::Tete) == 0);
    REQUIRE(static_cast<std::size_t>(Emplacement::Anneau2) == s_nombreEmplacements - 1);
}

TEST_CASE("peutAller : chaque categorie a son emplacement") {
    REQUIRE(peutAller(CategorieObjet::Casque, Emplacement::Tete));
    REQUIRE(peutAller(CategorieObjet::Armure, Emplacement::Torse));
    REQUIRE(peutAller(CategorieObjet::Jambieres, Emplacement::Jambes));
    REQUIRE(peutAller(CategorieObjet::Gants, Emplacement::Gants));
    REQUIRE(peutAller(CategorieObjet::Bottes, Emplacement::Bottes));
    REQUIRE(peutAller(CategorieObjet::Bouclier, Emplacement::MainGauche));
    REQUIRE(peutAller(CategorieObjet::Relique, Emplacement::MainGauche));
    REQUIRE(peutAller(CategorieObjet::Collier, Emplacement::Collier));
}

TEST_CASE("peutAller : un anneau va dans les deux emplacements d'anneau, pas ailleurs") {
    REQUIRE(peutAller(CategorieObjet::Anneau, Emplacement::Anneau1));
    REQUIRE(peutAller(CategorieObjet::Anneau, Emplacement::Anneau2));
    REQUIRE_FALSE(peutAller(CategorieObjet::Anneau, Emplacement::Collier));
    REQUIRE_FALSE(peutAller(CategorieObjet::Collier, Emplacement::Anneau1));
}

TEST_CASE("peutAller : un bouclier va seulement en main gauche") {
    REQUIRE_FALSE(peutAller(CategorieObjet::Bouclier, Emplacement::MainDroite));
    REQUIRE_FALSE(peutAller(CategorieObjet::Bouclier, Emplacement::Torse));
}

TEST_CASE("peutAller : une relique va seulement en main gauche, a la place d'un bouclier") {
    REQUIRE_FALSE(peutAller(CategorieObjet::Relique, Emplacement::MainDroite));
    REQUIRE_FALSE(peutAller(CategorieObjet::Relique, Emplacement::Torse));
}

TEST_CASE("peutAller : une arme va en main droite, et seulement la, ni en main gauche") {
    REQUIRE(peutAller(CategorieObjet::Arme, Emplacement::MainDroite));
    REQUIRE_FALSE(peutAller(CategorieObjet::Arme, Emplacement::MainGauche));
    REQUIRE_FALSE(peutAller(CategorieObjet::Arme, Emplacement::Torse));
}

TEST_CASE("peutAller : la main droite n'accepte que les armes") {
    for (const CategorieObjet categorie :
         {CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Gants,
          CategorieObjet::Bottes, CategorieObjet::Bouclier, CategorieObjet::Relique, CategorieObjet::Collier,
          CategorieObjet::Anneau}) {
        REQUIRE_FALSE(peutAller(categorie, Emplacement::MainDroite));
    }
}

TEST_CASE("peutAller : un emplacement du corps n'accepte que sa categorie") {
    REQUIRE_FALSE(peutAller(CategorieObjet::Armure, Emplacement::Tete));
    REQUIRE_FALSE(peutAller(CategorieObjet::Casque, Emplacement::Torse));
    REQUIRE_FALSE(peutAller(CategorieObjet::Gants, Emplacement::Bottes));
    REQUIRE_FALSE(peutAller(CategorieObjet::Bottes, Emplacement::Gants));
    REQUIRE_FALSE(peutAller(CategorieObjet::Jambieres, Emplacement::Torse));
}

TEST_CASE("peutAller : un seul emplacement par categorie, deux pour les anneaux") {
    REQUIRE(nombreDEmplacements(CategorieObjet::Casque) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Armure) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Jambieres) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Gants) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Bottes) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Bouclier) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Relique) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Collier) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Arme) == 1);
    REQUIRE(nombreDEmplacements(CategorieObjet::Anneau) == 2);
}
