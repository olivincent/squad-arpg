// ./tests/aides.hpp
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "AdresseSkill.hpp"
#include "Aleatoire.hpp"
#include "Cercle.hpp"
#include "Cible.hpp"
#include "Composition.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "EntiteVue.hpp"
#include "IdSkill.hpp"
#include "Niveau.hpp"
#include "Partie.hpp"
#include "Skill.hpp"
#include "TileMap.hpp"
#include "TypeSkill.hpp"

// Aides communes à plusieurs fichiers de tests. Chaque fonction est
// `inline` (définie dans un en-tête). Une aide utilisée par un seul
// fichier reste dans ce fichier, dans un `namespace {}`.

// Graine fixe des tests : la même suite de nombres à chaque exécution.
inline constexpr Generateur::result_type s_graineDeTest = 12345;

// Adresses des quatre skills de l'escouade par défaut (touches 1 à 4 du jeu) :
// le guerrier (place 1) a la frappe et la provocation, le prêtre (place 2) le soin,
// le mage (place 3) l'explosion.
inline constexpr AdresseSkill s_adresseFrappe{1, IdSkill::FrappePuissante};
inline constexpr AdresseSkill s_adresseProvocation{1, IdSkill::Provocation};
inline constexpr AdresseSkill s_adresseSoin{2, IdSkill::Soin};
inline constexpr AdresseSkill s_adresseExplosion{3, IdSkill::Explosion};

// Valeurs des personnages de test, figées : celles du catalogue au moment où elles ont été recopiées
// (étape 80), mais indépendantes de lui. Rééquilibrer une classe dans Specialite.cpp ne change aucun
// test de règle ; test_Specialite.cpp est le seul fichier de tests qui lit les valeurs du catalogue.
// Attaque de base (mêlées et tireurs).
inline constexpr int s_degatsBaseDeTest = 5;
inline constexpr float s_executionBaseDeTest = 0.15f;  // en secondes
inline constexpr float s_porteeMeleeDeTest = 20.0f;    // bord à bord
inline constexpr float s_porteeTireurDeTest = 120.0f;  // bord à bord
inline constexpr float s_rechargeMeleeMinDeTest = 0.3f;  // en secondes
inline constexpr float s_rechargeMeleeMaxDeTest = 0.7f;
inline constexpr float s_rechargeTireurMinDeTest = 0.4f;
inline constexpr float s_rechargeTireurMaxDeTest = 0.6f;
// Défense et contrôle (les tireurs n'ont ni armure ni zone de contrôle).
inline constexpr int s_armureMeleeDeTest = 50;
inline constexpr float s_zoneDeControleMeleeDeTest = 20.0f;  // bord à bord
// Rayon de vision des personnages de test, et de ceux construits à la main dans les tests.
inline constexpr float s_rayonVisionDeTest = 320.0f;
// Frappe puissante du guerrier.
inline constexpr int s_degatsFrappeDeTest = 25;
inline constexpr float s_rechargeFrappeDeTest = 5.0f;     // en secondes
inline constexpr float s_activationFrappeDeTest = 0.3f;   // en secondes
// Provocation du guerrier.
inline constexpr float s_rayonProvocationDeTest = 150.0f;  // depuis le centre du provocateur
inline constexpr int s_dureeProvocationDeTest = 4;         // en secondes (la puissance du skill)
inline constexpr float s_rechargeProvocationDeTest = 10.0f;
inline constexpr float s_activationProvocationDeTest = 0.2f;
// Soin du prêtre.
inline constexpr float s_porteeSoinDeTest = 40.0f;  // bord à bord
inline constexpr int s_puissanceSoinDeTest = 30;
inline constexpr float s_rechargeSoinDeTest = 8.0f;
inline constexpr float s_activationSoinDeTest = 0.6f;
// Explosion du mage (skill de zone).
inline constexpr float s_porteeZoneDeTest = 250.0f;  // portée de lancement, bord à bord
inline constexpr int s_degatsZoneDeTest = 15;
inline constexpr float s_rechargeZoneDeTest = 8.0f;
inline constexpr float s_activationZoneDeTest = 0.8f;
inline constexpr float s_rayonZoneDeTest = 60.0f;   // depuis le point visé

