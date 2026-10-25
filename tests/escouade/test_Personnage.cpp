// ./tests/escouade/test_Personnage.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/EtatSkill.hpp"
#include "combat/TypeDArme.hpp"
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
        if (const int degats = degatsOuZero(personnage.avancerAttaque(s_dureeTic, generateur)); degats > 0) {
            return degats;
        }
    }
    return 0;
}

// Lance le skill en un seul appel : engage, puis avance l'activation jusqu'au
// départ de l'effet. Rend l'attaque du skill (dégâts, soin ou durée de provocation selon son type), toute à 0
// s'il ne peut pas être engagé. La portée est vérifiée par Escouade, pas ici. Les skills de ce fichier ont
// une activation nulle (sauf mention) : l'effet part au premier tic.
Attaque lancerSkillEnEntier(Personnage& personnage, std::optional<Cible> cible, Generateur& generateur, IdSkill id = IdSkill::FrappePuissante) {
    const Attaque rien = {.degats = {}, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    if (!personnage.engagerSkill(id, cible)) {
        return rien;
    }
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        if (const std::optional<Attaque> attaque = personnage.avancerSkill(s_dureeTic, generateur)) {
            return *attaque;
        }
    }
    return rien;
}

}  // namespace

// Personnage centré en (100, 100), portée 80 px.

TEST_CASE("estAPortee renvoie vrai pour un point proche") {
    const Personnage personnage({100.0f, 100.0f},
                                definitionArmee(IdObjet::Sceptre));

    REQUIRE(personnage.estAPortee(pointCible({150.0f, 100.0f})));
}

TEST_CASE("estAPortee renvoie faux pour un point lointain") {
    const Personnage personnage({100.0f, 100.0f},
                                definitionArmee(IdObjet::Sceptre));

    REQUIRE_FALSE(personnage.estAPortee(pointCible({300.0f, 100.0f})));
}

TEST_CASE("frapper renvoie les degats pour une cible a portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));

    REQUIRE(frapperEnEntier(personnage, pointCible({150.0f, 100.0f}), generateur) == s_degatsBaseDeTest);
}

TEST_CASE("frapper renvoie 0 pour une cible hors de portee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));

    REQUIRE(frapperEnEntier(personnage, pointCible({300.0f, 100.0f}), generateur) == 0);
}

TEST_CASE("frapper renvoie de nouveau les degats apres une duree simulee genereuse") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE(frapperEnEntier(personnage, cible, generateur) == s_degatsBaseDeTest);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == s_degatsBaseDeTest);
}

TEST_CASE("Un personnage est vivant a la creation") {
    const Personnage personnage({100.0f, 100.0f},
                                definitionArmee(IdObjet::Sceptre));

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est encore vivant apres des degats inferieurs a ses points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));

    subir(personnage, personnage.getPointsDeVieMax() - 20);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est mort apres autant de degats que ses points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));

    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("getCentre renvoie le centre donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f},
                                definitionArmee(IdObjet::Sceptre));

    REQUIRE(personnage.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getCercle renvoie un cercle centre sur le personnage, de rayon la moitie du cote") {
    const Personnage personnage({100.0f, 100.0f},
                                definitionArmee(IdObjet::Sceptre));
    const Cercle cercle = personnage.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(100.0f, 100.0f));
    REQUIRE(cercle.rayon == 10.0f);
}

TEST_CASE("getRayonVision renvoie le rayon donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = 123.0f, .caracteristiques = s_caracteristiquesDeTest});

    REQUIRE(personnage.getRayonVision() == 123.0f);
}

TEST_CASE("getZoneDeControle vient de l'arme : une attaque de contact engage, un projectile non, les mains nues engagent") {
    const Personnage epee = unPersonnageArme({100.0f, 100.0f}, IdObjet::EpeeLongue);
    const Personnage arc = unPersonnageArme({100.0f, 100.0f}, IdObjet::Arc);
    const Personnage mainsNues({100.0f, 100.0f}, definitionSansArme());

    REQUIRE(epee.getZoneDeControle() == zoneDeControleDe(NatureDAttaque::Contact));
    REQUIRE(epee.getZoneDeControle() > 0.0f);
    REQUIRE(arc.getZoneDeControle() == 0.0f);
    REQUIRE(mainsNues.getZoneDeControle() == zoneDeControleDe(NatureDAttaque::Contact));
}

TEST_CASE("changer d'arme change la zone de controle, la portee et l'attaque de base") {
    Personnage personnage({100.0f, 100.0f}, capableDePorter(definitionArmee(IdObjet::EpeeLongue), {IdObjet::Arc}));
    const DescriptionDAttaque epee = definitionObjet(IdObjet::EpeeLongue).arme.value().attaque;
    const DescriptionDAttaque arc = definitionObjet(IdObjet::Arc).arme.value().attaque;
    REQUIRE(personnage.getPorteeAttaqueDeBase() == epee.portee);
    REQUIRE(personnage.getAttaqueDeBase().degats.total() == montantEffectif(epee.degats[Element::Physique], Coefficients::seul(Caracteristique::Puissance, 1.0f),
                                                                     personnage.getCaracteristiques()));

    static_cast<void>(personnage.retirerEquipement(Emplacement::MainDroite));
    REQUIRE(personnage.getPorteeAttaqueDeBase() == mainsNues().portee);   // sans arme : les mains nues
    REQUIRE(personnage.getAttaqueDeBase().nature == NatureDAttaque::Contact);

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Arc}));
    REQUIRE(personnage.getPorteeAttaqueDeBase() == arc.portee);
    REQUIRE(personnage.getZoneDeControle() == 0.0f);
    REQUIRE(personnage.getAttaqueDeBase().nature == NatureDAttaque::Projectile);
}

