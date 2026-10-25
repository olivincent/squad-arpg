// ./tests/escouade/test_Personnage.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "escouade/Personnage.hpp"
#include "base/Cercle.hpp"
#include "combat/Blocage.hpp"
#include "combat/Esquive.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "combat/Defense.hpp"
#include "combat/IdSkill.hpp"
#include "combat/Skill.hpp"
#include "monde/TileMap.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Equipement.hpp"

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
        personnage.mettreAJour(s_dureeTic, centre, carte, {}, {});
    }
}

// Frappe en un seul appel, comme le faisait l'ancien frapper() : refuse une
// cible hors de portée (règle d'Escouade), engage le coup, puis avance
// l'exécution jusqu'à son départ. Les personnages de ce fichier ont une
// attaque à exécution nulle (sauf mention) : le coup part au premier tic.
int frapperEnEntier(Personnage& personnage, Cercle cible, Generateur& generateur) {
    if (!personnage.estAPortee(cible) || !personnage.engagerAttaque(1)) {
        return 0;
    }
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        if (const int degats = montantOuZero(personnage.avancerAttaque(s_dureeTic, generateur)); degats > 0) {
            return degats;
        }
    }
    return 0;
}

// Lance le skill en un seul appel : engage, puis avance l'activation jusqu'au
// départ de l'effet. Rend la puissance du skill, 0 s'il ne peut pas être engagé.
// La portée est vérifiée par Escouade, pas ici. Les skills de ce fichier ont
// une activation nulle (sauf mention) : l'effet part au premier tic.
int lancerSkillEnEntier(Personnage& personnage, std::optional<Cible> cible, Generateur& generateur, IdSkill id = IdSkill::FrappePuissante) {
    if (!personnage.engagerSkill(id, cible)) {
        return 0;
    }
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        if (const int puissance = montantOuZero(personnage.avancerSkill(s_dureeTic, generateur)); puissance > 0) {
            return puissance;
        }
    }
    return 0;
}

}  // namespace

// Personnage centré en (100, 100), portée 80 px.

TEST_CASE("estAPortee renvoie vrai pour un point proche") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.estAPortee(pointCible({150.0f, 100.0f})));
}

TEST_CASE("estAPortee renvoie faux pour un point lointain") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE_FALSE(personnage.estAPortee(pointCible({300.0f, 100.0f})));
}

TEST_CASE("frapper renvoie les degats pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    REQUIRE(frapperEnEntier(personnage, pointCible({150.0f, 100.0f}), generateur) == 5);
}

TEST_CASE("frapper renvoie 0 pour une cible hors de portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    REQUIRE(frapperEnEntier(personnage, pointCible({300.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("frapper renvoie de nouveau les degats apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 5);
}

TEST_CASE("Un personnage est vivant a la creation") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est encore vivant apres des degats inferieurs a ses points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    subir(personnage, personnage.getPointsDeVieMax() - 20);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est mort apres autant de degats que ses points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("getCentre renvoie le centre donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le personnage, de rayon la moitie du cote") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest});
    const Cercle cercle = personnage.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 10.0f);
}

TEST_CASE("getRayonVision renvoie le rayon donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = 123.0f});

    REQUIRE(personnage.getRayonVision() == 123.0f);
}

TEST_CASE("getZoneDeControle renvoie la zone donnee au constructeur, 0 par defaut") {
    const Personnage engageant({100.0f, 100.0f},
                               {.couleur = sf::Color::White,
                                .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 20.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                .rayonVision = s_rayonVisionDeTest, .zoneDeControle = 20.0f});
    const Personnage sansZone({100.0f, 100.0f},
                              {.couleur = sf::Color::White,
                               .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 20.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                               .rayonVision = s_rayonVisionDeTest});

    REQUIRE(engageant.getZoneDeControle() == 20.0f);
    REQUIRE(sansZone.getZoneDeControle() == 0.0f);
}

TEST_CASE("getPuissanceAttaqueDeBase renvoie les degats de l'attaque de base") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 7, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.getAttaqueDeBase().montant == 7);
}

TEST_CASE("lire la puissance de l'attaque de base ne change ni le coup engage ni la recharge") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .tempsExecution = 0.15f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    Generateur generateur = generateurDeTest();
    REQUIRE(personnage.engagerAttaque(1));

    REQUIRE(personnage.getAttaqueDeBase().montant == 5);

    REQUIRE(personnage.cibleEngagee() == 1);
    REQUIRE(montantOuZero(personnage.avancerAttaque(0.2f, generateur)) == 5);  // le coup engagé part normalement
    REQUIRE(personnage.getAttaqueDeBase().montant == 5);
    REQUIRE_FALSE(personnage.engagerAttaque(1));                // la recharge court toujours
}

TEST_CASE("Un personnage est mort apres un coup qui depasse le reste de ses points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    subir(personnage, personnage.getPointsDeVieMax() - 20);  // reste 20 points de vie
    subir(personnage, 50);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("Un personnage mort ne frappe plus") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const Cercle cible = pointCible({150.0f, 100.0f});

    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 0);
}

TEST_CASE("Un personnage mort ne bouge plus") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const TileMap carte = carteDegagee5x5();
    const sf::Vector2f centreInitial = personnage.getCentre();

    subir(personnage, personnage.getPointsDeVieMax());
    ecoulerTemps(personnage, carte, {500.0f, 500.0f}, 2.0f); // cible lointaine, durée généreuse

    REQUIRE(personnage.getCentre() == centreInitial);
}

