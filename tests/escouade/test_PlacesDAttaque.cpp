// ./tests/escouade/test_PlacesDAttaque.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"
#include "escouade/PlacesDAttaque.hpp"
#include "monde/ChampDeVision.hpp"
#include "monde/TileMap.hpp"

namespace {

// Valeurs de test, indépendantes de celles du jeu.
constexpr float s_ecartDeTest = 30.0f;
constexpr float s_rayonAssaillantDeTest = 10.0f;
constexpr float s_porteeDeTest = 20.0f;
constexpr float s_porteeTireurDeTest = 120.0f;
constexpr std::size_t s_nombreDAssaillants = 3;
constexpr int s_idCible = 7;
constexpr int s_idAutreCible = 8;
constexpr float s_tolerance = 0.01f;

const EntiteVue s_cible{s_idCible, {{200.0f, 200.0f}, 15.0f}};
const sf::Vector2f s_ouest(100.0f, 200.0f);  // à l'ouest de la cible : le point de référence des tests
const sf::Vector2f s_sud(200.0f, 300.0f);    // au sud de la cible (y vers le bas)

using Occupants = std::vector<std::optional<sf::Vector2f>>;

Occupants aucunOccupant() {
    return Occupants(s_nombreDAssaillants);
}

PlacesDAttaque::Approche approche(std::size_t assaillant, sf::Vector2f position, EntiteVue cible = s_cible,
                                  float portee = s_porteeDeTest) {
    return {
        .cible = cible,
        .assaillant = assaillant,
        .cercleAssaillant = {position, s_rayonAssaillantDeTest},
        .portee = portee,
        .reference = s_ouest,
    };
}

// À portée de la cible sans la toucher : écart bord à bord entre 0 (exclu) et la portée.
bool estAPorteeSansContact(sf::Vector2f place, Cercle cible, float portee = s_porteeDeTest) {
    const float ecart = distanceBordABord({place, s_rayonAssaillantDeTest}, cible);
    return ecart > 0.0f && ecart <= portee;
}

bool chevaucheUnMur(const TileMap& carte, sf::Vector2f place) {
    const sf::Vector2f demiCote(s_rayonAssaillantDeTest, s_rayonAssaillantDeTest);
    return carte.chevaucheUnMur(sf::FloatRect(place - demiCote, 2.0f * demiCote));
}

}  // namespace

TEST_CASE("PlacesDAttaque : face a la reference, la place est dans sa direction, a portee") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);

    const auto place = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carteDegagee10x10());

    REQUIRE(place.has_value());
    REQUIRE(place->x < s_cible.cercle.centre.x);
    REQUIRE(std::abs(place->y - s_cible.cercle.centre.y) <= s_tolerance);
    REQUIRE(estAPorteeSansContact(*place, s_cible.cercle));
}

TEST_CASE("PlacesDAttaque : la place est la candidate la plus proche de l'assaillant") {
    // Les candidates partent de l'ouest (la référence), l'assaillant est au sud : il prend la place
    // du sud, à moins d'un demi-écart angulaire entre deux places.
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);

    const auto place = places.placer(approche(0, s_sud), std::nullopt, aucunOccupant(), carteDegagee10x10());

    REQUIRE(place.has_value());
    const sf::Angle angle = (*place - s_cible.cercle.centre).angle();
    REQUIRE(std::abs((angle - sf::degrees(90.0f)).wrapSigned().asDegrees()) <= 45.0f);
    REQUIRE(estAPorteeSansContact(*place, s_cible.cercle));
}

TEST_CASE("PlacesDAttaque : deux assaillants de la meme cible ont des places distinctes") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    const TileMap carte = carteDegagee10x10();

    const auto premiere = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);
    const auto seconde = places.placer(approche(1, s_ouest), std::nullopt, aucunOccupant(), carte);

    REQUIRE(premiere.has_value());
    REQUIRE(seconde.has_value());
    REQUIRE((*seconde - *premiere).length() > s_ecartDeTest);
    REQUIRE(estAPorteeSansContact(*seconde, s_cible.cercle));
}

