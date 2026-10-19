// ./tests/interface/test_TextesFlottants.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include "combat/EvenementDeCombat.hpp"
#include "interface/TextesFlottants.hpp"

namespace {

EvenementDeCombat degatsSurUnMonstre(int id, int montant = 10, sf::Vector2f position = {100.0f, 100.0f}) {
    return {TypeEvenement::Degats, Camp::Monstres, id, montant, position, false};
}

}  // namespace

TEST_CASE("l'opacite est pleine a la naissance et a mi-vie, puis tombe a zero a 0,8 s") {
    REQUIRE_THAT(opaciteDuTexte(0.0f), Catch::Matchers::WithinAbs(1.0, 1e-6));
    REQUIRE_THAT(opaciteDuTexte(0.4f), Catch::Matchers::WithinAbs(1.0, 1e-6));
    REQUIRE_THAT(opaciteDuTexte(0.6f), Catch::Matchers::WithinAbs(0.5, 1e-6));
    REQUIRE_THAT(opaciteDuTexte(0.8f), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(opaciteDuTexte(1.5f), Catch::Matchers::WithinAbs(0.0, 1e-6));
}

TEST_CASE("la hauteur part de 20 px et monte de 30 px sur toute la duree de vie") {
    REQUIRE_THAT(hauteurDuTexte(0.0f), Catch::Matchers::WithinAbs(20.0, 1e-4));
    REQUIRE_THAT(hauteurDuTexte(0.4f), Catch::Matchers::WithinAbs(35.0, 1e-4));
    REQUIRE_THAT(hauteurDuTexte(0.8f), Catch::Matchers::WithinAbs(50.0, 1e-4));
}

TEST_CASE("un texte nait 20 px au-dessus de la cible et monte avec son age") {
    TextesFlottants textes;
    textes.ajouter(degatsSurUnMonstre(1, 10, {100.0f, 100.0f}));

    REQUIRE(textes.actifs().size() == 1);
    REQUIRE(positionDuTexte(textes.actifs()[0]) == sf::Vector2f(100.0f, 80.0f));

    textes.mettreAJour(0.4f);

    REQUIRE_THAT(positionDuTexte(textes.actifs()[0]).y, Catch::Matchers::WithinAbs(65.0, 1e-4));
    REQUIRE_THAT(positionDuTexte(textes.actifs()[0]).x, Catch::Matchers::WithinAbs(100.0, 1e-4));
}

TEST_CASE("un texte est retire a 0,8 s, pas avant") {
    TextesFlottants textes;
    textes.ajouter(degatsSurUnMonstre(1));

    textes.mettreAJour(0.4f);
    REQUIRE(textes.actifs().size() == 1);
    textes.mettreAJour(0.3f);
    REQUIRE(textes.actifs().size() == 1);
    textes.mettreAJour(0.1f);

    REQUIRE(textes.actifs().empty());
}

TEST_CASE("deux textes de la meme image sur la meme cible s'empilent de 14 px") {
    TextesFlottants textes;
    textes.ajouter(degatsSurUnMonstre(1, 10));
    textes.ajouter(degatsSurUnMonstre(1, 7));
    textes.ajouter(degatsSurUnMonstre(1, 5));

    REQUIRE(textes.actifs()[0].rang == 0);
    REQUIRE(textes.actifs()[1].rang == 1);
    REQUIRE(textes.actifs()[2].rang == 2);
    REQUIRE(positionDuTexte(textes.actifs()[0]).y - positionDuTexte(textes.actifs()[1]).y == 14.0f);
    REQUIRE(positionDuTexte(textes.actifs()[1]).y - positionDuTexte(textes.actifs()[2]).y == 14.0f);
}

TEST_CASE("l'empilement ne touche pas deux cibles differentes dans la meme image") {
    TextesFlottants textes;
    textes.ajouter(degatsSurUnMonstre(1));
    textes.ajouter(degatsSurUnMonstre(2));
    // Même identifiant, mais un personnage : une autre cible.
    textes.ajouter({TypeEvenement::Degats, Camp::Escouade, 1, 10, {100.0f, 100.0f}, false});

    for (const TexteFlottant& texte : textes.actifs()) {
        REQUIRE(texte.rang == 0);
        REQUIRE(positionDuTexte(texte) == sf::Vector2f(100.0f, 80.0f));
    }
}

TEST_CASE("l'empilement ne touche pas deux images differentes") {
    TextesFlottants textes;
    textes.ajouter(degatsSurUnMonstre(1));
    textes.mettreAJour(0.1f);
    textes.ajouter(degatsSurUnMonstre(1));

    REQUIRE(textes.actifs()[0].rang == 0);
    REQUIRE(textes.actifs()[1].rang == 0);
}

TEST_CASE("le libelle : degats, coup gratuit et soin") {
    REQUIRE(libelleDuTexte(degatsSurUnMonstre(1, 12)) == "12");
    REQUIRE(libelleDuTexte({TypeEvenement::Degats, Camp::Escouade, 1, 7, {}, true}) == "7!");
    REQUIRE(libelleDuTexte({TypeEvenement::Soin, Camp::Escouade, 2, 20, {}, false}) == "+20");
}

TEST_CASE("la couleur et la taille : blanc sur un monstre, rouge sur un personnage, vert pour un soin, orange et plus gros pour un coup gratuit") {
    const EvenementDeCombat surUnMonstre = degatsSurUnMonstre(1);
    const EvenementDeCombat surUnPersonnage{TypeEvenement::Degats, Camp::Escouade, 1, 10, {}, false};
    const EvenementDeCombat soin{TypeEvenement::Soin, Camp::Escouade, 2, 10, {}, false};
    const EvenementDeCombat gratuitSurUnMonstre{TypeEvenement::Degats, Camp::Monstres, 1, 10, {}, true};
    const EvenementDeCombat gratuitSurUnPersonnage{TypeEvenement::Degats, Camp::Escouade, 1, 10, {}, true};

    REQUIRE(couleurDuTexte(surUnMonstre) == sf::Color::White);
    REQUIRE(couleurDuTexte(surUnPersonnage).r == 255);
    REQUIRE(couleurDuTexte(surUnPersonnage).g < 100);
    REQUIRE(couleurDuTexte(soin).g > couleurDuTexte(soin).r);
    REQUIRE(couleurDuTexte(gratuitSurUnMonstre) == couleurDuTexte(gratuitSurUnPersonnage));
    REQUIRE(couleurDuTexte(gratuitSurUnMonstre) != couleurDuTexte(surUnMonstre));
    REQUIRE(tailleDuTexte(surUnMonstre) == 16);
    REQUIRE(tailleDuTexte(soin) == 16);
    REQUIRE(tailleDuTexte(gratuitSurUnMonstre) == 20);
}
