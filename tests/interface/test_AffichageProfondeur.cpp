// ./tests/interface/test_AffichageProfondeur.cpp
#include <catch2/catch_test_macros.hpp>

#include "interface/AffichageProfondeur.hpp"
#include "jeu/Expedition.hpp"

TEST_CASE("ligneDeProfondeur : la profondeur sur la profondeur maximale, en anglais") {
    REQUIRE(ligneDeProfondeur(1) == "Depth 1 / 5");
    REQUIRE(ligneDeProfondeur(5) == "Depth 5 / 5");
    REQUIRE(s_profondeurMaximale == 5);  // le texte ci-dessus en dépend
}
