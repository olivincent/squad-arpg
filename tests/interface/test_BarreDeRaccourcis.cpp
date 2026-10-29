// ./tests/interface/test_BarreDeRaccourcis.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <vector>

#include "aides_escouade.hpp"
#include "interface/BarreDeRaccourcis.hpp"

TEST_CASE("une barre a la taille demandee et tous ses emplacements sont vides") {
    const BarreDeRaccourcis barre(5);

    REQUIRE(barre.taille() == 5);
    for (std::size_t i = 0; i < barre.taille(); ++i) {
        REQUIRE_FALSE(barre.adresse(i).has_value());
    }
}

TEST_CASE("placer remplit un emplacement, vider le libere, les autres ne bougent pas") {
    BarreDeRaccourcis barre(4);

    barre.placer(2, s_adresseSoin);
    REQUIRE(barre.adresse(2) == s_adresseSoin);
    REQUIRE_FALSE(barre.adresse(1).has_value());
    REQUIRE_FALSE(barre.adresse(3).has_value());

    barre.placer(2, s_adresseFrappe);
    REQUIRE(barre.adresse(2) == s_adresseFrappe);

    barre.vider(2);
    REQUIRE_FALSE(barre.adresse(2).has_value());
}

TEST_CASE("une meme adresse peut etre placee sur deux emplacements") {
    BarreDeRaccourcis barre(4);

    barre.placer(0, s_adresseFrappe);
    barre.placer(3, s_adresseFrappe);

    REQUIRE(barre.adresse(0) == s_adresseFrappe);
    REQUIRE(barre.adresse(3) == s_adresseFrappe);
}

TEST_CASE("barreParDefaut : une colonne de 3 par personnage avec la composition par defaut") {
    const Escouade escouade = uneEscouade({200.0f, 200.0f});

    const BarreDeRaccourcis barre = barreParDefaut(escouade.adressesDesSkills());

    REQUIRE(barre.taille() == s_emplacementsPrincipaux);
    REQUIRE(barre.adresse(0) == s_adresseFrappe);      // Q : le guerrier, colonne 1
    REQUIRE(barre.adresse(4) == s_adresseProvocation); // A
    REQUIRE(barre.adresse(1) == s_adresseSoin);        // W : le pretre, colonne 2
    REQUIRE(barre.adresse(2) == s_adresseExplosion);   // E : le mage, colonne 3
    for (const std::size_t vide : {3, 5, 6, 7, 8, 9, 10, 11}) {
        REQUIRE_FALSE(barre.adresse(vide).has_value());
    }
}

TEST_CASE("barreParDefaut : les skills au-dela du troisieme d'un personnage ne sont pas places") {
    const std::vector<AdresseSkill> adresses = {
        {Place{1}, IdSkill::FrappePuissante}, {Place{1}, IdSkill::Provocation}, {Place{1}, IdSkill::Soin},
        {Place{1}, IdSkill::Explosion},  // le quatrieme du personnage 1
        {Place{2}, IdSkill::Soin},
    };

    const BarreDeRaccourcis barre = barreParDefaut(adresses);

    REQUIRE(barre.adresse(0) == s_adresseFrappe);
    REQUIRE(barre.adresse(4) == s_adresseProvocation);
    REQUIRE(barre.adresse(8) == AdresseSkill{Place{1}, IdSkill::Soin});  // le troisieme, en bas de la colonne
    REQUIRE(barre.adresse(1) == s_adresseSoin);  // la colonne suivante n'est pas touchee
    REQUIRE_FALSE(barre.adresse(5).has_value());
}

TEST_CASE("barreParDefaut : la place p occupe les emplacements p-1, p+3 et p+7") {
    std::vector<AdresseSkill> adresses;
    for (std::size_t numero = 1; numero <= 4; ++numero) {
        for (const IdSkill skill : {IdSkill::FrappePuissante, IdSkill::Provocation, IdSkill::Soin}) {
            adresses.push_back({Place{numero}, skill});
        }
    }

    const BarreDeRaccourcis barre = barreParDefaut(adresses);

    for (std::size_t numero = 1; numero <= 4; ++numero) {
        REQUIRE(barre.adresse(numero - 1) == AdresseSkill{Place{numero}, IdSkill::FrappePuissante});
        REQUIRE(barre.adresse(numero + 3) == AdresseSkill{Place{numero}, IdSkill::Provocation});
        REQUIRE(barre.adresse(numero + 7) == AdresseSkill{Place{numero}, IdSkill::Soin});
    }
}