TEST_CASE("changer d'arme relance une recharge complete tiree dans l'intervalle de la nouvelle arme") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f}, capableDePorter(definitionArmee(IdObjet::EpeeLongue), {IdObjet::EpeeADeuxMains}));
    const TileMap carte = carteDegagee5x5();
    const DescriptionDAttaque lente = definitionObjet(IdObjet::EpeeADeuxMains).arme.value().attaque;
    REQUIRE(personnage.engagerAttaque(1));  // une arme neuve est prête
    personnage.annulerAttaque();

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::EpeeADeuxMains}));
    REQUIRE_FALSE(personnage.engagerAttaque(1));  // la recharge reste à tirer : pas prête
    personnage.relancerSiLArmeAChange(generateur);

    REQUIRE_FALSE(personnage.engagerAttaque(1));  // en recharge
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, lente.rechargeMin * 0.9f);
    REQUIRE_FALSE(personnage.engagerAttaque(1));  // le minimum de l'intervalle n'est pas écoulé
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, lente.rechargeMax);
    REQUIRE(personnage.engagerAttaque(1));        // le maximum l'est
}

TEST_CASE("changer d'arme annule le coup de base en cours, sans le faire partir") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageArme({100.0f, 100.0f}, IdObjet::EpeeLongue);
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE(personnage.cibleEngagee() == 1);

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Dague}));

    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
    REQUIRE_FALSE(personnage.estEnExecution());
    REQUIRE_FALSE(personnage.avancerAttaque(1.0f, generateur).has_value());
}

TEST_CASE("un changement qui laisse l'arme intacte ne touche pas la recharge ni le coup en cours") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageArme({100.0f, 100.0f}, IdObjet::EpeeLongue);
    REQUIRE(personnage.engagerAttaque(1));

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDeProtection}));
    personnage.relancerSiLArmeAChange(generateur);

    REQUIRE(personnage.cibleEngagee() == 1);  // le coup engagé continue
    REQUIRE(personnage.avancerAttaque(1.0f, generateur).has_value());
    // L'anneau n'a pas non plus remis l'attaque prête : elle est en recharge après son départ.
    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau2, {IdObjet::AnneauDeProtection}));
    personnage.relancerSiLArmeAChange(generateur);
    REQUIRE_FALSE(personnage.engagerAttaque(1));
}

TEST_CASE("getPuissanceAttaqueDeBase renvoie les degats de l'attaque de base") {
    const Personnage personnage({100.0f, 100.0f},
                                definitionArmee(IdObjet::Sceptre));

    REQUIRE(personnage.getAttaqueDeBase().degats.total() == s_degatsBaseDeTest);
}

TEST_CASE("lire la puissance de l'attaque de base ne change ni le coup engage ni la recharge") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    Generateur generateur = generateurDeTest();
    REQUIRE(personnage.engagerAttaque(1));

    REQUIRE(personnage.getAttaqueDeBase().degats.total() == s_degatsBaseDeTest);

    REQUIRE(personnage.cibleEngagee() == 1);
    REQUIRE(degatsOuZero(personnage.avancerAttaque(0.2f, generateur)) == s_degatsBaseDeTest);  // le coup engagé part normalement
    REQUIRE(personnage.getAttaqueDeBase().degats.total() == s_degatsBaseDeTest);
    REQUIRE_FALSE(personnage.engagerAttaque(1));                // la recharge court toujours
}

TEST_CASE("Un personnage est mort apres un coup qui depasse le reste de ses points de vie") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));

    subir(personnage, personnage.getPointsDeVieMax() - 20);  // reste 20 points de vie
    subir(personnage, 50);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("Un personnage mort ne frappe plus") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    const Cercle cible = pointCible({150.0f, 100.0f});

    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 0);
}

TEST_CASE("Un personnage mort ne bouge plus") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
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
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});

    REQUIRE(lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, pointCible({150.0f, 100.0f})}), generateur).degats.total() == 25);
}

TEST_CASE("Un personnage avec un skill de provocation en expose le type et le lance") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});

    REQUIRE(personnage.typeSkill(IdSkill::Provocation) == TypeSkill::Provocation);
    REQUIRE(personnage.skillEstPret(IdSkill::Provocation));
    REQUIRE(lancerSkillEnEntier(personnage, std::nullopt, generateur, IdSkill::Provocation).dureeDeProvocation == 4);
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::Provocation));
}

TEST_CASE("zoneDuSkill est centree sur le personnage, au rayon de zone du skill") {
    Personnage provocateur({100.0f, 100.0f},
                           {.couleur = sf::Color::White,
                            .skills = {{IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                            .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
    Personnage sansSkill({100.0f, 100.0f},
                         definitionArmee(IdObjet::Sceptre));

    REQUIRE(provocateur.zoneDuSkill(IdSkill::Provocation).centre == provocateur.getCentre());
    REQUIRE(provocateur.zoneDuSkill(IdSkill::Provocation).rayon == 150.0f);
    REQUIRE(sansSkill.zoneDuSkill(IdSkill::Provocation).rayon == 0.0f);
}

TEST_CASE("engagerSkill refuse un personnage sans skill, sans toucher a l'attaque de base") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    const Cercle cible = pointCible({150.0f, 100.0f});

    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, Cible(CibleMonstre{1, cible})));
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == s_degatsBaseDeTest);
}

TEST_CASE("engagerSkill refuse un personnage mort") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});

    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, Cible(CibleMonstre{1, pointCible({150.0f, 100.0f})})));
}

TEST_CASE("skillEstAPortee mesure la portee du skill, sans toucher a l'attaque de base") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});

    REQUIRE(personnage.skillEstAPortee(IdSkill::FrappePuissante, pointCible({150.0f, 100.0f})));
    REQUIRE_FALSE(personnage.skillEstAPortee(IdSkill::FrappePuissante, pointCible({300.0f, 100.0f})));
    REQUIRE(frapperEnEntier(personnage, pointCible({150.0f, 100.0f}), generateur) == s_degatsBaseDeTest);
}

TEST_CASE("Un skill ne peut pas etre relance pendant sa recharge, puis le peut une fois ecoulee") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
    const TileMap carte = carteDegagee5x5();
    const Cible cible = CibleMonstre{1, pointCible({150.0f, 100.0f})};

    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur).degats.total() == 25);
    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur).degats.total() == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 4.8f);
    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur).degats.total() == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.4f);
    REQUIRE(lancerSkillEnEntier(personnage, cible, generateur).degats.total() == 25);
}