inline Skill attaqueDeMeleeDeTest() {
    return Skill(TypeSkill::Degats, s_porteeMeleeDeTest, s_degatsBaseDeTest, s_rechargeMeleeMinDeTest,
                 s_rechargeMeleeMaxDeTest, 0.0f, s_executionBaseDeTest);
}

inline Skill attaqueDeTireurDeTest() {
    return Skill(TypeSkill::Degats, s_porteeTireurDeTest, s_degatsBaseDeTest, s_rechargeTireurMinDeTest,
                 s_rechargeTireurMaxDeTest, 0.0f, s_executionBaseDeTest);
}

// Guerrier de test : devant, mêlée, frappe puissante et provocation.
inline DefinitionPersonnage guerrierDeTest() {
    return {sf::Color::Red,
            attaqueDeMeleeDeTest(),
            {{IdSkill::FrappePuissante, Skill(TypeSkill::Degats, s_porteeMeleeDeTest, s_degatsFrappeDeTest, s_rechargeFrappeDeTest,
                                              s_rechargeFrappeDeTest, 0.0f, s_activationFrappeDeTest)},
             {IdSkill::Provocation, Skill(TypeSkill::Provocation, 0.0f, s_dureeProvocationDeTest, s_rechargeProvocationDeTest,
                                          s_rechargeProvocationDeTest, s_rayonProvocationDeTest, s_activationProvocationDeTest)}},
            s_armureMeleeDeTest,
            s_rayonVisionDeTest,
            s_zoneDeControleMeleeDeTest,
            Rang::Avant};
}

// Prêtre de test : derrière, tireur, soin.
inline DefinitionPersonnage pretreDeTest() {
    return {sf::Color(255, 240, 150),
            attaqueDeTireurDeTest(),
            {{IdSkill::Soin, Skill(TypeSkill::Soin, s_porteeSoinDeTest, s_puissanceSoinDeTest, s_rechargeSoinDeTest,
                                   s_rechargeSoinDeTest, 0.0f, s_activationSoinDeTest)}},
            0,
            s_rayonVisionDeTest,
            0.0f,
            Rang::Arriere};
}

// Mage de test : derrière, tireur, explosion (skill de zone).
inline DefinitionPersonnage mageDeTest() {
    return {sf::Color(160, 80, 220),
            attaqueDeTireurDeTest(),
            {{IdSkill::Explosion, Skill(TypeSkill::Zone, s_porteeZoneDeTest, s_degatsZoneDeTest, s_rechargeZoneDeTest,
                                        s_rechargeZoneDeTest, s_rayonZoneDeTest, s_activationZoneDeTest)}},
            0,
            s_rayonVisionDeTest,
            0.0f,
            Rang::Arriere};
}

// La composition par défaut des tests : guerrier, prêtre, mage (celle du jeu avant l'écran de choix).
inline std::vector<DefinitionPersonnage> compositionDeTest() {
    return {guerrierDeTest(), pretreDeTest(), mageDeTest()};
}

// Une escouade / une partie avec la composition par défaut des tests.
inline Escouade uneEscouade(sf::Vector2f positionDepart) {
    return Escouade(positionDepart, compositionDeTest());
}

inline Partie unePartie(const Niveau& niveau, Generateur::result_type graine) {
    return Partie(niveau, compositionDeTest(), graine);
}

// Une escouade de quatre : deux guerriers devant (mêlées), le prêtre et le mage derrière. Pour les tests
// de places d'attaque et de séparation qui ont besoin de deux mêlées (la composition par défaut n'en a qu'une).
inline Escouade uneEscouadeADeuxGuerriers(sf::Vector2f positionDepart) {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition.insert(composition.begin(), composition[0]);
    return Escouade(positionDepart, std::move(composition));
}

