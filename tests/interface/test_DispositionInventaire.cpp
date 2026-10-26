// ./tests/interface/test_DispositionInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "combat/Caracteristiques.hpp"
#include "interface/DispositionInventaire.hpp"

namespace {

const sf::Vector2f s_vueMinimale{1200.0f, 800.0f};
const sf::Vector2f s_vueLarge{1422.0f, 800.0f};  // 16:9

// Tous les rectangles de l'écran, pour les vérifications globales.
std::vector<sf::FloatRect> tousLesRectangles(sf::Vector2f vue) {
    std::vector<sf::FloatRect> rectangles;
    for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
        rectangles.push_back(rectangleDeLEnTete(place, vue));
        rectangles.push_back(rectangleDeLArmure(place, vue));
        for (std::size_t cellule = 0; cellule <= s_nombreDeCaracteristiques; ++cellule) {
            rectangles.push_back(rectangleDeLaCellule(place, cellule, vue));
        }
        for (std::size_t cellule = 0; cellule < s_cellulesDeStatistiques; ++cellule) {
            rectangles.push_back(rectangleDeLaStatistique(place, cellule, vue));
        }
        for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
            rectangles.push_back(rectangleDeLEmplacement(place, static_cast<Emplacement>(i), vue));
        }
    }
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        rectangles.push_back(rectangleDeLaCaseDuSac(indice, vue));
    }
    return rectangles;
}

sf::Vector2f centre(sf::FloatRect rectangle) {
    return rectangle.position + rectangle.size / 2.0f;
}

}  // namespace

TEST_CASE("disposition : tout tient dans la vue, dans le panneau") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        const sf::FloatRect panneau = rectangleDuPanneau(vue);
        const sf::Vector2f coinBasDroit = panneau.position + panneau.size;
        for (const sf::FloatRect& rectangle : tousLesRectangles(vue)) {
            REQUIRE(rectangle.position.x >= panneau.position.x);
            REQUIRE(rectangle.position.y >= panneau.position.y);
            REQUIRE(rectangle.position.x + rectangle.size.x <= coinBasDroit.x);
            REQUIRE(rectangle.position.y + rectangle.size.y <= coinBasDroit.y);
        }
    }
}

TEST_CASE("disposition : aucun chevauchement") {
    const std::vector<sf::FloatRect> rectangles = tousLesRectangles(s_vueMinimale);

    for (std::size_t i = 0; i < rectangles.size(); ++i) {
        for (std::size_t j = i + 1; j < rectangles.size(); ++j) {
            REQUIRE_FALSE(rectangles[i].findIntersection(rectangles[j]).has_value());
        }
    }
}

TEST_CASE("disposition : les colonnes sont dans l'ordre des places, centrees") {
    const sf::FloatRect premiere = rectangleDeLEnTete(1, s_vueMinimale);
    const sf::FloatRect derniere = rectangleDeLEnTete(s_nombreDePlaces, s_vueMinimale);

    REQUIRE(premiere.position.x < rectangleDeLEnTete(2, s_vueMinimale).position.x);
    REQUIRE(rectangleDeLEnTete(2, s_vueMinimale).position.x < derniere.position.x);
    REQUIRE(premiere.position.x == s_vueMinimale.x - (derniere.position.x + derniere.size.x));
}

TEST_CASE("disposition : les emplacements suivent l'ordre de l'enum, en deux colonnes de cinq") {
    const sf::FloatRect tete = rectangleDeLEmplacement(1, Emplacement::Tete, s_vueMinimale);
    const sf::FloatRect bottes = rectangleDeLEmplacement(1, Emplacement::Bottes, s_vueMinimale);
    const sf::FloatRect mainDroite = rectangleDeLEmplacement(1, Emplacement::MainDroite, s_vueMinimale);
    const sf::FloatRect anneau2 = rectangleDeLEmplacement(1, Emplacement::Anneau2, s_vueMinimale);

    REQUIRE(tete.position.x == bottes.position.x);
    REQUIRE(tete.position.y < bottes.position.y);
    REQUIRE(mainDroite.position.x > tete.position.x);
    REQUIRE(mainDroite.position.y == tete.position.y);
    REQUIRE(anneau2.position.y == bottes.position.y);
}

