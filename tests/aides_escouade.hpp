// ./tests/aides_escouade.hpp
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "combat/AdresseSkill.hpp"
#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Cible.hpp"
#include "combat/DescriptionDAttaque.hpp"
#include "combat/Element.hpp"
#include "escouade/Composition.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "escouade/Personnage.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/IdSkill.hpp"
#include "combat/Place.hpp"
#include "combat/ResultatDEffet.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "monstres/Monstre.hpp"
#include "objets/Affixe.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"
#include "monde/Niveau.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/ObjetAuSol.hpp"
#include "jeu/Partie.hpp"
#include "combat/Skill.hpp"
#include "monde/TileMap.hpp"
#include "combat/TypeDArme.hpp"
#include "combat/ModeDeCible.hpp"
#include "combat/EffetDeSkill.hpp"

// Fixtures d'Escouade et de Partie communes à plusieurs fichiers de tests : personnages et escouades
// de test, simulations, niveaux de Partie. Séparées d'aides.hpp (étape 80) : un fichier de tests qui
// n'en a pas besoin (Mobile, Chemin…) ne compile pas toute la logique de jeu.

// Adresses des quatre skills de l'escouade par défaut (touches 1 à 4 du jeu) :
// le guerrier (place 1) a la frappe et la provocation, le prêtre (place 2) le soin,
// le mage (place 3) l'explosion.
inline constexpr AdresseSkill s_adresseFrappe{Place{1}, IdSkill::FrappePuissante};
inline constexpr AdresseSkill s_adresseProvocation{Place{1}, IdSkill::Provocation};
inline constexpr AdresseSkill s_adresseSoin{Place{2}, IdSkill::Soin};
inline constexpr AdresseSkill s_adresseExplosion{Place{3}, IdSkill::Explosion};

// Valeurs des personnages de test, figées : celles du catalogue des classes au moment où elles ont été recopiées
// (étape 80), mais indépendantes de lui. Rééquilibrer une classe dans Specialite.cpp ne change aucun
// test de règle ; test_Specialite.cpp est le seul fichier de tests qui lit les valeurs du catalogue des classes.
// Exception depuis l'étape 91 : l'attaque de base vient de l'arme que le personnage porte, donc du catalogue des
// objets ; les mêlées de test portent l'épée à une main, le prêtre le sceptre, le mage le bâton, et les constantes ci-dessous
// **sont lues** dans la description de leur attaque (`descriptionDeLArme`, jamais écrites en littéral).
// Depuis l'étape 102, les dégâts d'une arme sont une plage dont chaque coup tire sa valeur : un test ne compare plus un coup à un
// nombre, il vérifie qu'il est dans la plage (`estUnCoupDeBase`, `sontDesCoupsDeBase`, `dansLaPlage`).
// Caractéristiques des personnages de test : la Puissance de référence (10), pour que l'attaque de base, qui en profite
// à plein, fasse exactement les dégâts de son arme ; les quatre autres à 0 (Adresse 0 : capacité de 50, Endurance 0 :
// 240 points de vie), comme avant l'étape 91.
inline const Caracteristiques s_caracteristiquesDeTest({s_valeurDeReference, 0, 0, 0, 0});

// Attaque de base (mêlées et tireurs).
inline const DescriptionDAttaque s_attaqueDeMeleeDeTest = descriptionDeLArme(Objet{IdObjet::EpeeAUneMain});
inline const DescriptionDAttaque s_attaqueDeTireurDeTest = descriptionDeLArme(Objet{IdObjet::Sceptre});
// La plage des dégâts physiques de la mêlée de test (l'épée à une main), et le coup de base de n'importe quelle arme.
inline const int s_degatsBaseMinDeTest = s_attaqueDeMeleeDeTest.degats.min[Element::Physique];
inline const int s_degatsBaseMaxDeTest = s_attaqueDeMeleeDeTest.degats.max[Element::Physique];

// Ce total est-il dans la plage de cette arme, avec la Puissance de référence ? (un seul coup)
inline bool dansLaPlage(int degats, const DescriptionDAttaque& arme) {
    return degats >= arme.degats.min.total() && degats <= arme.degats.max.total();
}

// Ce total est-il un coup de cette arme pour un porteur de ces caractéristiques ? La Puissance joue sur chaque borne de la plage
// (`montantEffectif`, croissant avec la base : un coup tiré dans la plage reste entre les deux bornes ainsi calculées).
inline bool dansLaPlageAvecPuissance(int degats, const DescriptionDAttaque& arme, const Caracteristiques& caracteristiques) {
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    return degats >= montantEffectif(arme.degats.min.total(), puissance, caracteristiques) &&
           degats <= montantEffectif(arme.degats.max.total(), puissance, caracteristiques);
}