TEST_CASE("Apres un skill reussi, l'attaque de base attend une recharge complete") {
    Generateur generateur = generateurDeTest();
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
    const TileMap carte = carteDegagee5x5();
    const Cercle cible = pointCible({150.0f, 100.0f});

    // Attaque de base lancée, puis 0,3 s écoulées : il ne lui reste que
    // 0,2 s de recharge. Le skill la relance au complet (0,5 s) : 0,3 s
    // plus tard, elle ne doit toujours pas partir, alors qu'elle l'aurait
    // fait sans le skill.
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == s_degatsBaseDeTest);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, cible}), generateur).degats.total() == 25);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == 0);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(frapperEnEntier(personnage, cible, generateur) == s_degatsBaseDeTest);
}

TEST_CASE("mettreAJour s'arrete avant d'entrer dans un obstacle") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
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
                          definitionArmee(IdObjet::Sceptre));
    subir(personnage, 50);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax() - 50 + 30);
}

TEST_CASE("recevoirDegats applique l'armure du personnage") {
    Personnage melee({100.0f, 100.0f},
                     {.couleur = sf::Color::White,
                      .equipementDeDepart = s_equipementMeleeDeTest, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = caracteristiquesPourPorter(s_equipementMeleeDeTest)});
    Personnage tireur({100.0f, 100.0f},
                      definitionArmee(IdObjet::Sceptre));

    subir(melee, 10);
    subir(tireur, 10);

    REQUIRE(melee.getPointsDeVie() == melee.getPointsDeVieMax() - appliquerArmure(10, s_armureMeleeDeTest));
    REQUIRE(melee.getPointsDeVie() > tireur.getPointsDeVie());
    REQUIRE(tireur.getPointsDeVie() == tireur.getPointsDeVieMax() - 10);
}

TEST_CASE("recevoirSoin n'est pas reduit par l'armure") {
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .equipementDeDepart = s_equipementMeleeDeTest, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = caracteristiquesPourPorter(s_equipementMeleeDeTest)});
    subir(personnage, 60);
    const int retires = personnage.getPointsDeVieMax() - personnage.getPointsDeVie();
    REQUIRE(retires == appliquerArmure(60, s_armureMeleeDeTest));

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax() - retires + 30);
}

TEST_CASE("recevoirSoin ne depasse pas le maximum") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    subir(personnage, 10);

    personnage.recevoirSoin(30);

    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

TEST_CASE("recevoirSoin est sans effet sur un mort") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    subir(personnage, personnage.getPointsDeVieMax());

    personnage.recevoirSoin(30);

    REQUIRE_FALSE(personnage.estVivant());
    REQUIRE(personnage.getPointsDeVie() == 0);
}

TEST_CASE("En terrain degage, le personnage avance en ligne droite de vitesse * dt") {
    const TileMap carte = carteDegagee5x5();
    Personnage personnage({60.0f, 60.0f},
                          definitionArmee(IdObjet::Sceptre));

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
                          definitionArmee(IdObjet::Sceptre));
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
                          definitionArmee(IdObjet::Sceptre));

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
                          definitionArmee(IdObjet::Sceptre));

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
                          definitionArmee(IdObjet::Sceptre));

    ecoulerTemps(personnage, carte, {220.0f, 60.0f}, 0.2f);
    // Nouvelle cible à 50 px, dans le mur : le chemin est recalculé, le A*
    // n'en trouve pas, et le personnage va tout droit contre le mur du milieu.
    ecoulerTemps(personnage, carte, {270.0f, 60.0f}, 1.0f);

    REQUIRE(personnage.getCentre().x <= 110.01f);
}

TEST_CASE("Quand le trajet direct redevient libre, le chemin en cours est abandonne") {
    const TileMap carte = deuxPiecesAvecPorte();
    Personnage personnage({60.0f, 60.0f},
                          definitionArmee(IdObjet::Sceptre));

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
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
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
        total += degatsOuZero(personnage.avancerAttaque(s_dureeTic, generateur));
    }
    REQUIRE(total == 0);
    REQUIRE(personnage.cibleEngagee().has_value());

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += degatsOuZero(personnage.avancerAttaque(s_dureeTic, generateur));
    }
    REQUIRE(total == s_degatsBaseDeTest);
    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
}

TEST_CASE("l'attaque de base d'un personnage porte sa capacite offensive") {
    Generateur generateur = generateurDeTest();
    Personnage rodeur({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = Caracteristiques({10, 10, 14, 8, 11})});  // 11 de Volonté : le sceptre est actif
    REQUIRE(rodeur.getAttaqueDeBase().capaciteOffensive == 120);
    REQUIRE(rodeur.engagerAttaque(1));

    const std::optional<Attaque> attaque = rodeur.avancerAttaque(s_executionBaseDeTest, generateur);

    REQUIRE(attaque.has_value());
    REQUIRE(attaque->degats.total() == s_degatsBaseDeTest);
    REQUIRE(attaque->capaciteOffensive == 120);
}

TEST_CASE("l'attaque d'un personnage porte la nature de son attaque de base, celle d'un skill la sienne") {
    Generateur generateur = generateurDeTest();
    Personnage tireur({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 20.0f, .puissance = 7,
                                                                    .rechargeMin = 1.0f, .rechargeMax = 1.0f, .nature = NatureDAttaque::Contact})}},
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
    REQUIRE(tireur.getAttaqueDeBase().nature == NatureDAttaque::Projectile);
    REQUIRE(tireur.engagerAttaque(1));
    REQUIRE(tireur.avancerAttaque(s_executionBaseDeTest, generateur)->nature == NatureDAttaque::Projectile);

    REQUIRE(tireur.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));
    REQUIRE(tireur.avancerSkill(0.1f, generateur)->nature == NatureDAttaque::Contact);
}

TEST_CASE("annulerAttaque ne consomme pas de recharge") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE(degatsOuZero(personnage.avancerAttaque(0.05f, generateur)) == 0);

    personnage.annulerAttaque();

    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
    REQUIRE(personnage.engagerAttaque(1));  // aussitôt prête de nouveau
}