TEST_CASE("barreParDefaut : une place sans colonne est ignoree, sans erreur") {
    const std::vector<AdresseSkill> adresses = {{Place{5}, IdSkill::Soin}, {Place{0}, IdSkill::Soin}};

    const BarreDeRaccourcis barre = barreParDefaut(adresses);

    for (std::size_t i = 0; i < barre.taille(); ++i) {
        REQUIRE_FALSE(barre.adresse(i).has_value());
    }
}

namespace {

// Deux largeurs de vue d'interface (800 px de haut) : 4:3 et 16:9.
constexpr sf::Vector2f s_vueEtroite{1067.0f, 800.0f};
constexpr sf::Vector2f s_vueLarge{1422.0f, 800.0f};

bool seChevauchent(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

}  // namespace

TEST_CASE("rectangleDeLEmplacement : les 12 icones sont dans la vue et ne se chevauchent pas") {
    for (const sf::Vector2f vue : {s_vueEtroite, s_vueLarge}) {
        const sf::FloatRect cadre({0.0f, 0.0f}, vue);
        for (std::size_t i = 0; i < s_emplacementsPrincipaux; ++i) {
            const sf::FloatRect rect = rectangleDeLEmplacement(i, vue);
            REQUIRE(cadre.contains(rect.position));
            REQUIRE(cadre.contains(rect.position + rect.size - sf::Vector2f{1.0f, 1.0f}));
            for (std::size_t j = i + 1; j < s_emplacementsPrincipaux; ++j) {
                REQUIRE_FALSE(seChevauchent(rect, rectangleDeLEmplacement(j, vue)));
            }
        }
    }
}

TEST_CASE("rectangleDeLEmplacement : grille de 4 colonnes, rangee du haut = emplacements 0 a 3") {
    const sf::FloatRect premier = rectangleDeLEmplacement(0, s_vueLarge);
    const sf::FloatRect deuxieme = rectangleDeLEmplacement(1, s_vueLarge);
    const sf::FloatRect cinquieme = rectangleDeLEmplacement(4, s_vueLarge);
    const sf::FloatRect dernier = rectangleDeLEmplacement(11, s_vueLarge);

    REQUIRE(premier.size == sf::Vector2f{48.0f, 48.0f});
    REQUIRE(deuxieme.position.y == premier.position.y);
    REQUIRE(deuxieme.position.x == premier.position.x + 52.0f);
    REQUIRE(cinquieme.position.x == premier.position.x);
    REQUIRE(cinquieme.position.y == premier.position.y + 52.0f);
    REQUIRE(dernier.position.y + dernier.size.y == s_vueLarge.y - 10.0f);  // marge de 10 px en bas
}

TEST_CASE("la barre reste centree quand la largeur de la vue change") {
    for (const sf::Vector2f vue : {s_vueEtroite, s_vueLarge}) {
        const sf::FloatRect gauche = rectangleDeLEmplacement(0, vue);
        const sf::FloatRect droite = rectangleDeLEmplacement(3, vue);
        const float margeGauche = gauche.position.x;
        const float margeDroite = vue.x - (droite.position.x + droite.size.x);
        REQUIRE_THAT(margeGauche, Catch::Matchers::WithinAbs(margeDroite, 0.01));
    }
}

TEST_CASE("emplacementSous : le centre de chaque icone donne son emplacement") {
    for (const sf::Vector2f vue : {s_vueEtroite, s_vueLarge}) {
        for (std::size_t i = 0; i < s_emplacementsPrincipaux; ++i) {
            const sf::FloatRect rect = rectangleDeLEmplacement(i, vue);
            REQUIRE(emplacementSous(rect.position + rect.size / 2.0f, vue) == i);
        }
    }
}

TEST_CASE("emplacementSous : un ecart entre icones ou un point hors de la barre est vide") {
    const sf::FloatRect premier = rectangleDeLEmplacement(0, s_vueLarge);
    const sf::FloatRect deuxieme = rectangleDeLEmplacement(1, s_vueLarge);
    const sf::FloatRect cinquieme = rectangleDeLEmplacement(4, s_vueLarge);

    const float xEcart = (premier.position.x + premier.size.x + deuxieme.position.x) / 2.0f;
    REQUIRE_FALSE(emplacementSous({xEcart, premier.position.y + 20.0f}, s_vueLarge).has_value());
    const float yEcart = (premier.position.y + premier.size.y + cinquieme.position.y) / 2.0f;
    REQUIRE_FALSE(emplacementSous({premier.position.x + 20.0f, yEcart}, s_vueLarge).has_value());
    REQUIRE_FALSE(emplacementSous({10.0f, 10.0f}, s_vueLarge).has_value());
    REQUIRE_FALSE(emplacementSous({s_vueLarge.x / 2.0f, s_vueLarge.y - 2.0f}, s_vueLarge).has_value());  // sous la barre
}