// Ce montant, infligé à ce défenseur, est-il celui d'un coup de cette arme ? `modele` donne ce que l'attaque porte hors des dégâts
// (capacité offensive, nature) ; les dégâts vont de la borne basse à la borne haute de la plage, `critique` dit si le coup l'était
// (`degatsAttendus`, croissant avec les dégâts), pour un porteur de ces caractéristiques (la Puissance de référence par défaut).
inline bool montantAttendu(int montant, const Attaque& modele, const DescriptionDAttaque& arme, const Defenses& defenses, bool critique,
                           const Caracteristiques& porteur = s_caracteristiquesDeTest) {
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    Attaque basse = modele;
    Attaque haute = modele;
    for (const Element element : s_elements) {
        basse.degats[element] = montantEffectif(arme.degats.min[element], puissance, porteur);
        haute.degats[element] = montantEffectif(arme.degats.max[element], puissance, porteur);
    }
    return montant >= degatsAttendus(basse, defenses, critique) && montant <= degatsAttendus(haute, defenses, critique);
}

// Ce total est-il celui de `nombre` coups de l'attaque de base de la mêlée de test ? Chaque coup est dans la plage : le total
// est entre `nombre` fois le minimum et `nombre` fois le maximum.
inline bool sontDesCoupsDeBase(int total, int nombre) {
    return total >= nombre * s_degatsBaseMinDeTest && total <= nombre * s_degatsBaseMaxDeTest;
}

inline bool estUnCoupDeBase(int degats) {
    return sontDesCoupsDeBase(degats, 1);
}

// Ce total est-il celui de `nombre` coups de base de personnages de la composition par défaut (épée à une main, sceptre, bâton,
// qui ne frappent pas du même nombre) ? Quels que soient ceux qui frappent, le total est entre les `nombre` plus petits
// minimums et les `nombre` plus grands maximums ; avec les trois, il est exact.
inline bool sontDesCoupsDeLaComposition(int total, std::size_t nombre) {
    std::vector<int> minimums;
    std::vector<int> maximums;
    for (const IdObjet arme : {IdObjet::EpeeAUneMain, IdObjet::Sceptre, IdObjet::Baton}) {
        const DescriptionDAttaque description = descriptionDeLArme(Objet{arme});
        minimums.push_back(description.degats.min.total());
        maximums.push_back(description.degats.max.total());
    }
    std::ranges::sort(minimums);
    std::ranges::sort(maximums, std::greater<>());
    int plusBas = 0;
    int plusHaut = 0;
    for (std::size_t i = 0; i < nombre; ++i) {
        plusBas += minimums[i];
        plusHaut += maximums[i];
    }
    return total >= plusBas && total <= plusHaut;
}

// Ce total est-il un coup de base du sceptre, l'arme des tireurs de test (Puissance de référence) ?
inline bool estUnCoupDeTireur(int degats) {
    return dansLaPlage(degats, s_attaqueDeTireurDeTest);
}

// Ce total est-il un coup de base de l'une des armes de la composition par défaut (épée à une main, sceptre, bâton) ?
inline bool estUnCoupDeBaseDeLaComposition(int degats) {
    for (const IdObjet arme : {IdObjet::EpeeAUneMain, IdObjet::Sceptre, IdObjet::Baton}) {
        if (dansLaPlage(degats, descriptionDeLArme(Objet{arme}))) {
            return true;
        }
    }
    return false;
}
inline const float s_executionBaseDeTest = s_attaqueDeMeleeDeTest.tempsExecution;  // en secondes
inline const float s_porteeMeleeDeTest = s_attaqueDeMeleeDeTest.portee;            // bord à bord
inline const float s_porteeTireurDeTest = s_attaqueDeTireurDeTest.portee;          // bord à bord
inline const float s_rechargeMeleeMinDeTest = s_attaqueDeMeleeDeTest.rechargeMin;  // en secondes
inline const float s_rechargeMeleeMaxDeTest = s_attaqueDeMeleeDeTest.rechargeMax;
inline const float s_rechargeTireurMinDeTest = s_attaqueDeTireurDeTest.rechargeMin;
inline const float s_rechargeTireurMaxDeTest = s_attaqueDeTireurDeTest.rechargeMax;
// Contrôle : une mêlée engage ce qui la touche presque, un tireur personne. L'armure d'une mêlée est dans aides.hpp.
inline const float s_zoneDeControleMeleeDeTest = zoneDeControleDe(NatureDAttaque::Contact);  // bord à bord
// Frappe puissante du guerrier.
inline constexpr int s_degatsFrappeDeTest = 75;
inline constexpr float s_rechargeFrappeDeTest = 5.0f;     // en secondes
inline constexpr float s_activationFrappeDeTest = 0.3f;   // en secondes
// Provocation du guerrier.
inline constexpr float s_rayonProvocationDeTest = 150.0f;  // depuis le centre du provocateur
inline constexpr int s_dureeProvocationDeTest = 4;         // en secondes (la puissance du skill)
inline constexpr float s_rechargeProvocationDeTest = 10.0f;
inline constexpr float s_activationProvocationDeTest = 0.2f;
// Soin du prêtre.
inline constexpr float s_porteeSoinDeTest = 40.0f;  // bord à bord
inline constexpr int s_puissanceSoinDeTest = 90;
inline constexpr float s_rechargeSoinDeTest = 8.0f;
inline constexpr float s_activationSoinDeTest = 0.6f;
// Explosion du mage (skill de zone).
inline constexpr float s_porteeZoneDeTest = 250.0f;  // portée de lancement, bord à bord
inline constexpr int s_degatsZoneDeTest = 45;
inline constexpr float s_rechargeZoneDeTest = 8.0f;
inline constexpr float s_activationZoneDeTest = 0.8f;
inline constexpr float s_rayonZoneDeTest = 60.0f;   // depuis le point visé