TEST_CASE("Apres le depart du coup, la recharge court avant le coup suivant") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegagee5x5();
    Personnage personnage = personnageAvecExecution();
    REQUIRE(personnage.engagerAttaque(1));
    REQUIRE(degatsOuZero(personnage.avancerAttaque(0.2f, generateur)) == s_degatsBaseDeTest);

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
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .tempsExecution = 0.3f, .nature = NatureDAttaque::Contact})}},
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
}

const Cible s_cibleDuSkill = CibleMonstre{1, {{150.0f, 100.0f}, 0.0f}};

}  // namespace

TEST_CASE("avancerSkill rend la puissance une seule fois, a la fin de l'activation") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = personnageAvecActivation();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_cibleDuSkill));

    int total = 0;
    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += degatsOuZero(personnage.avancerSkill(s_dureeTic, generateur));
    }
    REQUIRE(total == 0);
    REQUIRE(personnage.skillEngage().has_value());

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {
        total += degatsOuZero(personnage.avancerSkill(s_dureeTic, generateur));
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
    REQUIRE(degatsOuZero(personnage.avancerSkill(0.1f, generateur)) == 0);

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
    REQUIRE(degatsOuZero(personnage.avancerSkill(0.4f, generateur)) == 25);

    REQUIRE_FALSE(personnage.engagerAttaque(1));
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(personnage.engagerAttaque(1));
}

TEST_CASE("mettreAJour garde la ligne droite en terrain degage meme avec un obstacle a contourner loin du trajet") {
    const TileMap carte = carteDegagee10x10();
    Personnage personnage({60.0f, 60.0f},
                          definitionArmee(IdObjet::Sceptre));
    const std::vector<Cercle> loin = {{{180.0f, 200.0f}, 15.0f}};

    personnage.mettreAJour(s_dureeTic, {300.0f, 60.0f}, carte, loin, loin);

    REQUIRE(personnage.cheminRestant() == std::vector<sf::Vector2f>{{300.0f, 60.0f}});
}

TEST_CASE("mettreAJour contourne un obstacle a contourner place sur la ligne droite") {
    const TileMap carte = carteDegagee10x10();
    Personnage personnage({60.0f, 60.0f},
                          definitionArmee(IdObjet::Sceptre));
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
                          definitionArmee(IdObjet::Sceptre));
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
                          definitionArmee(IdObjet::Sceptre));
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
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .tempsExecution = 0.3f, .nature = NatureDAttaque::Contact})}, {IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .tempsExecution = 0.2f, .nature = NatureDAttaque::Contact})}},
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest, .caracteristiques = s_caracteristiquesDeLArmeDeTireurDeTest});
}

}  // namespace

TEST_CASE("Un personnage expose chacun de ses skills par identifiant") {
    const Personnage personnage = personnageAvecDeuxSkills();

    REQUIRE(personnage.typeSkill(IdSkill::FrappePuissante) == TypeSkill::DegatsSurCible);
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

    REQUIRE(lancerSkillEnEntier(personnage, s_cibleDuSkill, generateur, IdSkill::FrappePuissante).degats.total() == 25);
    // La frappe est en recharge, la provocation reste utilisable aussitôt.
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE(personnage.skillEstPret(IdSkill::Provocation));
    REQUIRE(lancerSkillEnEntier(personnage, std::nullopt, generateur, IdSkill::Provocation).dureeDeProvocation == 4);

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
                      .equipementDeDepart = s_equipementMeleeDeTest, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = caracteristiquesPourPorter(s_equipementMeleeDeTest)});
    Personnage tireur({100.0f, 100.0f},
                      definitionArmee(IdObjet::Sceptre));

    REQUIRE(subir(melee, 10) == appliquerArmure(10, s_armureMeleeDeTest));
    REQUIRE(subir(tireur, 10) == 10);
}

TEST_CASE("recevoirDegats sur un mort renvoie zero") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE(subir(personnage, 10) == 0);
}

TEST_CASE("recevoirSoin renvoie les points reellement rendus") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    subir(personnage, 10);

    REQUIRE(personnage.recevoirSoin(30) == 10);  // plafonne au maximum
    REQUIRE(personnage.recevoirSoin(30) == 0);   // deja plein
}

TEST_CASE("recevoirSoin sur un mort renvoie zero") {
    Personnage personnage({100.0f, 100.0f},
                          definitionArmee(IdObjet::Sceptre));
    subir(personnage, personnage.getPointsDeVieMax());

    REQUIRE(personnage.recevoirSoin(30) == 0);
}

TEST_CASE("l'armure du personnage est celle de son equipement de depart") {
    const std::vector<ObjetDeDepart> casqueSeul = {{Emplacement::Tete, IdObjet::CasqueDeFer}};
    Personnage casque({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .equipementDeDepart = casqueSeul, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = caracteristiquesPourPorter(casqueSeul)});
    Personnage nu({100.0f, 100.0f},
                  definitionArmee(IdObjet::Sceptre));

    REQUIRE(subir(casque, 40) == appliquerArmure(40, definitionObjet(IdObjet::CasqueDeFer).armure));
    REQUIRE(subir(nu, 40) == 40);
}

TEST_CASE("l'armure du personnage additionne tous les objets de depart") {
    const std::vector<ObjetDeDepart> depart = {{Emplacement::Tete, IdObjet::CasqueDeFer},
                                               {Emplacement::Torse, IdObjet::CuirasseDeMailles}};
    Personnage equipe({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .equipementDeDepart = depart, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = caracteristiquesPourPorter(depart)});

    REQUIRE(subir(equipe, 40) == appliquerArmure(40, armureDe(depart)));
}

TEST_CASE("les caracteristiques effectives d'un personnage sont celles de sa definition") {
    const Caracteristiques base({12, 14, 10, 6, 8});
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                                 .caracteristiques = base});

    REQUIRE(personnage.getCaracteristiques() == base);
}

TEST_CASE("les statistiques de combat d'un personnage viennent de son Adresse") {
    const Personnage rodeur({100.0f, 100.0f},
                            {.couleur = sf::Color::White,
                             .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                             .caracteristiques = Caracteristiques({10, 10, 14, 8, 8})});

    REQUIRE(rodeur.getStatistiques() == statistiquesDuPersonnage(rodeur.getCaracteristiques(), StatistiquesDeCombat()));
    REQUIRE(rodeur.getStatistiques().capaciteOffensive == 120);
    REQUIRE(rodeur.getStatistiques().capaciteDefensive == 120);
}

