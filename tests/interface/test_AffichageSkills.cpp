// ./tests/interface/test_AffichageSkills.cpp
#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>

#include "combat/IdSkill.hpp"
#include "interface/AffichageSkills.hpp"

TEST_CASE("affichage : chaque skill a un nom, tous differents") {
    std::set<std::string> noms;
    for (const IdSkill id : {IdSkill::FrappePuissante, IdSkill::Provocation, IdSkill::Soin, IdSkill::Explosion,
                             IdSkill::FrappeSacree, IdSkill::ImpositionDesMains, IdSkill::TirPercant, IdSkill::PluieDeFleches, IdSkill::Benediction, IdSkill::Renouvellement,
                             IdSkill::Contresort, IdSkill::Aegis}) {
        const std::string nom = nomDuSkill(id);
        REQUIRE_FALSE(nom.empty());
        noms.insert(nom);
    }
    REQUIRE(noms.size() == 12);
}

TEST_CASE("affichage : le nom d'un skill est en anglais") {
    REQUIRE(nomDuSkill(IdSkill::FrappeSacree) == "Holy Strike");
    REQUIRE(nomDuSkill(IdSkill::ImpositionDesMains) == "Lay on Hands");
    REQUIRE(nomDuSkill(IdSkill::Renouvellement) == "Renew");
    REQUIRE(nomDuSkill(IdSkill::Contresort) == "Counterspell");
    REQUIRE(nomDuSkill(IdSkill::Aegis) == "Aegis");
    REQUIRE(nomDeLaCapacite(IdCapacite::CoupAssommant) == "Stunning Blow");
    REQUIRE(nomDeLaSource(IdSkill::Soin) == "Heal");
    REQUIRE(nomDeLaSource(IdCapacite::CoupAssommant) == "Stunning Blow");
}
