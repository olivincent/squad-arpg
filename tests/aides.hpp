// ./tests/aides.hpp
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/Attaque.hpp"
#include "combat/Critique.hpp"
#include "combat/Defense.hpp"
#include "combat/ResolutionDuCoup.hpp"
#include "monde/TileMap.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Equipement.hpp"

// Aides de base communes à plusieurs fichiers de tests : graine et générateur, tolérances, cibles,
// cartes, temps. Les fixtures d'Escouade et de Partie sont dans aides_escouade.hpp, les aides des
// tests de Monstre dans aides_monstre.hpp. Chaque fonction est `inline` (définie dans un en-tête).
// Une aide utilisée par un seul fichier reste dans ce fichier, dans un `namespace {}`.

// Graine fixe des tests : la même suite de nombres à chaque exécution.
inline constexpr Generateur::result_type s_graineDeTest = 12345;

// Rayon de vision des personnages de test, et de ceux construits à la main dans les tests.
inline constexpr float s_rayonVisionDeTest = 320.0f;

// Un coup assez fort pour tuer n'importe quelle entité du jeu, quelle que soit son armure.
inline constexpr int s_coupMortel = 100000;

// Équipement de départ des mêlées de test : les mêmes objets que le catalogue des classes, mais
// l'armure qui en résulte est toujours lue dans le catalogue d'objets, jamais écrite en littéral.
inline const std::vector<ObjetDeDepart> s_equipementMeleeDeTest = {{Emplacement::Tete, IdObjet::CasqueDeFer},
                                                                   {Emplacement::Torse, IdObjet::CuirasseDeMailles},
                                                                   {Emplacement::Jambes, IdObjet::JambieresDeFer},
                                                                   {Emplacement::MainGauche, IdObjet::BouclierRond}};

// Somme de l'armure des objets de départ.
inline int armureDe(const std::vector<ObjetDeDepart>& equipement) {
    int total = 0;
    for (const ObjetDeDepart& entree : equipement) {
        total += definitionObjet(entree.objet).armure;
    }
    return total;
}

// Armure d'une mêlée de test.
inline const int s_armureMeleeDeTest = armureDe(s_equipementMeleeDeTest);

// Un générateur neuf à graine fixe. Chaque test en crée un à lui
// (`Generateur generateur = generateurDeTest();`) et le passe par référence :
// jamais de générateur partagé entre deux tests.
inline Generateur generateurDeTest() {
    return Generateur(s_graineDeTest);
}

// Une attaque sans hasard : sans capacité offensive (donc ni critique) et de nature zone (donc jamais esquivée
// ni, plus tard, bloquée) : aucun tirage. Pour les tests qui ne portent ni sur le critique, ni sur l'esquive,
// ni sur le blocage.
inline Attaque attaqueSansHasard(int montant) {
    return {.montant = montant, .capaciteOffensive = 0, .nature = NatureDAttaque::Zone};
}

// Défenses d'un défenseur de test : cette armure et la capacité défensive d'un personnage d'Adresse 10.
inline Defenses defensesDeTest(int armure = 0) {
    return {.armure = armure, .statistiques = {.capaciteOffensive = 0, .capaciteDefensive = 100}};
}

// Le montant d'une attaque qui vient de partir, 0 si rien n'est parti (ce que rendaient avancerAttaque et
// avancerSkill avant de rendre une attaque entière).
inline int montantOuZero(const std::optional<Attaque>& attaque) {
    return attaque.has_value() ? attaque->montant : 0;
}

// Les dégâts qu'une attaque inflige à ce défenseur, selon que le jeu en a fait un critique ou non : la
// multiplication du critique (au tirage 0, donc toujours critique) puis l'armure. Pour les tests de Partie, où
// le tirage n'est pas à eux : ils lisent `critique` dans l'événement et vérifient le montant qui va avec.
inline int degatsAttendus(const Attaque& attaque, const Defenses& defenses, bool critique) {
    int montant = attaque.montant;
    if (critique) {
        const float rapport = rapportDeCapacites(attaque.capaciteOffensive, defenses.statistiques.capaciteDefensive);
        montant = appliquerCritique(montant, rapport, 0.0f).montant;
    }
    return appliquerArmure(montant, defenses.armure);
}

// Fait subir un coup de `montant` dégâts sans hasard (ni tirage) à une entité (`Vitalite`, `Personnage`,
// `Monstre`) et rend les dégâts subis. Le générateur est neuf et jamais lu : l'attaque est sans hasard.
template <typename Entite>
int subir(Entite& entite, int montant) {
    Generateur generateur = generateurDeTest();
    return entite.recevoirDegats(attaqueSansHasard(montant), generateur).montant;
}

// Compare un point à un point attendu avec une tolérance : les positions
// manipulées par les entités sont des flottants accumulés sur plusieurs tics
// (vitesse * dt, racines carrées de sf::Vector2f::length...), donc deux
// calculs mathématiquement égaux peuvent différer de quelques poussières
// d'erreur d'arrondi. Comparer avec `==` serait fragile.
inline void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

// Cible ponctuelle (rayon nul), pour les tests qui ne portent pas sur
// la forme de la cible elle-même.
inline Cercle pointCible(sf::Vector2f centre) {
    return {centre, 0.0f};
}

// Cible de la taille d'un monstre (rayon 15 px), pour les tests de combat
// où la taille de la cible décide si les mêlées peuvent l'atteindre.
inline Cercle cibleMonstre(sf::Vector2f centre) {
    return {centre, 15.0f};
}

// Carte dégagée de 10 × 10 cases de 40 px, sans mur.
inline TileMap carteDegagee10x10() {
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
    return TileMap(plan, 40.0f);
}

// Carte dégagée de 800 × 600 px (cases de 40 px), entourée de murs.
inline TileMap carteDegageeEntouree() {
    std::vector<std::string> plan;
    plan.push_back(std::string(20, '#'));
    for (int i = 0; i < 13; ++i) {
        plan.push_back("#" + std::string(18, '.') + "#");
    }
    plan.push_back(std::string(20, '#'));
    return TileMap(plan, 40.0f);
}

// Grande carte sans mur (4000 × 4000 px) pour les tests qui ne portent pas sur la vue.
inline const TileMap& carteOuverte() {
    static const TileMap carte(std::vector<std::string>(100, std::string(100, '.')), 40.0f);
    return carte;
}

// Durée d'un tic des simulations : 1/60 s (comme setFramerateLimit(60) dans
// main.cpp).
inline constexpr float s_dureeTic = 1.0f / 60.0f;

// Nombre de tics de 1/60 s dans une durée, arrondi au plus proche : calculé
// une fois, puis les boucles comptent des entiers (additionner des float
// jusqu'à la durée accumulerait des arrondis et ferait un tic de trop ou de
// moins, par exemple 5 au lieu de 6 pour 0,1 s).
inline int nombreDeTics(float dureeSecondes) {
    return static_cast<int>(std::lround(dureeSecondes / s_dureeTic));
}
