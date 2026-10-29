// ./tests/interface/test_DispositionInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "combat/Caracteristiques.hpp"
#include "escouade/TailleDeLEscouade.hpp"
#include "interface/DispositionInventaire.hpp"

namespace {

const sf::Vector2f s_vueMinimale{1200.0f, 800.0f};
const sf::Vector2f s_vueLarge{1422.0f, 800.0f};  // 16:9

// Tous les rectangles de l'écran, pour les vérifications globales : les onglets, la colonne du personnage ouvert, le sac.
std::vector<sf::FloatRect> tousLesRectangles(sf::Vector2f vue) {
    std::vector<sf::FloatRect> rectangles;
    for (const Place place : s_places) {
        rectangles.push_back(rectangleDeLOnglet(place, vue));
    }
    rectangles.push_back(rectangleDeLArmure(vue));
    for (std::size_t cellule = 0; cellule <= s_nombreDeCaracteristiques; ++cellule) {
        rectangles.push_back(rectangleDeLaCellule(cellule, vue));
    }
    for (std::size_t cellule = 0; cellule < s_cellulesDeStatistiques; ++cellule) {
        rectangles.push_back(rectangleDeLaStatistique(cellule, vue));
    }
    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        rectangles.push_back(rectangleDeLEmplacement(static_cast<Emplacement>(i), vue));
    }
    rectangles.push_back(rectangleDeLEtiquetteDuSac(vue));
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        rectangles.push_back(rectangleDeLaCaseDuSac(indice, vue));
    }
    return rectangles;
}

sf::Vector2f centre(sf::FloatRect rectangle) {
    return rectangle.position + rectangle.size / 2.0f;
}

constexpr Place s_ouverte{2};  // l'onglet ouvert des requêtes ci-dessous

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

TEST_CASE("disposition : aucun chevauchement, onglets compris, en 1200 x 800 comme en 16:9") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        const std::vector<sf::FloatRect> rectangles = tousLesRectangles(vue);

        for (std::size_t i = 0; i < rectangles.size(); ++i) {
            for (std::size_t j = i + 1; j < rectangles.size(); ++j) {
                REQUIRE_FALSE(rectangles[i].findIntersection(rectangles[j]).has_value());
            }
        }
    }
}

TEST_CASE("disposition : les onglets sont dans l'ordre des places, sur la largeur de la colonne, au-dessus d'elle") {
    const sf::FloatRect premier = rectangleDeLOnglet(Place{1}, s_vueMinimale);
    const sf::FloatRect dernier = rectangleDeLOnglet(Place{s_tailleDeLEscouade}, s_vueMinimale);
    const sf::FloatRect armure = rectangleDeLArmure(s_vueMinimale);

    for (std::size_t numero = 1; numero < s_tailleDeLEscouade; ++numero) {
        REQUIRE(rectangleDeLOnglet(Place{numero}, s_vueMinimale).position.x < rectangleDeLOnglet(Place{numero + 1}, s_vueMinimale).position.x);
        REQUIRE(rectangleDeLOnglet(Place{numero}, s_vueMinimale).position.y == premier.position.y);
    }
    REQUIRE(premier.position.x == armure.position.x);
    REQUIRE(dernier.position.x + dernier.size.x == armure.position.x + armure.size.x);
    REQUIRE(premier.position.y + premier.size.y <= armure.position.y);
}

TEST_CASE("disposition : les emplacements suivent l'ordre de l'enum, en deux colonnes de cinq") {
    const sf::FloatRect tete = rectangleDeLEmplacement(Emplacement::Tete, s_vueMinimale);
    const sf::FloatRect bottes = rectangleDeLEmplacement(Emplacement::Bottes, s_vueMinimale);
    const sf::FloatRect mainDroite = rectangleDeLEmplacement(Emplacement::MainDroite, s_vueMinimale);
    const sf::FloatRect anneau2 = rectangleDeLEmplacement(Emplacement::Anneau2, s_vueMinimale);

    REQUIRE(tete.position.x == bottes.position.x);
    REQUIRE(tete.position.y < bottes.position.y);
    REQUIRE(mainDroite.position.x > tete.position.x);
    REQUIRE(mainDroite.position.y == tete.position.y);
    REQUIRE(anneau2.position.y == bottes.position.y);
}

