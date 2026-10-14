// ./tests/test_Personnage.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Personnage.hpp"
#include "Cercle.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"

namespace {

// Carte dégagée dédiée aux tests de recharge : le personnage ne bouge
// pas (cible = son propre centre), seul le temps doit s'écouler.
TileMap carteDegagee5x5() {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    return TileMap(plan, 40.0f);
}

void ecoulerTemps(Personnage& personnage, const TileMap& carte, sf::Vector2f centre, float duree) {
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        personnage.mettreAJour(s_dureeTic, centre, carte, {});
    }
}

}  // namespace

// Personnage centré en (100, 100), portée 80 px.

TEST_CASE("estAPortee renvoie vrai pour un point proche") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE(personnage.estAPortee(pointCible({150.0f, 100.0f})));
}

TEST_CASE("estAPortee renvoie faux pour un point lointain") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE_FALSE(personnage.estAPortee(pointCible({300.0f, 100.0f})));
}

TEST_CASE("frapper renvoie les degats pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE(personnage.frapper(pointCible({150.0f, 100.0f}), generateur) == 5);
}

TEST_CASE("frapper renvoie 0 pour une cible hors de portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE(personnage.frapper(pointCible({300.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("frapper renvoie de nouveau les degats apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(personnage.frapper(cible, generateur) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(personnage.frapper(cible, generateur) == 5);
}

TEST_CASE("Un personnage est vivant a la creation") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est encore vivant apres 80 degats") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    personnage.recevoirDegats(80);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est mort apres 100 degats") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    personnage.recevoirDegats(100);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("getCentre renvoie le centre donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE(personnage.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le personnage, de rayon la moitie du cote") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const Cercle cercle = personnage.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 10.0f);
}

TEST_CASE("Un personnage est mort apres un coup qui depasse le reste de ses points de vie") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    personnage.recevoirDegats(80);
    personnage.recevoirDegats(50);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("Un personnage mort ne frappe plus") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const Cercle cible = pointCible({150.0f, 100.0f});

    personnage.recevoirDegats(100);

    REQUIRE(personnage.frapper(cible, generateur) == 0);
}

TEST_CASE("Un personnage mort ne bouge plus") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const TileMap carte = carteDegagee5x5();
    const sf::Vector2f centreInitial = personnage.getCentre();

    personnage.recevoirDegats(100);
    ecoulerTemps(personnage, carte, {500.0f, 500.0f}, 2.0f); // cible lointaine, durée généreuse

    REQUIRE(personnage.getCentre() == centreInitial);
}

// Skill : portée 80 px comme l'attaque de base, 25 dégâts, 5 s de recharge.

TEST_CASE("declencherSkill renvoie les degats du skill pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 5.0f, 0.0f), 0);

    REQUIRE(personnage.declencherSkill(pointCible({150.0f, 100.0f}), generateur) == 25);
}

TEST_CASE("Un personnage avec un skill de provocation en expose le type et le declenche") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 20.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Provocation, 0.0f, 4, 10.0f, 10.0f, 150.0f), 0);

    REQUIRE(personnage.typeSkill() == TypeSkill::Provocation);
    REQUIRE(personnage.skillEstPret());
    REQUIRE(personnage.declencherSkill(personnage.getCercle(), generateur) == 4);
    REQUIRE_FALSE(personnage.skillEstPret());
}