namespace {
Personnage unPersonnageSansEsquive() {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = Caracteristiques({10, 12, 10, 10, 11})});  // Endurance pour le bouclier, Volonté pour le sceptre
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
                                 .equipementDeDepart = {{Emplacement::Bottes, IdObjet::BottesDeCuir}},
                                 .rayonVision = s_rayonVisionDeTest});

    REQUIRE(personnage.getStatistiques().esquiveContact == 5);
}

TEST_CASE("le bouclier rond donne au personnage sa chance de blocage et son montant") {
    Personnage personnage = unPersonnageSansEsquive();

    static_cast<void>(personnage.poserEquipement(Emplacement::MainGauche, {IdObjet::BouclierRond}));

    const DonneesDeBouclier& bouclier = definitionObjet(IdObjet::BouclierRond).bouclier.value();
    REQUIRE(personnage.getStatistiques().chanceDeBlocage == bouclier.chanceDeBlocage);
    REQUIRE(personnage.getStatistiques().montantBloque == bouclier.montantBloque);
    static_cast<void>(personnage.retirerEquipement(Emplacement::MainGauche));
    REQUIRE(personnage.getStatistiques().montantBloque == 0);
}

TEST_CASE("un personnage au bouclier : apres un blocage, plus aucun pendant 3 s, puis de nouveau") {
    // Endurance énorme : il encaisse les centaines de coups de 1 point sans mourir.
    Personnage personnage({100.0f, 100.0f}, avecEnduranceEnorme(definitionArmee()));
    static_cast<void>(personnage.poserEquipement(Emplacement::MainGauche, {IdObjet::BouclierRond}));
    const TileMap carte = carteDegagee5x5();
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(1), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    // À 30 % par coup, la chance de ne voir aucun blocage sur 200 coups est de l'ordre de 10^-31.
    const auto jusquAuBlocage = [&] {
        bool bloque = false;
        for (int i = 0; i < 200 && !bloque; ++i) {
            bloque = personnage.recevoirDegats(attaque, generateur).bloque > 0;
        }
        return bloque;
    };

    REQUIRE(jusquAuBlocage());

    // Récupération : aucun blocage sur 100 coups, tant que le temps ne passe pas.
    for (int i = 0; i < 100; ++i) {
        REQUIRE(personnage.recevoirDegats(attaque, generateur).bloque == 0);
    }

    // Après 3 s (mettreAJour du personnage), le bouclier bloque de nouveau.
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, s_recuperationDuBlocage + 0.1f);
    REQUIRE(jusquAuBlocage());
}

TEST_CASE("un personnage qui esquive ne perd aucun point de vie") {
    Personnage personnage({100.0f, 100.0f}, avecEnduranceEnorme(definitionArmee()));
    static_cast<void>(personnage.poserEquipement(Emplacement::Bottes, {IdObjet::BottesDeCuir}));
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(10), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    int esquives = 0;

    // À 5 % par coup, la chance de n'en voir aucune sur 1 000 coups est de l'ordre de 10^-23.
    for (int i = 0; i < 1000; ++i) {
        const int avant = personnage.getPointsDeVie();
        if (personnage.recevoirDegats(attaque, generateur).esquive) {
            ++esquives;
            REQUIRE(personnage.getPointsDeVie() == avant);
        }
    }
    REQUIRE(esquives > 0);
}

TEST_CASE("les points de vie maximum d'un personnage viennent de la base commune et de son Endurance") {
    const Caracteristiques base({12, 14, 10, 6, 8});
    const Personnage personnage({100.0f, 100.0f},
                                {.couleur = sf::Color::White,
                                 .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                                 .caracteristiques = base});

    REQUIRE(personnage.getPointsDeVieMax() == pointsDeVieMax(base));
    REQUIRE(personnage.getPointsDeVie() == personnage.getPointsDeVieMax());
}

namespace {
Personnage unPersonnageAvecCaracteristiques(const Caracteristiques& base) {
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
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
                       .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 250, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .coefficients = puissance, .nature = NatureDAttaque::Contact})},
                                  {IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                       .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = base});
}
}  // namespace

TEST_CASE("equiper l'anneau de puissance augmente les degats de l'attaque de base") {
    Personnage personnage = unPersonnageDePuissance(Caracteristiques({12, 10, 10, 10, 11}));
    const int avant = personnage.getAttaqueDeBase().degats.total();
    REQUIRE(avant == montantEffectif(s_degatsBaseDeTest, Coefficients::seul(Caracteristique::Puissance, 1.0f), personnage.getCaracteristiques()));

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));

    REQUIRE(personnage.getAttaqueDeBase().degats.total() > avant);
    REQUIRE(personnage.getAttaqueDeBase().degats.total() ==
            montantEffectif(s_degatsBaseDeTest, Coefficients::seul(Caracteristique::Puissance, 1.0f), personnage.getCaracteristiques()));

    static_cast<void>(personnage.retirerEquipement(Emplacement::Anneau1));
    REQUIRE(personnage.getAttaqueDeBase().degats.total() == avant);
}

TEST_CASE("le coup de l'attaque de base et le skill de degats rendent le montant effectif") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageDePuissance(Caracteristiques({20, 10, 10, 10, 11}));
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    const Cercle cible{{120.0f, 100.0f}, 10.0f};

    REQUIRE(frapperEnEntier(personnage, cible, generateur) == montantEffectif(s_degatsBaseDeTest, puissance, personnage.getCaracteristiques()));
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, cible}));
    int rendu = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && rendu == 0; ++i) {
        rendu = degatsOuZero(personnage.avancerSkill(s_dureeTic, generateur));
    }
    REQUIRE(rendu == montantEffectif(250, puissance, personnage.getCaracteristiques()));
}

TEST_CASE("la duree d'une provocation ne profite d'aucune caracteristique") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageDePuissance(Caracteristiques({40, 40, 40, 40, 40}));

    REQUIRE(personnage.engagerSkill(IdSkill::Provocation, std::nullopt));
    int rendu = 0;
    for (int i = 0; i < nombreDeTics(1.0f) && rendu == 0; ++i) {
        if (const std::optional<Attaque> attaque = personnage.avancerSkill(s_dureeTic, generateur)) {
            rendu = attaque->dureeDeProvocation;
        }
    }
    REQUIRE(rendu == 4);
}

