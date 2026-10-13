// ./tests/test_Escouade.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "Cercle.hpp"
#include "Cible.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"

namespace {

// Cible ponctuelle (rayon nul), pour les tests qui ne portent pas sur
// la forme de la cible elle-même.
Cercle pointCible(sf::Vector2f centre) {
    return {centre, 0.0f};
}

// Cible de la taille d'un monstre (rayon 15 px), pour les tests de combat
// où la taille de la cible décide si les mêlées peuvent l'atteindre.
Cercle cibleMonstre(sf::Vector2f centre) {
    return {centre, 15.0f};
}

// Le monstre d'identifiant 1 tel que l'escouade le voit, seul dans la liste.
std::vector<MonstreVu> unSeulMonstre(Cercle cercle) {
    return {MonstreVu{1, cercle}};
}

// Déclenche le skill de cette place et renvoie le total des dégâts de la
// liste de coups.
int declencher(Escouade& escouade, std::size_t place, const std::vector<MonstreVu>& monstres) {
    int total = 0;
    for (const Coup& coup : escouade.declencherSkill(place, monstres)) {
        total += coup.degats;
    }
    return total;
}

// Compare un point à un point attendu avec une tolérance : les positions
// manipulées par Escouade sont des flottants accumulés sur plusieurs tics
// (vitesse * dt, racines carrées de sf::Vector2f::length...), donc deux
// calculs mathématiquement égaux peuvent différer de quelques poussières
// d'erreur d'arrondi. Comparer avec `==` serait fragile.
void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

// Simule le temps qui passe par tics de 1/60 s (comme setFramerateLimit(60)
// dans main.cpp), pendant la durée demandée. Les tests passent une durée
// généreuse par rapport au trajet attendu et ne vérifient que l'état final.
void simuler(Escouade& escouade, const TileMap& carte, float dureeSecondes) {
    constexpr float dt = 1.0f / 60.0f;
    const int nombreTics = static_cast<int>(dureeSecondes / dt);
    for (int i = 0; i < nombreTics; ++i) {
        escouade.mettreAJour(dt, carte, {});
    }
}

// Appelle attaquer() avec un seul monstre (identifiant 1) et renvoie le
// total des dégâts de la liste de coups.
int attaquerCible(Escouade& escouade, const TileMap& carte, Cercle cible) {
    int total = 0;
    for (const Coup& coup : escouade.attaquer({MonstreVu{1, cible}}, carte)) {
        total += coup.degats;
    }
    return total;
}

// Simule attaquer() + mettreAJour() par tics de 1/60 s, et renvoie le
// total des dégâts cumulés sur toute la durée.
int simulerAttaque(Escouade& escouade, const TileMap& carte, Cercle cible, float dureeSecondes) {
    constexpr float dt = 1.0f / 60.0f;
    const int nombreTics = static_cast<int>(dureeSecondes / dt);
    int degats = 0;
    for (int i = 0; i < nombreTics; ++i) {
        degats += attaquerCible(escouade, carte, cible);
        escouade.mettreAJour(dt, carte, {});
    }
    return degats;
}

// Place de l'allié désigné par choisirCible, vide si aucune cible.
std::optional<std::size_t> placeAlliePourSoin(const Escouade& escouade) {
    const std::optional<Cible> cible = escouade.choisirCible(TypeSkill::Soin, 3, {});
    if (!cible.has_value()) {
        return std::nullopt;
    }
    return std::get<CibleAllie>(*cible).place;
}

} // namespace
TEST_CASE("Le centre initial est au centre du carré de départ") {
    const Escouade escouade({50.0f, 50.0f});

    verifierPointProche(escouade.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("Un seul tic avance le centre de vitesse * dt") {
    // Terrain dégagé, destination à plusieurs cases : le premier point visé
    // est donc loin, bien au-delà de la distance parcourue en un seul tic
    // (180 px/s * 0.1 s = 18 px), quelle que soit sa direction exacte.
    const std::vector<std::string> plan = {
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f centreInitial = escouade.getCentre();
    escouade.allerVers({300.0f, 300.0f}, carte);
    escouade.mettreAJour(0.1f, carte, {});

    const float distanceParcourue = (escouade.getCentre() - centreInitial).length();
    REQUIRE_THAT(distanceParcourue, Catch::Matchers::WithinAbs(18.0f, 0.5f));
}

TEST_CASE("En terrain dégagé, l'escouade finit centrée sur le point exact demandé") {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    // Point choisi hors de tout centre de case (les centres de case sont à
    // 20, 60, 100, 140... pour une taille de case de 40).
    const sf::Vector2f destination{137.0f, 251.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 10.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("Avec un mur entre le départ et la destination, l'escouade le contourne et l'atteint") {
    // Mur sur la rangée 3, colonnes 1 à 3 ; ouverture par la colonne 0 (et
    // par les colonnes 4 à 6). Départ en case (1, 1), destination en case
    // (1, 5) : directement sous le départ, mais le mur bloque le trajet
    // direct en colonne 1.
    const std::vector<std::string> plan = {
        ".......",
        ".......",
        ".......",
        ".###...",
        ".......",
        ".......",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f destination = carte.centreDeCase({1, 5});
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 10.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("Sans ordre, l'escouade ne bouge pas") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    for (int i = 0; i < 60; ++i) {
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    verifierPointProche(escouade.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("À l'arrêt, un ordre vers un mur ne fait pas bouger l'escouade") {
    // Mur en case (2, 2) ; départ en case (1, 1), comme pour tous les
    // autres tests.
    const std::vector<std::string> plan = {
        "....",
        "....",
        "..#.",
        "....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f centreInitial = escouade.getCentre();

    escouade.allerVers(carte.centreDeCase({2, 2}), carte);
    simuler(escouade, carte, 1.0f);

    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("Un ordre vers un mur pendant un déplacement n'interrompt pas le trajet en cours") {
    // Mur isolé en case (0, 0), loin du trajet entre le départ (1, 1) et
    // la première destination (case (5, 5)) : il ne sert qu'à fournir un
    // second ordre invalide, sans jamais bloquer le chemin réel.
    const std::vector<std::string> plan = {
        "#.....",
        "......",
        "......",
        "......",
        "......",
        "......",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f destination{220.0f, 220.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 0.3f); // en route, loin d'avoir fini

    escouade.allerVers(carte.centreDeCase({0, 0}), carte); // ordre invalide, ignoré
    simuler(escouade, carte, 10.0f); // largement assez pour finir le trajet d'origine

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("Un second ordre valide pendant un déplacement remplace la destination") {
    const std::vector<std::string> plan = {
        "........",
        "........",
        "........",
        "........",
        "........",
        "........",
        "........",
        "........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    escouade.allerVers({280.0f, 100.0f}, carte);
    simuler(escouade, carte, 0.3f); // en route vers la première destination

    const sf::Vector2f secondeDestination{100.0f, 280.0f};
    escouade.allerVers(secondeDestination, carte);
    simuler(escouade, carte, 10.0f); // largement assez pour atteindre la seconde

    verifierPointProche(escouade.getCentre(), secondeDestination);
}

TEST_CASE("attaquer renvoie 0 pour une cible lointaine et l'escouade s'en rapproche") {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const sf::Vector2f cible{380.0f, 380.0f};
    const float distanceInitiale = (cible - escouade.getCentre()).length();

    REQUIRE(attaquerCible(escouade, carte, pointCible(cible)) == 0);
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    const float distanceApres = (cible - escouade.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("attaquer finit par infliger des dégâts sans que l'escouade atteigne la cible") {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const sf::Vector2f cible{300.0f, 300.0f};
    const int degatsTotal = simulerAttaque(escouade, carte, cibleMonstre(cible), 5.0f);

    REQUIRE(degatsTotal > 0);
    REQUIRE((cible - escouade.getCentre()).length() > 1.0f);
}

TEST_CASE("attaquer ignore les personnages morts pour décider si l'escouade doit s'arrêter") {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    escouade.recevoirDegats(1, {1000.0f, 1000.0f}, 100); // tue le personnage le plus proche de ce point

    escouade.allerVers({300.0f, 60.0f}, carte); // éloigne l'escouade, laisse le mort derrière
    simuler(escouade, carte, 10.0f); // largement assez pour arriver
    const sf::Vector2f centreAvant = escouade.getCentre();

    // À portée des 3 vivants (qui ont suivi l'ancre), loin du mort resté
    // en arrière.
    const sf::Vector2f cible = centreAvant + sf::Vector2f(30.0f, 0.0f);

    REQUIRE(attaquerCible(escouade, carte, cibleMonstre(cible)) == 15); // 3 vivants x 5 degats
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    verifierPointProche(escouade.getCentre(), centreAvant);
}

TEST_CASE("attaquer renvoie les dégâts de tous les personnages pour une cible déjà proche") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Centre initial (60, 60) ; mêlées en (85, 35) et (85, 85), tireurs en
    // (35, 35) et (35, 85). Une cible de la taille d'un monstre en (85, 60)
    // est à 25 px du centre de chaque mêlée, soit 0 px bord à bord : les
    // 4 personnages sont déjà à portée.
    const sf::Vector2f cible{85.0f, 60.0f};
    const sf::Vector2f centreInitial = escouade.getCentre();

    REQUIRE(attaquerCible(escouade, carte, cibleMonstre(cible)) == 20);
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("Le lissage rend un trajet diagonal plus rapide qu'un escalier") {
    // Terrain dégagé, destination 240 px plus loin en x et en y. Sans
    // lissage, le A* produirait un escalier dont la longueur totale vaut
    // la distance de Manhattan (240 + 240 = 480 px, soit 480 / 180 ≈
    // 2,67 s). Avec lissage, le trajet devient la ligne droite directe,
    // de longueur euclidienne (240 * racine(2) ≈ 339,4 px, soit ≈ 1,89 s).
    // Une simulation de 2 s est donc suffisante pour la ligne droite, mais
    // insuffisante pour l'escalier : si l'escouade est bien arrivée après
    // seulement 2 s, c'est que le chemin a été lissé.
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f destination{300.0f, 300.0f};
    escouade.allerVers(destination, carte);
    simuler(escouade, carte, 2.0f);

    verifierPointProche(escouade.getCentre(), destination);
}

TEST_CASE("proieDuMonstre renvoie un centre plus proche du point que le centre de l'escouade") {
    const Escouade escouade({50.0f, 50.0f});

    const sf::Vector2f pointLointain{1000.0f, 1000.0f};
    const std::optional<Cercle> cerclePersonnage = escouade.proieDuMonstre(1, pointLointain);
    REQUIRE(cerclePersonnage.has_value());

    const float distancePersonnage = (cerclePersonnage->centre - pointLointain).length();
    const float distanceEscouade = (escouade.getCentre() - pointLointain).length();

    REQUIRE(distancePersonnage < distanceEscouade);
}

TEST_CASE("proieDuMonstre ignore les degats insuffisants pour tuer") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> avant = escouade.proieDuMonstre(1, point);
    REQUIRE(avant.has_value());

    escouade.recevoirDegats(1, point, 50);
    const std::optional<Cercle> apres = escouade.proieDuMonstre(1, point);
    REQUIRE(apres.has_value());

    REQUIRE(apres->centre == avant->centre);
}

TEST_CASE("proieDuMonstre change de cible une fois le plus proche mort") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> avant = escouade.proieDuMonstre(1, point);
    REQUIRE(avant.has_value());

    escouade.recevoirDegats(1, point, 100);
    const std::optional<Cercle> apres = escouade.proieDuMonstre(1, point);
    REQUIRE(apres.has_value());

    REQUIRE_FALSE(apres->centre == avant->centre);
}

TEST_CASE("proieDuMonstre est vide une fois tous les personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, point, 100);
    }

    REQUIRE_FALSE(escouade.proieDuMonstre(1, point).has_value());

    escouade.recevoirDegats(1, point, 100);
    REQUIRE_FALSE(escouade.proieDuMonstre(1, point).has_value());
}

TEST_CASE("estVivante est vraie a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante reste vraie apres la mort d'un seul personnage") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    escouade.recevoirDegats(1, point, 100);

    REQUIRE(escouade.estVivante());
}

TEST_CASE("estVivante devient fausse une fois les 4 personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, point, 100);
    }

    REQUIRE_FALSE(escouade.estVivante());
}

TEST_CASE("cerclesDesVivants renvoie 4 cercles a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.cerclesDesVivants().size() == 4);
}

TEST_CASE("cerclesDesVivants n'inclut plus le cercle d'un personnage mort") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    const std::optional<Cercle> futurMort = escouade.proieDuMonstre(1, point);
    REQUIRE(futurMort.has_value());

    escouade.recevoirDegats(1, point, 100);
    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();

    REQUIRE(cercles.size() == 3);
    for (const Cercle& cercle : cercles) {
        REQUIRE_FALSE(cercle.centre == futurMort->centre);
    }
}

TEST_CASE("cerclesDesVivants est vide une fois tous les personnages morts") {
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f point{1000.0f, 1000.0f};

    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, point, 100);
    }

    REQUIRE(escouade.cerclesDesVivants().empty());
}

TEST_CASE("getCibleVerrouillee est vide a la creation") {
    const Escouade escouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("verrouiller pose la cible donnee") {
    Escouade escouade({50.0f, 50.0f});

    escouade.verrouiller(3);

    REQUIRE(escouade.getCibleVerrouillee() == std::optional<int>(3));
}

TEST_CASE("verrouiller remplace une cible deja posee") {
    Escouade escouade({50.0f, 50.0f});

    escouade.verrouiller(3);
    escouade.verrouiller(7);

    REQUIRE(escouade.getCibleVerrouillee() == std::optional<int>(7));
}

TEST_CASE("deverrouiller vide la cible") {
    Escouade escouade({50.0f, 50.0f});

    escouade.verrouiller(3);
    escouade.deverrouiller();

    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("En combat, un personnage a portee de la cible reste immobile pendant que les autres avancent") {
    const std::vector<std::string> plan = {
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1); // id arbitraire, Escouade ne connaît pas Monstre

    // Centre initial (60, 60). Les deux tireurs (portée 120 px bord à bord)
    // démarrent à l'arrière, en (35, 35) et (35, 85) ; les deux mêlées
    // (portée 20 px) à l'avant, en (85, 35) et (85, 85). Rayons : 10 px
    // pour un personnage, 15 px pour la cible. Cette cible met déjà les
    // tireurs à portée (≈142 - 25 ≈ 117 px bord à bord) mais pas les mêlées
    // (≈93 - 25 ≈ 68 px) : l'ancre doit donc encore avancer.
    const sf::Vector2f cible{175.0f, 60.0f};

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;
    const sf::Vector2f meleeAvant = escouade.proieDuMonstre(1, {85.0f, 60.0f})->centre;

    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;
    verifierPointProche(tireurApres, tireurAvant, 0.01f);

    const sf::Vector2f meleeApres = escouade.proieDuMonstre(1, {85.0f, 60.0f})->centre;
    REQUIRE((meleeApres - meleeAvant).length() > 5.0f);
}

TEST_CASE("Sans verrou, un personnage deja a portee suit tout de meme son emplacement") {
    const std::vector<std::string> plan = {
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f}); // pas de verrouiller()

    const sf::Vector2f cible{175.0f, 60.0f}; // même configuration que le test précédent
    escouade.allerVers(cible, carte); // sans verrou, attaquer() n'approche pas : l'ordre vient du joueur

    const sf::Vector2f tireurAvant = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;

    for (int i = 0; i < 10; ++i) {
        attaquerCible(escouade, carte, cibleMonstre(cible));
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }

    const sf::Vector2f tireurApres = escouade.proieDuMonstre(1, {35.0f, 60.0f})->centre;
    REQUIRE((tireurApres - tireurAvant).length() > 5.0f);
}

// Pour les tests du skill : centre initial (60, 60), mêlées en (85, 35) et
// (85, 85). Une cible de la taille d'un monstre en (85, 60) est à 0 px bord
// à bord de chacun d'eux, donc à portée du skill (20 px, comme la mêlée).

// Sans verrou, le skill de dégâts vise le monstre que son lanceur frapperait
// en autodéfense, avec la portée du skill (20 px). Le lanceur de la place 1
// est en (85, 35), rayon 10 : un monstre (rayon 15) à 45 px ou moins est à portée.

TEST_CASE("Sans verrou, le skill de degats part sur un monstre a portee, sans creer de verrou") {
    Escouade escouade({50.0f, 50.0f});

    const std::vector<Coup> coups = escouade.declencherSkill(1, {MonstreVu{4, cibleMonstre({85.0f, 60.0f})}});

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 4);
    REQUIRE(coups[0].degats == 25);
    REQUIRE_FALSE(escouade.getCibleVerrouillee().has_value());
}

TEST_CASE("Sans verrou, le skill de degats vise le plus proche parmi plusieurs monstres a portee") {
    Escouade escouade({50.0f, 50.0f});

    // Le monstre 1 (premier de la liste) est à 15 px bord à bord, le 2 à 0.
    const std::vector<Coup> coups = escouade.declencherSkill(
        1, {MonstreVu{1, cibleMonstre({125.0f, 35.0f})}, MonstreVu{2, cibleMonstre({110.0f, 35.0f})}});

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 2);
}

TEST_CASE("Sans verrou, le skill de degats est refuse sans monstre a portee, et sans recharge") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f}))) == 0);
    REQUIRE(declencher(escouade, 1, {}) == 0);
    // Pas de mise en attente ni de recharge : un monstre à portée est frappé aussitôt.
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 25);
}