// Skill : portée 80 px comme l'attaque de base, 25 dégâts, 5 s de recharge.

TEST_CASE("Lancer un skill rend sa puissance") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .rayonVision = s_rayonVisionDeTest});

    REQUIRE(lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, pointCible({150.0f, 100.0f})}), generateur) == 25);
}

TEST_CASE("Un personnage avec un skill de provocation en expose le type et le lance") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 20.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .skills = {{IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                           .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.typeSkill(IdSkill::Provocation) == TypeSkill::Provocation);
    REQUIRE(personnage.skillEstPret(IdSkill::Provocation));
    REQUIRE(lancerSkillEnEntier(personnage, std::nullopt, generateur, IdSkill::Provocation) == 4);
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::Provocation));
}

TEST_CASE("zoneDuSkill est centree sur le personnage, au rayon de zone du skill") {
    Personnage provocateur({100.0f, 100.0f},
                           {.couleur = sf::Color::White,
                            .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 20.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                            .skills = {{IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                            .rayonVision = s_rayonVisionDeTest});
    Personnage sansSkill({100.0f, 100.0f},
                         {.couleur = sf::Color::White,
                          .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 20.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                          .rayonVision = s_rayonVisionDeTest});

    REQUIRE(provocateur.zoneDuSkill(IdSkill::Provocation).centre == provocateur.getCentre());
    REQUIRE(provocateur.zoneDuSkill(IdSkill::Provocation).rayon == 150.0f);
    REQUIRE(sansSkill.zoneDuSkill(IdSkill::Provocation).rayon == 0.0f);
}

TEST_CASE("engagerSkill refuse un personnage sans skill, sans toucher a l'attaque de base") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, Cible(CibleMonstre{1, cible})));
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 5);
}

TEST_CASE("engagerSkill refuse un personnage mort") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .rayonVision = s_rayonVisionDeTest});

    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, Cible(CibleMonstre{1, pointCible({150.0f, 100.0f})})));
}

TEST_CASE("skillEstAPortee mesure la portee du skill, sans toucher a l'attaque de base") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.skillEstAPortee(IdSkill::FrappePuissante, pointCible({150.0f, 100.0f})));
    REQUIRE_FALSE(personnage.skillEstAPortee(IdSkill::FrappePuissante, pointCible({300.0f, 100.0f})));
    REQUIRE(frapperEnEntier(personnage, pointCible({150.0f, 100.0f}), generateur) == 5);
}

TEST_CASE("Un skill ne peut pas etre relance pendant sa recharge, puis le peut une fois ecoulee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .rayonVision = s_rayonVisionDeTest});
    const TileMap carte = carteDegagee5x5();
    const Cible cible = CibleMonstre{1, pointCible({150.0f, 100.0f})};

    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur) == 25);
    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 4.8f);
    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.4f);
    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur) == 25);
}

TEST_CASE("Apres un skill reussi, l'attaque de base attend une recharge complete") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .rayonVision = s_rayonVisionDeTest});
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    // Attaque de base lancée, puis 0,3 s écoulées : il ne lui reste que
    // 0,2 s de recharge. Le skill la relance au complet (0,5 s) : 0,3 s
    // plus tard, elle ne doit toujours pas partir, alors qu'elle l'aurait
    // fait sans le skill.
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, cible}), generateur) == 25);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 5);
}

TEST_CASE("mettreAJour s'arrete avant d'entrer dans un obstacle") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const std::vector<Cercle> obstacles = {{{140.0f, 100.0f}, 15.0f}};

    for (int i = 0; i < 60; ++i) {
        personnage.mettreAJour(1.0f / 60.0f, {170.0f, 100.0f}, carte, obstacles, {});
    }

    // distanceBordABord est bornée à 0 : on compare donc les distances
    // entre centres et la somme des rayons.
    const float ecartCentres = (personnage.getCentre() - obstacles[0].centre).length();
    REQUIRE(ecartCentres >= personnage.getCercle().rayon + obstacles[0].rayon - 0.001f);
    REQUIRE(personnage.getCentre().x > 100.0f);
    REQUIRE(personnage.getCentre().x < 140.0f);
}

TEST_CASE("recevoirSoin rend des points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    subir(personnage, 50);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax() - 50 + 30);
}

TEST_CASE("recevoirDegats applique l'armure du personnage") {
    Personnage melee({100.0f, 100.0f},
                     {.couleur = sf::Color::White,
                      .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                      .equipementDeDepart = s_equipementMeleeDeTest, .rayonVision = s_rayonVisionDeTest});
    Personnage tireur({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .rayonVision = s_rayonVisionDeTest});

    subir(melee, 10);
    subir(tireur, 10);

    REQUIRE(melee.getPointsDeVie() == melee.getPointsDeVieMax() - appliquerArmure(10, s_armureMeleeDeTest));
    REQUIRE(melee.getPointsDeVie() > tireur.getPointsDeVie());
    REQUIRE(tireur.getPointsDeVie() == tireur.getPointsDeVieMax() - 10);
}