// L'attaque de base de ce personnage (ou du personnage de cette place), tirée avec un générateur de test neuf : pour les tests qui
// regardent ce que porte une attaque (nature, pénétration, capacité) ou des dégâts qui ne dépendent pas du tirage.
inline Attaque attaqueDeBaseDe(const Personnage& personnage) {
    Generateur generateur = generateurDeTest();
    return personnage.getAttaqueDeBase(generateur);
}

inline std::optional<Attaque> attaqueDeBaseDe(const Escouade& escouade, Place place) {
    Generateur generateur = generateurDeTest();
    return escouade.attaqueDeBase(place, generateur);
}

// Guerrier de test : devant, mêlée, frappe puissante et provocation.
inline DefinitionPersonnage guerrierDeTest() {
    return {.couleur = sf::Color::Red,
            .skills = {{IdSkill::FrappePuissante, Skill({.mode = ModeDeCible::Monstre,
                                                         .portee = s_porteeMeleeDeTest,
                                                         .rechargeMin = s_rechargeFrappeDeTest,
                                                         .rechargeMax = s_rechargeFrappeDeTest,
                                                         .effet = EffetDegats{.montant = s_degatsFrappeDeTest, .nature = NatureDAttaque::Contact},
                                                         .tempsExecution = s_activationFrappeDeTest})},
             {IdSkill::Provocation, Skill({.mode = ModeDeCible::Soi,
                                           .portee = 0.0f,
                                           .rechargeMin = s_rechargeProvocationDeTest,
                                           .rechargeMax = s_rechargeProvocationDeTest,
                                           .effet = EffetProvocation{.duree = s_dureeProvocationDeTest},
                                           .rayonZone = s_rayonProvocationDeTest,
                                           .tempsExecution = s_activationProvocationDeTest})}},
            .equipementDeDepart = s_equipementMeleeDeTest,
            .rayonVision = s_rayonVisionDeTest,
            .caracteristiques = caracteristiquesPourPorter(s_equipementMeleeDeTest),
            .rang = Rang::Avant};
}

// Prêtre de test : derrière, tireur, soin.
inline DefinitionPersonnage pretreDeTest() {
    return {.couleur = sf::Color(255, 240, 150),
            .skills = {{IdSkill::Soin, Skill({.mode = ModeDeCible::Allie,
                                              .portee = s_porteeSoinDeTest,
                                              .rechargeMin = s_rechargeSoinDeTest,
                                              .rechargeMax = s_rechargeSoinDeTest,
                                              .effet = EffetSoin{.montant = s_puissanceSoinDeTest},
                                              .tempsExecution = s_activationSoinDeTest})}},
            .equipementDeDepart = {{Emplacement::MainDroite, IdObjet::Sceptre}},
            .rayonVision = s_rayonVisionDeTest,
            .caracteristiques = caracteristiquesPourPorter({{Emplacement::MainDroite, IdObjet::Sceptre}}),
            .rang = Rang::Arriere};
}