TEST_CASE("disposition : le sac a 2 rangees de 20 cases carrees dans l'ordre des indices") {
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, s_vueMinimale);
    const sf::FloatRect derniereDeLaRangee = rectangleDeLaCaseDuSac(s_colonnesDuSac - 1, s_vueMinimale);
    const sf::FloatRect premiereDeLaSuivante = rectangleDeLaCaseDuSac(s_colonnesDuSac, s_vueMinimale);

    REQUIRE(s_colonnesDuSac == 20);
    REQUIRE(s_casesDuSac / s_colonnesDuSac == 2);
    REQUIRE(premiere.size.x == premiere.size.y);
    REQUIRE(derniereDeLaRangee.position.y == premiere.position.y);
    REQUIRE(derniereDeLaRangee.position.x > premiere.position.x);
    REQUIRE(premiereDeLaSuivante.position.x == premiere.position.x);
    REQUIRE(premiereDeLaSuivante.position.y > premiere.position.y);
    REQUIRE(rectangleDeLaCaseDuSac(s_casesDuSac - 1, s_vueMinimale).position.y == premiereDeLaSuivante.position.y);
}

TEST_CASE("disposition : le sac est sous les colonnes des personnages") {
    const sf::FloatRect dernierEmplacement = rectangleDeLEmplacement(1, Emplacement::Bottes, s_vueMinimale);

    REQUIRE(rectangleDeLaCaseDuSac(0, s_vueMinimale).position.y > dernierEmplacement.position.y + dernierEmplacement.size.y);
}

TEST_CASE("elementSous : le centre de chaque emplacement le designe") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
            for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
                const Emplacement emplacement = static_cast<Emplacement>(i);

                const std::optional<PositionDObjet> element = elementSous(centre(rectangleDeLEmplacement(place, emplacement, vue)), vue);

                REQUIRE(element == PositionDObjet{PositionPortee{place, emplacement}});
            }
        }
    }
}

TEST_CASE("elementSous : le centre de chaque case du sac la designe") {
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        const std::optional<PositionDObjet> element = elementSous(centre(rectangleDeLaCaseDuSac(indice, s_vueMinimale)), s_vueMinimale);

        REQUIRE(element == PositionDObjet{PositionDansLeSac{indice}});
    }
}

TEST_CASE("elementSous : rien dans un ecart, sur un en-tete ou hors des elements") {
    REQUIRE_FALSE(elementSous({5.0f, 5.0f}, s_vueMinimale).has_value());
    REQUIRE_FALSE(elementSous(centre(rectangleDeLEnTete(1, s_vueMinimale)), s_vueMinimale).has_value());
    REQUIRE_FALSE(elementSous(centre(rectangleDeLArmure(2, s_vueMinimale)), s_vueMinimale).has_value());
    // L'écart entre deux cases du sac.
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, s_vueMinimale);
    REQUIRE_FALSE(elementSous({premiere.position.x + premiere.size.x + 1.0f, premiere.position.y + 1.0f}, s_vueMinimale).has_value());
}

TEST_CASE("infobulle : sous et a droite du curseur quand il y a la place") {
    const sf::FloatRect zone = placerInfobulle({100.0f, 100.0f}, {200.0f, 70.0f}, s_vueMinimale);

    REQUIRE(zone.position.x > 100.0f);
    REQUIRE(zone.position.y > 100.0f);
    REQUIRE(zone.size == sf::Vector2f{200.0f, 70.0f});
}

TEST_CASE("infobulle : du cote oppose du curseur pres du bord droit ou du bas") {
    const sf::FloatRect zone = placerInfobulle({1190.0f, 790.0f}, {200.0f, 70.0f}, s_vueMinimale);

    REQUIRE(zone.position.x + zone.size.x <= 1190.0f);
    REQUIRE(zone.position.y + zone.size.y <= 790.0f);
}

TEST_CASE("infobulle : toujours entierement dans la vue, curseur n'importe ou") {
    const sf::Vector2f taille{220.0f, 80.0f};
    for (float x = 0.0f; x <= s_vueMinimale.x; x += 25.0f) {
        for (float y = 0.0f; y <= s_vueMinimale.y; y += 25.0f) {
            const sf::FloatRect zone = placerInfobulle({x, y}, taille, s_vueMinimale);

            REQUIRE(zone.position.x >= 0.0f);
            REQUIRE(zone.position.y >= 0.0f);
            REQUIRE(zone.position.x + zone.size.x <= s_vueMinimale.x);
            REQUIRE(zone.position.y + zone.size.y <= s_vueMinimale.y);
        }
    }
}