TEST_CASE("Avec un verrou, le skill de degats vise le monstre verrouille, pas le plus proche") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    const std::vector<Coup> coups = escouade.declencherSkill(
        1, {MonstreVu{1, cibleMonstre({125.0f, 35.0f})}, MonstreVu{2, cibleMonstre({110.0f, 35.0f})}});

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Avec un verrou absent de la liste des monstres, le skill de degats est refuse") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(9);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie les degats du seul personnage qui a un skill") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    // Les deux mêlées sont à portée, mais un seul a un skill : 25, pas 50.
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 25);
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une cible hors de portee du skill") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f}))) == 0);
}

TEST_CASE("En combat, declencherSkill renvoie 0 au second appel immediat (recharge du skill)") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cible)) == 25);
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cible)) == 0);
}

TEST_CASE("Les obstacles bloquent les personnages mais pas l'ancre") {
    const std::vector<std::string> plan = {
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
        "............",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f}); // centre (60, 60), personnages en (35|85, 35|85)
    const std::vector<Cercle> obstacles = {{{200.0f, 60.0f}, 30.0f}};

    escouade.allerVers({400.0f, 60.0f}, carte);
    for (int i = 0; i < 300; ++i) {
        escouade.mettreAJour(1.0f / 60.0f, carte, obstacles);
    }

    // L'ancre a traversé l'obstacle (bord droit en x = 230)...
    REQUIRE(escouade.getCentre().x > 230.0f);

    // ... mais aucun personnage n'y est entré.
    for (const Cercle& cercle : escouade.cerclesDesVivants()) {
        const float ecartCentres = (cercle.centre - obstacles[0].centre).length();
        REQUIRE(ecartCentres >= cercle.rayon + obstacles[0].rayon - 0.001f);
    }
}

