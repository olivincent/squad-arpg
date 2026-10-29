// ./tests/interface/test_EcranChoixEscouade.cpp
#include <catch2/catch_test_macros.hpp>

#include <variant>

#include "interface/EcranChoixEscouade.hpp"
#include "interface/ListeDeBoutons.hpp"
#include "escouade/SelectionDEscouade.hpp"
#include "escouade/Specialite.hpp"
#include "escouade/TailleDeLEscouade.hpp"

namespace {

using Liste = std::vector<IdSpecialite>;

const sf::Vector2f s_centre{600.0f, 400.0f};

sf::Vector2f milieu(sf::FloatRect rect) {
    return rect.position + rect.size / 2.0f;
}

// Centre du bouton de la classe `id` (même liste que l'écran : le catalogue).
sf::Vector2f centreDeLaClasse(IdSpecialite id) {
    std::vector<std::string> noms;
    for (const Specialite& specialite : catalogue()) {
        noms.push_back(specialite.nom);
    }
    return milieu(ListeDeBoutons(noms).rectangleDuBouton(static_cast<std::size_t>(id), centreDesClasses(s_centre)));
}

sf::Vector2f centreDeLaCase(std::size_t indice) {
    return milieu(rectangleDeLaCase(indice, centreDesCases(s_centre)));
}

// 0 = « Start », 1 = « Back ».
sf::Vector2f centreDeLAction(std::size_t indice) {
    return milieu(ListeDeBoutons({"Start", "Back"}).rectangleDuBouton(indice, centreDesActions(s_centre)));
}

// Clique la classe, en ignorant la transition (toujours `Aucune`).
void choisir(EcranChoixEscouade& ecran, IdSpecialite id) {
    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLaClasse(id), s_centre)));
}

// Une escouade complete : guerrier, pretre, mage, rodeur.
void choisirQuatre(EcranChoixEscouade& ecran) {
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Pretre);
    choisir(ecran, IdSpecialite::Mage);
    choisir(ecran, IdSpecialite::Rodeur);
}

}  // namespace

TEST_CASE("choix : la selection est vide au depart") {
    const EcranChoixEscouade ecran;

    REQUIRE(ecran.getSelection().liste().empty());
}

TEST_CASE("choix : cliquer une classe l'ajoute a la selection") {
    EcranChoixEscouade ecran;

    choisir(ecran, IdSpecialite::Rodeur);
    choisir(ecran, IdSpecialite::Paladin);

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Rodeur, IdSpecialite::Paladin});
}

TEST_CASE("choix : on peut prendre quatre fois la meme classe, pas une cinquieme") {
    EcranChoixEscouade ecran;

    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Mage);

    REQUIRE(ecran.getSelection().liste() == Liste(4, IdSpecialite::Guerrier));
}

TEST_CASE("choix : cliquer une case remplie la retire et les suivantes remontent") {
    EcranChoixEscouade ecran;
    choisirQuatre(ecran);

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLaCase(0), s_centre)));

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Pretre, IdSpecialite::Mage, IdSpecialite::Rodeur});
}

TEST_CASE("choix : cliquer une case vide ne fait rien") {
    EcranChoixEscouade ecran;
    choisir(ecran, IdSpecialite::Guerrier);

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLaCase(2), s_centre)));

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Guerrier});
}

TEST_CASE("choix : Commencer est inactif tant que la selection n'est pas complete") {
    EcranChoixEscouade ecran;
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Pretre);

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLAction(0), s_centre)));
}

TEST_CASE("choix : Commencer est inactif avec trois classes seulement") {
    EcranChoixEscouade ecran;
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Pretre);
    choisir(ecran, IdSpecialite::Mage);

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLAction(0), s_centre)));
}

TEST_CASE("choix : Commencer empile l'ecran de jeu quand la selection est complete") {
    EcranChoixEscouade ecran;
    choisirQuatre(ecran);

    const Transition transition = ecran.cliquer(centreDeLAction(0), s_centre);

    REQUIRE(std::holds_alternative<Empiler>(transition));
    REQUIRE(std::get<Empiler>(transition).ecran != nullptr);
}

TEST_CASE("choix : Commencer redevient inactif si on retire une classe") {
    EcranChoixEscouade ecran;
    choisirQuatre(ecran);
    static_cast<void>(ecran.cliquer(centreDeLaCase(1), s_centre));

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLAction(0), s_centre)));
}

TEST_CASE("choix : Retour depile, meme avec une selection complete") {
    EcranChoixEscouade ecran;
    choisirQuatre(ecran);

    REQUIRE(std::holds_alternative<Depiler>(ecran.cliquer(centreDeLAction(1), s_centre)));
}

TEST_CASE("choix : Commencer ne change pas la selection (elle survit a la partie)") {
    EcranChoixEscouade ecran;
    choisirQuatre(ecran);

    static_cast<void>(ecran.cliquer(centreDeLAction(0), s_centre));

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Guerrier, IdSpecialite::Pretre, IdSpecialite::Mage, IdSpecialite::Rodeur});
}

TEST_CASE("choix : un clic hors des boutons ne fait rien") {
    EcranChoixEscouade ecran;

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer({5.0f, 5.0f}, s_centre)));
    REQUIRE(ecran.getSelection().liste().empty());
}

TEST_CASE("choix : aucune colonne ne chevauche une autre, en 1200 x 800 comme en 16:9") {
    std::vector<std::string> noms;
    for (const Specialite& specialite : catalogue()) {
        noms.push_back(specialite.nom);
    }
    const ListeDeBoutons classes(noms);
    const ListeDeBoutons actions({"Start", "Back"});

    for (const sf::Vector2f centre : {sf::Vector2f{600.0f, 400.0f}, sf::Vector2f{711.0f, 400.0f}}) {
        std::vector<sf::FloatRect> rectangles;
        for (std::size_t i = 0; i < noms.size(); ++i) {
            rectangles.push_back(classes.rectangleDuBouton(i, centreDesClasses(centre)));
        }
        for (std::size_t i = 0; i < s_tailleDeLEscouade; ++i) {
            rectangles.push_back(rectangleDeLaCase(i, centreDesCases(centre)));
        }
        for (std::size_t i = 0; i < 2; ++i) {
            rectangles.push_back(actions.rectangleDuBouton(i, centreDesActions(centre)));
        }

        for (std::size_t i = 0; i < rectangles.size(); ++i) {
            // Dans la vue (800 px de haut, 1200 px de large au moins) et sans toucher les autres.
            REQUIRE(rectangles[i].position.y >= 0.0f);
            REQUIRE(rectangles[i].position.y + rectangles[i].size.y <= 800.0f);
            for (std::size_t j = i + 1; j < rectangles.size(); ++j) {
                REQUIRE_FALSE(rectangles[i].findIntersection(rectangles[j]).has_value());
            }
        }
    }
}

TEST_CASE("choix : quatre cases de selection, la colonne des actions sous la derniere sans la toucher") {
    const sf::Vector2f centre{600.0f, 400.0f};
    const sf::FloatRect derniereCase = rectangleDeLaCase(s_tailleDeLEscouade - 1, centreDesCases(centre));
    const sf::FloatRect start = ListeDeBoutons({"Start", "Back"}).rectangleDuBouton(0, centreDesActions(centre));

    REQUIRE(s_tailleDeLEscouade == 4);
    REQUIRE(start.position.y > derniereCase.position.y + derniereCase.size.y);
}
