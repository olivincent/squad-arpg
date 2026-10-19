// ./tests/monstres/test_Monstre_engagement.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"
#include "aides_monstre.hpp"

// Monstre centré en (100, 100), rayon 15 px, portée 15 px (bord à bord). Ses dégâts ne sont pas
// injectables : les tests les lisent par getPuissanceAttaque().

TEST_CASE("Monstre.frapper renvoie les degats pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(frapperEnEntier(monstre, pointCible({110.0f, 100.0f}), generateur) == monstre.getPuissanceAttaque());
}

TEST_CASE("Monstre.frapper renvoie 0 pour une cible hors de portee") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE(frapperEnEntier(monstre, pointCible({300.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("Monstre.frapper renvoie de nouveau les degats apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    const Cercle cible = pointCible({110.0f, 100.0f});

    REQUIRE(frapperEnEntier(monstre, cible, generateur) == monstre.getPuissanceAttaque());
    ecoulerTemps(monstre, 1.5f, std::nullopt, carte);
    REQUIRE(frapperEnEntier(monstre, cible, generateur) == monstre.getPuissanceAttaque());
}

// Temps d'exécution de l'attaque (0,3 s) : le monstre frappe immobile, le coup
// engagé part à la fin de l'exécution, quelle que soit la cible du moment.

TEST_CASE("Monstre.engagerAttaque exige une cible a portee") {
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    REQUIRE_FALSE(engager(monstre, pointCible({300.0f, 100.0f}), 3, carteOuverte()));
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 3, carteOuverte()));
    REQUIRE(monstre.cibleEngagee() == 3);
}

TEST_CASE("Monstre.engagerAttaque refuse un monstre en marche") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);

    ecoulerTemps(monstre, 0.1f, pointCible({180.0f, 100.0f}), carte);  // en poursuite, pas encore à portée

    REQUIRE_FALSE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

TEST_CASE("Monstre.avancerAttaque rend les degats une seule fois, a la fin de l'execution") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.25f); ++i) {
        total += monstre.avancerAttaque(s_dureeTic, generateur);
    }
    REQUIRE(total == 0);

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += monstre.avancerAttaque(s_dureeTic, generateur);
    }
    REQUIRE(total == monstre.getPuissanceAttaque());
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
}

TEST_CASE("Monstre.mettreAJour ne bouge pas pendant l'execution, meme si la cible s'eloigne") {
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));

    ecoulerTemps(monstre, 0.25f, pointCible({300.0f, 100.0f}), carte);

    verifierPointProche(monstre.getCentre(), {100.0f, 100.0f}, 0.01f);
}

TEST_CASE("Monstre.annulerAttaque ne consomme pas de recharge") {
    Generateur generateur = generateurDeTest();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    REQUIRE(monstre.avancerAttaque(0.1f, generateur) == 0);

    monstre.annulerAttaque();

    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

TEST_CASE("Monstre : apres le depart du coup, la recharge court avant le suivant") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteLongue();
    Monstre monstre({100.0f, 100.0f}, 1, 0, 0);
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    REQUIRE(monstre.avancerAttaque(0.4f, generateur) == monstre.getPuissanceAttaque());

    REQUIRE_FALSE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
    ecoulerTemps(monstre, 1.4f, std::nullopt, carte);  // recharge de 0,7 à 1,3 s écoulée
    REQUIRE(engager(monstre, pointCible({110.0f, 100.0f}), 1, carteOuverte()));
}

