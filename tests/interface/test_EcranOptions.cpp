// ./tests/interface/test_EcranOptions.cpp
#include <catch2/catch_test_macros.hpp>

#include <variant>

#include "interface/EcranOptions.hpp"
#include "interface/ListeDeBoutons.hpp"

namespace {

const sf::Vector2f s_centre{600.0f, 400.0f};

const std::vector<Resolution> s_resolutions = {{1200, 800}, {1280, 720}, {1600, 900}};

// Où cliquer sur la ligne `indice`, à cette fraction de sa largeur (la
// géométrie ne dépend pas des libellés).
sf::Vector2f pointSurLigne(std::size_t indice, float fractionDeLargeur) {
    const ListeDeBoutons boutons({"", "", ""});
    const sf::FloatRect rect = boutons.rectangleDuBouton(indice, s_centre);
    return {rect.position.x + rect.size.x * fractionDeLargeur, rect.position.y + rect.size.y / 2.0f};
}

EcranOptions optionsEn(Reglages reglages) {
    return EcranOptions(reglages, s_resolutions);
}

const Reglages& reglagesDe(const Transition& transition) {
    return std::get<AppliquerReglages>(transition).reglages;
}

constexpr float s_milieu = 0.5f;
constexpr float s_flecheGauche = 0.1f;
constexpr float s_flecheDroite = 0.9f;

}  // namespace

TEST_CASE("options : cliquer sur Mode bascule en plein ecran") {
    EcranOptions options = optionsEn(Reglages{});

    const Transition transition = options.cliquer(pointSurLigne(0, s_milieu), s_centre);

    REQUIRE(std::holds_alternative<AppliquerReglages>(transition));
    REQUIRE(reglagesDe(transition).mode == ModeAffichage::PleinEcran);
}

TEST_CASE("options : un second clic sur Mode revient en fenetre, a la resolution choisie") {
    EcranOptions options = optionsEn(Reglages{1280, 720, ModeAffichage::Fenetre});

    static_cast<void>(options.cliquer(pointSurLigne(0, s_milieu), s_centre));
    const Transition transition = options.cliquer(pointSurLigne(0, s_milieu), s_centre);

    REQUIRE(reglagesDe(transition).mode == ModeAffichage::Fenetre);
    REQUIRE(reglagesDe(transition).largeur == 1280);
    REQUIRE(reglagesDe(transition).hauteur == 720);
}

TEST_CASE("options : la fleche droite passe a la resolution suivante") {
    EcranOptions options = optionsEn(Reglages{});

    const Transition transition = options.cliquer(pointSurLigne(1, s_flecheDroite), s_centre);

    REQUIRE(reglagesDe(transition).largeur == 1280);
    REQUIRE(reglagesDe(transition).hauteur == 720);
}

TEST_CASE("options : la fleche gauche revient a la resolution precedente") {
    EcranOptions options = optionsEn(Reglages{1600, 900, ModeAffichage::Fenetre});

    const Transition transition = options.cliquer(pointSurLigne(1, s_flecheGauche), s_centre);

    REQUIRE(reglagesDe(transition).largeur == 1280);
    REQUIRE(reglagesDe(transition).hauteur == 720);
}

TEST_CASE("options : les fleches ne depassent pas les bouts de la liste") {
    EcranOptions premiere = optionsEn(Reglages{1200, 800, ModeAffichage::Fenetre});
    EcranOptions derniere = optionsEn(Reglages{1600, 900, ModeAffichage::Fenetre});

    REQUIRE(std::holds_alternative<Aucune>(premiere.cliquer(pointSurLigne(1, s_flecheGauche), s_centre)));
    REQUIRE(std::holds_alternative<Aucune>(derniere.cliquer(pointSurLigne(1, s_flecheDroite), s_centre)));
}

TEST_CASE("options : les fleches se suivent d'un clic a l'autre") {
    EcranOptions options = optionsEn(Reglages{});

    static_cast<void>(options.cliquer(pointSurLigne(1, s_flecheDroite), s_centre));
    const Transition transition = options.cliquer(pointSurLigne(1, s_flecheDroite), s_centre);

    REQUIRE(reglagesDe(transition).largeur == 1600);
}

TEST_CASE("options : le milieu de la ligne de resolution ne fait rien") {
    EcranOptions options = optionsEn(Reglages{});

    REQUIRE(std::holds_alternative<Aucune>(options.cliquer(pointSurLigne(1, s_milieu), s_centre)));
}

TEST_CASE("options : la ligne de resolution est inactive en plein ecran") {
    EcranOptions options = optionsEn(Reglages{1200, 800, ModeAffichage::PleinEcran});

    REQUIRE(std::holds_alternative<Aucune>(options.cliquer(pointSurLigne(1, s_flecheDroite), s_centre)));
}

TEST_CASE("options : la ligne de resolution se reactive en revenant en fenetre") {
    EcranOptions options = optionsEn(Reglages{1200, 800, ModeAffichage::PleinEcran});

    static_cast<void>(options.cliquer(pointSurLigne(0, s_milieu), s_centre));
    const Transition transition = options.cliquer(pointSurLigne(1, s_flecheDroite), s_centre);

    REQUIRE(reglagesDe(transition).largeur == 1280);
}

TEST_CASE("options : sans resolution disponible, la ligne est inactive") {
    EcranOptions options(Reglages{}, {});

    REQUIRE(std::holds_alternative<Aucune>(options.cliquer(pointSurLigne(1, s_flecheDroite), s_centre)));
}

TEST_CASE("options : Retour depile") {
    EcranOptions options = optionsEn(Reglages{});

    REQUIRE(std::holds_alternative<Depiler>(options.cliquer(pointSurLigne(2, s_milieu), s_centre)));
}

TEST_CASE("options : un clic hors des boutons ne fait rien") {
    EcranOptions options = optionsEn(Reglages{});

    REQUIRE(std::holds_alternative<Aucune>(options.cliquer({5.0f, 5.0f}, s_centre)));
}