TEST_CASE("recevoirSoin n'est pas reduit par l'armure") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .equipementDeDepart = s_equipementMeleeDeTest, .rayonVision = s_rayonVisionDeTest});
    subir(personnage, 60);
    const int retires = personnage.getPointsDeVieMax() - personnage.getPointsDeVie();
    REQUIRE(retires == appliquerArmure(60, s_armureMeleeDeTest));

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax() - retires + 30);
}

TEST_CASE("recevoirSoin ne depasse pas le maximum") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    subir(personnage, 10);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

TEST_CASE("recevoirSoin est sans effet sur un mort") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    subir(personnage, personnage.getPointsDeVieMax());

    personnage.recevoirSoin(30);

    REQUIRE_FALSE(personnage.estVivant());
    REQUIRE(personnage.getPointsDeVie() == 0);
}

TEST_CASE("En terrain degage, le personnage avance en ligne droite de vitesse * dt") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    // La vitesse n'est pas injectable : le premier tic la mesure (vitesse * dt), les suivants
    // doivent avancer d'autant, sans dévier (la cible est à 120 px, hors d'atteinte en 10 tics).
    personnage.mettreAJour(1.0f / 60.0f, {180.0f, 60.0f}, carte, {}, {});
    const float pasParTic = personnage.getCentre().x - 60.0f;
    REQUIRE(pasParTic > 0.0f);
    for (int i = 1; i < 10; ++i) {
        personnage.mettreAJour(1.0f / 60.0f, {180.0f, 60.0f}, carte, {}, {});
    }

    REQUIRE_THAT(personnage.getCentre().x, Catch::Matchers::WithinAbs(60.0f + 10.0f * pasParTic, 0.01f));
    REQUIRE_THAT(personnage.getCentre().y, Catch::Matchers::WithinAbs(60.0f, 0.01f));
}

TEST_CASE("Un personnage separe de sa cible par un mur avec une ouverture finit par l'atteindre") {
    const TileMap carte({"#######",
                         "#..#..#",
                         "#..#..#",
                         "#.....#",
                         "#######"},
                        40.0f);
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
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
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

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
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    // Nouvelle cible à 30 px de la précédente, mais dans le mur de droite :
    // un recalcul ne trouverait aucun chemin et le personnage irait tout
    // droit contre le mur du milieu. Le chemin conservé le mène par la porte.
    ecoulerTemps(personnage, carte, {250.0f, 60.0f}, 1.0f);

    REQUIRE(personnage.getCentre().x > 160.0f);
}

TEST_CASE("Une cible qui bouge de plus d'une case fait recalculer le chemin") {
    const TileMap carte = deuxPiecesAvecPorte();
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    // Nouvelle cible à 50 px, dans le mur : le chemin est recalculé, le A*
    // n'en trouve pas, et le personnage va tout droit contre le mur du milieu.
    ecoulerTemps(personnage, carte, {270.0f, 60.0f}, 1.0f);

    REQUIRE(personnage.getCentre().x <= 110.01f);
}

TEST_CASE("Quand le trajet direct redevient libre, le chemin en cours est abandonne") {
    const TileMap carte = deuxPiecesAvecPorte();
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    ecoulerTemps(personnage, carte, {60.0f, 100.0f}, 1.0f);

    REQUIRE_THAT(personnage.getCentre().x, Catch::Matchers::WithinAbs(60.0f, 1.0f));
    REQUIRE_THAT(personnage.getCentre().y, Catch::Matchers::WithinAbs(100.0f, 1.0f));
}

// Coup à temps d'exécution : engager, avancer, annuler. Attaque de 5 dégâts,
// recharge 0,5 s, exécution 0,15 s.

namespace {

Personnage personnageAvecExecution() {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .tempsExecution = 0.15f, .nature = NatureDAttaque::Contact}),
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                       .rayonVision = s_rayonVisionDeTest});
}

}  // namespace

TEST_CASE("engagerAttaque exige un personnage immobile") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage = personnageAvecExecution();

    // 80 px à parcourir : il est en marche après quelques tics.
    ecoulerTemps(personnage, carte, {180.0f, 100.0f}, 0.1f);
    REQUIRE_FALSE(personnage.engagerAttaque(1));
    REQUIRE_FALSE(personnage.cibleEngagee().has_value());

    ecoulerTemps(personnage, carte, {180.0f, 100.0f}, 1.0f);
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE(personnage.cibleEngagee() == 1);
}

TEST_CASE("engagerAttaque refuse un personnage mort ou deja en execution") {
    Personnage mort = personnageAvecExecution();
    subir(mort, mort.getPointsDeVieMax());
    REQUIRE_FALSE(mort.engagerAttaque(1));

    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE_FALSE(personnage.engagerAttaque(2));
    REQUIRE(personnage.cibleEngagee() == 1);
}

TEST_CASE("avancerAttaque rend les degats une seule fois, a la fin de l'execution") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.1f); ++i) {
        total += montantOuZero(personnage.avancerAttaque(s_dureeTic, generateur));
    }
    REQUIRE(total == 0);
    REQUIRE(personnage.cibleEngagee().has_value());

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += montantOuZero(personnage.avancerAttaque(s_dureeTic, generateur));
    }
    REQUIRE(total == 5);
    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
}