namespace {

// Depuis l'étape 73 la vue se décide en cases : deux points voisins ne sont cachés l'un à
// l'autre que si un mur d'une case se trouve entre eux. Deux fixtures :
//
// carteAvecMurMince : 20 × 20 cases de 10 px, un mur en case (4, 4) (x et y de 40 à 50).
// Un monstre en (25, 45), dont le carré touche le mur sans le chevaucher, a (60, 45) à
// 35 px : à portée (15 px bord à bord, rayon du monstre 15 px, cible ponctuelle), mais le
// mur est entre les deux. (25, 15) est à portée et en vue ; (25, 150) est en vue mais
// trop loin. Pour les tests qui ne déplacent pas le monstre : sur des cases aussi petites
// le A* le coincerait, son carré de 30 px ne passant pas entre les centres de cases.
TileMap carteAvecMurMince() {
    std::vector<std::string> plan(20, std::string(20, '.'));
    plan[4][4] = '#';
    return TileMap(plan, 10.0f);
}

// carteAvecPilier : 5 × 5 cases de 40 px, un mur en case (1, 1) (x et y de 40 à 80), pour
// les tests où le monstre doit pouvoir se déplacer. Un monstre en (25, 65) est dans la case
// (0, 1) ; une cible de rayon 55 centrée en (100, 60), dans la case (2, 1), est à 75 px :
// à portée (5 px bord à bord), mais cachée par le mur. Le rayon est grand parce que le mur
// fait 40 px d'épaisseur. (25, 45) est dans la case du monstre : à portée et en vue ;
// (25, 150) est en vue mais trop loin.
TileMap carteAvecPilier() {
    std::vector<std::string> plan(5, ".....");
    plan[1][1] = '#';
    return TileMap(plan, 40.0f);
}

}  // namespace

TEST_CASE("Monstre.peutAtteindre exige la portee ET la vue") {
    const TileMap carte = carteAvecMurMince();
    const Monstre monstre({25.0f, 45.0f}, 1, 0, 0);

    REQUIRE_FALSE(monstre.peutAtteindre(pointCible({60.0f, 45.0f}), carte));   // à portée, caché
    REQUIRE_FALSE(monstre.peutAtteindre(pointCible({25.0f, 150.0f}), carte));  // en vue, trop loin
    REQUIRE(monstre.peutAtteindre(pointCible({25.0f, 15.0f}), carte));         // à portée et en vue
}

TEST_CASE("Monstre.engagerAttaque refuse une proie a portee mais cachee, l'accepte en vue") {
    const TileMap carte = carteAvecMurMince();
    Monstre monstre({25.0f, 45.0f}, 1, 0, 0);

    REQUIRE_FALSE(engager(monstre, pointCible({60.0f, 45.0f}), 1, carte));
    REQUIRE_FALSE(monstre.cibleEngagee().has_value());
    REQUIRE(engager(monstre, pointCible({25.0f, 15.0f}), 1, carte));
    REQUIRE(monstre.cibleEngagee() == 1);
}

TEST_CASE("Monstre en poursuite ne s'arrete pas a portee mais cachee derriere un mur") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0, 0);  // son carré touche le mur sans le chevaucher
    const Cercle proie{{100.0f, 60.0f}, 55.0f};  // à 75 px : à portée (bord à bord), mais cachée
    REQUIRE_FALSE(monstre.peutAtteindre(proie, carte));
    monstre.recevoirDegats(1);  // actif sans dépendre de la vue

    ecoulerTemps(monstre, 0.3f, proie, carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(25.0f, 65.0f));
}

// Cible de frappe : la proie (place 1) si elle est atteignable, sinon le
// candidat atteignable le plus proche. Les identifiants sont des places.

namespace {

void ecoulerTempsAvecCandidats(Monstre& monstre, float duree, std::optional<EntiteVue> proie,
                               const std::vector<EntiteVue>& candidats, const TileMap& carte) {
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, {}, {});
    }
}

}  // namespace

TEST_CASE("Monstre.engagerAttaque prefere la proie atteignable a un candidat plus proche") {
    Monstre monstre({30.0f, 60.0f}, 1, 0, 0);
    const EntiteVue proie{1, pointCible({55.0f, 60.0f})};
    const EntiteVue plusProche{2, pointCible({40.0f, 60.0f})};

    REQUIRE(monstre.engagerAttaque(proie, {plusProche, proie}, carteOuverte()));

    REQUIRE(monstre.cibleEngagee() == 1);
}