TEST_CASE("En combat, declencherSkill renvoie 0 pour une place sans skill ou hors formation") {
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    const Cercle cible = cibleMonstre({85.0f, 60.0f});

    REQUIRE(declencher(escouade, 2, unSeulMonstre(cible)) == 0);
    REQUIRE(declencher(escouade, 0, unSeulMonstre(cible)) == 0);
    REQUIRE(declencher(escouade, 5, unSeulMonstre(cible)) == 0);
}

// Escouade en (50, 50), orientée vers +x : les quatre places sont en
// (85, 35), (85, 85), (35, 35) et (35, 85). Des dégâts d'origine égale à
// l'une de ces positions touchent la place correspondante.

TEST_CASE("choisirCible (soin) est vide quand tout le monde est à pleine vie") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}).has_value());
}

TEST_CASE("choisirCible (soin) désigne le seul blessé") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("choisirCible (soin) désigne celui qui a le moins de points de vie") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 30);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 50);

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("choisirCible (soin) choisit la plus petite place en cas d'égalité") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 20);
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("choisirCible (soin) ignore les morts") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 100);
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 10);

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("choisirCible (soin) est vide quand tout le monde est mort") {
    Escouade escouade({50.0f, 50.0f});
    for (const sf::Vector2f origine : {sf::Vector2f(85.0f, 35.0f), sf::Vector2f(85.0f, 85.0f),
                                       sf::Vector2f(35.0f, 35.0f), sf::Vector2f(35.0f, 85.0f)}) {
        escouade.recevoirDegats(1, origine, 100);
    }

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}).has_value());
}

// Soin (place 3, en (35, 35), portée 40 px bord à bord = 60 px entre
// centres, 30 points de vie, recharge 8 s). Place 4 en (35, 85) : 50 px,
// à portée. Place 2 en (85, 85) : environ 70,7 px, hors de portée.

TEST_CASE("Le soin de la place 3 soigne un allié à portée, sans combat engagé") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    REQUIRE(declencher(escouade, 3, {}) == 0);

    // 80 + 30 plafonné à 100 : plus personne à soigner.
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}).has_value());
}

TEST_CASE("Le soin ne part pas si la cible est hors de portée") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);

    declencher(escouade, 3, {});

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("Le soin ne part pas à pleine vie et ne déclenche alors pas la recharge") {
    Escouade escouade({50.0f, 50.0f});
    declencher(escouade, 3, {});

    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Soin, 3, {}).has_value());
}

TEST_CASE("Le soin est en recharge juste après son lancement") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 60);

    declencher(escouade, 3, {});   // 40 -> 70
    declencher(escouade, 3, {});   // refusé : recharge

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("Un soigneur mort ne soigne pas") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 100);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    declencher(escouade, 3, {});

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

