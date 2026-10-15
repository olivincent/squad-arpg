// ./tests/aides.hpp
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "Aleatoire.hpp"
#include "Cercle.hpp"
#include "Cible.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "MonstreVu.hpp"
#include "TileMap.hpp"
#include "TypeSkill.hpp"

// Aides communes à plusieurs fichiers de tests. Chaque fonction est
// `inline` (définie dans un en-tête). Une aide utilisée par un seul
// fichier reste dans ce fichier, dans un `namespace {}`.

// Graine fixe des tests : la même suite de nombres à chaque exécution.
inline constexpr Generateur::result_type s_graineDeTest = 12345;

// Un générateur neuf à graine fixe. Chaque test en crée un à lui
// (`Generateur generateur = generateurDeTest();`) et le passe par référence :
// jamais de générateur partagé entre deux tests.
inline Generateur generateurDeTest() {
    return Generateur(s_graineDeTest);
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

// Simule le temps qui passe par tics de 1/60 s, pendant la durée demandée.
// Les tests passent une durée généreuse par rapport au trajet attendu et ne
// vérifient que l'état final.
inline void simuler(Escouade& escouade, const TileMap& carte, float dureeSecondes) {
    constexpr float dt = s_dureeTic;
    const int nombreTics = nombreDeTics(dureeSecondes);
    for (int i = 0; i < nombreTics; ++i) {
        escouade.mettreAJour(dt, carte, {});
    }
}

// Comme simuler, avec attaquer() avant mettreAJour() à chaque tic (sans
// monstre), dans l'ordre d'une image de Partie : les skills engagés
// avancent et partent dans attaquer().
inline void simulerAvecAttaque(Escouade& escouade, const TileMap& carte, float dureeSecondes, Generateur& generateur) {
    const int nombreTics = nombreDeTics(dureeSecondes);
    for (int i = 0; i < nombreTics; ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur));
        escouade.mettreAJour(s_dureeTic, carte, {});
    }
}

// Appelle attaquer() par tics de 1/60 s pendant 0,2 s : un peu plus que
// l'exécution de l'attaque de base (0,15 s), moins que sa plus petite recharge
// (0,3 s). Chaque personnage qui atteint un monstre l'engage au premier tic et
// le frappe une fois, au plus ; renvoie tous les coups partis. Aucun
// mettreAJour : personne ne bouge, personne n'est donc « en mouvement ».
inline std::vector<Coup> attaquerJusquAuDepart(Escouade& escouade, const std::vector<MonstreVu>& monstres, const TileMap& carte,
                                               Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        const std::vector<Coup> coupsDuTic = escouade.attaquer(s_dureeTic, monstres, carte, generateur);
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
    }
    return coups;
}

// Comme attaquerJusquAuDepart() avec un seul monstre (identifiant 1) ; renvoie
// le total des dégâts de la liste de coups.
inline int attaquerCible(Escouade& escouade, const TileMap& carte, Cercle cible) {
    int total = 0;
    Generateur generateur = generateurDeTest();
    for (const Coup& coup : attaquerJusquAuDepart(escouade, {MonstreVu{1, cible}}, carte, generateur)) {
        total += coup.degats;
    }
    return total;
}

// Place de l'allié désigné par choisirCible, vide si aucune cible.
inline std::optional<std::size_t> placeAlliePourSoin(const Escouade& escouade) {
    const std::optional<Cible> cible = escouade.choisirCible(TypeSkill::Soin, 3, {}, carteOuverte());
    if (!cible.has_value()) {
        return std::nullopt;
    }
    return std::get<CibleAllie>(*cible).place;
}
