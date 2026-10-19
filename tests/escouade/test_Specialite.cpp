// ./tests/escouade/test_Specialite.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

#include "interface/BarreDeRaccourcis.hpp"
#include "escouade/Composition.hpp"
#include "base/Eclaircir.hpp"
#include "escouade/Escouade.hpp"
#include "escouade/Specialite.hpp"

TEST_CASE("catalogue : les specialites suivent l'ordre de IdSpecialite") {
    const std::vector<Specialite>& specialites = catalogue();

    REQUIRE(specialites.size() == 5);
    REQUIRE(specialites[0].id == IdSpecialite::Guerrier);
    REQUIRE(specialites[1].id == IdSpecialite::Pretre);
    REQUIRE(specialites[2].id == IdSpecialite::Mage);
    REQUIRE(specialites[3].id == IdSpecialite::Paladin);
    REQUIRE(specialites[4].id == IdSpecialite::Rodeur);
}

TEST_CASE("catalogue : chaque specialite a un nom, de la vision et des skills sans doublon") {
    for (const Specialite& specialite : catalogue()) {
        REQUIRE_FALSE(specialite.nom.empty());
        REQUIRE(specialite.definition.rayonVision == 320.0f);
        const std::vector<SkillConnu>& skills = specialite.definition.skills;
        for (std::size_t i = 0; i < skills.size(); ++i) {
            for (std::size_t j = i + 1; j < skills.size(); ++j) {
                REQUIRE(skills[i].id != skills[j].id);
            }
        }
    }
}

TEST_CASE("specialite : rend l'entree du catalogue de cet identifiant") {
    REQUIRE(specialite(IdSpecialite::Guerrier).nom == "Guerrier");
    REQUIRE(specialite(IdSpecialite::Pretre).definition.rang == Rang::Arriere);
    REQUIRE(specialite(IdSpecialite::Mage).definition.skills.front().id == IdSkill::Explosion);
}

TEST_CASE("composer : une definition par identifiant, dans l'ordre recu") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Mage, IdSpecialite::Guerrier, IdSpecialite::Pretre});

    REQUIRE(composition.size() == 3);
    REQUIRE(composition[0].rang == Rang::Arriere);
    REQUIRE(composition[0].skills.front().id == IdSkill::Explosion);
    REQUIRE(composition[1].rang == Rang::Avant);
    REQUIRE(composition[1].skills.front().id == IdSkill::FrappePuissante);
    REQUIRE(composition[2].skills.front().id == IdSkill::Soin);
}

TEST_CASE("composer : une liste vide donne une composition vide") {
    REQUIRE(composer({}).empty());
}

TEST_CASE("composer : des classes differentes gardent leur couleur de classe") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Pretre, IdSpecialite::Mage});

    for (std::size_t i = 0; i < composition.size(); ++i) {
        REQUIRE(composition[i].couleur == catalogue()[i].definition.couleur);
    }
}

TEST_CASE("composer : la meme classe prise plusieurs fois s'assombrit de 0, 30 puis 55 pour cent") {
    const sf::Color couleurDeClasse = specialite(IdSpecialite::Guerrier).definition.couleur;

    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Guerrier, IdSpecialite::Guerrier});

    REQUIRE(composition[0].couleur == couleurDeClasse);
    REQUIRE(composition[1].couleur == assombrir(couleurDeClasse, 0.30f));
    REQUIRE(composition[2].couleur == assombrir(couleurDeClasse, 0.55f));
    REQUIRE(composition[1].couleur != composition[0].couleur);
    REQUIRE(composition[2].couleur != composition[1].couleur);
}

TEST_CASE("composer : le rang d'occurrence se compte par classe, pas sur toute la liste") {
    const sf::Color couleurDuMage = specialite(IdSpecialite::Mage).definition.couleur;

    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Mage, IdSpecialite::Guerrier, IdSpecialite::Mage});

    REQUIRE(composition[0].couleur == couleurDuMage);
    REQUIRE(composition[1].couleur == specialite(IdSpecialite::Guerrier).definition.couleur);
    REQUIRE(composition[2].couleur == assombrir(couleurDuMage, 0.30f));
}

TEST_CASE("composer : seule la couleur change entre deux occurrences d'une classe") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Pretre, IdSpecialite::Pretre});

    REQUIRE(composition[1].armure == composition[0].armure);
    REQUIRE(composition[1].rang == composition[0].rang);
    REQUIRE(composition[1].skills.size() == composition[0].skills.size());
}

TEST_CASE("le paladin est une melee qui frappe et soigne au contact") {
    const DefinitionPersonnage& paladin = specialite(IdSpecialite::Paladin).definition;
    const DefinitionPersonnage& guerrier = specialite(IdSpecialite::Guerrier).definition;

    REQUIRE(specialite(IdSpecialite::Paladin).nom == "Paladin");
    REQUIRE(paladin.rang == Rang::Avant);
    REQUIRE(paladin.armure == guerrier.armure);
    REQUIRE(paladin.attaqueDeBase.getPortee() == guerrier.attaqueDeBase.getPortee());
    REQUIRE(paladin.skills.size() == 2);

    REQUIRE(paladin.skills[0].id == IdSkill::FrappeSacree);
    REQUIRE(paladin.skills[0].skill.getType() == TypeSkill::Degats);
    REQUIRE(paladin.skills[0].skill.getPuissance() == 20);
    REQUIRE(paladin.skills[0].skill.getPortee() == 20.0f);

    REQUIRE(paladin.skills[1].id == IdSkill::ImpositionDesMains);
    REQUIRE(paladin.skills[1].skill.getType() == TypeSkill::Soin);
    REQUIRE(paladin.skills[1].skill.getPuissance() == 25);
    REQUIRE(paladin.skills[1].skill.getPortee() == 20.0f);
}