// Une escouade par défaut dont le prêtre a un soin de portée plus courte (33 px bord à bord,
// soit 53 px entre centres) : le guerrier (55,9 px du prêtre) est hors de portée, le mage (50 px)
// à portée. Dans le triangle de la composition par défaut, tous les alliés sont à portée du
// soin normal (40 px) : il faut ce soin court pour tester un soin hors de portée.
inline Escouade uneEscouadeAuSoinCourt(sf::Vector2f positionDepart) {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition[1].skills = {{IdSkill::Soin, Skill(TypeSkill::Soin, 33.0f, s_puissanceSoinDeTest, s_rechargeSoinDeTest,
                                                   s_rechargeSoinDeTest, 0.0f, s_activationSoinDeTest)}};
    return Escouade(positionDepart, std::move(composition));
}

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
inline std::vector<Coup> attaquerJusquAuDepart(Escouade& escouade, const std::vector<EntiteVue>& monstres, const TileMap& carte,
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
    for (const Coup& coup : attaquerJusquAuDepart(escouade, {EntiteVue{1, cible}}, carte, generateur)) {
        total += coup.degats;
    }
    return total;
}

// Le plus long temps d'activation d'un skill (la zone, 0,8 s) avec une marge.
inline constexpr float s_dureeAttenteSkill = 1.0f;

// Fait avancer par attaquer() (sans mettreAJour : personne ne bouge, les
// recharges ne courent pas) pendant `s_dureeAttenteSkill`, et renvoie les coups
// de skill partis. Les coups de l'attaque de base (s_degatsBaseDeTest) que les
// autres personnages portent entre-temps sont écartés : seuls les skills comptent
// (leurs dégâts doivent donc différer de ceux de l'attaque de base).
static_assert(s_degatsFrappeDeTest != s_degatsBaseDeTest && s_degatsZoneDeTest != s_degatsBaseDeTest);
inline std::vector<Coup> attendreLeDepartDesSkills(Escouade& escouade, const std::vector<EntiteVue>& monstres, const TileMap& carte,
                                                   Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(s_dureeAttenteSkill); ++i) {
        for (const Coup& coup : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            if (coup.degats != s_degatsBaseDeTest) {
                coups.push_back(coup);
            }
        }
    }
    return coups;
}

// Déclenche le skill de cette place (touches 1 à 4), puis attend son départ :
// les coups de skill sont ceux de l'appel et ceux de l'attente.
inline std::vector<Coup> declencherEtAttendre(Escouade& escouade, AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte,
                                              Generateur& generateur) {
    std::vector<Coup> coups = escouade.declencherSkill(adresse, monstres, carte, generateur);
    const std::vector<Coup> suite = attendreLeDepartDesSkills(escouade, monstres, carte, generateur);
    coups.insert(coups.end(), suite.begin(), suite.end());
    return coups;
}

// Comme declencherEtAttendre, pour le skill de zone lancé en un point.
inline std::vector<Coup> declencherZoneEtAttendre(Escouade& escouade, AdresseSkill adresse, sf::Vector2f point, const std::vector<EntiteVue>& monstres,
                                                  const TileMap& carte, Generateur& generateur) {
    std::vector<Coup> coups = escouade.declencherSkillDeZone(adresse, point, monstres, carte, generateur);
    const std::vector<Coup> suite = attendreLeDepartDesSkills(escouade, monstres, carte, generateur);
    coups.insert(coups.end(), suite.begin(), suite.end());
    return coups;
}

// Place de l'allié désigné par choisirCible, vide si aucune cible.
inline std::optional<std::size_t> placeAlliePourSoin(const Escouade& escouade) {
    const std::optional<Cible> cible = escouade.choisirCible(TypeSkill::Soin, s_adresseSoin, {}, carteOuverte());
    if (!cible.has_value()) {
        return std::nullopt;
    }
    return std::get<CibleAllie>(*cible).place;
}