TEST_CASE("Le skill de dégâts est refusé sans monstre verrouillé même si une cible est donnée") {
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(declencher(escouade, 1, {}) == 0);
}

// Skill de soin en attente. Carte dégagée dédiée ; escouade immobile en
// (50, 50). Le blessé est la place 2 (85, 85), à environ 70,7 px du
// soigneur (35, 35) : hors de la portée du soin (60 px entre centres).

namespace {
TileMap carteDegageeSoin() {
    const std::vector<std::string> plan = {
        "..........", "..........", "..........", "..........", "..........",
        "..........", "..........", "..........", "..........", "..........",
    };
    return TileMap(plan, 40.0f);
}
}  // namespace

TEST_CASE("Un soin hors de portée est mis en attente : le soigneur marche, soigne, puis revient à sa place") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    escouade.attaquer({}, carte); // le monstre fantôme n'existe pas : il n'est pas un agresseur

    declencher(escouade, 3, {});
    REQUIRE(placeAlliePourSoin(escouade) == 2);   // pas encore soigné

    simuler(escouade, carte, 0.5f);
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());  // soigné en route

    simuler(escouade, carte, 2.0f);
    const std::optional<Cercle> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    REQUIRE(soigneur.has_value());
    verifierPointProche(soigneur->centre, {35.0f, 35.0f});
}

TEST_CASE("Le soin en attente est annulé si la cible meurt, sans recharge") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    escouade.recevoirDegats(1, {85.0f, 85.0f}, 100);
    escouade.attaquer({}, carte); // le monstre fantôme n'existe pas : il n'est pas un agresseur
    simuler(escouade, carte, 2.0f);

    // Le soigneur est revenu à sa place et son skill est prêt : un nouveau
    // blessé à portée est soigné aussitôt.
    const std::optional<Cercle> soigneur = escouade.proieDuMonstre(1, {35.0f, 35.0f});
    verifierPointProche(soigneur->centre, {35.0f, 35.0f});
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);
    declencher(escouade, 3, {});
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Le soin en attente est annulé si le soigneur meurt") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    escouade.recevoirDegats(1, {35.0f, 35.0f}, 100);
    simuler(escouade, carte, 2.0f);

    REQUIRE(placeAlliePourSoin(escouade) == 2);
}

TEST_CASE("Un ordre de déplacement n'annule pas un soin en attente") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});

    escouade.allerVers({300.0f, 300.0f}, carte);
    simuler(escouade, carte, 5.0f);

    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

TEST_CASE("Un seul skill en attente à la fois") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 20);
    declencher(escouade, 3, {});   // en attente

    // Le skill de dégâts de la place 1, à portée d'une cible, est refusé.
    escouade.verrouiller(1);
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);
}

// Skill de dégâts en attente. Aucune attaque de base ne dépasse 20 dégâts
// en un tic (4 personnages x 5) : un tic qui rapporte 25 ou plus contient
// donc forcément le skill.

TEST_CASE("Un skill de dégâts hors de portée est mis en attente, puis part à l'arrivée du lanceur") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    const Cercle cible = cibleMonstre({300.0f, 60.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cible);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cible)) == 0);

    int degatsMax = 0;
    for (int i = 0; i < 600; ++i) {
        const int degats = attaquerCible(escouade, carte, cible);
        degatsMax = std::max(degatsMax, degats);
        escouade.mettreAJour(1.0f / 60.0f, carte, {});
    }
    REQUIRE(degatsMax >= 25);
}

TEST_CASE("Un skill de dégâts en attente est annulé par deverrouiller, sans recharge") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    escouade.deverrouiller();
    escouade.verrouiller(1);

    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 25);
}

TEST_CASE("Un skill de dégâts en attente est annulé par un changement de cible, pas par la même cible") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    escouade.verrouiller(1);   // même cible : toujours en attente
    REQUIRE(declencher(escouade, 1, unSeulMonstre(cibleMonstre({85.0f, 60.0f}))) == 0);

    escouade.verrouiller(2);   // autre cible : annulé
    REQUIRE(declencher(escouade, 1, {MonstreVu{2, cibleMonstre({85.0f, 60.0f})}}) == 25);
}

TEST_CASE("Un soin est refusé pendant qu'un skill de dégâts est en attente") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(1);
    attaquerCible(escouade, carte, cibleMonstre({300.0f, 60.0f}));
    declencher(escouade, 1, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 20);

    declencher(escouade, 3, unSeulMonstre(cibleMonstre({300.0f, 60.0f})));

    REQUIRE(placeAlliePourSoin(escouade) == 4);
}

// Autodéfense : sans verrou, chaque personnage vivant frappe de lui-même un
// monstre à sa portée. Centre initial (60, 60) ; mêlées en (85, 35) et
// (85, 85) (portée 20), tireurs en (35, 35) et (35, 85) (portée 120).

TEST_CASE("Sans verrou, tous les personnages frappent un monstre a leur portee, sans bouger") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    const sf::Vector2f centreInitial = escouade.getCentre();

    const std::vector<Coup> coups = escouade.attaquer({MonstreVu{4, cibleMonstre({85.0f, 60.0f})}}, carte);
    escouade.mettreAJour(1.0f / 60.0f, carte, {});

    REQUIRE(coups.size() == 4);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 4);
        REQUIRE(coup.degats == 5);
    }
    verifierPointProche(escouade.getCentre(), centreInitial);
}

TEST_CASE("Sans verrou, un monstre hors de portee de tous ne recoit aucun coup") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});

    REQUIRE(escouade.attaquer({MonstreVu{1, cibleMonstre({300.0f, 60.0f})}}, carte).empty());
}

TEST_CASE("Sans verrou, seuls les tireurs frappent un monstre eloigne") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});

    // À 92 px bord à bord des tireurs, à 44 px des mêlées.
    const std::vector<Coup> coups = escouade.attaquer({MonstreVu{1, cibleMonstre({150.0f, 60.0f})}}, carte);

    REQUIRE(coups.size() == 2);
}

TEST_CASE("Un verrou hors de portee n'empeche pas de frapper un autre monstre a portee") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(2);

    const std::vector<Coup> coups = escouade.attaquer(
        {MonstreVu{1, cibleMonstre({150.0f, 60.0f})}, MonstreVu{2, cibleMonstre({300.0f, 60.0f})}}, carte);

    REQUIRE(coups.size() == 2);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 1);
    }
}

TEST_CASE("Le verrou a portee passe avant un monstre qui est a portee aussi") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.verrouiller(2);

    // Le monstre 1 est plus proche des tireurs, mais le 2 est verrouillé.
    const std::vector<Coup> coups = escouade.attaquer(
        {MonstreVu{1, cibleMonstre({150.0f, 60.0f})}, MonstreVu{2, cibleMonstre({150.0f, 90.0f})}}, carte);

    REQUIRE(coups.size() == 2);
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 2);
    }
}

TEST_CASE("Deux personnages peuvent frapper deux monstres differents dans la meme image") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 150.0f});

    // Le monstre 1 est à portée du tireur du haut seulement, le 2 du tireur du bas seulement
    // (tous deux dans la carte, en vue : la ligne de vue exige des points dans la carte).
    const std::vector<Coup> coups = escouade.attaquer(
        {MonstreVu{1, cibleMonstre({140.0f, 50.0f})}, MonstreVu{2, cibleMonstre({140.0f, 270.0f})}}, carte);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre != coups[1].idMonstre);
}

