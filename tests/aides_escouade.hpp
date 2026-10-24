// ./tests/aides_escouade.hpp
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "combat/AdresseSkill.hpp"
#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/Cible.hpp"
#include "escouade/Composition.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/IdSkill.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Partie.hpp"
#include "combat/Skill.hpp"
#include "monde/TileMap.hpp"
#include "combat/TypeSkill.hpp"

// Fixtures d'Escouade et de Partie communes à plusieurs fichiers de tests : personnages et escouades
// de test, simulations, niveaux de Partie. Séparées d'aides.hpp (étape 80) : un fichier de tests qui
// n'en a pas besoin (Mobile, Chemin…) ne compile pas toute la logique de jeu.

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
inline constexpr int s_degatsBaseDeTest = 50;
inline constexpr float s_executionBaseDeTest = 0.15f;  // en secondes
inline constexpr float s_porteeMeleeDeTest = 20.0f;    // bord à bord
inline constexpr float s_porteeTireurDeTest = 120.0f;  // bord à bord
inline constexpr float s_rechargeMeleeMinDeTest = 0.3f;  // en secondes
inline constexpr float s_rechargeMeleeMaxDeTest = 0.7f;
inline constexpr float s_rechargeTireurMinDeTest = 0.4f;
inline constexpr float s_rechargeTireurMaxDeTest = 0.6f;
// Contrôle (les tireurs n'ont ni équipement ni zone de contrôle). L'armure d'une mêlée est dans aides.hpp.
inline constexpr float s_zoneDeControleMeleeDeTest = 20.0f;  // bord à bord
// Frappe puissante du guerrier.
inline constexpr int s_degatsFrappeDeTest = 250;
inline constexpr float s_rechargeFrappeDeTest = 5.0f;     // en secondes
inline constexpr float s_activationFrappeDeTest = 0.3f;   // en secondes
// Provocation du guerrier.
inline constexpr float s_rayonProvocationDeTest = 150.0f;  // depuis le centre du provocateur
inline constexpr int s_dureeProvocationDeTest = 4;         // en secondes (la puissance du skill)
inline constexpr float s_rechargeProvocationDeTest = 10.0f;
inline constexpr float s_activationProvocationDeTest = 0.2f;
// Soin du prêtre.
inline constexpr float s_porteeSoinDeTest = 40.0f;  // bord à bord
inline constexpr int s_puissanceSoinDeTest = 300;
inline constexpr float s_rechargeSoinDeTest = 8.0f;
inline constexpr float s_activationSoinDeTest = 0.6f;
// Explosion du mage (skill de zone).
inline constexpr float s_porteeZoneDeTest = 250.0f;  // portée de lancement, bord à bord
inline constexpr int s_degatsZoneDeTest = 150;
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
            s_equipementMeleeDeTest,
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
            {},
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
            {},
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