TEST_CASE("zoneDuSkill est centree sur le personnage, au rayon de zone du skill") {
    Personnage provocateur({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 20.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Provocation, 0.0f, 4, 10.0f, 10.0f, 150.0f), 0);
    Personnage sansSkill({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 20.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    REQUIRE(provocateur.zoneDuSkill().centre == provocateur.getCentre());
    REQUIRE(provocateur.zoneDuSkill().rayon == 150.0f);
    REQUIRE(sansSkill.zoneDuSkill().rayon == 0.0f);
}

TEST_CASE("declencherSkill renvoie 0 pour un personnage sans skill, sans toucher a l'attaque de base") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(personnage.declencherSkill(cible, generateur) == 0);
    REQUIRE(personnage.frapper(cible, generateur) == 5);
}

TEST_CASE("declencherSkill renvoie 0 pour un personnage mort") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 5.0f, 0.0f), 0);

    personnage.recevoirDegats(100);

    REQUIRE(personnage.declencherSkill(pointCible({150.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("declencherSkill renvoie 0 hors de portee, sans toucher a l'attaque de base") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 5.0f, 0.0f), 0);

    REQUIRE(personnage.declencherSkill(pointCible({300.0f, 100.0f}), generateur) == 0);
    REQUIRE(personnage.frapper(pointCible({150.0f, 100.0f}), generateur) == 5);
}

TEST_CASE("declencherSkill renvoie 0 pendant sa recharge, puis de nouveau les degats une fois ecoulee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 5.0f, 0.0f), 0);
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(personnage.declencherSkill(cible, generateur) == 25);
    REQUIRE(personnage.declencherSkill(cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 4.8f);
    REQUIRE(personnage.declencherSkill(cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.4f);
    REQUIRE(personnage.declencherSkill(cible, generateur) == 25);
}

TEST_CASE("Apres un skill reussi, l'attaque de base attend une recharge complete") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), Skill(TypeSkill::Degats, 80.0f, 25, 5.0f, 5.0f, 0.0f), 0);
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    // Attaque de base lancée, puis 0,3 s écoulées : il ne lui reste que
    // 0,2 s de recharge. Le skill la relance au complet (0,5 s) : 0,3 s
    // plus tard, elle ne doit toujours pas partir, alors qu'elle l'aurait
    // fait sans le skill.
    REQUIRE(personnage.frapper(cible, generateur) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.declencherSkill(cible, generateur) == 25);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.frapper(cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.frapper(cible, generateur) == 5);
}

TEST_CASE("mettreAJour s'arrete avant d'entrer dans un obstacle") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const std::vector<Cercle> obstacles = {{{140.0f, 100.0f}, 15.0f}};

    for (int i = 0; i < 60; ++i) {
        personnage.mettreAJour(1.0f / 60.0f, {170.0f, 100.0f}, carte, obstacles);
    }

    // distanceBordABord est bornée à 0 : on compare donc les distances
    // entre centres et la somme des rayons.
    const float ecartCentres = (personnage.getCentre() - obstacles[0].centre).length();
    REQUIRE(ecartCentres >= personnage.getCercle().rayon + obstacles[0].rayon - 0.001f);
    REQUIRE(personnage.getCentre().x > 100.0f);
    REQUIRE(personnage.getCentre().x < 140.0f);
}

TEST_CASE("recevoirSoin rend des points de vie") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    personnage.recevoirDegats(50);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == 80);
}

TEST_CASE("recevoirDegats applique l'armure du personnage") {
    Personnage melee({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 50);
    Personnage tireur({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    melee.recevoirDegats(10);
    tireur.recevoirDegats(10);

    REQUIRE(melee.getPointsDeVie() == 93);   // 10 x 100 / 150 = 6,67 -> 7
    REQUIRE(tireur.getPointsDeVie() == 90);
}

TEST_CASE("recevoirSoin n'est pas reduit par l'armure") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 50);
    personnage.recevoirDegats(60);  // 60 x 2/3 = 40 retirés

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == 90);
}

TEST_CASE("recevoirSoin ne depasse pas le maximum") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    personnage.recevoirDegats(10);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

TEST_CASE("recevoirSoin est sans effet sur un mort") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    personnage.recevoirDegats(100);

    personnage.recevoirSoin(30);

    REQUIRE_FALSE(personnage.estVivant());
    REQUIRE(personnage.getPointsDeVie() == 0);
}

