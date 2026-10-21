// ./tests/interface/test_PileDEcrans.cpp
#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <memory>
#include <utility>

#include "interface/Ecran.hpp"
#include "interface/PileDEcrans.hpp"

namespace {

// Un écran sans comportement, qui laisse voir dessous ou non.
class EcranDeTest : public Ecran {
public:
    explicit EcranDeTest(bool transparent) : m_transparent(transparent) {}

    Transition traiterEvenement(const sf::Event&, const Contexte&) override { return Aucune{}; }
    void mettreAJour(float, const Contexte&) override {}
    void dessiner(const Contexte&) const override {}
    [[nodiscard]] bool laisseVoirDessous() const override { return m_transparent; }

private:
    bool m_transparent;
};

PileDEcrans pile(std::initializer_list<bool> transparents) {
    PileDEcrans resultat;
    for (const bool transparent : transparents) {
        resultat.push_back(std::make_unique<EcranDeTest>(transparent));
    }
    return resultat;
}

}  // namespace

TEST_CASE("pile : un seul ecran se dessine seul") {
    REQUIRE(premierEcranADessiner(pile({false})) == 0);
}

TEST_CASE("pile : un ecran opaque au dessus cache tout ce qui est dessous") {
    REQUIRE(premierEcranADessiner(pile({false, false, false})) == 2);
}

TEST_CASE("pile : un ecran qui laisse voir dessous montre l'ecran opaque en dessous") {
    REQUIRE(premierEcranADessiner(pile({false, false, true})) == 1);
}

TEST_CASE("pile : on descend tant que les ecrans laissent voir dessous") {
    REQUIRE(premierEcranADessiner(pile({false, false, true, true})) == 1);
}

TEST_CASE("pile : le plus bas ecran est dessine meme s'il laisse voir dessous") {
    REQUIRE(premierEcranADessiner(pile({true, true})) == 0);
}