TEST_CASE("Monstre.engagerAttaque frappe un autre candidat si la proie est cachee") {
    const TileMap carte = carteAvecMurMince();
    Monstre monstre({25.0f, 45.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, pointCible({60.0f, 45.0f})};
    const EntiteVue enVue{2, pointCible({25.0f, 15.0f})};

    REQUIRE(monstre.engagerAttaque(proieCachee, {proieCachee, enVue}, carte));

    REQUIRE(monstre.cibleEngagee() == 2);
}

TEST_CASE("Monstre.engagerAttaque choisit le candidat atteignable le plus proche") {
    Monstre monstre({30.0f, 60.0f}, 1, 0, 0);
    const EntiteVue proieLointaine{1, pointCible({30.0f, 300.0f})};
    const EntiteVue moinsProche{2, pointCible({55.0f, 60.0f})};
    const EntiteVue plusProche{3, pointCible({40.0f, 60.0f})};

    REQUIRE(monstre.engagerAttaque(proieLointaine, {proieLointaine, moinsProche, plusProche}, carteOuverte()));

    REQUIRE(monstre.cibleEngagee() == 3);
}

TEST_CASE("Monstre.engagerAttaque est refuse sans aucun candidat atteignable") {
    Monstre monstre({30.0f, 60.0f}, 1, 0, 0);
    const EntiteVue loin{1, pointCible({300.0f, 60.0f})};

    REQUIRE_FALSE(monstre.engagerAttaque(loin, {loin}, carteOuverte()));
    REQUIRE_FALSE(monstre.engagerAttaque(std::nullopt, {}, carteOuverte()));
}

TEST_CASE("Monstre.mettreAJour s'arrete en poursuite quand un autre candidat devient atteignable") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, {{100.0f, 60.0f}, 55.0f}};
    const EntiteVue enVue{2, {{25.0f, 45.0f}, 10.0f}};
    REQUIRE(monstre.peutAtteindre(enVue.cercle, carte));
    monstre.recevoirDegats(1);  // actif sans dépendre de la vue

    ecoulerTempsAvecCandidats(monstre, 0.3f, proieCachee, {proieCachee, enVue}, carte);

    REQUIRE(monstre.getCentre() == sf::Vector2f(25.0f, 65.0f));
}

TEST_CASE("Monstre.mettreAJour continue vers sa proie si aucun candidat n'est atteignable") {
    const TileMap carte = carteAvecPilier();
    Monstre monstre({25.0f, 65.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, {{100.0f, 60.0f}, 55.0f}};
    const EntiteVue tropLoin{2, {{25.0f, 150.0f}, 10.0f}};
    monstre.recevoirDegats(1);

    ecoulerTempsAvecCandidats(monstre, 0.3f, proieCachee, {proieCachee, tropLoin}, carte);

    REQUIRE(monstre.getCentre() != sf::Vector2f(25.0f, 65.0f));
}

TEST_CASE("Monstre le coup engage touche la place choisie et rend les degats") {
    const TileMap carte = carteAvecMurMince();
    Monstre monstre({25.0f, 45.0f}, 1, 0, 0);
    const EntiteVue proieCachee{1, pointCible({60.0f, 45.0f})};
    const EntiteVue enVue{2, pointCible({25.0f, 15.0f})};
    REQUIRE(monstre.engagerAttaque(proieCachee, {proieCachee, enVue}, carte));
    Generateur generateur(s_graineDeTest);

    int degats = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && degats == 0; ++i) {
        REQUIRE(monstre.cibleEngagee() == 2);
        degats = monstre.avancerAttaque(s_dureeTic, generateur);
    }

    REQUIRE(degats == monstre.getPuissanceAttaque());
}

TEST_CASE("Monstre ne contourne pas un personnage entre lui et sa proie : il s'arrete pour le frapper") {
    const TileMap carte = carteDegagee10x10();
    Monstre monstre({60.0f, 200.0f}, 1, 0, 0);
    const EntiteVue proie{1, pointCible({240.0f, 200.0f})};
    const EntiteVue devant{2, {{110.0f, 200.0f}, 10.0f}};
    const std::vector<EntiteVue> candidats = {proie, devant};
    const std::vector<Cercle> obstacles = {proie.cercle, devant.cercle};

    for (int i = 0; i < nombreDeTics(3.0f); ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, obstacles, {});
    }

    REQUIRE(monstre.getCentre().x < 110.0f);
    REQUIRE(monstre.engagerAttaque(proie, candidats, carte));
    REQUIRE(monstre.cibleEngagee() == 2);
}