// Mage de test : derrière, tireur, explosion (skill de zone, de feu comme celle de la classe).
inline DefinitionPersonnage mageDeTest() {
    return {.couleur = sf::Color(160, 80, 220),
            .skills = {{IdSkill::Explosion, Skill({.mode = ModeDeCible::Point,
                                                   .portee = s_porteeZoneDeTest,
                                                   .rechargeMin = s_rechargeZoneDeTest,
                                                   .rechargeMax = s_rechargeZoneDeTest,
                                                   .effet = EffetDegats{.montant = s_degatsZoneDeTest, .nature = NatureDAttaque::Zone, .element = Element::Feu},
                                                   .rayonZone = s_rayonZoneDeTest,
                                                   .tempsExecution = s_activationZoneDeTest})}},
            .equipementDeDepart = {{Emplacement::MainDroite, IdObjet::Baton}},
            .rayonVision = s_rayonVisionDeTest,
            .caracteristiques = caracteristiquesPourPorter({{Emplacement::MainDroite, IdObjet::Baton}}),
            .rang = Rang::Arriere};
}

// Un skill de dégâts de contact qui exige l'un de ces types d'arme (vide : aucune exigence).
inline Skill skillExigeantDesArmes(std::vector<TypeDArme> armes) {
    return Skill({.mode = ModeDeCible::Monstre,
                  .portee = s_porteeMeleeDeTest,
                  .rechargeMin = s_rechargeFrappeDeTest,
                  .rechargeMax = s_rechargeFrappeDeTest,
                  .effet = EffetDegats{.montant = s_degatsFrappeDeTest, .nature = NatureDAttaque::Contact},
                  .tempsExecution = s_activationFrappeDeTest,
                  .armesExigees = std::move(armes)});
}

// Test Data Builder : les tests de `Personnage` et d'`Escouade` créent leurs personnages par ces aides et non par la
// signature complète de `DefinitionPersonnage`, pour qu'un changement de cette signature ne touche que les aides.
// Une définition sans classe (ni skill, ni caractéristiques : Adresse 0), armée de cette arme en main droite, d'où
// viennent son attaque de base, sa portée et sa zone de contrôle.
inline DefinitionPersonnage definitionArmee(IdObjet arme = IdObjet::EpeeAUneMain) {
    return {.couleur = sf::Color::White, .equipementDeDepart = {{Emplacement::MainDroite, arme}},
            .rayonVision = s_rayonVisionDeTest, .caracteristiques = caracteristiquesPourPorter({{Emplacement::MainDroite, arme}})};
}

// Idem, sans arme : les mains nues.
inline DefinitionPersonnage definitionSansArme() {
    return {.couleur = sf::Color::White, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeTest};
}

// Seule l'arme de main droite d'un équipement de départ, pour les définitions qui précisent d'autres champs, et les
// caractéristiques qu'il faut pour la porter.
inline const std::vector<ObjetDeDepart> s_armeDeTireurSeuleDeTest = {{Emplacement::MainDroite, IdObjet::Sceptre}};
inline const Caracteristiques s_caracteristiquesDeLArmeDeTireurDeTest = caracteristiquesPourPorter(s_armeDeTireurSeuleDeTest);

// La composition par défaut des tests : guerrier, prêtre, mage (celle du jeu avant l'écran de choix).
inline std::vector<DefinitionPersonnage> compositionDeTest() {
    return {guerrierDeTest(), pretreDeTest(), mageDeTest()};
}

// Une expédition qui commence (profondeur 1, équipement de départ, sac vide) avec cette composition.
inline Expedition expeditionDeTest(std::vector<DefinitionPersonnage> composition = compositionDeTest()) {
    return nouvelleExpedition(std::move(composition), s_graineDeTest);
}

// Les membres à construire pour une escouade de cette composition : l'équipement de départ des recettes passe par la
// conversion de l'expédition (`nouvelleExpedition`), seul endroit qui le lit, comme dans le jeu.
inline std::vector<MembreDEscouade> membresDeTest(std::vector<DefinitionPersonnage> composition = compositionDeTest()) {
    return compositionDeLExpedition(expeditionDeTest(std::move(composition)));
}

// Une escouade de cette composition, à cette position.
inline Escouade uneEscouadeDe(sf::Vector2f positionDepart, std::vector<DefinitionPersonnage> composition) {
    return Escouade(positionDepart, membresDeTest(std::move(composition)));
}

// Un personnage de cette définition (équipement de départ converti comme dans le jeu), à ce centre.
inline Personnage unPersonnage(sf::Vector2f centre, DefinitionPersonnage definition) {
    std::vector<MembreDEscouade> membres = membresDeTest({std::move(definition)});
    return Personnage(centre, std::move(membres[0].definition), std::move(membres[0].equipement), membres[0].pointsDepenses, 1);
}

// Un personnage de cette définition qui porte en plus cet objet à cet emplacement (à la place de ce qui s'y trouvait). L'équipement
// de départ d'une définition ne dit que des modèles, donc des objets de niveau 1 : un objet d'un autre niveau (un seuil plus haut)
// se pose ici.
inline Personnage unPersonnageEquipe(sf::Vector2f centre, DefinitionPersonnage definition, Emplacement emplacement, const Objet& objet) {
    std::vector<MembreDEscouade> membres = membresDeTest({std::move(definition)});
    static_cast<void>(membres[0].equipement.poser(emplacement, objet));
    return Personnage(centre, std::move(membres[0].definition), std::move(membres[0].equipement), membres[0].pointsDepenses, 1);
}

