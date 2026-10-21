// ./tests/interface/test_AffichageSkills.cpp
#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>

#include "combat/IdSkill.hpp"
#include "interface/AffichageSkills.hpp"

TEST_CASE("affichage : chaque skill a un nom, tous differents") {
    std::set<std::string> noms;
    for (const IdSkill id : {IdSkill::FrappePuissante, IdSkill::Provocation, IdSkill::Soin, IdSkill::Explosion,
                             IdSkill::FrappeSacree, IdSkill::ImpositionDesMains, IdSkill::TirPercant, IdSkill::PluieDeFleches}) {
        const std::string nom = nomDuSkill(id);
        REQUIRE_FALSE(nom.empty());
        noms.insert(nom);
    }
    REQUIRE(noms.size() == 8);
}

TEST_CASE("affichage : le nom d'un skill est en UTF-8") {
    REQUIRE(nomDuSkill(IdSkill::FrappeSacree) == "Frappe sacrée");
    REQUIRE(nomDuSkill(IdSkill::ImpositionDesMains) == "Imposition des mains");
}
