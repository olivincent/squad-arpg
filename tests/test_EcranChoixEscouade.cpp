// ./tests/test_EcranChoixEscouade.cpp
#include "aides.hpp"

#include <variant>

#include "EcranChoixEscouade.hpp"
#include "ListeDeBoutons.hpp"
#include "SelectionDEscouade.hpp"

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

// 0 = « Commencer », 1 = « Retour ».
sf::Vector2f centreDeLAction(std::size_t indice) {
    return milieu(ListeDeBoutons({"Commencer", "Retour"}).rectangleDuBouton(indice, centreDesActions(s_centre)));
}

// Clique la classe, en ignorant la transition (toujours `Aucune`).
void choisir(EcranChoixEscouade& ecran, IdSpecialite id) {
    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLaClasse(id), s_centre)));
}

void choisirTrois(EcranChoixEscouade& ecran) {
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Pretre);
    choisir(ecran, IdSpecialite::Mage);
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

TEST_CASE("choix : on peut prendre trois fois la meme classe, pas une quatrieme") {
    EcranChoixEscouade ecran;

    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Guerrier);
    choisir(ecran, IdSpecialite::Mage);

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Guerrier, IdSpecialite::Guerrier, IdSpecialite::Guerrier});
}

TEST_CASE("choix : cliquer une case remplie la retire et les suivantes remontent") {
    EcranChoixEscouade ecran;
    choisirTrois(ecran);

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLaCase(0), s_centre)));

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Pretre, IdSpecialite::Mage});
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

TEST_CASE("choix : Commencer empile l'ecran de jeu quand la selection est complete") {
    EcranChoixEscouade ecran;
    choisirTrois(ecran);

    const Transition transition = ecran.cliquer(centreDeLAction(0), s_centre);

    REQUIRE(std::holds_alternative<Empiler>(transition));
    REQUIRE(std::get<Empiler>(transition).ecran != nullptr);
}

TEST_CASE("choix : Commencer redevient inactif si on retire une classe") {
    EcranChoixEscouade ecran;
    choisirTrois(ecran);
    static_cast<void>(ecran.cliquer(centreDeLaCase(1), s_centre));

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer(centreDeLAction(0), s_centre)));
}

TEST_CASE("choix : Retour depile, meme avec une selection complete") {
    EcranChoixEscouade ecran;
    choisirTrois(ecran);

    REQUIRE(std::holds_alternative<Depiler>(ecran.cliquer(centreDeLAction(1), s_centre)));
}

TEST_CASE("choix : Commencer ne change pas la selection (elle survit a la partie)") {
    EcranChoixEscouade ecran;
    choisirTrois(ecran);

    static_cast<void>(ecran.cliquer(centreDeLAction(0), s_centre));

    REQUIRE(ecran.getSelection().liste() == Liste{IdSpecialite::Guerrier, IdSpecialite::Pretre, IdSpecialite::Mage});
}

TEST_CASE("choix : un clic hors des boutons ne fait rien") {
    EcranChoixEscouade ecran;

    REQUIRE(std::holds_alternative<Aucune>(ecran.cliquer({5.0f, 5.0f}, s_centre)));
    REQUIRE(ecran.getSelection().liste().empty());
}