// Skills qui exigent un type d'arme (étape 91.5).
namespace {
Personnage unPersonnageAuSkillExigeant(std::optional<IdObjet> arme) {
    DefinitionPersonnage definition = arme.has_value() ? definitionArmee(*arme) : definitionSansArme();
    definition.skills = {{IdSkill::FrappePuissante, skillExigeantDesArmes({TypeDArme::EpeeLongue, TypeDArme::Masse})}};
    return Personnage({100.0f, 100.0f}, std::move(definition));
}
}  // namespace

TEST_CASE("un skill qui exige une arme est pret et engageable avec l'une de ces armes") {
    Personnage personnage = unPersonnageAuSkillExigeant(IdObjet::Masse);

    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE(personnage.peutEngagerSkill(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.etatDuSkill(IdSkill::FrappePuissante)->armeAbsente);
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));
}

TEST_CASE("un skill qui exige une arme est refuse avec une autre arme ou aux mains nues, sans recharge consommee") {
    for (const std::optional<IdObjet> arme : {std::optional<IdObjet>(IdObjet::Arc), std::optional<IdObjet>(IdObjet::Dague), std::optional<IdObjet>()}) {
        Personnage personnage = unPersonnageAuSkillExigeant(arme);

        REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
        REQUIRE_FALSE(personnage.peutEngagerSkill(IdSkill::FrappePuissante));
        REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));
        REQUIRE_FALSE(personnage.skillEngage().has_value());
        const std::optional<EtatSkill> etat = personnage.etatDuSkill(IdSkill::FrappePuissante);
        REQUIRE(etat->armeAbsente);
        REQUIRE(etat->lanceurVivant);
        REQUIRE(etat->tempsRestant == 0.0f);  // aucune recharge
        REQUIRE(etat->progression == 1.0f);
    }
}

TEST_CASE("un skill sans exigence reste utilisable aux mains nues") {
    DefinitionPersonnage definition = definitionSansArme();
    definition.skills = {{IdSkill::FrappePuissante, skillExigeantDesArmes({})}};
    const Personnage personnage({100.0f, 100.0f}, std::move(definition));

    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.etatDuSkill(IdSkill::FrappePuissante)->armeAbsente);
}

TEST_CASE("retirer ou poser l'arme exigee rend le skill refuse ou pret") {
    Personnage personnage = unPersonnageAuSkillExigeant(IdObjet::EpeeLongue);
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));

    static_cast<void>(personnage.retirerEquipement(Emplacement::MainDroite));
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE(personnage.etatDuSkill(IdSkill::FrappePuissante)->armeAbsente);

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Arc}));
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Masse}));
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.etatDuSkill(IdSkill::FrappePuissante)->armeAbsente);
}

TEST_CASE("retirer l'arme exigee annule l'activation du skill, sans recharge consommee") {
    Personnage personnage = unPersonnageAuSkillExigeant(IdObjet::EpeeLongue);
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));
    REQUIRE(personnage.skillEngage().has_value());

    static_cast<void>(personnage.retirerEquipement(Emplacement::MainDroite));

    REQUIRE_FALSE(personnage.skillEngage().has_value());
    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Masse}));
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));  // la recharge n'a pas été consommée
}

TEST_CASE("changer pour une autre arme qui convient laisse le skill en activation") {
    Personnage personnage = unPersonnageAuSkillExigeant(IdObjet::EpeeLongue);
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Masse}));

    REQUIRE(personnage.skillEngage().has_value());
}

// Objets inactifs (étape 92.2).
TEST_CASE("une arme dont le seuil n'est pas atteint reste portee mais le personnage frappe aux mains nues") {
    Personnage personnage({100.0f, 100.0f},
                          avecCaracteristique(definitionArmee(IdObjet::EpeeLongue), Caracteristique::Puissance, 5));  // sous les 11 exigés

    REQUIRE(personnage.getEquipement().a(Emplacement::MainDroite) == Objet{IdObjet::EpeeLongue});
    REQUIRE_FALSE(personnage.getEquipementActif().a(Emplacement::MainDroite).has_value());
    REQUIRE(personnage.getPorteeAttaqueDeBase() == mainsNues().portee);
    REQUIRE(personnage.getAttaqueDeBase().degats.total() ==
            montantEffectif(mainsNues().degats[Element::Physique], Coefficients::seul(Caracteristique::Puissance, 1.0f), personnage.getCaracteristiques()));
}

TEST_CASE("retirer l'anneau de puissance rend l'epee inactive, le remettre la remet en service") {
    Personnage personnage({100.0f, 100.0f},
                          avecCaracteristique(definitionArmee(IdObjet::EpeeLongue), Caracteristique::Puissance, 9));
    REQUIRE_FALSE(personnage.getEquipementActif().a(Emplacement::MainDroite).has_value());

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));  // 9 + 3 = 12
    REQUIRE(personnage.getEquipementActif().a(Emplacement::MainDroite).has_value());
    REQUIRE(personnage.getPorteeAttaqueDeBase() == definitionObjet(IdObjet::EpeeLongue).arme.value().attaque.portee);

    static_cast<void>(personnage.retirerEquipement(Emplacement::Anneau1));
    REQUIRE_FALSE(personnage.getEquipementActif().a(Emplacement::MainDroite).has_value());
    REQUIRE(personnage.getEquipement().a(Emplacement::MainDroite).has_value());  // toujours portée
    REQUIRE(personnage.getPorteeAttaqueDeBase() == mainsNues().portee);

    static_cast<void>(personnage.poserEquipement(Emplacement::Anneau1, {IdObjet::AnneauDePuissance}));
    REQUIRE(personnage.getEquipementActif().a(Emplacement::MainDroite).has_value());
}