TEST_CASE("l'attaque de base d'un personnage porte sa capacite offensive") {
    Generateur generateur = generateurDeTest();
    Personnage rodeur({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = Caracteristiques({10, 10, 14, 8, 8})});
    REQUIRE(rodeur.getAttaqueDeBase().capaciteOffensive == 120);
    REQUIRE(rodeur.engagerAttaque(1));

    const std::optional<Attaque> attaque = rodeur.avancerAttaque(0.1f, generateur);

    REQUIRE(attaque.has_value());
    REQUIRE(attaque->montant == 5);
    REQUIRE(attaque->capaciteOffensive == 120);
}

TEST_CASE("l'attaque d'un personnage porte la nature de son attaque de base, celle d'un skill la sienne") {
    Generateur generateur = generateurDeTest();
    Personnage tireur({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f,
                                               .rechargeMax = 0.5f, .nature = NatureDAttaque::Projectile}),
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 20.0f, .puissance = 7,
                                                                    .rechargeMin = 1.0f, .rechargeMax = 1.0f, .nature = NatureDAttaque::Contact})}},
                       .rayonVision = s_rayonVisionDeTest});
    REQUIRE(tireur.getAttaqueDeBase().nature == NatureDAttaque::Projectile);
    REQUIRE(tireur.engagerAttaque(1));
    REQUIRE(tireur.avancerAttaque(0.1f, generateur)->nature == NatureDAttaque::Projectile);

    REQUIRE(tireur.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));
    REQUIRE(tireur.avancerSkill(0.1f, generateur)->nature == NatureDAttaque::Contact);
}

TEST_CASE("annulerAttaque ne consomme pas de recharge") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE(montantOuZero(personnage.avancerAttaque(0.05f, generateur)) == 0);

    personnage.annulerAttaque();

    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
    REQUIRE(personnage.engagerAttaque(1));  // aussitôt prête de nouveau
}

TEST_CASE("Apres le depart du coup, la recharge court avant le coup suivant") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee5x5();
    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE(montantOuZero(personnage.avancerAttaque(0.2f, generateur)) == 5);

    REQUIRE_FALSE(personnage.engagerAttaque(1));
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.4f);
    REQUIRE_FALSE(personnage.engagerAttaque(1));
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.2f);
    REQUIRE(personnage.engagerAttaque(1));
}

TEST_CASE("Engager un skill annule le coup en execution") {
    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));

    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, Cible(CibleMonstre{1, pointCible({150.0f, 100.0f})})));

    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
    REQUIRE(personnage.skillEngage().has_value());
}

// Skill dont l'activation dure 0,3 s ; l'attaque de base n'a pas d'exécution.
namespace {

Personnage personnageAvecActivation() {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .tempsExecution = 0.15f, .nature = NatureDAttaque::Contact}),
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .tempsExecution = 0.3f, .nature = NatureDAttaque::Contact})}},
                       .rayonVision = s_rayonVisionDeTest});
}

const Cible s_cibleDuSkill = CibleMonstre{1, {{150.0f, 100.0f}, 0.0f}};

}  // namespace

TEST_CASE("avancerSkill rend la puissance une seule fois, a la fin de l'activation") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += montantOuZero(personnage.avancerSkill(s_dureeTic, generateur));
    }
    REQUIRE(total == 0);
    REQUIRE(personnage.skillEngage().has_value());

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += montantOuZero(personnage.avancerSkill(s_dureeTic, generateur));
    }
    REQUIRE(total == 25);
    REQUIRE_FALSE(personnage.skillEngage().has_value());
    REQUIRE_FALSE(personnage.estEnExecution());
}

TEST_CASE("Un skill en activation n'est pas pret et ne peut pas etre engage de nouveau") {
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));

    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));

    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));
    REQUIRE(personnage.estEnExecution());
}

TEST_CASE("engagerSkill n'exige pas un personnage immobile") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage = personnageAvecActivation();
    ecoulerTemps(personnage, carte, {180.0f, 100.0f}, 0.1f);  // en marche

    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));
}

TEST_CASE("annulerSkill ne consomme pas de recharge") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));
    REQUIRE(montantOuZero(personnage.avancerSkill(0.1f, generateur)) == 0);

    personnage.annulerSkill();

    REQUIRE_FALSE(personnage.skillEngage().has_value());
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));
}

TEST_CASE("annulerAttaque n'annule pas l'activation d'un skill") {
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));

    personnage.annulerAttaque();

    REQUIRE(personnage.skillEngage().has_value());
}

TEST_CASE("Aucune attaque de base ne s'engage pendant l'activation d'un skill") {
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));

    REQUIRE_FALSE(personnage.engagerAttaque(1));
}

TEST_CASE("Au depart du skill, l'attaque de base repart pour une recharge complete") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee5x5();
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));
    REQUIRE(montantOuZero(personnage.avancerSkill(0.4f, generateur)) == 25);

    REQUIRE_FALSE(personnage.engagerAttaque(1));
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(personnage.engagerAttaque(1));
}

TEST_CASE("mettreAJour garde la ligne droite en terrain degage meme avec un obstacle a contourner loin du trajet") {
    const TileMap carte = carteDegagee10x10();
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const std::vector<Cercle> loin = {{{180.0f, 200.0f}, 15.0f}};

    personnage.mettreAJour(s_dureeTic, {300.0f, 60.0f}, carte, loin, loin);

    REQUIRE(personnage.cheminRestant() == std::vector<sf::Vector2f>{{300.0f, 60.0f}});
}