TEST_CASE("PlacesDAttaque : les places retenues autour d'une autre cible ne genent pas") {
    // Deux cibles au même endroit : la place du premier assaillant n'est pas autour de la cible du second.
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    const TileMap carte = carteDegagee10x10();
    const EntiteVue autreCible{s_idAutreCible, s_cible.cercle};

    const auto premiere = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);
    const auto seconde = places.placer(approche(1, s_ouest, autreCible), std::nullopt, aucunOccupant(), carte);

    REQUIRE(premiere.has_value());
    REQUIRE(seconde.has_value());
    verifierPointProche(*seconde, *premiere, s_tolerance);
}

TEST_CASE("PlacesDAttaque : une place retiree ne gene plus les autres assaillants") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    const TileMap carte = carteDegagee10x10();

    const auto premiere = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);
    places.retirer(0);  // la place du premier est oubliée
    const auto seconde = places.placer(approche(1, s_ouest), std::nullopt, aucunOccupant(), carte);

    REQUIRE(premiere.has_value());
    REQUIRE(seconde.has_value());
    verifierPointProche(*seconde, *premiere, s_tolerance);
}

TEST_CASE("PlacesDAttaque : l'angle de l'ancienne place est garde quand la cible bouge") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    const TileMap carte = carteDegagee10x10();
    const auto avant = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);
    const std::optional<PlacesDAttaque::Place> ancienne = places.retirer(0);

    // La cible part vers le sud-est : sans l'ancienne place, la nouvelle serait dans la direction de
    // la référence, plus au nord.
    const sf::Vector2f deplacement(40.0f, 60.0f);
    const EntiteVue cibleDeplacee{s_idCible, {s_cible.cercle.centre + deplacement, s_cible.cercle.rayon}};
    const auto apres = places.placer(approche(0, s_ouest, cibleDeplacee), ancienne, aucunOccupant(), carte);

    REQUIRE(avant.has_value());
    REQUIRE(apres.has_value());
    verifierPointProche(*apres, *avant + deplacement, s_tolerance);
}

TEST_CASE("PlacesDAttaque : une ancienne place autour d'une autre cible n'est pas gardee") {
    const TileMap carte = carteDegagee10x10();
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    // Seule la place retenue compte ici : au sud de la cible, du côté de l'assaillant.
    static_cast<void>(places.placer(approche(0, s_sud), std::nullopt, aucunOccupant(), carte));
    const std::optional<PlacesDAttaque::Place> ancienne = places.retirer(0);

    const EntiteVue autreCible{s_idAutreCible, s_cible.cercle};
    const auto place = places.placer(approche(0, s_ouest, autreCible), ancienne, aucunOccupant(), carte);

    // Le même calcul sans ancienne place.
    PlacesDAttaque sansMemoire(s_nombreDAssaillants, s_ecartDeTest);
    const auto attendue = sansMemoire.placer(approche(0, s_ouest, autreCible), std::nullopt, aucunOccupant(), carte);
    REQUIRE(place.has_value());
    REQUIRE(attendue.has_value());
    verifierPointProche(*place, *attendue, s_tolerance);
    REQUIRE(places.cibleApprochee(0) == s_idAutreCible);
}

TEST_CASE("PlacesDAttaque : une ancienne place occupee par un autre est reattribuee") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    const TileMap carte = carteDegagee10x10();
    const auto avant = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);
    const std::optional<PlacesDAttaque::Place> ancienne = places.retirer(0);
    REQUIRE(avant.has_value());

    Occupants occupants = aucunOccupant();
    occupants[1] = *avant;  // un autre se tient sur l'ancienne place
    const auto apres = places.placer(approche(0, s_ouest), ancienne, occupants, carte);

    REQUIRE(apres.has_value());
    REQUIRE((*apres - *avant).length() > s_ecartDeTest);
}