TEST_CASE("Une escouade dont tous les personnages sont morts ne frappe plus") {
    const TileMap carte = carteDegageeSoin();
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(1, {85.0f, 35.0f}, 100);
    escouade.recevoirDegats(1, {85.0f, 85.0f}, 100);
    escouade.recevoirDegats(1, {35.0f, 35.0f}, 100);
    escouade.recevoirDegats(1, {35.0f, 85.0f}, 100);

    REQUIRE(escouade.attaquer({MonstreVu{1, cibleMonstre({85.0f, 60.0f})}}, carte).empty());
}

TEST_CASE("En terrain dégagé, les places de formation ne sont pas modifiées") {
    const TileMap carte({"##########",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "##########"},
                        40.0f);
    Escouade escouade({150.0f, 110.0f});
    escouade.mettreAJour(0.016f, carte, {});

    const sf::Vector2f centre = escouade.getCentre();
    const std::vector<sf::Vector2f> attendues = {centre + sf::Vector2f(25.0f, -25.0f),
                                                 centre + sf::Vector2f(25.0f, 25.0f),
                                                 centre + sf::Vector2f(-25.0f, -25.0f),
                                                 centre + sf::Vector2f(-25.0f, 25.0f)};
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 4);
    for (std::size_t i = 0; i < places.size(); ++i) {
        REQUIRE_THAT(places[i].x, Catch::Matchers::WithinAbs(attendues[i].x, 0.01f));
        REQUIRE_THAT(places[i].y, Catch::Matchers::WithinAbs(attendues[i].y, 0.01f));
    }
}

TEST_CASE("Ancre collée à un mur, aucune place de formation n'est dans un mur") {
    const TileMap carte({"##########",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "#........#",
                         "##########"},
                        40.0f);
    Escouade escouade({40.0f, 80.0f}); // centre (50, 90) : le bord gauche de l'ancre touche le mur
    escouade.mettreAJour(0.016f, carte, {});

    const sf::Vector2f taille{20.0f, 20.0f};
    const std::vector<sf::Vector2f>& places = escouade.getPlacesDeFormation();
    REQUIRE(places.size() == 4);
    for (const sf::Vector2f& place : places) {
        REQUIRE_FALSE(carte.chevaucheUnMur(sf::FloatRect(place - taille / 2.0f, taille)));
    }
    // Les places de gauche, qui tombaient dans le mur, se sont rapprochées de l'ancre.
    REQUIRE(places[2].x > escouade.getCentre().x - 25.0f);
}

TEST_CASE("Un coup qui fait des degats enregistre son auteur comme agresseur") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);

    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(agresseurs.size() == 1);
    REQUIRE(agresseurs[0].id == 7);
    verifierPointProche(agresseurs[0].cercle.centre, {300.0f, 60.0f});
    REQUIRE(agresseurs[0].tempsRestant == 3.0f);
}

TEST_CASE("Un coup a zero degat n'enregistre aucun agresseur") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 0);

    REQUIRE(escouade.getAgresseurs().empty());
}

TEST_CASE("Un coup d'un agresseur deja connu ne le duplique pas et remet son temps au maximum") {
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.recevoirDegats(8, {310.0f, 60.0f}, 10);
    escouade.recevoirDegats(7, {320.0f, 60.0f}, 10);

    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(agresseurs.size() == 2);
    REQUIRE(agresseurs[0].id == 7);
    verifierPointProche(agresseurs[0].cercle.centre, {320.0f, 60.0f});
    REQUIRE(agresseurs[1].id == 8);
}

TEST_CASE("Sans personnage vivant, un coup n'enregistre aucun agresseur") {
    Escouade escouade({50.0f, 50.0f});
    for (int i = 0; i < 4; ++i) {
        escouade.recevoirDegats(1, {50.0f, 50.0f}, 100);
    }
    escouade.recevoirDegats(9, {50.0f, 50.0f}, 10);

    REQUIRE_FALSE(escouade.estVivante());
    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(std::none_of(agresseurs.begin(), agresseurs.end(), [](const Escouade::Agresseur& a) { return a.id == 9; }));
}

TEST_CASE("Un agresseur est oublie 3 s apres son dernier coup") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);

    escouade.mettreAJour(2.9f, carte, {});
    REQUIRE(escouade.getAgresseurs().size() == 1);
    escouade.mettreAJour(0.2f, carte, {});
    REQUIRE(escouade.getAgresseurs().empty());
}

TEST_CASE("Un nouveau coup repousse l'oubli de l'agresseur") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.mettreAJour(2.0f, carte, {});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.mettreAJour(2.0f, carte, {});

    REQUIRE(escouade.getAgresseurs().size() == 1);
}

TEST_CASE("attaquer retire l'agresseur absent de la liste et met a jour le cercle des autres") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.recevoirDegats(7, {300.0f, 60.0f}, 10);
    escouade.recevoirDegats(8, {310.0f, 60.0f}, 10);

    escouade.attaquer({MonstreVu{8, cibleMonstre({320.0f, 70.0f})}}, carte);

    const auto& agresseurs = escouade.getAgresseurs();
    REQUIRE(agresseurs.size() == 1);
    REQUIRE(agresseurs[0].id == 8);
    verifierPointProche(agresseurs[0].cercle.centre, {320.0f, 70.0f});
    REQUIRE(agresseurs[0].cercle.rayon == 15.0f);
}

namespace {

// Carte dégagée de 800 × 600 px, entourée de murs.
TileMap carteDegagee() {
    std::vector<std::string> plan;
    plan.push_back(std::string(20, '#'));
    for (int i = 0; i < 13; ++i) {
        plan.push_back("#" + std::string(18, '.') + "#");
    }
    plan.push_back(std::string(20, '#'));
    return TileMap(plan, 40.0f);
}

// Comme main.cpp : attaquer puis mettreAJour à chaque tic, le monstre étant
// un obstacle pour les personnages.
void simulerAvecMonstre(Escouade& escouade, const TileMap& carte, Cercle monstre, int tics) {
    for (int i = 0; i < tics; ++i) {
        escouade.attaquer({MonstreVu{1, monstre}}, carte);
        escouade.mettreAJour(1.0f / 60.0f, carte, {monstre});
    }
}

float distanceDuPlusProche(const Escouade& escouade, Cercle monstre) {
    return (escouade.proieDuMonstre(1, monstre.centre)->centre - monstre.centre).length();
}

}  // namespace

// Escouade en (210, 210), orientée vers +x : mêlées en (235, 185) et
// (235, 235), tireurs en (185, 185) et (185, 235). Monstre en (180, 120) :
// le tireur arrière-gauche est à 65 px, la mêlée avant-gauche à 85 px de sa
// place (dans la laisse), l'autre mêlée à 127 px (hors laisse).