TEST_CASE("disposition : les cellules du bloc des caracteristiques tiennent dans le bloc, en deux colonnes") {
    const sf::FloatRect bloc = rectangleDesCaracteristiques(2, s_vueMinimale);

    for (std::size_t cellule = 0; cellule <= s_nombreDeCaracteristiques; ++cellule) {
        const sf::FloatRect zone = rectangleDeLaCellule(2, cellule, s_vueMinimale);
        REQUIRE(zone.position.x >= bloc.position.x);
        REQUIRE(zone.position.y >= bloc.position.y);
        REQUIRE(zone.position.x + zone.size.x <= bloc.position.x + bloc.size.x);
        REQUIRE(zone.position.y + zone.size.y <= bloc.position.y + bloc.size.y);
    }
    REQUIRE(rectangleDeLaCellule(2, 0, s_vueMinimale).position.x == bloc.position.x);
    REQUIRE(rectangleDeLaCellule(2, s_lignesDeCaracteristiques, s_vueMinimale).position.x > bloc.position.x);
    REQUIRE(rectangleDeLaCellule(2, 1, s_vueMinimale).position.y > rectangleDeLaCellule(2, 0, s_vueMinimale).position.y);
}

TEST_CASE("disposition : les cellules du bloc des statistiques tiennent dans le bloc, en deux colonnes") {
    const sf::FloatRect bloc = rectangleDesStatistiques(2, s_vueMinimale);

    for (std::size_t cellule = 0; cellule < s_cellulesDeStatistiques; ++cellule) {
        const sf::FloatRect zone = rectangleDeLaStatistique(2, cellule, s_vueMinimale);
        REQUIRE(zone.position.x >= bloc.position.x);
        REQUIRE(zone.position.y >= bloc.position.y);
        REQUIRE(zone.position.x + zone.size.x <= bloc.position.x + bloc.size.x);
        REQUIRE(zone.position.y + zone.size.y <= bloc.position.y + bloc.size.y);
    }
    REQUIRE(rectangleDeLaStatistique(2, 0, s_vueMinimale).position.x == bloc.position.x);
    REQUIRE(rectangleDeLaStatistique(2, s_lignesDeStatistiques, s_vueMinimale).position.x > bloc.position.x);
    REQUIRE(rectangleDeLaStatistique(2, 1, s_vueMinimale).position.y > rectangleDeLaStatistique(2, 0, s_vueMinimale).position.y);
}

TEST_CASE("disposition : le bloc des statistiques est sous celui des caracteristiques, et l'etiquette du sac en dessous") {
    constexpr float hauteurDeLEtiquetteDuSac = 24.0f;  // voir EcranInventaire
    const sf::FloatRect caracteristiques = rectangleDesCaracteristiques(1, s_vueMinimale);
    const sf::FloatRect statistiques = rectangleDesStatistiques(1, s_vueMinimale);

    REQUIRE(statistiques.position.y >= caracteristiques.position.y + caracteristiques.size.y);
    REQUIRE(rectangleDeLaCaseDuSac(0, s_vueMinimale).position.y - hauteurDeLEtiquetteDuSac >=
            statistiques.position.y + statistiques.size.y);
}

TEST_CASE("disposition : le bloc des caracteristiques est sous les emplacements et au-dessus du sac") {
    const sf::FloatRect dernierEmplacement = rectangleDeLEmplacement(1, Emplacement::Bottes, s_vueMinimale);
    const sf::FloatRect bloc = rectangleDesCaracteristiques(1, s_vueMinimale);

    REQUIRE(bloc.position.y >= dernierEmplacement.position.y + dernierEmplacement.size.y);
    REQUIRE(bloc.position.y + bloc.size.y < rectangleDeLaCaseDuSac(0, s_vueMinimale).position.y);
}

TEST_CASE("le panneau : centre, de la meme taille quelle que soit la vue, et tous les elements y tiennent") {
    const sf::FloatRect surLaVueMinimale = rectangleDuPanneau(s_vueMinimale);

    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        const sf::FloatRect panneau = rectangleDuPanneau(vue);

        REQUIRE(panneau.size == surLaVueMinimale.size);
        REQUIRE(panneau.position.x == vue.x - (panneau.position.x + panneau.size.x));
        REQUIRE(panneau.position.y == vue.y - (panneau.position.y + panneau.size.y));
        for (const sf::FloatRect& rectangle : tousLesRectangles(vue)) {
            REQUIRE(panneau.contains(rectangle.position));
            REQUIRE(panneau.contains(rectangle.position + rectangle.size - sf::Vector2f(1.0f, 1.0f)));
        }
    }
}

TEST_CASE("le panneau : une bande de jeu reste visible tout autour, assez large pour y lacher un objet") {
    constexpr float bandeMinimale = 50.0f;
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        const sf::FloatRect panneau = rectangleDuPanneau(vue);

        REQUIRE(panneau.position.x >= bandeMinimale);
        REQUIRE(panneau.position.y >= bandeMinimale);
        REQUIRE(vue.x - (panneau.position.x + panneau.size.x) >= bandeMinimale);
        REQUIRE(vue.y - (panneau.position.y + panneau.size.y) >= bandeMinimale);
    }
}