// Une escouade avec la composition par défaut des tests.
inline Escouade uneEscouade(sf::Vector2f positionDepart) {
    return uneEscouadeDe(positionDepart, compositionDeTest());
}

// Un personnage de test à ce centre, armé de cette arme (le sceptre par défaut : une attaque de portée, comme les
// tireurs, pour que la géométrie des tests de `Personnage` ne dépende pas du contact).
inline Personnage unPersonnageArme(sf::Vector2f centre, IdObjet arme = IdObjet::Sceptre) {
    return unPersonnage(centre, definitionArmee(arme));
}

// Une partie de test : les monstres n'ont aucune esquive (`monstreSansHasard`), pour que les dégâts qu'ils
// reçoivent ne dépendent pas de la chance (reste le critique, qui se lit dans les événements).
inline Partie partieDeTest(const Niveau& niveau, std::vector<DefinitionPersonnage> composition,
                           Generateur::result_type graine = s_graineDeTest,
                           const DefinitionDeMonstre& monstre = monstreSansHasard()) {
    return Partie(niveau, expeditionDeTest(std::move(composition)), graine, catalogueDeTest(monstre));
}

// Le monstre sans esquive aux points de vie énormes : il survit à tous les coups d'un test qui dure.
inline DefinitionDeMonstre monstreIndestructible() {
    DefinitionDeMonstre definition = monstreSansHasard();
    definition.pointsDeVieMax = 10'000'000;
    return definition;
}

// Cette définition, avec cette arme en main droite à la place de la sienne (son attaque de base, sa nature et sa
// zone de contrôle en viennent). `assert` : la définition portait une arme.
inline DefinitionPersonnage avecArme(DefinitionPersonnage definition, IdObjet arme) {
    for (ObjetDeDepart& objet : definition.equipementDeDepart) {
        if (objet.emplacement == Emplacement::MainDroite) {
            objet.objet = arme;
            return definition;
        }
    }
    assert(false);
    return definition;
}

// Cette définition, avec cette Puissance de base (les autres à 0) : son attaque de base fait
// `montantEffectif(degats, Puissance seule, ...)`, `puissance` assez grande pour la rendre reconnaissable ou mortelle.
inline DefinitionPersonnage avecCaracteristique(DefinitionPersonnage definition, Caracteristique caracteristique, int valeur) {
    std::array<int, s_nombreDeCaracteristiques> valeurs{};
    for (const Caracteristique autre : s_caracteristiques) {
        valeurs[static_cast<std::size_t>(autre)] = autre == caracteristique ? valeur : definition.caracteristiques.valeur(autre);
    }
    definition.caracteristiques = Caracteristiques(valeurs);
    return definition;
}

// Cette définition, avec juste ce qu'il faut de caractéristiques en plus pour porter ces objets de niveau 1 (`seuilsDeLObjet`).
inline DefinitionPersonnage capableDePorter(DefinitionPersonnage definition, const std::vector<IdObjet>& objets) {
    for (const IdObjet objet : objets) {
        for (const SeuilDeCaracteristique& seuil : seuilsDeLObjet(Objet{objet})) {
            if (definition.caracteristiques.valeur(seuil.caracteristique) < seuil.valeur) {
                definition = avecCaracteristique(std::move(definition), seuil.caracteristique, seuil.valeur);
            }
        }
    }
    return definition;
}

// Cette définition, capable de porter tous les objets du catalogue : pour les tests qui déplacent des objets entre
// personnages sans parler de seuils.
inline DefinitionPersonnage capableDeToutPorter(DefinitionPersonnage definition) {
    std::vector<IdObjet> tous;
    for (const DefinitionObjet& objet : catalogueObjets()) {
        tous.push_back(objet.id);
    }
    return capableDePorter(std::move(definition), tous);
}

inline DefinitionPersonnage avecPuissance(DefinitionPersonnage definition, int puissance) {
    return avecCaracteristique(std::move(definition), Caracteristique::Puissance, puissance);
}

// Un guerrier increvable (l'Endurance donne les points de vie) : il reçoit les coups d'un test qui dure sans mourir.
inline DefinitionPersonnage avecEnduranceEnorme(DefinitionPersonnage definition) {
    return avecCaracteristique(std::move(definition), Caracteristique::Endurance, 2'000'000);
}

inline Partie unePartie(const Niveau& niveau, Generateur::result_type graine) {
    return partieDeTest(niveau, compositionDeTest(), graine);
}