TEST_CASE("disposition : le sac a 5 rangees de 8 cases carrees dans l'ordre des indices") {
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, s_vueMinimale);
    const sf::FloatRect derniereDeLaRangee = rectangleDeLaCaseDuSac(s_colonnesDuSac - 1, s_vueMinimale);
    const sf::FloatRect premiereDeLaSuivante = rectangleDeLaCaseDuSac(s_colonnesDuSac, s_vueMinimale);

    REQUIRE(s_colonnesDuSac == 8);
    REQUIRE(s_casesDuSac / s_colonnesDuSac == 5);
    REQUIRE(premiere.size.x == premiere.size.y);
    REQUIRE(derniereDeLaRangee.position.y == premiere.position.y);
    REQUIRE(derniereDeLaRangee.position.x > premiere.position.x);
    REQUIRE(premiereDeLaSuivante.position.x == premiere.position.x);
    REQUIRE(premiereDeLaSuivante.position.y > premiere.position.y);
    REQUIRE(rectangleDeLaCaseDuSac(s_casesDuSac - 1, s_vueMinimale).position.y > premiereDeLaSuivante.position.y);
}

TEST_CASE("disposition : le sac est a droite de la colonne, toujours le meme, sous son etiquette") {
    const sf::FloatRect etiquette = rectangleDeLEtiquetteDuSac(s_vueMinimale);

    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        const sf::FloatRect emplacement = rectangleDeLEmplacement(static_cast<Emplacement>(i), s_vueMinimale);
        REQUIRE(rectangleDeLaCaseDuSac(0, s_vueMinimale).position.x >= emplacement.position.x + emplacement.size.x);
    }
    REQUIRE(rectangleDeLaCaseDuSac(0, s_vueMinimale).position.y >= etiquette.position.y + etiquette.size.y);
    REQUIRE(rectangleDeLaCaseDuSac(0, s_vueMinimale).position.x == etiquette.position.x);
}

TEST_CASE("elementSous : le centre de chaque emplacement le designe, pour la place ouverte") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        for (const Place ouverte : s_places) {
            for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
                const Emplacement emplacement = static_cast<Emplacement>(i);

                const std::optional<PositionDObjet> element = elementSous(centre(rectangleDeLEmplacement(emplacement, vue)), ouverte, vue);

                REQUIRE(element == PositionDObjet{PositionPortee{ouverte, emplacement}});
            }
        }
    }
}

TEST_CASE("elementSous : le centre de chaque case du sac la designe, quelle que soit la place ouverte") {
    for (const Place ouverte : s_places) {
        for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
            const std::optional<PositionDObjet> element = elementSous(centre(rectangleDeLaCaseDuSac(indice, s_vueMinimale)), ouverte, s_vueMinimale);

            REQUIRE(element == PositionDObjet{PositionDansLeSac{indice}});
        }
    }
}