TEST_CASE("PlacesDAttaque : l'assaillant n'occupe pas sa propre place") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    const TileMap carte = carteDegagee10x10();
    const auto avant = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);
    const std::optional<PlacesDAttaque::Place> ancienne = places.retirer(0);
    REQUIRE(avant.has_value());

    Occupants occupants = aucunOccupant();
    occupants[0] = *avant;  // l'assaillant lui-même, arrivé à sa place
    const auto apres = places.placer(approche(0, s_ouest), ancienne, occupants, carte);

    REQUIRE(apres.has_value());
    verifierPointProche(*apres, *avant, s_tolerance);
}

TEST_CASE("PlacesDAttaque : aucune place dans un mur") {
    // Mur sur toute la colonne 3 (x de 120 à 160 px) : la place à l'ouest de la cible, face à la
    // référence, chevaucherait le mur.
    const TileMap carte(std::vector<std::string>(10, "...#......"), 40.0f);
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);

    const auto place = places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carte);

    REQUIRE(place.has_value());
    REQUIRE_FALSE(chevaucheUnMur(carte, *place));
    REQUIRE(estAPorteeSansContact(*place, s_cible.cercle));
}

TEST_CASE("PlacesDAttaque : aucune place sans vue sur la cible") {
    // Mur sur toute la colonne 4 (x de 160 à 200 px), cible à l'est du mur. L'anneau d'un tireur passe
    // de l'autre côté : la place face à la référence ne touche pas le mur, mais ne voit pas la cible.
    const TileMap carte(std::vector<std::string>(10, "....#....."), 40.0f);
    const EntiteVue cible{s_idCible, {{260.0f, 200.0f}, 15.0f}};
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);

    const auto place = places.placer(approche(0, s_ouest, cible, s_porteeTireurDeTest), std::nullopt, aucunOccupant(), carte);

    REQUIRE(place.has_value());
    REQUIRE(ligneDeVue(carte, *place, cible.cercle.centre));
    REQUIRE_FALSE(chevaucheUnMur(carte, *place));
    REQUIRE(estAPorteeSansContact(*place, cible.cercle, s_porteeTireurDeTest));
}

TEST_CASE("PlacesDAttaque : sans place libre, rien n'est renvoye et la place retenue ne change pas") {
    // La cible est enfermée dans une case : tout l'anneau est dans les murs.
    std::vector<std::string> plan(10, "..........");
    plan[4] = "....###...";
    plan[5] = "....#.#...";
    plan[6] = "....###...";
    const TileMap carte(plan, 40.0f);
    const EntiteVue cibleEnfermee{s_idCible, {{220.0f, 220.0f}, 15.0f}};
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    places.retenir(0, PlacesDAttaque::Place{s_idAutreCible, 50.0f, sf::degrees(0.0f)});

    const auto place = places.placer(approche(0, s_ouest, cibleEnfermee), std::nullopt, aucunOccupant(), carte);

    REQUIRE_FALSE(place.has_value());
    REQUIRE(places.cibleApprochee(0) == s_idAutreCible);
}

TEST_CASE("PlacesDAttaque : retirer renvoie la place retenue et l'oublie, retenir la remet") {
    PlacesDAttaque places(s_nombreDAssaillants, s_ecartDeTest);
    REQUIRE_FALSE(places.cibleApprochee(0).has_value());

    // Seule la place retenue compte ici.
    static_cast<void>(places.placer(approche(0, s_ouest), std::nullopt, aucunOccupant(), carteDegagee10x10()));
    REQUIRE(places.cibleApprochee(0) == s_idCible);

    const std::optional<PlacesDAttaque::Place> retiree = places.retirer(0);
    REQUIRE(retiree.has_value());
    REQUIRE(retiree->idCible == s_idCible);
    REQUIRE_FALSE(places.cibleApprochee(0).has_value());

    places.retenir(0, retiree);
    REQUIRE(places.cibleApprochee(0) == s_idCible);
}