// L'escouade par défaut dont les trois personnages peuvent porter tous les objets du catalogue : pour les tests qui
// déplacent des objets sans parler de seuils.
inline Escouade uneEscouadeCapableDeToutPorter(sf::Vector2f positionDepart) {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = capableDeToutPorter(std::move(definition));
    }
    return uneEscouadeDe(positionDepart, std::move(composition));
}

// Une escouade de quatre : deux guerriers devant (mêlées), le prêtre et le mage derrière. Pour les tests
// de places d'attaque et de séparation qui ont besoin de deux mêlées (la composition par défaut n'en a qu'une).
inline Escouade uneEscouadeADeuxGuerriers(sf::Vector2f positionDepart) {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition.insert(composition.begin(), composition[0]);
    return uneEscouadeDe(positionDepart, std::move(composition));
}

// Une escouade par défaut dont le prêtre a un soin de portée plus courte (33 px bord à bord,
// soit 53 px entre centres) : le guerrier (55,9 px du prêtre) est hors de portée, le mage (50 px)
// à portée. Dans le triangle de la composition par défaut, tous les alliés sont à portée du
// soin normal (40 px) : il faut ce soin court pour tester un soin hors de portée.
inline Escouade uneEscouadeAuSoinCourt(sf::Vector2f positionDepart) {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition[1].skills = {{IdSkill::Soin, Skill({.mode = ModeDeCible::Allie,
                                                    .portee = 33.0f,
                                                    .rechargeMin = s_rechargeSoinDeTest,
                                                    .rechargeMax = s_rechargeSoinDeTest,
                                                    .effet = EffetSoin{.montant = s_puissanceSoinDeTest},
                                                    .tempsExecution = s_activationSoinDeTest})}};
    return uneEscouadeDe(positionDepart, std::move(composition));
}

// Ce que les personnages de test (sans caractéristiques : Adresse 0) opposent à un coup, pour cette armure.
inline Defenses defensesDuPersonnageDeTest(int armure) {
    return {.armure = armure, .statistiques = statistiquesDuPersonnage(Caracteristiques(), StatistiquesDeCombat())};
}

// Un coup de `montant` dégâts sans hasard (ni tirage) du monstre `idMonstre` sur le personnage de cette place
// (`Escouade::recevoirCoup`) ; rend les dégâts subis.
inline int subirCoup(Escouade& escouade, IdMonstre idMonstre, Place place, int montant) {
    Generateur generateur = generateurDeTest();
    return escouade.recevoirCoup(idMonstre, place, attaqueSansHasard(montant), generateur).montant;
}

// Un coup du monstre `idMonstre`, posté en `origine`, sur la proie que l'escouade lui donne
// (proieDuMonstre : provocateur, sinon le plus proche). Renvoie les dégâts subis ; 0 si personne n'est
// touchable. Le chemin d'un coup de Partie, sans Partie.
inline int infliger(Escouade& escouade, IdMonstre idMonstre, sf::Vector2f origine, int quantite) {
    const std::optional<PersonnageVu> proie = escouade.proieDuMonstre(idMonstre, origine);
    if (!proie.has_value()) {
        return 0;
    }
    return subirCoup(escouade, idMonstre, proie->id, quantite);
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

// Les coups d'une liste de résultats (attaquer et declencherSkill rendent tous leurs résultats, par un seul canal) : les tests des
// dégâts ne lisent que ceux-là.
inline std::vector<Coup> coupsDe(const std::vector<ResultatDEffet>& resultats) {
    std::vector<Coup> coups;
    for (const ResultatDEffet& resultat : resultats) {
        if (const Coup* coup = std::get_if<Coup>(&resultat)) {
            coups.push_back(*coup);
        }
    }
    return coups;
}

// Les résultats d'une sorte (SkillParti, SoinSurAllie, ProvocationPosee), dans l'ordre.
template <class Sorte>
std::vector<Sorte> parmiLesResultats(const std::vector<ResultatDEffet>& resultats) {
    std::vector<Sorte> choisis;
    for (const ResultatDEffet& resultat : resultats) {
        if (const Sorte* sorte = std::get_if<Sorte>(&resultat)) {
            choisis.push_back(*sorte);
        }
    }
    return choisis;
}

// Appelle attaquer() par tics de 1/60 s pendant 0,35 s : un peu plus que l'exécution
// de l'attaque de base la plus lente de la composition par défaut (0,3 s, le bâton à
// deux mains), moins que le plus court intervalle entre deux coups d'un même
// personnage (0,6 s : exécution et recharge minimale de l'épée à une main). Chaque
// personnage qui atteint un monstre l'engage au premier tic et le frappe une fois,
// au plus ; renvoie tous les coups partis. Aucun
// mettreAJour : personne ne bouge, personne n'est donc « en mouvement ».
inline std::vector<Coup> attaquerJusquAuDepart(Escouade& escouade, const std::vector<MonstreVu>& monstres, const TileMap& carte,
                                               Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(0.35f); ++i) {
        const std::vector<Coup> coupsDuTic = coupsDe(escouade.attaquer(s_dureeTic, monstres, carte, generateur));
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
    }
    return coups;
}