TEST_CASE("une armure ou un bouclier inactif ne donne ni armure ni blocage") {
    const std::vector<ObjetDeDepart> lourd = {{Emplacement::Torse, IdObjet::CuirasseDeMailles}, {Emplacement::MainGauche, IdObjet::BouclierRond}};
    Personnage nu({100.0f, 100.0f}, {.couleur = sf::Color::White, .equipementDeDepart = lourd, .rayonVision = s_rayonVisionDeTest});  // Endurance 0
    Personnage fort({100.0f, 100.0f}, {.couleur = sf::Color::White, .equipementDeDepart = lourd, .rayonVision = s_rayonVisionDeTest,
                                       .caracteristiques = caracteristiquesPourPorter(lourd)});

    // L'armure sans le bouclier (qui bloquerait au hasard) ; le blocage se lit dans les statistiques.
    const std::vector<ObjetDeDepart> cuirasse = {{Emplacement::Torse, IdObjet::CuirasseDeMailles}};
    Personnage cuirasseSansForce({100.0f, 100.0f}, {.couleur = sf::Color::White, .equipementDeDepart = cuirasse, .rayonVision = s_rayonVisionDeTest});
    Personnage cuirasseAvecForce({100.0f, 100.0f}, {.couleur = sf::Color::White, .equipementDeDepart = cuirasse, .rayonVision = s_rayonVisionDeTest,
                                                    .caracteristiques = caracteristiquesPourPorter(cuirasse)});

    REQUIRE(subir(cuirasseSansForce, 40) == 40);
    REQUIRE(nu.getStatistiques().montantBloque == 0);
    REQUIRE(nu.getEquipement().armureTotale() > 0);  // portée, sans effet
    REQUIRE(subir(cuirasseAvecForce, 40) == appliquerArmure(40, armureDe(cuirasse)));
    REQUIRE(fort.getStatistiques().montantBloque == definitionObjet(IdObjet::BouclierRond).bouclier->montantBloque);
}

TEST_CASE("une arme inactive ne permet pas les skills qui l'exigent") {
    DefinitionPersonnage definition = avecCaracteristique(definitionArmee(IdObjet::EpeeLongue), Caracteristique::Puissance, 5);
    definition.skills = {{IdSkill::FrappePuissante, skillExigeantDesArmes({TypeDArme::EpeeLongue})}};
    const Personnage personnage({100.0f, 100.0f}, std::move(definition));

    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE(personnage.etatDuSkill(IdSkill::FrappePuissante)->armeAbsente);
}

TEST_CASE("un bonus d'objet actif compte dans les points de vie maximum") {
    Personnage personnage({100.0f, 100.0f}, definitionSansArme());
    const int avant = personnage.getPointsDeVieMax();

    static_cast<void>(personnage.poserEquipement(Emplacement::Collier, {IdObjet::CollierDEndurance}));

    REQUIRE(personnage.getPointsDeVieMax() > avant);
}

// Pénétration d'armure (étape 92.4).
TEST_CASE("l'attaque de base d'une dague porte sa penetration, celle d'une epee non, celle d'un skill jamais") {
    DefinitionPersonnage definition = capableDePorter(definitionArmee(IdObjet::Dague), {IdObjet::Dague});
    definition.skills = {{IdSkill::FrappePuissante, skillExigeantDesArmes({})}};
    Personnage dague({100.0f, 100.0f}, definition);
    Personnage epee({100.0f, 100.0f}, definitionArmee(IdObjet::EpeeLongue));
    Generateur generateur = generateurDeTest();

    REQUIRE(dague.getAttaqueDeBase().penetration == definitionObjet(IdObjet::Dague).arme->attaque.penetration);
    REQUIRE(dague.getAttaqueDeBase().penetration > 0);
    REQUIRE(epee.getAttaqueDeBase().penetration == 0);
    REQUIRE(dague.engagerAttaque(1));
    REQUIRE(dague.avancerAttaque(1.0f, generateur)->penetration == dague.getAttaqueDeBase().penetration);  // part avec le coup

    REQUIRE(dague.engagerSkill(IdSkill::FrappePuissante, CibleMonstre{1, Cercle{}}));
    REQUIRE(dague.avancerSkill(1.0f, generateur)->penetration == 0);
}

TEST_CASE("changer pour une dague donne la penetration, la retirer la reprend") {
    Personnage personnage({100.0f, 100.0f}, capableDePorter(definitionArmee(IdObjet::EpeeLongue), {IdObjet::Dague}));
    REQUIRE(personnage.getAttaqueDeBase().penetration == 0);

    static_cast<void>(personnage.poserEquipement(Emplacement::MainDroite, {IdObjet::Dague}));
    REQUIRE(personnage.getAttaqueDeBase().penetration == definitionObjet(IdObjet::Dague).arme->attaque.penetration);

    static_cast<void>(personnage.retirerEquipement(Emplacement::MainDroite));
    REQUIRE(personnage.getAttaqueDeBase().penetration == 0);  // les mains nues
}

TEST_CASE("l'attaque d'un skill range son montant dans le champ de son type, les autres restent a zero") {
    Generateur generateur = generateurDeTest();
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})},
                                      {IdSkill::Soin, Skill({.type = TypeSkill::Soin, .portee = 80.0f, .puissance = 90, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .coefficients = puissance, .nature = NatureDAttaque::Contact})},
                                      {IdSkill::Provocation, Skill({.type = TypeSkill::Provocation, .portee = 0.0f, .puissance = 4, .rechargeMin = 10.0f, .rechargeMax = 10.0f, .rayonZone = 150.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                           .caracteristiques = Caracteristiques({10, 10, 10, 10, 10})});

    const Attaque degats = lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur);
    REQUIRE(degats.degats.total() == 25);
    REQUIRE(degats.soin == 0);
    REQUIRE(degats.dureeDeProvocation == 0);

    const Attaque soin = lancerSkillEnEntier(personnage, Cible(CiblePoint{{120.0f, 100.0f}}), generateur, IdSkill::Soin);
    REQUIRE(soin.soin == 90);
    REQUIRE(soin.degats.total() == 0);
    REQUIRE(soin.dureeDeProvocation == 0);

    const Attaque provocation = lancerSkillEnEntier(personnage, std::nullopt, generateur, IdSkill::Provocation);
    REQUIRE(provocation.dureeDeProvocation == 4);
    REQUIRE(provocation.degats.total() == 0);
    REQUIRE(provocation.soin == 0);
}