TEST_CASE("Un personnage sans monstre a portee va vers l'agresseur de sa laisse") {
    const TileMap carte = carteDegagee();
    const Cercle monstre = cibleMonstre({180.0f, 120.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);

    simulerAvecMonstre(escouade, carte, monstre, 90);

    REQUIRE(distanceDuPlusProche(escouade, monstre) < 45.0f);
}

TEST_CASE("Un agresseur hors de la laisse de tous laisse chacun a sa place") {
    const TileMap carte = carteDegagee();
    const Cercle monstre = cibleMonstre({210.0f, 380.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);

    simulerAvecMonstre(escouade, carte, monstre, 90);

    REQUIRE(distanceDuPlusProche(escouade, monstre) > 130.0f);
}

TEST_CASE("Quand l'ancre se deplace, personne ne va vers l'agresseur") {
    const TileMap carte = carteDegagee();
    const Cercle monstre = cibleMonstre({180.0f, 120.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);
    escouade.allerVers({700.0f, 210.0f}, carte);

    simulerAvecMonstre(escouade, carte, monstre, 10);

    REQUIRE(distanceDuPlusProche(escouade, monstre) > 60.0f);
}

TEST_CASE("Apres l'oubli de l'agresseur, tout le monde reprend sa place") {
    const TileMap carte = carteDegagee();
    const Cercle monstre = cibleMonstre({180.0f, 120.0f});
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, monstre.centre, 10);

    simulerAvecMonstre(escouade, carte, monstre, 300);

    REQUIRE(escouade.getAgresseurs().empty());
    REQUIRE(distanceDuPlusProche(escouade, monstre) > 60.0f);
}

TEST_CASE("provoquer enregistre le monstre, le provocateur et la duree") {
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 1);
    REQUIRE(provocations[0].id == 7);
    REQUIRE(provocations[0].place == 2);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
}

TEST_CASE("Provoquer de nouveau un monstre deja provoque ne le duplique pas et remet sa duree au maximum") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.provoquer(8, 2, 4.0f);
    escouade.mettreAJour(3.0f, carte, {});
    escouade.provoquer(7, 2, 4.0f);

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 2);
    REQUIRE(provocations[0].id == 7);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
    REQUIRE(provocations[1].id == 8);
}

TEST_CASE("Une provocation prend fin apres sa duree") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);

    escouade.mettreAJour(3.9f, carte, {});
    REQUIRE(escouade.getProvocations().size() == 1);
    escouade.mettreAJour(0.2f, carte, {});
    REQUIRE(escouade.getProvocations().empty());
}

TEST_CASE("Une provocation prend fin a la mort du provocateur") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.provoquer(8, 1, 4.0f);

    escouade.recevoirDegats(1, {235.0f, 235.0f}, 100);  // tue la place 2
    escouade.mettreAJour(0.016f, carte, {});

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 1);
    REQUIRE(provocations[0].id == 8);
}

TEST_CASE("attaquer retire la provocation d'un monstre absent de la liste") {
    const TileMap carte({"..........", "..........", ".........."}, 40.0f);
    Escouade escouade({50.0f, 50.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.provoquer(8, 2, 4.0f);

    escouade.attaquer({MonstreVu{8, cibleMonstre({300.0f, 60.0f})}}, carte);

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 1);
    REQUIRE(provocations[0].id == 8);
}

// Escouade en (200, 200), orientée vers +x, personnages en (235, 185),
// (235, 235), (185, 185) et (185, 235). Un monstre en (185, 185) est tout
// près de la place 3 ; la place 2 est le provocateur.

TEST_CASE("Sans provocation, la proie d'un monstre est le personnage vivant le plus proche") {
    Escouade escouade({200.0f, 200.0f});

    verifierPointProche(escouade.proieDuMonstre(7, {170.0f, 170.0f})->centre, {185.0f, 185.0f});
}

TEST_CASE("Un monstre provoque vise le provocateur, pas le plus proche") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);

    verifierPointProche(escouade.proieDuMonstre(7, {170.0f, 170.0f})->centre, {235.0f, 235.0f});
}

TEST_CASE("La provocation d'un monstre ne change pas la proie des autres") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);

    verifierPointProche(escouade.proieDuMonstre(8, {170.0f, 170.0f})->centre, {185.0f, 185.0f});
}

TEST_CASE("Un monstre provoque dont le provocateur est mort reprend le plus proche") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);
    escouade.recevoirDegats(1, {235.0f, 235.0f}, 100);  // tue la place 2

    verifierPointProche(escouade.proieDuMonstre(7, {170.0f, 170.0f})->centre, {185.0f, 185.0f});
}

TEST_CASE("recevoirDegats frappe le meme personnage que proieDuMonstre") {
    Escouade escouade({200.0f, 200.0f});
    escouade.provoquer(7, 2, 4.0f);
    const sf::Vector2f monstre{170.0f, 170.0f};
    const Cercle proie = *escouade.proieDuMonstre(7, monstre);

    escouade.recevoirDegats(7, monstre, 100);

    const std::vector<Cercle> vivants = escouade.cerclesDesVivants();
    REQUIRE(vivants.size() == 3);
    REQUIRE(std::none_of(vivants.begin(), vivants.end(), [&](const Cercle& c) { return c.centre == proie.centre; }));
}

// Provocation (touche 2) : lanceur en (235, 235), rayon 150 px bord à bord.

TEST_CASE("Provocation : provoque tous les monstres dans le rayon, sans degats") {
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({335.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({235.0f, 335.0f})},
        MonstreVu{3, cibleMonstre({800.0f, 235.0f})},
    };

    REQUIRE(escouade.declencherSkill(2, monstres).empty());

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 2);
    REQUIRE(provocations[0].id == 1);
    REQUIRE(provocations[1].id == 2);
    REQUIRE(provocations[0].place == 2);
    REQUIRE(provocations[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : la zone se mesure depuis le centre du lanceur, bord du monstre compris") {
    Escouade escouade({200.0f, 200.0f});
    // Centre du lanceur (235, 235), rayon de zone 150, rayon du monstre 15 :
    // le monstre est touché jusqu'à 165 px de distance entre centres.
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({400.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({401.0f, 235.0f})},
    };

    escouade.declencherSkill(2, monstres);

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == 1);
}

TEST_CASE("Provocation : sans monstre dans le rayon, elle part quand même et consomme sa recharge") {
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> loin = {MonstreVu{1, cibleMonstre({800.0f, 235.0f})}};
    REQUIRE(escouade.skillEstPret(2));

    REQUIRE(escouade.declencherSkill(2, loin).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE_FALSE(escouade.skillEstPret(2));

    // Recharge en cours : un monstre qui arrive n'est pas provoqué.
    escouade.declencherSkill(2, {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}});
    REQUIRE(escouade.getProvocations().empty());
}


TEST_CASE("Provocation : un nouvel appui pendant la recharge est ignore, puis accepte apres 10 s") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}};

    escouade.declencherSkill(2, monstres);
    escouade.mettreAJour(3.0f, carte, {});
    escouade.declencherSkill(2, monstres);
    REQUIRE(escouade.getProvocations()[0].tempsRestant < 1.5f);  // pas remis à 4

    escouade.mettreAJour(7.1f, carte, {});
    escouade.declencherSkill(2, monstres);
    REQUIRE(escouade.getProvocations()[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : refusee si le lanceur est mort") {
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {235.0f, 235.0f}, 100);  // tue la place 2

    escouade.declencherSkill(2, {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}});

    REQUIRE(escouade.getProvocations().empty());
}

TEST_CASE("Provocation : refusee si un autre skill est en attente") {
    Escouade escouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({335.0f, 235.0f})}};
    escouade.declencherSkill(1, monstres);  // dégâts hors de portée de mêlée : mis en attente

    escouade.declencherSkill(2, monstres);

    REQUIRE(escouade.getProvocations().empty());
}