// Comme attaquerJusquAuDepart() avec un seul monstre (identifiant 1) ; renvoie
// le total des dégâts de la liste de coups.
inline int attaquerCible(Escouade& escouade, const TileMap& carte, Cercle cible) {
    int total = 0;
    Generateur generateur = generateurDeTest();
    for (const Coup& coup : attaquerJusquAuDepart(escouade, {MonstreVu{IdMonstre{1}, cible}}, carte, generateur)) {
        total += coup.attaque.degats.total();
    }
    return total;
}

// Le plus long temps d'activation d'un skill (la zone, 0,8 s) avec une marge.
inline constexpr float s_dureeAttenteSkill = 1.0f;

// Fait avancer par attaquer() (sans mettreAJour : personne ne bouge, les
// recharges ne courent pas) pendant `s_dureeAttenteSkill`, et renvoie les résultats
// des skills partis (départs, coups, soins, provocations). Les coups de l'attaque de
// base que les autres personnages portent entre-temps (ceux des armes de la composition par
// défaut : `estUnCoupDeBaseDeLaComposition`) sont écartés : seuls les skills comptent (leurs
// dégâts doivent donc sortir de la plage de ces armes).
inline std::vector<ResultatDEffet> attendreLeDepartDesSkillsTout(Escouade& escouade, const std::vector<MonstreVu>& monstres,
                                                                 const TileMap& carte, Generateur& generateur) {
    assert(!estUnCoupDeBaseDeLaComposition(s_degatsFrappeDeTest) && !estUnCoupDeBaseDeLaComposition(s_degatsZoneDeTest));
    std::vector<ResultatDEffet> resultats;
    for (int i = 0; i < nombreDeTics(s_dureeAttenteSkill); ++i) {
        for (const ResultatDEffet& resultat : escouade.attaquer(s_dureeTic, monstres, carte, generateur)) {
            const Coup* coup = std::get_if<Coup>(&resultat);
            if (coup == nullptr || !estUnCoupDeBaseDeLaComposition(coup->attaque.degats.total())) {
                resultats.push_back(resultat);
            }
        }
    }
    return resultats;
}

// Les coups de skill partis (le reste des résultats est écarté).
inline std::vector<Coup> attendreLeDepartDesSkills(Escouade& escouade, const std::vector<MonstreVu>& monstres, const TileMap& carte,
                                                   Generateur& generateur) {
    return coupsDe(attendreLeDepartDesSkillsTout(escouade, monstres, carte, generateur));
}

// Déclenche le skill de cette place (touches 1 à 4), puis attend son départ :
// les résultats sont ceux de l'appel et ceux de l'attente.
inline std::vector<ResultatDEffet> declencherEtAttendreTout(Escouade& escouade, AdresseSkill adresse, const std::vector<MonstreVu>& monstres,
                                                            const TileMap& carte, Generateur& generateur) {
    std::vector<ResultatDEffet> resultats = escouade.declencherSkill(adresse, monstres, carte, generateur);
    const std::vector<ResultatDEffet> suite = attendreLeDepartDesSkillsTout(escouade, monstres, carte, generateur);
    resultats.insert(resultats.end(), suite.begin(), suite.end());
    return resultats;
}

// Les coups de skill de declencherEtAttendreTout.
inline std::vector<Coup> declencherEtAttendre(Escouade& escouade, AdresseSkill adresse, const std::vector<MonstreVu>& monstres, const TileMap& carte,
                                              Generateur& generateur) {
    return coupsDe(declencherEtAttendreTout(escouade, adresse, monstres, carte, generateur));
}

// Comme declencherEtAttendreTout, pour le skill de zone lancé en un point.
inline std::vector<ResultatDEffet> declencherZoneEtAttendreTout(Escouade& escouade, AdresseSkill adresse, sf::Vector2f point,
                                                                const std::vector<MonstreVu>& monstres, const TileMap& carte,
                                                                Generateur& generateur) {
    std::vector<ResultatDEffet> resultats = escouade.declencherSkillDeZone(adresse, point, monstres, carte, generateur);
    const std::vector<ResultatDEffet> suite = attendreLeDepartDesSkillsTout(escouade, monstres, carte, generateur);
    resultats.insert(resultats.end(), suite.begin(), suite.end());
    return resultats;
}