TEST_CASE("l'attaque d'un skill de zone de feu porte toute sa puissance dans la portion de feu") {
    Generateur generateur = generateurDeTest();
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    Personnage personnage({100.0f, 100.0f},
                          {.couleur = sf::Color::White,
                           .skills = {{IdSkill::Explosion, Skill({.type = TypeSkill::Zone, .portee = 200.0f, .puissance = 40, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .rayonZone = 60.0f, .coefficients = puissance, .nature = NatureDAttaque::Zone, .element = Element::Feu})},
                                      {IdSkill::FrappePuissante, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 25, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .nature = NatureDAttaque::Contact})}},
                           .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                           .caracteristiques = Caracteristiques({10, 10, 10, 10, 10})});

    const Attaque explosion = lancerSkillEnEntier(personnage, Cible(CiblePoint{{150.0f, 100.0f}}), generateur, IdSkill::Explosion);
    const Attaque frappe = lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur);

    REQUIRE(explosion.degats[Element::Feu] == 40);
    REQUIRE(explosion.degats[Element::Physique] == 0);
    REQUIRE(explosion.degats[Element::Sacre] == 0);
    REQUIRE(frappe.degats[Element::Physique] == 25);  // un skill sans element est physique
    REQUIRE(frappe.degats[Element::Feu] == 0);
}

TEST_CASE("l'attaque de base et la Puissance : toutes les portions profitent de la Puissance") {
    Generateur generateur = generateurDeTest();
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    Personnage fort({100.0f, 100.0f},
                    {.couleur = sf::Color::White,
                     .skills = {{IdSkill::Explosion, Skill({.type = TypeSkill::Zone, .portee = 200.0f, .puissance = 100, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .rayonZone = 60.0f, .coefficients = puissance, .nature = NatureDAttaque::Zone, .element = Element::Feu})}},
                     .equipementDeDepart = s_armeDeTireurSeuleDeTest, .rayonVision = s_rayonVisionDeTest,
                     .caracteristiques = Caracteristiques({20, 10, 10, 10, 10})});

    const Attaque explosion = lancerSkillEnEntier(fort, Cible(CiblePoint{{150.0f, 100.0f}}), generateur, IdSkill::Explosion);

    REQUIRE(explosion.degats[Element::Feu] == montantEffectif(100, puissance, fort.getCaracteristiques()));
    REQUIRE(explosion.degats[Element::Feu] > 100);
}

namespace {
// Un personnage dont l'unique skill est un skill de frappe de test (bonus de feu 10, puissance 0) qui accepte ces armes.
Personnage unPersonnageDeFrappe(IdObjet arme, std::vector<TypeDArme> armes) {
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    return Personnage({100.0f, 100.0f},
                      {.couleur = sf::Color::White,
                       .skills = {{IdSkill::FrappeSacree, Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, .puissance = 0, .rechargeMin = 5.0f, .rechargeMax = 5.0f, .coefficients = puissance, .nature = NatureDAttaque::Contact, .armesExigees = std::move(armes), .bonusDeFrappe = degatsDe(Element::Feu, 10)})}},
                       .equipementDeDepart = {{Emplacement::MainDroite, arme}}, .rayonVision = s_rayonVisionDeTest,
                       .caracteristiques = Caracteristiques({20, 20, 20, 20, 20})});
}
}  // namespace

TEST_CASE("un skill de frappe prend la penetration de l'arme, pas celle du skill") {
    Generateur generateur = generateurDeTest();
    Personnage dague = unPersonnageDeFrappe(IdObjet::Dague, {TypeDArme::Dague, TypeDArme::EpeeLongue});
    Personnage epee = unPersonnageDeFrappe(IdObjet::EpeeLongue, {TypeDArme::Dague, TypeDArme::EpeeLongue});

    const Attaque avecDague = lancerSkillEnEntier(dague, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur, IdSkill::FrappeSacree);
    const Attaque avecEpee = lancerSkillEnEntier(epee, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur, IdSkill::FrappeSacree);

    REQUIRE(avecDague.penetration == definitionObjet(IdObjet::Dague).arme->attaque.penetration);
    REQUIRE(avecDague.penetration > 0);
    REQUIRE(avecEpee.penetration == 0);
}

TEST_CASE("un skill de frappe prend les degats physiques et la nature de l'arme, le feu du bonus") {
    Generateur generateur = generateurDeTest();
    const Coefficients puissance = Coefficients::seul(Caracteristique::Puissance, 1.0f);
    Personnage archer = unPersonnageDeFrappe(IdObjet::Arc, {TypeDArme::Arc});

    const Attaque tir = lancerSkillEnEntier(archer, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur, IdSkill::FrappeSacree);

    REQUIRE(tir.nature == NatureDAttaque::Projectile);  // celle de l'arc, pas la nature déclarée par le skill
    REQUIRE(tir.degats[Element::Physique] == montantEffectif(definitionObjet(IdObjet::Arc).arme->attaque.degats[Element::Physique], puissance, archer.getCaracteristiques()));
    REQUIRE(tir.degats[Element::Feu] == montantEffectif(10, puissance, archer.getCaracteristiques()));
}

TEST_CASE("changer d'arme change les degats physiques d'un skill de frappe, pas son bonus") {
    Generateur generateur = generateurDeTest();
    Personnage personnage = unPersonnageDeFrappe(IdObjet::EpeeLongue, {TypeDArme::EpeeLongue, TypeDArme::Dague});
    const Attaque epee = lancerSkillEnEntier(personnage, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur, IdSkill::FrappeSacree);

    Personnage poignard = unPersonnageDeFrappe(IdObjet::Dague, {TypeDArme::EpeeLongue, TypeDArme::Dague});
    const Attaque dague = lancerSkillEnEntier(poignard, Cible(CibleMonstre{1, Cercle{{120.0f, 100.0f}, 10.0f}}), generateur, IdSkill::FrappeSacree);

    REQUIRE(epee.degats[Element::Physique] != dague.degats[Element::Physique]);  // 15 et 14, avec la Puissance
    REQUIRE(epee.degats[Element::Feu] == dague.degats[Element::Feu]);
}
