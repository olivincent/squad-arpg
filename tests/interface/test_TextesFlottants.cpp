// ./tests/interface/test_TextesFlottants.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "combat/Element.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/IdSkill.hpp"
#include "interface/TextesFlottants.hpp"

namespace {

EvenementDeCombat degatsSurUnMonstre(int id, int montant = 10, sf::Vector2f position = {100.0f, 100.0f}) {
    return {IdMonstre{id}, position, DegatsSubis{montant, false, false}};
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
    textes.ajouter({Place{1}, {100.0f, 100.0f}, DegatsSubis{10, false, false}});

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
    REQUIRE(libelleDuTexte({Place{1}, {}, DegatsSubis{7, true, false}}) == "7!");
    REQUIRE(libelleDuTexte({Place{2}, {}, Soin{20, false}}) == "+20");
}

TEST_CASE("le libelle d'un critique : le montant suivi de Critique !") {
    REQUIRE(libelleDuTexte({IdMonstre{1}, {}, DegatsSubis{12, false, true}}) == "12 Critical!");
    REQUIRE(libelleDuTexte({IdMonstre{1}, {}, DegatsSubis{7, true, true}}) == "7! Critical!");  // le « ! » du coup gratuit reste
    REQUIRE(libelleDuTexte({Place{2}, {}, Soin{20, true}}) == "+20 Critical!");
}

TEST_CASE("un critique est jaune et de 18 px, meme un coup gratuit ou sur un personnage") {
    const EvenementDeCombat surUnMonstre{IdMonstre{1}, {}, DegatsSubis{10, false, true}};
    const EvenementDeCombat surUnPersonnage{Place{1}, {}, DegatsSubis{10, false, true}};
    const EvenementDeCombat gratuit{IdMonstre{1}, {}, DegatsSubis{10, true, true}};
    const EvenementDeCombat normal{IdMonstre{1}, {}, DegatsSubis{10, false, false}};

    REQUIRE(couleurDuTexte(surUnMonstre) == couleurDuTexte(surUnPersonnage));
    REQUIRE(couleurDuTexte(surUnMonstre) == couleurDuTexte(gratuit));
    REQUIRE(couleurDuTexte(surUnMonstre).r == 255);
    REQUIRE(couleurDuTexte(surUnMonstre).g > 200);
    REQUIRE(couleurDuTexte(surUnMonstre).b < 50);
    REQUIRE(couleurDuTexte(surUnMonstre) != couleurDuTexte(normal));
    REQUIRE(tailleDuTexte(surUnMonstre) == 18);
    REQUIRE(tailleDuTexte(gratuit) == 18);  // le style critique l'emporte sur celui du coup gratuit
    REQUIRE(tailleDuTexte(surUnMonstre) > tailleDuTexte(normal));
}

TEST_CASE("un soin critique reste vert, plus gros qu'un soin normal") {
    const EvenementDeCombat critique{Place{2}, {}, Soin{10, true}};
    const EvenementDeCombat normal{Place{2}, {}, Soin{10, false}};

    REQUIRE(couleurDuTexte(critique) == couleurDuTexte(normal));
    REQUIRE(couleurDuTexte(critique).g > couleurDuTexte(critique).r);
    REQUIRE(tailleDuTexte(critique) == 18);
    REQUIRE(tailleDuTexte(normal) == 16);
}

TEST_CASE("le libelle d'une esquive et d'un blocage : Esquive, Bloque total et Bloque partiel") {
    REQUIRE(libelleDuTexte({IdMonstre{1}, {}, Esquive{}}) == "Dodged");
    REQUIRE(libelleDuTexte({Place{1}, {}, Bloque{40, true}}) == "Blocked");
    REQUIRE(libelleDuTexte({Place{1}, {}, Bloque{40, false}}) == "Blocked \u2212" "40");
}

TEST_CASE("l'esquive est en gris clair et le blocage en gris plus fonce, tous deux de 14 px") {
    const EvenementDeCombat esquive{IdMonstre{1}, {}, Esquive{}};
    const EvenementDeCombat bloque{Place{1}, {}, Bloque{40, false}};

    for (const EvenementDeCombat& evenement : {esquive, bloque}) {
        const sf::Color couleur = couleurDuTexte(evenement);
        REQUIRE(couleur.r == couleur.g);
        REQUIRE(couleur.g == couleur.b);
        REQUIRE(tailleDuTexte(evenement) == 14);
    }
    REQUIRE(couleurDuTexte(esquive).r > couleurDuTexte(bloque).r);
}

TEST_CASE("le texte du sac plein : Bag full, de 14 px, d'une couleur que les autres textes n'ont pas") {
    const EvenementDeCombat sacPlein{EscouadeEntiere{}, {}, SacPlein{}};

    REQUIRE(libelleDuTexte(sacPlein) == "Bag full");
    REQUIRE(tailleDuTexte(sacPlein) == 14);
    for (const EvenementDeCombat& autre :
         {degatsSurUnMonstre(1), EvenementDeCombat{Place{1}, {}, DegatsSubis{10, false, false}},
          EvenementDeCombat{Place{2}, {}, Soin{10, false}}, EvenementDeCombat{IdMonstre{1}, {}, Esquive{}},
          EvenementDeCombat{Place{1}, {}, Bloque{40, false}}, EvenementDeCombat{IdMonstre{1}, {}, Provoque{}}}) {
        REQUIRE(couleurDuTexte(sacPlein) != couleurDuTexte(autre));
    }
}

TEST_CASE("un blocage partiel s'empile au-dessus des degats de la meme cible et de la meme image") {
    TextesFlottants textes;

    textes.ajouter({Place{1}, {100.0f, 100.0f}, DegatsSubis{60, false, false}});
    textes.ajouter({Place{1}, {100.0f, 100.0f}, Bloque{40, false}});

    REQUIRE(textes.actifs()[0].rang == 0);
    REQUIRE(textes.actifs()[1].rang == 1);
    REQUIRE(positionDuTexte(textes.actifs()[1]).y < positionDuTexte(textes.actifs()[0]).y);  // plus haut à l'écran
}

TEST_CASE("la couleur et la taille : blanc sur un monstre, rouge sur un personnage, vert pour un soin, orange et plus gros pour un coup gratuit") {
    const EvenementDeCombat surUnMonstre = degatsSurUnMonstre(1);
    const EvenementDeCombat surUnPersonnage{Place{1}, {}, DegatsSubis{10, false, false}};
    const EvenementDeCombat soin{Place{2}, {}, Soin{10, false}};
    const EvenementDeCombat gratuitSurUnMonstre{IdMonstre{1}, {}, DegatsSubis{10, true, false}};
    const EvenementDeCombat gratuitSurUnPersonnage{Place{1}, {}, DegatsSubis{10, true, false}};

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

TEST_CASE("un skill lance : son nom en jaune dore, 14 px") {
    const EvenementDeCombat lance{Place{1}, {}, SkillLance{IdSkill::ImpositionDesMains}};
    const EvenementDeCombat autre{Place{1}, {}, SkillLance{IdSkill::FrappeSacree}};

    REQUIRE(libelleDuTexte(lance) == "Lay on Hands");
    REQUIRE(libelleDuTexte(autre) == "Holy Strike");
    REQUIRE(couleurDuTexte(lance).r == 255);
    REQUIRE(couleurDuTexte(lance).g > 150);
    REQUIRE(couleurDuTexte(lance).b < 100);
    REQUIRE(tailleDuTexte(lance) == 14);
}

TEST_CASE("provoque : le texte Provoque en rouge orange, 14 px") {
    const EvenementDeCombat provoque{IdMonstre{4}, {}, Provoque{}};

    REQUIRE(libelleDuTexte(provoque) == "Taunted");
    REQUIRE(couleurDuTexte(provoque).r == 255);
    REQUIRE(couleurDuTexte(provoque).g < 150);
    REQUIRE(couleurDuTexte(provoque) != couleurDuTexte({Place{1}, {}, SkillLance{IdSkill::Provocation}}));
    REQUIRE(tailleDuTexte(provoque) == 14);
}

namespace {
EvenementDeCombat degatsDeLElement(Element element, CibleDEvenement cible = IdMonstre{1}, bool opportunite = false, bool critique = false) {
    return {cible, {}, DegatsSubis{10, opportunite, critique, element}};
}

double distanceDeCouleur(sf::Color a, sf::Color b) {
    const double r = static_cast<double>(a.r) - static_cast<double>(b.r);
    const double g = static_cast<double>(a.g) - static_cast<double>(b.g);
    const double bleu = static_cast<double>(a.b) - static_cast<double>(b.b);
    return std::sqrt(r * r + g * g + bleu * bleu);
}
}  // namespace

TEST_CASE("le nombre d'un coup physique garde la couleur de sa cible, quel que soit l'element par defaut") {
    REQUIRE(couleurDuTexte(degatsDeLElement(Element::Physique, IdMonstre{1})) == sf::Color::White);
    REQUIRE(couleurDuTexte(degatsDeLElement(Element::Physique, Place{1})) == sf::Color(255, 60, 60));
}

TEST_CASE("le nombre d'un coup elementaire prend la couleur de son element, sur un monstre comme sur un personnage") {
    for (const Element element : s_elements) {
        if (element == Element::Physique) {
            continue;
        }
        INFO("element " << static_cast<int>(element));
        REQUIRE(couleurDuTexte(degatsDeLElement(element, IdMonstre{1})) == couleurDuTexte(degatsDeLElement(element, Place{1})));
        REQUIRE(couleurDuTexte(degatsDeLElement(element)) != couleurDuTexte(degatsDeLElement(Element::Physique, IdMonstre{1})));
        REQUIRE(couleurDuTexte(degatsDeLElement(element)) != couleurDuTexte(degatsDeLElement(Element::Physique, Place{1})));
    }
}

TEST_CASE("le feu est orange et le sacre dore") {
    const sf::Color feu = couleurDuTexte(degatsDeLElement(Element::Feu));
    const sf::Color sacre = couleurDuTexte(degatsDeLElement(Element::Sacre));

    REQUIRE(feu.r == 255);
    REQUIRE(feu.g > feu.b);
    REQUIRE(feu.g < 150);
    REQUIRE(sacre.r > 200);
    REQUIRE(sacre.g > 150);
    REQUIRE(sacre.b < 120);
    REQUIRE(sacre != feu);
}

TEST_CASE("les couleurs des nombres sont assez distinctes : physique, feu, sacre, coup gratuit, critique, soin") {
    const std::vector<sf::Color> couleurs = {
        couleurDuTexte(degatsDeLElement(Element::Physique, IdMonstre{1})),
        couleurDuTexte(degatsDeLElement(Element::Physique, Place{1})),
        couleurDuTexte(degatsDeLElement(Element::Feu)),
        couleurDuTexte(degatsDeLElement(Element::Sacre)),
        couleurDuTexte(degatsDeLElement(Element::Physique, IdMonstre{1}, true)),  // coup gratuit
        couleurDuTexte(degatsDeLElement(Element::Physique, IdMonstre{1}, false, true)),  // critique
        couleurDuTexte({Place{1}, {}, Soin{10, false}}),
    };

    for (std::size_t i = 0; i < couleurs.size(); ++i) {
        for (std::size_t j = i + 1; j < couleurs.size(); ++j) {
            INFO("couleurs " << i << " et " << j);
            REQUIRE(distanceDeCouleur(couleurs[i], couleurs[j]) >= 50.0);
        }
    }
}

TEST_CASE("un critique elementaire reste jaune, avec son Critique !") {
    const EvenementDeCombat critiqueDeFeu = degatsDeLElement(Element::Feu, IdMonstre{1}, false, true);
    const EvenementDeCombat critiquePhysique = degatsDeLElement(Element::Physique, IdMonstre{1}, false, true);

    REQUIRE(couleurDuTexte(critiqueDeFeu) == couleurDuTexte(critiquePhysique));
    REQUIRE(libelleDuTexte(critiqueDeFeu) == "10 Critical!");
    REQUIRE(tailleDuTexte(critiqueDeFeu) == 18);
}

TEST_CASE("le libelle d'un coup elementaire est le total, comme pour un coup physique") {
    REQUIRE(libelleDuTexte(degatsDeLElement(Element::Feu)) == "10");
    REQUIRE(libelleDuTexte(degatsDeLElement(Element::Sacre)) == libelleDuTexte(degatsDeLElement(Element::Physique)));
}

TEST_CASE("le texte du niveau atteint : Level up!, de 20 px, d'une couleur que les autres textes n'ont pas") {
    const EvenementDeCombat niveauAtteint{EscouadeEntiere{}, {}, NiveauAtteint{3}};

    REQUIRE(libelleDuTexte(niveauAtteint) == "Level up!");
    REQUIRE(tailleDuTexte(niveauAtteint) == 20);
    for (const EvenementDeCombat& autre :
         {degatsSurUnMonstre(1), EvenementDeCombat{Place{1}, {}, DegatsSubis{10, false, false}},
          EvenementDeCombat{Place{2}, {}, Soin{10, false}}, EvenementDeCombat{IdMonstre{1}, {}, Esquive{}},
          EvenementDeCombat{Place{1}, {}, Bloque{40, false}}, EvenementDeCombat{IdMonstre{1}, {}, Provoque{}},
          EvenementDeCombat{EscouadeEntiere{}, {}, SacPlein{}}}) {
        REQUIRE(couleurDuTexte(niveauAtteint) != couleurDuTexte(autre));
    }
}

TEST_CASE("le niveau atteint s'empile avec les autres textes de l'escouade entiere, comme Bag full") {
    TextesFlottants textes;

    textes.ajouter({EscouadeEntiere{}, {}, SacPlein{}});
    textes.ajouter({EscouadeEntiere{}, {}, NiveauAtteint{2}});

    REQUIRE(textes.actifs().size() == 2);
    REQUIRE(textes.actifs()[0].rang == 0);
    REQUIRE(textes.actifs()[1].rang == 1);
}