// Skill de zone (touche 4) : lanceur en (185, 235), 15 dégâts, recharge 8 s,
// zone de 60 px de rayon ; monstres de rayon 15, donc touchés jusqu'à 75 px
// de distance entre centres.

TEST_CASE("skillEstPret : vrai pour un lanceur vivant dont le skill est charge") {
    Escouade escouade({200.0f, 200.0f});

    REQUIRE(escouade.skillEstPret(4));
}

TEST_CASE("skillEstPret : faux pour une place invalide") {
    Escouade escouade({200.0f, 200.0f});

    REQUIRE_FALSE(escouade.skillEstPret(0));
    REQUIRE_FALSE(escouade.skillEstPret(5));
}

TEST_CASE("skillEstPret : faux si le lanceur est mort") {
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 100);  // tue la place 4

    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("skillEstPret : faux pendant la recharge, vrai apres 8 s") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, {}, carte);

    REQUIRE_FALSE(escouade.skillEstPret(4));

    escouade.mettreAJour(8.1f, carte, {});
    REQUIRE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : les monstres dans la zone sont touches, celui hors zone est epargne") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({300.0f, 300.0f})},
        MonstreVu{2, cibleMonstre({330.0f, 300.0f})},
        MonstreVu{3, cibleMonstre({500.0f, 300.0f})},
    };

    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, monstres, carte);

    REQUIRE(coups.size() == 2);
    REQUIRE(coups[0].idMonstre == 1);
    REQUIRE(coups[0].degats == 15);
    REQUIRE(coups[1].idMonstre == 2);
    REQUIRE(coups[1].degats == 15);
}

TEST_CASE("Zone : un monstre qui touche tout juste le bord de la zone est touche") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({375.0f, 300.0f})},
        MonstreVu{2, cibleMonstre({300.0f, 376.0f})},
    };

    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, monstres, carte);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Zone : une zone vide part quand meme et lance la recharge") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});

    REQUIRE(escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, {MonstreVu{1, cibleMonstre({700.0f, 300.0f})}}, carte).empty());

    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un point a portee (250 px bord a bord) part tout de suite") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    // Lanceur (place 4) en (185, 235), rayon 10 : (435, 235) est à 240 px du bord.
    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {435.0f, 235.0f}, {MonstreVu{1, cibleMonstre({435.0f, 235.0f})}}, carte);

    REQUIRE(coups.size() == 1);
    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un point hors de portee met le sort en attente, sans coup ni recharge") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({650.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkillDeZone(4, {650.0f, 235.0f}, monstres, carte).empty());

    REQUIRE(escouade.skillEstPret(4));
    // Un sort est en attente : un autre skill est refusé.
    escouade.declencherSkill(2, monstres);
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : le lanceur se rapproche, puis le sort part au point fixe") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({650.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({650.0f, 400.0f})},
    };
    escouade.declencherSkillDeZone(4, {650.0f, 235.0f}, monstres, carte);

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 300) {
        for (const Coup& coup : escouade.attaquer(monstres, carte)) {
            if (coup.degats == 15) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
        ++tics;
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
    REQUIRE(tics > 5);  // le lanceur a dû marcher
    REQUIRE_FALSE(escouade.skillEstPret(4));
    // Plus rien en attente : un autre skill peut maintenant partir.
    escouade.declencherSkill(2, monstres);
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : elle touche les monstres presents au depart, pas ceux du clic") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    escouade.declencherSkillDeZone(4, {650.0f, 235.0f}, {MonstreVu{1, cibleMonstre({650.0f, 235.0f})}}, carte);

    const std::vector<MonstreVu> ailleurs = {
        MonstreVu{1, cibleMonstre({650.0f, 500.0f})},
        MonstreVu{2, cibleMonstre({660.0f, 235.0f})},
    };
    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(ailleurs, carte)) {
            if (coup.degats == 15) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 2);
}

TEST_CASE("Zone en attente : la mort du lanceur l'annule sans recharge") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({650.0f, 235.0f})}};
    escouade.declencherSkillDeZone(4, {650.0f, 235.0f}, monstres, carte);
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 100);  // tue la place 4

    for (int i = 0; i < 300; ++i) {
        REQUIRE(escouade.attaquer(monstres, carte).empty());
        escouade.mettreAJour(0.016f, carte, {});
    }

    // Plus rien en attente : une provocation (place 2) est acceptée.
    escouade.declencherSkill(2, monstres);
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : verrouiller, deverrouiller et un ordre de deplacement ne l'annulent pas") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({650.0f, 235.0f})},
        MonstreVu{2, cibleMonstre({300.0f, 400.0f})},
    };
    escouade.declencherSkillDeZone(4, {650.0f, 235.0f}, monstres, carte);

    escouade.verrouiller(2);
    escouade.verrouiller(1);
    escouade.deverrouiller();
    escouade.allerVers({200.0f, 500.0f}, carte);

    std::vector<Coup> coupsDeZone;
    for (int i = 0; i < 300 && coupsDeZone.empty(); ++i) {
        for (const Coup& coup : escouade.attaquer(monstres, carte)) {
            if (coup.degats == 15) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
}

TEST_CASE("Zone : refusee pendant la recharge") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({300.0f, 300.0f})}};
    escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, monstres, carte);

    REQUIRE(escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, monstres, carte).empty());
}

TEST_CASE("Zone : refusee si le lanceur est mort") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    escouade.recevoirDegats(1, {185.0f, 235.0f}, 100);  // tue la place 4

    REQUIRE(escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, {MonstreVu{1, cibleMonstre({300.0f, 300.0f})}}, carte).empty());
}

TEST_CASE("Zone : refusee si un skill est en attente") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    escouade.verrouiller(1);
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({300.0f, 300.0f})}};
    escouade.declencherSkill(1, monstres);  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, monstres, carte).empty());
    REQUIRE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : refusee pour une place invalide ou dont le skill n'est pas de zone") {
    const TileMap carte = carteDegagee();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({235.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkillDeZone(0, {235.0f, 235.0f}, monstres, carte).empty());
    REQUIRE(escouade.declencherSkillDeZone(5, {235.0f, 235.0f}, monstres, carte).empty());
    REQUIRE(escouade.declencherSkillDeZone(1, {235.0f, 235.0f}, monstres, carte).empty());
    REQUIRE(escouade.declencherSkillDeZone(2, {235.0f, 235.0f}, monstres, carte).empty());
    REQUIRE(escouade.declencherSkillDeZone(3, {235.0f, 235.0f}, monstres, carte).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE(escouade.skillEstPret(1));
    REQUIRE(escouade.skillEstPret(2));
    REQUIRE(escouade.skillEstPret(3));
}

TEST_CASE("declencherSkill sans point refuse un skill de zone, et choisirCible ne renvoie rien") {
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({185.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkill(4, monstres).empty());
    REQUIRE(escouade.skillEstPret(4));
    REQUIRE_FALSE(escouade.choisirCible(TypeSkill::Zone, 4, monstres).has_value());
}

TEST_CASE("Zone : relance la recharge de l'attaque de base du lanceur") {
    Escouade escouade({200.0f, 200.0f});
    const TileMap carte = carteDegagee();
    // Ce monstre est à portée de tir des places 3 et 4 (120 px) et hors de
    // portée des mêlées : sans le skill, les deux tireurs frapperaient.
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({285.0f, 235.0f})}};
    escouade.declencherSkillDeZone(4, {300.0f, 300.0f}, {}, carte);

    const std::vector<Coup> coups = escouade.attaquer(monstres, carte);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].degats == 5);
}

