// ./tests/combat/test_Absorption_immunite.cpp
// L'immunité au contrôle que donnent Aegis et Ward (étape 114.4) : tant qu'il reste des points d'une famille, la cible est immunisée
// contre le contrôle de cette famille ; une pose refusée affiche « Immune » et ne compte pas dans le rendement décroissant ; un contrôle
// déjà en place n'est pas retiré.
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "aides.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Controle.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "combat/Vitalite.hpp"

using Catch::Approx;

namespace {
EffetActif bouclier(FamilleDAbsorption famille, int points, float duree = 8.0f) {
    return {.source = IdSkill::Soin, .dureeRestante = duree, .sorte = Absorption{.famille = famille, .points = points}};
}

// Un contrôle de cet état posé par ce skill (un skill différent à chaque fois évite le rafraîchissement).
EffetActif controle(EtatInflige etat, IdSkill source = IdSkill::FrappeSacree, float duree = 2.0f) {
    return unControle(etat, duree, etat == EtatInflige::Ralentissement || etat == EtatInflige::Entrave ? 30 : 0, source);
}

// Vrai si la dernière pose de cet état a été refusée par l'immunité.
bool refusee(Vitalite& vitalite, EtatInflige etat) {
    const std::vector<ControleApplique> poses = vitalite.prendreLesControles();
    return poses.size() == 1 && poses[0].etat == etat && poses[0].immunise;
}
}  // namespace

TEST_CASE("immunite : un Aegis immunise contre l'etourdissement, le ralentissement et l'entrave") {
    for (const EtatInflige etat : {EtatInflige::Etourdissement, EtatInflige::Ralentissement, EtatInflige::Entrave}) {
        Vitalite vitalite(100, defensesDeTest(0));
        vitalite.poserEffet(bouclier(FamilleDAbsorption::Aegis, 40));

        vitalite.poserEffet(controle(etat));

        REQUIRE(refusee(vitalite, etat));
        REQUIRE_FALSE(estSousEtat(vitalite.getEffets(), etat));
    }
}

TEST_CASE("immunite : un Aegis n'immunise pas contre la Disruption") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(FamilleDAbsorption::Aegis, 40));

    vitalite.poserEffet(controle(EtatInflige::Disruption));

    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Disruption));
    const std::vector<ControleApplique> poses = vitalite.prendreLesControles();
    REQUIRE(poses.size() == 1);
    REQUIRE_FALSE(poses[0].immunise);
}

TEST_CASE("immunite : un Ward immunise contre la Disruption mais pas contre l'etourdissement") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(FamilleDAbsorption::Ward, 40));

    vitalite.poserEffet(controle(EtatInflige::Disruption));
    REQUIRE(refusee(vitalite, EtatInflige::Disruption));
    REQUIRE_FALSE(estSousEtat(vitalite.getEffets(), EtatInflige::Disruption));

    vitalite.poserEffet(controle(EtatInflige::Etourdissement));
    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Etourdissement));
}

TEST_CASE("immunite : sans point, plus d'immunite") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(FamilleDAbsorption::Aegis, 20));
    Generateur generateur = generateurDeTest();
    static_cast<void>(vitalite.recevoirDegats({.degats = degatsPhysiques(20), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact}, generateur));
    REQUIRE(vitalite.getEffets().pointsDAbsorption().aegis == 0);

    vitalite.poserEffet(controle(EtatInflige::Etourdissement));

    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Etourdissement));
}

TEST_CASE("immunite : un bouclier expire par sa duree, l'immunite aussi") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(FamilleDAbsorption::Aegis, 40, 2.0f));

    vitalite.mettreAJour(2.5f);
    vitalite.poserEffet(controle(EtatInflige::Etourdissement));

    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Etourdissement));
}

TEST_CASE("immunite : un controle bloque ne compte pas dans le rendement decroissant") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(FamilleDAbsorption::Aegis, 1000));
    for (int i = 0; i < 5; ++i) {
        vitalite.poserEffet(controle(EtatInflige::Etourdissement));
    }
    static_cast<void>(vitalite.prendreLesControles());
    REQUIRE(vitalite.getEffets().rendements().applications(EtatInflige::Etourdissement) == 0);
    Generateur generateur = generateurDeTest();
    static_cast<void>(vitalite.recevoirDegats({.degats = degatsPhysiques(1000), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact}, generateur));
    REQUIRE(vitalite.getEffets().pointsDAbsorption().aegis == 0);

    vitalite.poserEffet(controle(EtatInflige::Etourdissement, IdSkill::FrappePuissante, 2.0f));

    // Rien n'a été posé avant : la première application réelle garde sa durée pleine (100 %), et n'est pas « Immune ».
    REQUIRE(vitalite.getEffets().liste().front().dureeRestante == Approx(2.0f));
    REQUIRE(vitalite.getEffets().rendements().applications(EtatInflige::Etourdissement) == 1);
}

TEST_CASE("immunite : poser un bouclier ne retire pas un controle deja en place") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(controle(EtatInflige::Etourdissement, IdSkill::FrappeSacree, 3.0f));
    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Etourdissement));

    vitalite.poserEffet(bouclier(FamilleDAbsorption::Aegis, 40));

    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Etourdissement));
    vitalite.poserEffet(controle(EtatInflige::Etourdissement, IdSkill::PluieDeFleches));  // mais le nouveau est refusé
    REQUIRE(vitalite.getEffets().nombreDeControles() == 1);
}

TEST_CASE("immunite : un coup qui laisse des points a l'Aegis n'etourdit pas, un coup qui l'epuise etourdit") {
    const Attaque coupAssommant = {.degats = degatsPhysiques(30),
                                   .capaciteOffensive = 0,
                                   .nature = NatureDAttaque::Contact,
                                   .effets = {unControle(EtatInflige::Etourdissement, 1.0f, 0, IdSkill::FrappeSacree)}};
    Generateur generateur = generateurDeTest();

    Vitalite protege(200, defensesDeTest(0));
    protege.poserEffet(bouclier(FamilleDAbsorption::Aegis, 50));
    static_cast<void>(protege.recevoirDegats(coupAssommant, generateur));
    REQUIRE_FALSE(estSousEtat(protege.getEffets(), EtatInflige::Etourdissement));
    REQUIRE(protege.getEffets().pointsDAbsorption().aegis == 20);

    Vitalite epuise(200, defensesDeTest(0));
    epuise.poserEffet(bouclier(FamilleDAbsorption::Aegis, 30));
    static_cast<void>(epuise.recevoirDegats(coupAssommant, generateur));
    REQUIRE(estSousEtat(epuise.getEffets(), EtatInflige::Etourdissement));  // l'Aegis est tombé à 0 par ce coup même
}

TEST_CASE("immunite : la pose refusee est notee pour l'affichage (Immune)") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(FamilleDAbsorption::Ward, 40));

    vitalite.poserEffet(controle(EtatInflige::Disruption));

    const std::vector<ControleApplique> poses = vitalite.prendreLesControles();
    REQUIRE(poses.size() == 1);
    REQUIRE(poses[0].etat == EtatInflige::Disruption);
    REQUIRE(poses[0].immunise);
}