TEST_CASE("mettreAJour contourne un obstacle a contourner place sur la ligne droite") {
    const TileMap carte = carteDegagee10x10();
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const std::vector<Cercle> obstacle = {{{180.0f, 60.0f}, 15.0f}};

    for (int i = 0; i < nombreDeTics(5.0f); ++i) {
        personnage.mettreAJour(s_dureeTic, {300.0f, 60.0f}, carte, obstacle, obstacle);
        if (i == 0) {
            REQUIRE(personnage.cheminRestant().size() > 1); // un A* lissé, pas la ligne droite
        }
    }

    verifierPointProche(personnage.getCentre(), {300.0f, 60.0f});
}

TEST_CASE("mettreAJour recalcule son chemin quand il est bloque") {
    // Mur sur la colonne 5, avec deux ouvertures d'une case : lignes 2 (y de 80 à 120) et 8.
    const TileMap carte({".....#....", ".....#....", "..........", ".....#....", ".....#....",
                         ".....#....", ".....#....", ".....#....", "..........", ".....#...."}, 40.0f);
    Personnage personnage({60.0f, 200.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const sf::Vector2f cible{340.0f, 100.0f};
    // Un obstacle qui bouche l'ouverture de la ligne 2, que le premier chemin ignore (aContourner vide au départ).
    const std::vector<Cercle> bouchon = {{{220.0f, 100.0f}, 15.0f}};

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        personnage.mettreAJour(s_dureeTic, cible, carte, bouchon, {});
    }
    // Il connaît maintenant le bouchon : la cible n'a pas bougé et le chemin n'est pas fini,
    // seul le blocage déclenche un recalcul, qui passe par l'autre ouverture (plus coûteux de la traverser).
    for (int i = 0; i < nombreDeTics(20.0f); ++i) {
        personnage.mettreAJour(s_dureeTic, cible, carte, bouchon, bouchon);
    }

    verifierPointProche(personnage.getCentre(), cible);
}

TEST_CASE("mettreAJour sans obstacle a contourner reste bloque contre un obstacle bloquant") {
    const TileMap carte = carteDegagee10x10();
    Personnage personnage({60.0f, 60.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    const std::vector<Cercle> obstacle = {{{180.0f, 60.0f}, 15.0f}};

    for (int i = 0; i < nombreDeTics(5.0f); ++i) {
        personnage.mettreAJour(s_dureeTic, {300.0f, 60.0f}, carte, obstacle, {});
    }

    REQUIRE(personnage.getCentre().x < 180.0f);
}

// Plusieurs skills connus : un guerrier avec une frappe (activation 0,3 s,
// recharge 5 s) et une provocation (activation 0,2 s, recharge 10 s).

namespace {

Personnage personnageAvecDeuxSkills() {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .tempsExecution = 0.3f, .nature = NatureDAttaque::Contact})}, {IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .tempsExecution = 0.2f, .nature = NatureDAttaque::Contact})}},
                       .rayonVision = s_rayonVisionDeTest});
}

}  // namespace

TEST_CASE("Un personnage expose chacun de ses skills par identifiant") {
    const Personnage personnage = personnageAvecDeuxSkills();

    REQUIRE(personnage.typeSkill(IdSkill::FrappePuissante) == TypeSkill::Degats);
    REQUIRE(personnage.typeSkill(IdSkill::Provocation) == TypeSkill::Provocation);
    REQUIRE(personnage.zoneDuSkill(IdSkill::Provocation).rayon == 150.0f);
    REQUIRE(personnage.zoneDuSkill(IdSkill::FrappePuissante).rayon == 0.0f);
}

TEST_CASE("Un identifiant de skill inconnu se comporte comme l'absence de skill") {
    Personnage personnage = personnageAvecDeuxSkills();

    REQUIRE_FALSE(personnage.typeSkill(IdSkill::Soin).has_value());
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::Soin));
    REQUIRE_FALSE(personnage.etatDuSkill(IdSkill::Soin).has_value());
    REQUIRE_FALSE(personnage.skillEstAPortee(IdSkill::Soin, pointCible({110.0f, 100.0f})));
    REQUIRE(personnage.zoneDuSkill(IdSkill::Soin).rayon == 0.0f);
    REQUIRE(personnage.cercleDePorteeDuSkill(IdSkill::Soin).rayon == personnage.getCercle().rayon);
    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::Soin, std::nullopt));
    REQUIRE_FALSE(personnage.estEnExecution());
}

TEST_CASE("Un second skill est refuse pendant l'activation d'un autre, sans toucher a sa recharge") {
    Personnage personnage = personnageAvecDeuxSkills();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));

    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::Provocation, std::nullopt));

    REQUIRE(personnage.skillEstPret(IdSkill::Provocation));
    REQUIRE(personnage.skillEngage()->skill == IdSkill::FrappePuissante);
}

TEST_CASE("Les recharges de deux skills sont independantes") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee5x5();
    Personnage personnage = personnageAvecDeuxSkills();

    REQUIRE(lancerSkillEnEntier(personnage, s_cibleDuSkill, generateur, IdSkill::FrappePuissante) == 25);
    // La frappe est en recharge, la provocation reste utilisable aussitôt.
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE(personnage.skillEstPret(IdSkill::Provocation));
    REQUIRE(lancerSkillEnEntier(personnage, std::nullopt, generateur, IdSkill::Provocation) == 4);

    // Après 5,5 s la frappe est prête, pas la provocation (10 s).
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 5.5f);
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::Provocation));
}