TEST_CASE("rayonZoneDuSkill : 60 pour la place 4, 150 pour la provocation, 0 sans zone ou place invalide") {
    Escouade escouade({200.0f, 200.0f});

    REQUIRE(escouade.rayonZoneDuSkill(4) == 60.0f);
    REQUIRE(escouade.rayonZoneDuSkill(2) == 150.0f);
    REQUIRE(escouade.rayonZoneDuSkill(1) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(0) == 0.0f);
    REQUIRE(escouade.rayonZoneDuSkill(5) == 0.0f);
}

// Carte 15 x 10 (cases de 40 px) avec un mur vertical en colonne 6
// (x: [240, 280)) sur les rangées 3 à 7 : le lanceur de la place 4, en
// (185, 235), est d'un côté, et (320, 235) de l'autre. On passe par le haut
// (rangées 0 à 2) ou par le bas (rangées 8 et 9).
static TileMap carteAvecMurVertical() {
    const std::vector<std::string> plan = {
        "...............",
        "...............",
        "...............",
        "......#........",
        "......#........",
        "......#........",
        "......#........",
        "......#........",
        "...............",
        "...............",
    };
    return TileMap(plan, 40.0f);
}

TEST_CASE("Zone : un point a portee et en vue part tout de suite (carte avec mur)") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});

    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {100.0f, 235.0f}, {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}}, carte);

    REQUIRE(coups.size() == 1);
    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un point a portee mais derriere un mur met le sort en attente, sans recharge") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkillDeZone(4, {320.0f, 235.0f}, monstres, carte).empty());

    REQUIRE(escouade.skillEstPret(4));
    // Un sort est en attente : un autre skill est refusé.
    escouade.declencherSkill(2, monstres);
    REQUIRE(escouade.skillEstPret(2));
}

TEST_CASE("Zone : un point dans un mur est refuse, sans attente ni recharge") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}};

    REQUIRE(escouade.declencherSkillDeZone(4, {260.0f, 235.0f}, monstres, carte).empty());

    REQUIRE(escouade.skillEstPret(4));
    // Rien en attente : une provocation (place 2) est acceptée.
    escouade.declencherSkill(2, monstres);
    REQUIRE_FALSE(escouade.skillEstPret(2));
}

TEST_CASE("Zone en attente : le lanceur contourne le mur, puis le sort part des qu'il voit le point") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};
    escouade.declencherSkillDeZone(4, {320.0f, 235.0f}, monstres, carte);

    std::vector<Coup> coupsDeZone;
    int tics = 0;
    while (coupsDeZone.empty() && tics < 600) {
        for (const Coup& coup : escouade.attaquer(monstres, carte)) {
            if (coup.degats == 15) {
                coupsDeZone.push_back(coup);
            }
        }
        escouade.mettreAJour(0.016f, carte, {});
        ++tics;
    }

    REQUIRE(coupsDeZone.size() == 1);
    REQUIRE(coupsDeZone[0].idMonstre == 1);
    REQUIRE(tics > 5);  // le lanceur a dû marcher
    REQUIRE_FALSE(escouade.skillEstPret(4));
}

TEST_CASE("Zone : un monstre dans la zone mais derriere un mur est epargne") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({285.0f, 235.0f})},  // dans la zone (60 px), de l'autre côté du mur
    };

    REQUIRE(escouade.declencherSkillDeZone(4, {230.0f, 235.0f}, monstres, carte).empty());

    REQUIRE_FALSE(escouade.skillEstPret(4));  // le sort est parti quand même
}

TEST_CASE("Zone : un monstre dans la zone et en vue est touche, celui derriere le mur non") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({285.0f, 235.0f})},  // derrière le mur
        MonstreVu{2, cibleMonstre({200.0f, 235.0f})},  // du même côté que le point
    };

    const std::vector<Coup> coups = escouade.declencherSkillDeZone(4, {230.0f, 235.0f}, monstres, carte);

    REQUIRE(coups.size() == 1);
    REQUIRE(coups[0].idMonstre == 2);
    REQUIRE(coups[0].degats == 15);
}

TEST_CASE("Autodéfense : un monstre à portée mais derrière un mur n'est pas frappé") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    // Place 4 en (185, 235) : portée de tir 120 px, monstre à 110 px bord à bord, mur entre les deux.
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};

    REQUIRE(escouade.attaquer(monstres, carte).empty());
}

TEST_CASE("Autodéfense : un monstre à portée et en vue est frappé") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}};

    const std::vector<Coup> coups = escouade.attaquer(monstres, carte);

    REQUIRE_FALSE(coups.empty());
    REQUIRE(coups[0].idMonstre == 1);
}

TEST_CASE("Autodéfense : le monstre le plus proche est caché, l'autre est frappé") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{1, cibleMonstre({285.0f, 235.0f})},  // plus proche de la place 4, mais derrière le mur
        MonstreVu{2, cibleMonstre({185.0f, 130.0f})},  // plus loin, en vue
    };

    const std::vector<Coup> coups = escouade.attaquer(monstres, carte);

    REQUIRE_FALSE(coups.empty());
    for (const Coup& coup : coups) {
        REQUIRE(coup.idMonstre == 2);
    }
}

// Ne laisse en vie que la place 4 (tireur en (185, 235)) : seule sa portée et sa vue comptent.
static void garderSeulementLaPlace4(Escouade& escouade) {
    escouade.recevoirDegats(1, {235.0f, 185.0f}, 100);
    escouade.recevoirDegats(1, {235.0f, 235.0f}, 100);
    escouade.recevoirDegats(1, {185.0f, 185.0f}, 100);
}

TEST_CASE("Combat verrouillé : une cible à portée mais cachée ne fait pas arrêter l'ancre") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    garderSeulementLaPlace4(escouade);
    escouade.verrouiller(1);
    const sf::Vector2f depart = escouade.getCentre();
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({320.0f, 235.0f})}};

    for (int i = 0; i < 20; ++i) {
        escouade.attaquer(monstres, carte);
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE((escouade.getCentre() - depart).length() > 5.0f);
}

TEST_CASE("Combat verrouillé : une cible à portée et en vue fait arrêter l'ancre") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    garderSeulementLaPlace4(escouade);
    escouade.verrouiller(1);
    const sf::Vector2f depart = escouade.getCentre();
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({100.0f, 235.0f})}};

    for (int i = 0; i < 20; ++i) {
        escouade.attaquer(monstres, carte);
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE((escouade.getCentre() - depart).length() < 1.0f);
}

TEST_CASE("Agresseur à portée mais caché : le personnage va vers lui") {
    const TileMap carte = carteAvecMurVertical();
    Escouade escouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{1, cibleMonstre({283.0f, 235.0f})}};
    escouade.recevoirDegats(1, {283.0f, 235.0f}, 1);  // enregistre l'agresseur (touche le plus proche)
    const sf::Vector2f depart = escouade.cerclesDesVivants()[3].centre;
    escouade.attaquer(monstres, carte);

    for (int i = 0; i < 20; ++i) {
        escouade.attaquer(monstres, carte);
        escouade.mettreAJour(0.016f, carte, {});
    }

    REQUIRE((escouade.cerclesDesVivants()[3].centre - depart).length() > 10.0f);
}
