// ./tests/jeu/test_Partie_effets.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/EffetActif.hpp"
#include "jeu/Partie.hpp"
#include "monstres/Monstre.hpp"

// Les effets actifs posés par un skill sur un monstre, dans une partie : le monstre est à portée de la frappe du guerrier (comme dans
// test_Partie_skills.cpp), sans esquive ni armure, et assez solide pour survivre à tout ce que ces tests lui infligent.

namespace {
Partie partieAuMonstreSolide() {
    return partieDeTest(niveauDegage({{235.0f, 170.0f}}), {avecEnduranceEnorme(guerrierQuiAffaiblit()), pretreDeTest(), mageDeTest()},
                        s_graineDeTest, monstreIndestructible());
}

const EffetsActifs& effetsDuMonstre(const Partie& partie) {
    return partie.getMonstres().front().getEffets();
}
}  // namespace

TEST_CASE("effets : une frappe qui touche pose l'affaiblissement sur le monstre") {
    Partie partie = partieAuMonstreSolide();
    REQUIRE(effetsDuMonstre(partie).liste().empty());

    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));  // l'activation dure 0,3 s

    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);
    const EffetActif& effet = effetsDuMonstre(partie).liste()[0];
    REQUIRE(effet.source == IdSkill::FrappePuissante);
    REQUIRE(std::get<ModificateurDeStatistique>(effet.sorte).pourcentage == s_malusDArmureDeTest);
    REQUIRE(effet.dureeRestante > static_cast<float>(s_dureeMalusDArmureDeTest) - 0.5f);
    REQUIRE(effet.dureeRestante <= static_cast<float>(s_dureeMalusDArmureDeTest));
    REQUIRE(effetsDuMonstre(partie).nombreDeMalus() == 1);
    REQUIRE(effetsDuMonstre(partie).nombreDeBonus() == 0);
}

TEST_CASE("effets : l'affaiblissement expire apres sa duree") {
    Partie partie = partieAuMonstreSolide();
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));
    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);

    avancer(partie, nombreDeTics(static_cast<float>(s_dureeMalusDArmureDeTest) - 1.0f));
    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);

    avancer(partie, nombreDeTics(1.5f));
    REQUIRE(effetsDuMonstre(partie).liste().empty());
}

TEST_CASE("effets : frapper de nouveau pendant l'effet le rafraichit, il n'y en a jamais deux") {
    Partie partie = partieAuMonstreSolide();
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));
    // La recharge de la frappe dure 5 s : on attend qu'elle finisse, l'effet (8 s) est alors à moins de 3 s de sa fin.
    avancer(partie, nombreDeTics(5.0f));
    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);
    const float avant = effetsDuMonstre(partie).liste()[0].dureeRestante;
    REQUIRE(avant < 4.0f);

    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));

    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);
    REQUIRE(effetsDuMonstre(partie).liste()[0].dureeRestante > static_cast<float>(s_dureeMalusDArmureDeTest) - 0.5f);
}

TEST_CASE("effets : un skill sans effet actif n'en pose pas") {
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);

    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));

    REQUIRE(partie.getMonstres().front().getEffets().liste().empty());
}