TEST_CASE("etatDuSkill n'indique l'activation que pour le skill engage") {
    Personnage personnage = personnageAvecDeuxSkills();
    REQUIRE(personnage.engagerSkill(IdSkill::Provocation, std::nullopt));

    REQUIRE(personnage.etatDuSkill(IdSkill::Provocation)->enActivation);
    REQUIRE_FALSE(personnage.etatDuSkill(IdSkill::FrappePuissante)->enActivation);
}

TEST_CASE("annulerSkill annule le skill engage sans consommer sa recharge, l'autre reste intact") {
    Personnage personnage = personnageAvecDeuxSkills();
    REQUIRE(personnage.engagerSkill(IdSkill::Provocation, std::nullopt));

    personnage.annulerSkill();

    REQUIRE_FALSE(personnage.skillEngage().has_value());
    REQUIRE(personnage.skillEstPret(IdSkill::Provocation));
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
}

TEST_CASE("recevoirDegats renvoie les degats subis apres armure") {
    Personnage melee({100.0f, 100.0f},
                     {.couleur = sf::Color::White,
                      .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                      .equipementDeDepart = s_equipementMeleeDeTest, .rayonVision = s_rayonVisionDeTest});
    Personnage tireur({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .rayonVision = s_rayonVisionDeTest});

    REQUIRE(subir(melee, 10) == appliquerArmure(10, s_armureMeleeDeTest));
    REQUIRE(subir(tireur, 10) == 10);
}

TEST_CASE("recevoirDegats sur un mort renvoie zero") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE(subir(personnage, 10) == 0);
}

TEST_CASE("recevoirSoin renvoie les points reellement rendus") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    subir(personnage, 10);

    REQUIRE(personnage.recevoirSoin(30) == 10);  // plafonne au maximum
    REQUIRE(personnage.recevoirSoin(30) == 0);   // deja plein
}

TEST_CASE("recevoirSoin sur un mort renvoie zero") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                           .rayonVision = s_rayonVisionDeTest});
    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE(personnage.recevoirSoin(30) == 0);
}

TEST_CASE("l'armure du personnage est celle de son equipement de depart") {
    const std::vector<ObjetDeDepart> casqueSeul = {{Emplacement::Tete, IdObjet::CasqueDeFer}};
    Personnage casque({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .equipementDeDepart = casqueSeul, .rayonVision = s_rayonVisionDeTest});
    Personnage nu({100.0f, 100.0f},
                  {.couleur = sf::Color::White,
                   .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                   .rayonVision = s_rayonVisionDeTest});

    REQUIRE(subir(casque, 40) == appliquerArmure(40, definitionObjet(IdObjet::CasqueDeFer).armure));
    REQUIRE(subir(nu, 40) == 40);
}

TEST_CASE("l'armure du personnage additionne tous les objets de depart") {
    const std::vector<ObjetDeDepart> depart = {{Emplacement::Tete, IdObjet::CasqueDeFer},
                                               {Emplacement::Torse, IdObjet::CuirasseDeMailles}};
    Personnage equipe({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .equipementDeDepart = depart, .rayonVision = s_rayonVisionDeTest});

    REQUIRE(subir(equipe, 40) == appliquerArmure(40, armureDe(depart)));
}

TEST_CASE("les caracteristiques effectives d'un personnage sont celles de sa definition") {
    const Caracteristiques base({12, 14, 10, 6, 8});
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest,
                                 .caracteristiques = base});

    REQUIRE(personnage.getCaracteristiques() == base);
}

TEST_CASE("les statistiques de combat d'un personnage viennent de son Adresse") {
    const Personnage rodeur({100.0f, 100.0f},
                            {.couleur = sf::Color::White,
                             .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                             .rayonVision = s_rayonVisionDeTest,
                             .caracteristiques = Caracteristiques({10, 10, 14, 8, 8})});

    REQUIRE(rodeur.getStatistiques() == statistiquesDuPersonnage(rodeur.getCaracteristiques(), StatistiquesDeCombat()));
    REQUIRE(rodeur.getStatistiques().capaciteOffensive == 120);
    REQUIRE(rodeur.getStatistiques().capaciteDefensive == 120);
}

namespace {
Personnage unPersonnageSansEsquive() {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = Caracteristiques({10, 10, 10, 10, 10})});
}
}  // namespace

TEST_CASE("l'esquive d'un personnage vient de son equipement, recalculee a chaque changement") {
    Personnage personnage = unPersonnageSansEsquive();
    REQUIRE(personnage.getStatistiques().esquiveContact == 0);
    REQUIRE(personnage.getStatistiques().esquiveProjectile == 0);

    static_cast<void>(personnage.poserEquipement(Emplacement::Bottes, {IdObjet::BottesDeCuir}));
    REQUIRE(personnage.getStatistiques().esquiveContact == 5);
    REQUIRE(personnage.getStatistiques().esquiveProjectile == 0);

    static_cast<void>(personnage.poserEquipement(Emplacement::Tete, {IdObjet::CasqueDeCuir}));
    REQUIRE(personnage.getStatistiques().esquiveProjectile == 5);

    static_cast<void>(personnage.retirerEquipement(Emplacement::Bottes));
    static_cast<void>(personnage.retirerEquipement(Emplacement::Tete));
    REQUIRE(personnage.getStatistiques() == statistiquesDuPersonnage(personnage.getCaracteristiques(), StatistiquesDeCombat()));
}