TEST_CASE("le rodeur est un tireur avec un tir de loin et un skill de zone") {
    const DefinitionPersonnage& rodeur = specialite(IdSpecialite::Rodeur).definition;
    const DefinitionPersonnage& mage = specialite(IdSpecialite::Mage).definition;

    REQUIRE(specialite(IdSpecialite::Rodeur).nom == "Rôdeur");
    REQUIRE(rodeur.rang == Rang::Arriere);
    REQUIRE(rodeur.armure == 0);
    REQUIRE(rodeur.attaqueDeBase.getPortee() == mage.attaqueDeBase.getPortee());
    REQUIRE(rodeur.skills.size() == 2);

    REQUIRE(rodeur.skills[0].id == IdSkill::TirPercant);
    REQUIRE(rodeur.skills[0].skill.getType() == TypeSkill::Degats);
    REQUIRE(rodeur.skills[0].skill.getPuissance() == 25);
    REQUIRE(rodeur.skills[0].skill.getPortee() == 200.0f);

    REQUIRE(rodeur.skills[1].id == IdSkill::PluieDeFleches);
    REQUIRE(rodeur.skills[1].skill.getType() == TypeSkill::Zone);
    REQUIRE(rodeur.skills[1].skill.getPuissance() == 12);
    REQUIRE(rodeur.skills[1].skill.getRayonZone() == 45.0f);
    REQUIRE(rodeur.skills[1].skill.getPortee() == 320.0f);
}

TEST_CASE("trois rodeurs : tous derriere, en colonne, avec trois nuances de vert") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Rodeur, IdSpecialite::Rodeur, IdSpecialite::Rodeur});

    std::vector<Rang> rangs;
    for (const DefinitionPersonnage& definition : composition) {
        rangs.push_back(definition.rang);
    }
    using Decalages = std::vector<sf::Vector2f>;
    REQUIRE(decalagesDeFormation(rangs) == Decalages{{-25.0f, -50.0f}, {-25.0f, 0.0f}, {-25.0f, 50.0f}});
    REQUIRE(composition[0].couleur != composition[1].couleur);
    REQUIRE(composition[1].couleur != composition[2].couleur);
    REQUIRE(composition[0].couleur != composition[2].couleur);
}

TEST_CASE("trois rodeurs : la barre par defaut a une rangee par rodeur") {
    const Escouade escouade({100.0f, 100.0f},
                            composer({IdSpecialite::Rodeur, IdSpecialite::Rodeur, IdSpecialite::Rodeur}));

    const BarreDeRaccourcis barre = barreParDefaut(escouade.adressesDesSkills());

    for (std::size_t place = 1; place <= 3; ++place) {
        const std::size_t debut = (place - 1) * 4;
        REQUIRE(barre.adresse(debut) == AdresseSkill{place, IdSkill::TirPercant});
        REQUIRE(barre.adresse(debut + 1) == AdresseSkill{place, IdSkill::PluieDeFleches});
        REQUIRE_FALSE(barre.adresse(debut + 2).has_value());
        REQUIRE_FALSE(barre.adresse(debut + 3).has_value());
    }
}

TEST_CASE("une escouade de paladin, de rodeur et de guerrier connait les skills de chacun") {
    const Escouade escouade({100.0f, 100.0f},
                            composer({IdSpecialite::Paladin, IdSpecialite::Rodeur, IdSpecialite::Guerrier}));

    REQUIRE(escouade.typeDuSkill({1, IdSkill::ImpositionDesMains}) == TypeSkill::Soin);
    REQUIRE(escouade.typeDuSkill({2, IdSkill::PluieDeFleches}) == TypeSkill::Zone);
    REQUIRE(escouade.typeDuSkill({3, IdSkill::FrappePuissante}) == TypeSkill::Degats);
    REQUIRE_FALSE(escouade.typeDuSkill({1, IdSkill::TirPercant}).has_value());
}

TEST_CASE("zone de controle : 20 px pour le guerrier et le paladin, 0 pour les trois autres") {
    REQUIRE(specialite(IdSpecialite::Guerrier).definition.zoneDeControle == 20.0f);
    REQUIRE(specialite(IdSpecialite::Paladin).definition.zoneDeControle == 20.0f);
    REQUIRE(specialite(IdSpecialite::Pretre).definition.zoneDeControle == 0.0f);
    REQUIRE(specialite(IdSpecialite::Mage).definition.zoneDeControle == 0.0f);
    REQUIRE(specialite(IdSpecialite::Rodeur).definition.zoneDeControle == 0.0f);
}

TEST_CASE("composer garde la zone de controle de la classe, quelle que soit la nuance") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Paladin, IdSpecialite::Paladin, IdSpecialite::Mage});

    REQUIRE(composition[0].zoneDeControle == 20.0f);
    REQUIRE(composition[1].zoneDeControle == 20.0f);
    REQUIRE(composition[2].zoneDeControle == 0.0f);
}