// Les coups de declencherZoneEtAttendreTout.
inline std::vector<Coup> declencherZoneEtAttendre(Escouade& escouade, AdresseSkill adresse, sf::Vector2f point, const std::vector<MonstreVu>& monstres,
                                                  const TileMap& carte, Generateur& generateur) {
    return coupsDe(declencherZoneEtAttendreTout(escouade, adresse, point, monstres, carte, generateur));
}

// Place de l'allié désigné par choisirCible, vide si aucune cible.
inline std::optional<Place> placeAlliePourSoin(const Escouade& escouade) {
    const std::optional<Cible> cible = escouade.choisirCible(ModeDeCible::Allie, s_adresseSoin, {}, carteOuverte());
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

// Un objet par valeur complète (modèle, rareté, affixes, niveau, qualité), sous une forme qui se trie : deux objets du même
// modèle mais d'affixes, de niveau ou de qualité différents sont deux exemplaires distincts.
using Cle = std::array<int, 8>;

inline Cle cle(const Objet& objet) {
    const auto affixe = [](const std::optional<Affixe>& a) { return a.has_value() ? std::pair{static_cast<int>(a->id), a->valeur} : std::pair{-1, 0}; };
    const auto [idPrefixe, valeurPrefixe] = affixe(objet.prefixe);
    const auto [idSuffixe, valeurSuffixe] = affixe(objet.suffixe);
    return {static_cast<int>(objet.modele), static_cast<int>(objet.rarete), idPrefixe, valeurPrefixe, idSuffixe, valeurSuffixe,
            objet.niveau, objet.qualite};
}

// Tous les objets qui existent dans le sac et dans tous les équipements, un par exemplaire, triés : comparer deux de
// ces listes compte les exemplaires par valeur complète.
using Exemplaires = std::vector<Cle>;

inline Exemplaires exemplaires(const Escouade& escouade) {
    Exemplaires tous;
    for (std::size_t i = 0; i < escouade.sac().capacite(); ++i) {
        if (const std::optional<Objet>& objet = escouade.sac().a(i)) {
            tous.push_back(cle(*objet));
        }
    }
    for (std::size_t i = 0; i < 3; ++i) {
        for (std::size_t e = 0; e < s_nombreEmplacements; ++e) {
            if (const std::optional<Objet>& objet = escouade.equipementDuPersonnage(Place::depuisIndice(i)).a(static_cast<Emplacement>(e))) {
                tous.push_back(cle(*objet));
            }
        }
    }
    std::ranges::sort(tous);
    return tous;
}

// Tous les objets de la partie, un par exemplaire, triés : ceux de l'escouade (sac et équipements) et ceux du sol. Un objet
// existe à un seul endroit : jeter, ramasser ou déplacer ne change jamais cette liste.
inline Exemplaires exemplaires(const Partie& partie) {
    Exemplaires tous = exemplaires(partie.getEscouade());
    for (const ObjetAuSol& objet : partie.getSol().tous()) {
        tous.push_back(cle(objet.objet));
    }
    std::ranges::sort(tous);
    return tous;
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
inline int pointsDeVieApresUnTic(Partie& partie, Place place) {
    partie.mettreAJour(s_dureeTic);
    return *partie.getEscouade().pointsDeVieDuPersonnage(place);
}

// Avance jusqu'à ce que le personnage de cette place perde des points de vie (le premier coup du
// monstre) ; renvoie ses points de vie à ce moment. Le coup lance la recharge du monstre (0,7 s au
// moins) puis un nouveau coup exige 0,3 s d'exécution : aucun coup normal avant 1 s.
inline int jusquAuPremierCoup(Partie& partie, Place place) {
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
inline std::vector<MonstreVu> unSeulMonstre(Cercle cercle) {
    return {MonstreVu{IdMonstre{1}, cercle}};
}

// Déclenche le skill de cette place, attend son départ, et rend tous ses résultats.
inline std::vector<ResultatDEffet> declencherTout(Escouade& escouade, AdresseSkill adresse, const std::vector<MonstreVu>& monstres) {
    Generateur generateur = generateurDeTest();
    return declencherEtAttendreTout(escouade, adresse, monstres, carteOuverte(), generateur);
}

// Déclenche le skill de cette place et renvoie le total des dégâts de la
// liste de coups.
inline int declencher(Escouade& escouade, AdresseSkill adresse, const std::vector<MonstreVu>& monstres) {
    int total = 0;
    for (const Coup& coup : coupsDe(declencherTout(escouade, adresse, monstres))) {
        total += coup.attaque.degats.total();
    }
    return total;
}