TEST_CASE("En terrain degage, le personnage avance en ligne droite de vitesse * dt") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage({60.0f, 60.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    for (int i = 0; i < 10; ++i) {
        personnage.mettreAJour(1.0f / 60.0f, {180.0f, 60.0f}, carte, {});
    }

    // 240 px/s * (1/60) s = 4 px par tic.
    REQUIRE_THAT(personnage.getCentre().x, Catch::Matchers::WithinAbs(100.0f, 0.01f));
    REQUIRE_THAT(personnage.getCentre().y, Catch::Matchers::WithinAbs(60.0f, 0.01f));
}

TEST_CASE("Un personnage separe de sa cible par un mur avec une ouverture finit par l'atteindre") {
    const TileMap carte({"#######",
                         "#..#..#",
                         "#..#..#",
                         "#.....#",
                         "#######"},
                        40.0f);
    Personnage personnage({60.0f, 60.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);
    const sf::Vector2f cible{220.0f, 60.0f};

    ecoulerTemps(personnage, carte, cible, 5.0f);

    REQUIRE_THAT(personnage.getCentre().x, Catch::Matchers::WithinAbs(cible.x, 1.0f));
    REQUIRE_THAT(personnage.getCentre().y, Catch::Matchers::WithinAbs(cible.y, 1.0f));
}

TEST_CASE("Sans aucun chemin possible, le personnage ne plante pas et reste bloque contre le mur") {
    const TileMap carte({"#####",
                         "#.#.#",
                         "#####"},
                        40.0f);
    Personnage personnage({60.0f, 60.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    ecoulerTemps(personnage, carte, {140.0f, 60.0f}, 2.0f);

    REQUIRE(personnage.getCentre().x <= 70.01f);
    REQUIRE(personnage.estVivant());
}

namespace {
// Deux pièces séparées par un mur (colonne 3) avec une ouverture en bas.
TileMap deuxPiecesAvecPorte() {
    return TileMap({"#######",
                    "#..#..#",
                    "#..#..#",
                    "#.....#",
                    "#######"},
                   40.0f);
}
}  // namespace

TEST_CASE("Une cible qui bouge de moins d'une case ne fait pas recalculer le chemin") {
    const TileMap carte = deuxPiecesAvecPorte();
    Personnage personnage({60.0f, 60.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    // Nouvelle cible à 30 px de la précédente, mais dans le mur de droite :
    // un recalcul ne trouverait aucun chemin et le personnage irait tout
    // droit contre le mur du milieu. Le chemin conservé le mène par la porte.
    ecoulerTemps(personnage, carte, {250.0f, 60.0f}, 1.0f);

    REQUIRE(personnage.getCentre().x > 160.0f);
}

TEST_CASE("Une cible qui bouge de plus d'une case fait recalculer le chemin") {
    const TileMap carte = deuxPiecesAvecPorte();
    Personnage personnage({60.0f, 60.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    // Nouvelle cible à 50 px, dans le mur : le chemin est recalculé, le A*
    // n'en trouve pas, et le personnage va tout droit contre le mur du milieu.
    ecoulerTemps(personnage, carte, {270.0f, 60.0f}, 1.0f);

    REQUIRE(personnage.getCentre().x <= 110.01f);
}

TEST_CASE("Quand le trajet direct redevient libre, le chemin en cours est abandonne") {
    const TileMap carte = deuxPiecesAvecPorte();
    Personnage personnage({60.0f, 60.0f}, sf::Color::White, Skill(TypeSkill::Degats, 80.0f, 5, 0.5f, 0.5f, 0.0f), std::nullopt, 0);

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    ecoulerTemps(personnage, carte, {60.0f, 100.0f}, 1.0f);

    REQUIRE_THAT(personnage.getCentre().x, Catch::Matchers::WithinAbs(60.0f, 1.0f));
    REQUIRE_THAT(personnage.getCentre().y, Catch::Matchers::WithinAbs(100.0f, 1.0f));
}
