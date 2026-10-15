// ./tests/test_TailleDeVue.cpp
#include "aides.hpp"

#include "TailleDeVue.hpp"

TEST_CASE("tailleDeVue : 1200 x 800 donne une vue de 1200 x 800") {
    const sf::Vector2f taille = tailleDeVue({1200, 800}, 800.0f);
    verifierPointProche(taille, {1200.0f, 800.0f});
}

TEST_CASE("tailleDeVue : 1920 x 1080 garde la hauteur et elargit la vue") {
    const sf::Vector2f taille = tailleDeVue({1920, 1080}, 800.0f);
    verifierPointProche(taille, {1422.2f, 800.0f}, 0.1f);
}