TEST_CASE("un equipement de depart qui donne de l'esquive compte des la construction") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .equipementDeDepart = {{Emplacement::Bottes, IdObjet::BottesDeCuir}},
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.getStatistiques().esquiveContact == 5);
}

TEST_CASE("le bouclier rond donne au personnage sa chance de blocage, son montant et sa recuperation") {
    Personnage personnage = unPersonnageSansEsquive();

    static_cast<void>(personnage.poserEquipement(Emplacement::MainGauche, {IdObjet::BouclierRond}));

    const StatistiquesDeCombat& bouclier = definitionObjet(IdObjet::BouclierRond).bonusDeCombat;
    REQUIRE(personnage.getStatistiques().chanceDeBlocage == bouclier.chanceDeBlocage);
    REQUIRE(personnage.getStatistiques().montantBloque == bouclier.montantBloque);
    REQUIRE(personnage.getStatistiques().recuperationDuBlocage == bouclier.recuperationDuBlocage);
    static_cast<void>(personnage.retirerEquipement(Emplacement::MainGauche));
    REQUIRE(personnage.getStatistiques().montantBloque == 0);
}

TEST_CASE("un personnage au bouclier : apres un blocage, plus aucun pendant 3 s, puis de nouveau") {
    Personnage personnage = unPersonnageSansEsquive();
    static_cast<void>(personnage.poserEquipement(Emplacement::MainGauche, {IdObjet::BouclierRond}));
    const TileMap carte = carteDegagee5x5();
    const Attaque attaque{.montant = 10, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    const float chance = chanceDeBlocage(NatureDAttaque::Contact, personnage.getStatistiques(), true);
    // Le seul tirage d'un coup est celui du blocage (ni esquive ni critique) ; un nouveau générateur par blocage voulu.
    Generateur quiBloque = generateurQuiTireSous(chance);

    REQUIRE(personnage.recevoirDegats(attaque, quiBloque).bloque > 0);

    // Récupération : aucun blocage, même sous la chance, tant que le temps ne passe pas (et aucun tirage : sans chance).
    for (int i = 0; i < 10; ++i) {  // dix coups de 10 : le personnage survit
        Generateur quiBloqueEncore = generateurQuiTireSous(chance);
        REQUIRE(personnage.recevoirDegats(attaque, quiBloqueEncore).bloque == 0);
    }

    // Après 3 s (mettreAJour du personnage), le bouclier bloque de nouveau.
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 3.1f);
    Generateur quiBloqueApres = generateurQuiTireSous(chance);
    REQUIRE(personnage.recevoirDegats(attaque, quiBloqueApres).bloque > 0);
}

TEST_CASE("un personnage qui esquive ne perd aucun point de vie") {
    Personnage personnage = unPersonnageSansEsquive();
    static_cast<void>(personnage.poserEquipement(Emplacement::Bottes, {IdObjet::BottesDeCuir}));
    const Attaque attaque{.montant = 10, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    // Le seul tirage du coup est celui de l'esquive (pas de critique ni de bouclier), choisi sous sa chance.
    Generateur generateur = generateurQuiTireSous(chanceDEsquive(NatureDAttaque::Contact, personnage.getStatistiques()));
    const int avant = personnage.getPointsDeVie();

    REQUIRE(personnage.recevoirDegats(attaque, generateur).esquive);
    REQUIRE(personnage.getPointsDeVie() == avant);
}

TEST_CASE("les points de vie maximum d'un personnage viennent de la base commune et de son Endurance") {
    const Caracteristiques base({12, 14, 10, 6, 8});
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .rayonVision = s_rayonVisionDeTest,
                                 .caracteristiques = base});

    REQUIRE(personnage.getPointsDeVieMax() == pointsDeVieMax(base));
    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

namespace {
Personnage unPersonnageAvecCaracteristiques(const Caracteristiques& base) {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                       .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = base});
}
}  // namespace

TEST_CASE("les caracteristiques effectives sont la base plus les bonus de l'equipement") {
    const Caracteristiques base({12, 14, 10, 6, 8});
    Personnage personnage = unPersonnageAvecCaracteristiques(base);

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));
    REQUIRE(personnage.getCaracteristiques() == base + definitionObjet(IdObjet::AnneauDePuissance).bonus);

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau2, {IdObjet::AnneauDePuissance}));
    REQUIRE(personnage.getCaracteristiques() ==
            base + definitionObjet(IdObjet::AnneauDePuissance).bonus + definitionObjet(IdObjet::AnneauDePuissance).bonus);

    static_cast<void>(personnage.retirerEquipement(Emplacement::Anneau1));
    static_cast<void>(personnage.retirerEquipement(Emplacement::Anneau2));
    REQUIRE(personnage.getCaracteristiques() == base);  // jamais ajustées : recalculées en entier
}

