// ./tests/jeu/test_Partie_performance.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Aleatoire.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/ChampDeVision.hpp"
#include "monde/GenerateurDonjon.hpp"
#include "monde/Niveau.hpp"
#include "monde/TileMap.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"

// Mesure de performance (étape 93), cachée de la suite normale : à lancer avec le preset `jeu` (Release), voir
// `docs/tests.md`. Elle n'affirme rien sur les temps : elle les affiche.

namespace {
constexpr int s_nombreDImages = 600;
constexpr int s_graineMax = 2000;           // on cherche, parmi les premières graines, un niveau avec une nuée de 7
constexpr int s_distanceAuContact = 3;      // en cases : où l'escouade est posée par rapport au premier monstre de la nuée

// Un monstre de la partie est-il de la nuée ? Il n'en connaît pas la catégorie : on le reconnaît à sa taille.
bool estUneNuee(const Monstre& monstre) {
    return monstre.getCercle().rayon == definitionDeMonstre(CategorieDeMonstre::Nuee).cote / 2.0f;
}

// La nuée la plus nombreuse d'un niveau : le groupe et ses monstres.
std::vector<MonstreDuNiveau> laPlusGrosseNuee(const Niveau& niveau) {
    std::vector<MonstreDuNiveau> meilleure;
    for (const MonstreDuNiveau& monstre : niveau.monstres) {
        std::vector<MonstreDuNiveau> nuee;
        for (const MonstreDuNiveau& autre : niveau.monstres) {
            if (autre.groupe == monstre.groupe && autre.categorie == CategorieDeMonstre::Nuee) {
                nuee.push_back(autre);
            }
        }
        if (nuee.size() > meilleure.size()) {
            meilleure = nuee;
        }
    }
    return meilleure;
}

// Le niveau de la première graine qui contient une nuée de 7 (à défaut, la plus grosse trouvée).
Niveau niveauAvecUneNuee() {
    Niveau retenu;
    std::size_t tailleRetenue = 0;
    for (Generateur::result_type graine = 0; graine < s_graineMax; ++graine) {
        Generateur generateur(graine);
        Niveau niveau = genererNiveau(generateur, ParametresDonjon{});
        const std::size_t taille = laPlusGrosseNuee(niveau).size();
        if (taille > tailleRetenue) {
            tailleRetenue = taille;
            retenu = std::move(niveau);
        }
        if (tailleRetenue >= 7) {
            break;
        }
    }
    return retenu;
}

// Une case de sol à `s_distanceAuContact` cases du premier monstre de la nuée, en vue de lui et loin des autres monstres.
std::optional<sf::Vector2f> departAuContact(const Niveau& niveau, const std::vector<MonstreDuNiveau>& nuee) {
    const TileMap carte(niveau.plan, niveau.tailleCase);
    const sf::Vector2i premier = carte.caseContenant(nuee.front().position);
    for (int decalageX = -s_distanceAuContact; decalageX <= s_distanceAuContact; ++decalageX) {
        for (int decalageY = -s_distanceAuContact; decalageY <= s_distanceAuContact; ++decalageY) {
            if (std::max(std::abs(decalageX), std::abs(decalageY)) != s_distanceAuContact) {
                continue;
            }
            const sf::Vector2i candidate = premier + sf::Vector2i(decalageX, decalageY);
            const bool libre =
                candidate.x >= 0 && candidate.y >= 0 && carte.getTile(candidate.x, candidate.y) == Tile::Floor &&
                estVisibleDepuis(carte, premier, candidate) &&
                std::ranges::none_of(niveau.monstres, [&](const MonstreDuNiveau& monstre) {
                    return carte.caseContenant(monstre.position) == candidate;
                });
            if (libre) {
                return carte.centreDeCase(candidate);
            }
        }
    }
    return std::nullopt;
}
}  // namespace

TEST_CASE("perf : une image de Partie au contact d'une nuee de 7", "[.][perf]") {
    Niveau niveau = niveauAvecUneNuee();
    const std::vector<MonstreDuNiveau> nuee = laPlusGrosseNuee(niveau);
    REQUIRE_FALSE(nuee.empty());
    const std::optional<sf::Vector2f> depart = departAuContact(niveau, nuee);
    REQUIRE(depart.has_value());
    niveau.departEscouade = *depart;
    // Une escouade increvable : le combat dure, au lieu de s'arrêter à la défaite.
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = avecEnduranceEnorme(std::move(definition));
    }
    Partie partie(niveau, std::move(composition), s_graineDeTest);

    double sommeEnCombat = 0.0;
    double piresEnCombat = 0.0;
    int imagesEnCombat = 0;
    double sommeTotale = 0.0;
    double pireTotal = 0.0;
    for (int image = 0; image < s_nombreDImages; ++image) {
        const bool nueeEnVie = std::ranges::any_of(partie.getMonstres(), estUneNuee);
        const auto debut = std::chrono::steady_clock::now();
        partie.mettreAJour(s_dureeTic);
        const std::chrono::duration<double, std::milli> duree = std::chrono::steady_clock::now() - debut;
        sommeTotale += duree.count();
        pireTotal = std::max(pireTotal, duree.count());
        if (nueeEnVie) {
            sommeEnCombat += duree.count();
            piresEnCombat = std::max(piresEnCombat, duree.count());
            ++imagesEnCombat;
        }
    }

    WARN("nuee de " << nuee.size() << " monstres, " << niveau.monstres.size() << " monstres dans le niveau");
    WARN("sur " << s_nombreDImages << " images : moyenne " << sommeTotale / s_nombreDImages << " ms, pire "
                << pireTotal << " ms");
    WARN("dont " << imagesEnCombat << " images avec une nuee en vie : moyenne "
                 << (imagesEnCombat > 0 ? sommeEnCombat / imagesEnCombat : 0.0) << " ms, pire " << piresEnCombat << " ms");
    WARN("nuees restantes : " << std::ranges::count_if(partie.getMonstres(), estUneNuee));
}