// Un coup du monstre `idMonstre`, posté en `origine`, sur la proie que l'escouade lui donne
// (proieDuMonstre : provocateur, sinon le plus proche). Renvoie les dégâts subis ; 0 si personne n'est
// touchable. Le chemin d'un coup de Partie, sans Partie.
inline int infliger(Escouade& escouade, int idMonstre, sf::Vector2f origine, int quantite) {
    const std::optional<EntiteVue> proie = escouade.proieDuMonstre(idMonstre, origine);
    if (!proie.has_value()) {
        return 0;
    }
    return escouade.recevoirCoup(idMonstre, static_cast<std::size_t>(proie->id), quantite);
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

// Niveaux et aides des tests de Partie (étape 80 : partagés par les fichiers test_Partie_*).

// Niveau dégagé de 15 × 15 cases de 40 px (600 × 600 px), sans mur
// intérieur ; l'escouade part de (200, 200), son centre est donc en (210, 210).
// Guerrier en (235, 210), tireurs en (185, 185) et (185, 235).
inline Niveau niveauDegageEnGroupes(const std::vector<MonstreDuNiveau>& monstres) {
    return {std::vector<std::string>(15, std::string(15, '.')), 40.0f, {200.0f, 200.0f}, monstres};
}

// Chaque monstre a son propre groupe.
inline Niveau niveauDegage(const std::vector<sf::Vector2f>& monstres) {
    std::vector<MonstreDuNiveau> deLaPartie;
    for (const sf::Vector2f position : monstres) {
        deLaPartie.push_back({position, static_cast<int>(deLaPartie.size())});
    }
    return niveauDegageEnGroupes(deLaPartie);
}

// Fait avancer la partie de ce nombre de tics de 1/60 s.
inline void avancer(Partie& partie, int nombreDeTicsAFaire) {
    for (int i = 0; i < nombreDeTicsAFaire; ++i) {
        partie.mettreAJour(s_dureeTic);
    }
}

// Une grille de 30 monstres serrés devant l'escouade, assez pour la vaincre.
inline std::vector<sf::Vector2f> groupeDeMonstres() {
    std::vector<sf::Vector2f> monstres;
    for (int colonne = 0; colonne < 5; ++colonne) {
        for (int ligne = 0; ligne < 6; ++ligne) {
            monstres.push_back({300.0f + 35.0f * static_cast<float>(colonne), 150.0f + 35.0f * static_cast<float>(ligne)});
        }
    }
    return monstres;
}

// Retour au poste (étape 69). Couloir dégagé de 60 × 15 cases de 40 px (2400 × 600) : le
// monstre est à son poste (400, 300), l'escouade en (560, 300), à 160 px de lui. L'escouade
// s'enfuit vers l'est : le monstre la poursuit jusqu'à 600 px de son poste (x = 1000), puis
// abandonne et rentre.
inline const sf::Vector2f s_posteDuFuyard{400.0f, 300.0f};

inline Niveau niveauDeLaFuite() {
    return {std::vector<std::string>(15, std::string(60, '.')), 40.0f, {550.0f, 290.0f}, {{s_posteDuFuyard, 0}}};
}

inline const Monstre& fuyard(const Partie& partie) {
    return partie.getMonstres().front();
}

// Envoie l'escouade au point d'arrivée et avance jusqu'à ce que le monstre abandonne (retour au poste).
inline void attirerHorsDuTerritoire(Partie& partie, sf::Vector2f arrivee) {
    partie.ordonnerDeplacement(arrivee);
    for (int i = 0; i < nombreDeTics(15.0f) && fuyard(partie).estCiblable(); ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE_FALSE(fuyard(partie).estCiblable());
}

// Fait avancer la partie d'un tic et renvoie les points de vie du personnage de cette place.
inline int pointsDeVieApresUnTic(Partie& partie, std::size_t place) {
    partie.mettreAJour(s_dureeTic);
    return *partie.getEscouade().pointsDeVieDuPersonnage(place);
}

// Avance jusqu'à ce que le personnage de cette place perde des points de vie (le premier coup du
// monstre) ; renvoie ses points de vie à ce moment. Le coup lance la recharge du monstre (0,7 s au
// moins) puis un nouveau coup exige 0,3 s d'exécution : aucun coup normal avant 1 s.
inline int jusquAuPremierCoup(Partie& partie, std::size_t place) {
    const int avant = *partie.getEscouade().pointsDeVieDuPersonnage(place);
    int pointsDeVie = avant;
    for (int i = 0; i < nombreDeTics(2.0f) && pointsDeVie == avant; ++i) {
        pointsDeVie = pointsDeVieApresUnTic(partie, place);
    }
    REQUIRE(pointsDeVie < avant);
    return pointsDeVie;
}

inline constexpr float s_fenetreSansCoupNormal = 0.9f;  // en secondes, après le premier coup du monstre

// Aides des tests de skills d'Escouade (étape 80 : partagées par les fichiers test_Escouade_skills*).

// Le monstre d'identifiant 1 tel que l'escouade le voit, seul dans la liste.
inline std::vector<EntiteVue> unSeulMonstre(Cercle cercle) {
    return {EntiteVue{1, cercle}};
}

// Déclenche le skill de cette place et renvoie le total des dégâts de la
// liste de coups.
inline int declencher(Escouade& escouade, AdresseSkill adresse, const std::vector<EntiteVue>& monstres) {
    int total = 0;
    Generateur generateur = generateurDeTest();
    for (const Coup& coup : declencherEtAttendre(escouade, adresse, monstres, carteOuverte(), generateur)) {
        total += coup.degats;
    }
    return total;
}