TEST_CASE("un equipement de depart qui donne des caracteristiques compte des la construction") {
    const Caracteristiques base({12, 14, 10, 6, 8});
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 5, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .nature = NatureDAttaque::Contact}),
                                 .equipementDeDepart = {{Emplacement::Collier, IdObjet::CollierDEndurance}},
                                 .rayonVision = s_rayonVisionDeTest,
                                 .caracteristiques = base});

    REQUIRE(personnage.getCaracteristiques() == base + definitionObjet(IdObjet::CollierDEndurance).bonus);
    REQUIRE(personnage.getPointsDeVieMax() == pointsDeVieMax(personnage.getCaracteristiques()));
    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

TEST_CASE("equiper un collier d'endurance monte les points de vie maximum et actuels de la meme difference") {
    Personnage personnage = unPersonnageAvecCaracteristiques(Caracteristiques({10, 10, 10, 10, 10}));
    const int maximumAvant = personnage.getPointsDeVieMax();
    const int vieAvant = personnage.getPointsDeVie();

    static_cast<void>(personnage.poserEquipement(Emplacement::Collier, {IdObjet::CollierDEndurance}));

    const int gain = s_pvParPointDEndurance * definitionObjet(IdObjet::CollierDEndurance).bonus.valeur(Caracteristique::Endurance);
    REQUIRE(personnage.getPointsDeVieMax() == maximumAvant + gain);
    REQUIRE(personnage.getPointsDeVie() == vieAvant + gain);
}

TEST_CASE("retirer le collier d'endurance ramene les points de vie, sans tuer un blesse") {
    Personnage personnage = unPersonnageAvecCaracteristiques(Caracteristiques({10, 10, 10, 10, 10}));
    static_cast<void>(personnage.poserEquipement(Emplacement::Collier, {IdObjet::CollierDEndurance}));
    const int maximumEquipe = personnage.getPointsDeVieMax();
    static_cast<void>(subir(personnage, maximumEquipe - 10));  // armure 0 : il lui reste 10 points de vie

    static_cast<void>(personnage.retirerEquipement(Emplacement::Collier));

    REQUIRE(personnage.estVivant());
    REQUIRE(personnage.getPointsDeVie() == 1);
}

TEST_CASE("changer d'equipement ne ressuscite pas un mort") {
    Personnage personnage = unPersonnageAvecCaracteristiques(Caracteristiques({10, 10, 10, 10, 10}));
    static_cast<void>(subir(personnage, personnage.getPointsDeVieMax()));
    REQUIRE_FALSE(personnage.estVivant());

    static_cast<void>(personnage.poserEquipement(Emplacement::Collier, {IdObjet::CollierDEndurance}));

    REQUIRE_FALSE(personnage.estVivant());
    REQUIRE(personnage.getPointsDeVie() == 0);
}

namespace {
// Un personnage dont l'attaque de base et un skill de dégâts profitent de la Puissance.
Personnage unPersonnageDePuissance(const Caracteristiques& base) {
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .attaqueDeBase = Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 100, .rechargeMin = 0.5f, .rechargeMax = 0.5f, .coefficients = puissance, .nature = NatureDAttaque::Contact}),
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::Degats, .portee = 80.0f, .puissance = 250, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .coefficients = puissance, .nature = NatureDAttaque::Contact})},
                                  {IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                       .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = base});
}
}  // namespace

TEST_CASE("equiper l'anneau de puissance augmente les degats de l'attaque de base") {
    Personnage personnage = unPersonnageDePuissance(Caracteristiques({12, 10, 10, 10, 10}));
    const int avant = personnage.getAttaqueDeBase().montant;
    REQUIRE(avant == montantEffectif(100, Coefficients::seul(Caracteristique::Puissance, 1.0f), personnage.getCaracteristiques()));

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));

    REQUIRE(personnage.getAttaqueDeBase().montant > avant);
    REQUIRE(personnage.getAttaqueDeBase().montant ==
            montantEffectif(100, Coefficients::seul(Caracteristique::Puissance, 1.0f), personnage.getCaracteristiques()));

    static_cast<void>(personnage.retirerEquipement(Emplacement::Anneau1));
    REQUIRE(personnage.getAttaqueDeBase().montant == avant);
}

TEST_CASE("le coup de l'attaque de base et le skill de degats rendent le montant effectif") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageDePuissance(Caracteristiques({20, 10, 10, 10, 10}));
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    const Cercle cible{{120.0f, 100.0f}, 10.0f};

    REQUIRE(frapperEnEntier(personnage, cible, generateur) == montantEffectif(100, puissance, personnage.getCaracteristiques()));
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, cible}));
    int rendu = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && rendu == 0; ++i) {
        rendu = montantOuZero(personnage.avancerSkill(s_dureeTic, generateur));
    }
    REQUIRE(rendu == montantEffectif(250, puissance, personnage.getCaracteristiques()));
}

TEST_CASE("la duree d'une provocation ne profite d'aucune caracteristique") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageDePuissance(Caracteristiques({40, 40, 40, 40, 40}));

    REQUIRE(personnage.engagerSkill(IdSkill::Provocation, std::nullopt));
    int rendu = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && rendu == 0; ++i) {
        rendu = montantOuZero(personnage.avancerSkill(s_dureeTic, generateur));
    }
    REQUIRE(rendu == 4);
}