TEST_CASE("elementSous : rien dans un ecart, sur un onglet, sur l'armure ou hors des elements") {
    REQUIRE_FALSE(elementSous({5.0f, 5.0f}, s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(elementSous(centre(rectangleDeLOnglet(Place{1}, s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(elementSous(centre(rectangleDeLArmure(s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
    // L'écart entre deux cases du sac.
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, s_vueMinimale);
    REQUIRE_FALSE(elementSous({premiere.position.x + premiere.size.x + 1.0f, premiere.position.y + 1.0f}, s_ouverte, s_vueMinimale).has_value());
}

TEST_CASE("ongletSous : le centre de chaque onglet le designe") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        for (const Place place : s_places) {
            REQUIRE(ongletSous(centre(rectangleDeLOnglet(place, vue)), vue) == place);
        }
    }
}

TEST_CASE("ongletSous : rien dans l'ecart entre deux onglets, sous eux ou hors du panneau") {
    const sf::FloatRect premier = rectangleDeLOnglet(Place{1}, s_vueMinimale);

    REQUIRE_FALSE(ongletSous({premier.position.x + premier.size.x + 1.0f, centre(premier).y}, s_vueMinimale).has_value());  // l'écart
    REQUIRE_FALSE(ongletSous({centre(premier).x, premier.position.y + premier.size.y + 1.0f}, s_vueMinimale).has_value());  // sous l'onglet
    REQUIRE_FALSE(ongletSous({centre(premier).x, premier.position.y - 1.0f}, s_vueMinimale).has_value());                   // au-dessus
    REQUIRE_FALSE(ongletSous({1.0f, 1.0f}, s_vueMinimale).has_value());
    REQUIRE_FALSE(ongletSous(centre(rectangleDeLEmplacement(Emplacement::Tete, s_vueMinimale)), s_vueMinimale).has_value());
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

TEST_CASE("disposition : les lignes du bloc des caracteristiques tiennent dans le bloc, en une colonne de haut en bas") {
    const sf::FloatRect bloc = rectangleDesCaracteristiques(s_vueMinimale);

    for (std::size_t cellule = 0; cellule <= s_nombreDeCaracteristiques; ++cellule) {
        const sf::FloatRect zone = rectangleDeLaCellule(cellule, s_vueMinimale);
        REQUIRE(zone.position.x == bloc.position.x);
        REQUIRE(zone.size.x == bloc.size.x);
        REQUIRE(zone.position.y >= bloc.position.y);
        REQUIRE(zone.position.y + zone.size.y <= bloc.position.y + bloc.size.y);
        if (cellule > 0) {
            REQUIRE(zone.position.y >= rectangleDeLaCellule(cellule - 1, s_vueMinimale).position.y + zone.size.y);
        }
    }
    REQUIRE(s_lignesDeCaracteristiques == s_nombreDeCaracteristiques + 1);
}

TEST_CASE("disposition : les cellules du bloc des statistiques tiennent dans le bloc, en deux colonnes") {
    const sf::FloatRect bloc = rectangleDesStatistiques(s_vueMinimale);

    for (std::size_t cellule = 0; cellule < s_cellulesDeStatistiques; ++cellule) {
        const sf::FloatRect zone = rectangleDeLaStatistique(cellule, s_vueMinimale);
        REQUIRE(zone.position.x >= bloc.position.x);
        REQUIRE(zone.position.y >= bloc.position.y);
        REQUIRE(zone.position.x + zone.size.x <= bloc.position.x + bloc.size.x);
        REQUIRE(zone.position.y + zone.size.y <= bloc.position.y + bloc.size.y);
    }
    REQUIRE(rectangleDeLaStatistique(0, s_vueMinimale).position.x == bloc.position.x);
    REQUIRE(rectangleDeLaStatistique(s_lignesDeStatistiques, s_vueMinimale).position.x > bloc.position.x);
    REQUIRE(rectangleDeLaStatistique(1, s_vueMinimale).position.y > rectangleDeLaStatistique(0, s_vueMinimale).position.y);
}

TEST_CASE("disposition : de haut en bas, les onglets, l'armure, les emplacements, les caracteristiques puis les statistiques") {
    const sf::FloatRect onglet = rectangleDeLOnglet(Place{1}, s_vueMinimale);
    const sf::FloatRect armure = rectangleDeLArmure(s_vueMinimale);
    const sf::FloatRect dernierEmplacement = rectangleDeLEmplacement(Emplacement::Bottes, s_vueMinimale);
    const sf::FloatRect caracteristiques = rectangleDesCaracteristiques(s_vueMinimale);
    const sf::FloatRect statistiques = rectangleDesStatistiques(s_vueMinimale);

    REQUIRE(armure.position.y >= onglet.position.y + onglet.size.y);
    REQUIRE(rectangleDeLEmplacement(Emplacement::Tete, s_vueMinimale).position.y >= armure.position.y + armure.size.y);
    REQUIRE(caracteristiques.position.y >= dernierEmplacement.position.y + dernierEmplacement.size.y);
    REQUIRE(statistiques.position.y >= caracteristiques.position.y + caracteristiques.size.y);
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

TEST_CASE("le panneau : plus etroit que les trois colonnes d'avant (1076 px)") {
    REQUIRE(rectangleDuPanneau(s_vueMinimale).size.x < 1076.0f);
}

// Les boutons des points libres (étape 100.2).
TEST_CASE("disposition : les boutons + et - sont dans la ligne de leur caracteristique, a droite, sans se toucher") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        for (const Caracteristique caracteristique : s_caracteristiques) {
            const sf::FloatRect ligne = rectangleDeLaCellule(static_cast<std::size_t>(caracteristique), vue);
            const sf::FloatRect plus = rectangleDuBoutonPlus(caracteristique, vue);
            const sf::FloatRect moins = rectangleDuBoutonMoins(caracteristique, vue);

            for (const sf::FloatRect& bouton : {plus, moins}) {
                REQUIRE(bouton.position.x >= ligne.position.x);
                REQUIRE(bouton.position.y >= ligne.position.y);
                REQUIRE(bouton.position.x + bouton.size.x <= ligne.position.x + ligne.size.x);
                REQUIRE(bouton.position.y + bouton.size.y <= ligne.position.y + ligne.size.y);
            }
            REQUIRE(moins.position.x + moins.size.x < plus.position.x);  // le « − » est à gauche du « + »
            REQUIRE(moins.position.x > ligne.position.x + ligne.size.x / 2.0f);  // et il reste de la place pour le texte
        }
    }
}

TEST_CASE("disposition : Apply et Reset sont dans la ligne des points de vie, a droite, sans se toucher") {
    const sf::FloatRect ligne = rectangleDeLaCellule(s_nombreDeCaracteristiques, s_vueMinimale);
    const sf::FloatRect appliquer = rectangleDuBoutonAppliquer(s_vueMinimale);
    const sf::FloatRect reinitialiser = rectangleDuBoutonReinitialiser(s_vueMinimale);

    for (const sf::FloatRect& bouton : {appliquer, reinitialiser}) {
        REQUIRE(bouton.position.y >= ligne.position.y);
        REQUIRE(bouton.position.y + bouton.size.y <= ligne.position.y + ligne.size.y);
        REQUIRE(bouton.position.x >= ligne.position.x + ligne.size.x / 2.0f);
        REQUIRE(bouton.position.x + bouton.size.x <= ligne.position.x + ligne.size.x);
    }
    REQUIRE(appliquer.position.x + appliquer.size.x < reinitialiser.position.x);
}

TEST_CASE("boutonSous : le centre de chaque bouton le designe, avec la place ouverte") {
    for (const Place ouverte : s_places) {
        for (const Caracteristique caracteristique : s_caracteristiques) {
            REQUIRE(boutonSous(centre(rectangleDuBoutonPlus(caracteristique, s_vueMinimale)), ouverte, s_vueMinimale) ==
                    BoutonDePoints{BoutonPlus{ouverte, caracteristique}});
            REQUIRE(boutonSous(centre(rectangleDuBoutonMoins(caracteristique, s_vueMinimale)), ouverte, s_vueMinimale) ==
                    BoutonDePoints{BoutonMoins{ouverte, caracteristique}});
        }
        REQUIRE(boutonSous(centre(rectangleDuBoutonAppliquer(s_vueMinimale)), ouverte, s_vueMinimale) == BoutonDePoints{BoutonAppliquer{ouverte}});
        REQUIRE(boutonSous(centre(rectangleDuBoutonReinitialiser(s_vueMinimale)), ouverte, s_vueMinimale) ==
                BoutonDePoints{BoutonReinitialiser{ouverte}});
    }
}

TEST_CASE("boutonSous : rien sur le texte d'une caracteristique, un emplacement, un onglet ou en dehors du panneau") {
    const sf::FloatRect ligne = rectangleDeLaCellule(0, s_vueMinimale);

    REQUIRE_FALSE(boutonSous(ligne.position + sf::Vector2f{10.0f, 5.0f}, s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(boutonSous(centre(rectangleDeLEmplacement(Emplacement::Torse, s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(boutonSous(centre(rectangleDeLOnglet(Place{1}, s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(boutonSous({1.0f, 1.0f}, s_ouverte, s_vueMinimale).has_value());
}

TEST_CASE("caracteristiqueSous : toute la ligne d'une caracteristique la designe, boutons compris, pour la place ouverte") {
    for (const Place ouverte : s_places) {
        for (const Caracteristique caracteristique : s_caracteristiques) {
            const sf::FloatRect ligne = rectangleDeLaCellule(static_cast<std::size_t>(caracteristique), s_vueMinimale);
            const CaracteristiqueSurvolee attendue{ouverte, caracteristique};

            REQUIRE(caracteristiqueSous(ligne.position + sf::Vector2f{3.0f, 3.0f}, ouverte, s_vueMinimale) == attendue);
            REQUIRE(caracteristiqueSous(centre(ligne), ouverte, s_vueMinimale) == attendue);
            REQUIRE(caracteristiqueSous(centre(rectangleDuBoutonPlus(caracteristique, s_vueMinimale)), ouverte, s_vueMinimale) == attendue);
        }
    }
}

TEST_CASE("caracteristiqueSous : rien sur la ligne des points de vie, les statistiques ou un emplacement") {
    REQUIRE_FALSE(caracteristiqueSous(centre(rectangleDeLaCellule(s_nombreDeCaracteristiques, s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(caracteristiqueSous(centre(rectangleDeLaStatistique(0, s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
    REQUIRE_FALSE(caracteristiqueSous(centre(rectangleDeLEmplacement(Emplacement::Tete, s_vueMinimale)), s_ouverte, s_vueMinimale).has_value());
}
