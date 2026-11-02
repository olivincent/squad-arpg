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

TEST_CASE("effets : descendre d'un etage retire tous les effets actifs, la benediction comprise") {
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), {guerrierDeTest(), pretreQuiBenitToutLeMonde(), mageDeTest()},
                                 s_graineDeTest, monstreIndestructible());
    partie.demanderSkill({Place{2}, IdSkill::Benediction});
    avancer(partie, nombreDeTics(0.8f));
    for (const Place place : {Place{1}, Place{2}, Place{3}}) {
        REQUIRE(partie.getEscouade().effetsDuPersonnage(place)->liste().size() == 1);
    }
    const int armureBenie = *partie.getEscouade().armureDuPersonnage(Place{1});

    const Partie suivante(niveauDegage({{235.0f, 170.0f}}), etageSuivant(partie.expedition()), s_graineDeTest, catalogueDeTest(monstreSansHasard()));

    for (const Place place : {Place{1}, Place{2}, Place{3}}) {
        REQUIRE(suivante.getEscouade().effetsDuPersonnage(place)->liste().empty());
    }
    REQUIRE(*suivante.getEscouade().armureDuPersonnage(Place{1}) < armureBenie);
}

TEST_CASE("effets : un skill sans effet actif n'en pose pas") {
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);

    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));

    REQUIRE(partie.getMonstres().front().getEffets().liste().empty());
}

// Les effets dans le temps dans une partie (étape 112) : les ticks sortent par le même canal qu'un coup ou un soin.

namespace {
// Les ticks d'événements de cette cible pendant `duree` secondes de partie.
std::vector<EvenementDeCombat> ticksPendant(Partie& partie, float duree) {
    std::vector<EvenementDeCombat> ticks;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            const auto* degats = std::get_if<DegatsSubis>(&evenement.detail);
            const auto* soin = std::get_if<Soin>(&evenement.detail);
            if ((degats != nullptr && degats->dansLeTemps) || (soin != nullptr && soin->dansLeTemps)) {
                ticks.push_back(evenement);
            }
        }
    }
    return ticks;
}
}  // namespace

TEST_CASE("effets dans le temps : une frappe qui touche pose le saignement, et chaque seconde un petit chiffre sort") {
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), {avecEnduranceEnorme(guerrierQuiSaigne()), pretreDeTest(), mageDeTest()},
                                 s_graineDeTest, monstreIndestructible());
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));  // l'activation dure 0,3 s
    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);
    REQUIRE(effetsDuMonstre(partie).nombreDeMalus() == 1);
    static_cast<void>(partie.prendreEvenements());

    const std::vector<EvenementDeCombat> ticks = ticksPendant(partie, static_cast<float>(s_dureeDeSaignementDeTest) + 1.0f);

    REQUIRE(ticks.size() == static_cast<std::size_t>(s_dureeDeSaignementDeTest));
    for (const EvenementDeCombat& tick : ticks) {
        REQUIRE(std::holds_alternative<IdMonstre>(tick.cible));
        const DegatsSubis& degats = std::get<DegatsSubis>(tick.detail);
        REQUIRE(degats.montant == s_montantDeSaignementDeTest);
        REQUIRE_FALSE(degats.critique);
        REQUIRE_FALSE(degats.opportunite);
        REQUIRE(degats.element == Element::Physique);
    }
    REQUIRE(effetsDuMonstre(partie).liste().empty());
}

TEST_CASE("effets dans le temps : un monstre tue par un tick meurt comme les autres et rapporte son experience") {
    // Une armure énorme réduit les coups physiques à leur plancher (20 %) : la frappe fait 15, l'attaque de base presque rien, tandis que le
    // saignement, qui ignore l'armure, fait ses 8 points entiers chaque seconde.
    DefinitionDeMonstre fragile = monstreSansHasard();
    fragile.armure = 100'000;
    fragile.pointsDeVieMax = 40;
    fragile.experience = 17;
    fragile.statistiques.capaciteDefensive = 1'000'000;  // aucun critique : la frappe ne tue pas d'un coup
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), {avecEnduranceEnorme(guerrierQuiSaigne()), pretreDeTest(), mageDeTest()},
                                 s_graineDeTest, fragile);
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));
    REQUIRE(partie.monstresRestants() == 1);
    const int experienceAvant = partie.getExperience();

    const std::vector<EvenementDeCombat> ticks = ticksPendant(partie, static_cast<float>(s_dureeDeSaignementDeTest) + 1.0f);

    REQUIRE(partie.monstresRestants() == 0);
    REQUIRE_FALSE(ticks.empty());
    REQUIRE(ticks.size() < static_cast<std::size_t>(s_dureeDeSaignementDeTest));  // il est mort avant la fin du saignement
    REQUIRE(partie.getExperience() == experienceAvant + 17);
}

TEST_CASE("effets dans le temps : le saignement continue si le lanceur meurt") {
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), {guerrierQuiSaigne(), pretreDeTest(), mageDeTest()},
                                 s_graineDeTest, monstreIndestructible());
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));
    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);
    static_cast<void>(partie.prendreEvenements());
    // Le guerrier meurt : plus aucune main pour finir le travail.
    static_cast<void>(subirCoup(partie.getEscouade(), IdMonstre{1}, Place{1}, 10'000'000));
    REQUIRE_FALSE(partie.getEscouade().placeEstVivante(Place{1}));

    const std::vector<EvenementDeCombat> ticks = ticksPendant(partie, 2.5f);

    REQUIRE(effetsDuMonstre(partie).liste().size() == 1);
    REQUIRE(ticks.size() >= 2);
}

TEST_CASE("effets dans le temps : un soin dans le temps rend des points de vie chaque seconde, en vert, jamais au-dela du maximum") {
    Partie partie = partieDeTest(niveauDegage({}), {guerrierDeTest(), pretreQuiRegenere(), mageDeTest()}, s_graineDeTest);
    const int vieMax = *partie.getEscouade().pointsDeVieMaxDuPersonnage(Place{1});
    static_cast<void>(subirCoup(partie.getEscouade(), IdMonstre{1}, Place{1}, 20));
    const int perte = vieMax - *partie.getEscouade().pointsDeVieDuPersonnage(Place{1});  // l'armure en a retenu une part
    REQUIRE(perte > 0);
    REQUIRE(perte < 2 * s_montantDeSoinDansLeTempsDeTest);  // un tick ne suffit pas toujours, deux plafonnent
    static_cast<void>(partie.prendreEvenements());
    partie.demanderSkill({Place{2}, IdSkill::Soin});
    avancer(partie, nombreDeTics(0.8f));
    REQUIRE(partie.getEscouade().effetsDuPersonnage(Place{1})->liste().size() == 1);
    static_cast<void>(partie.prendreEvenements());

    const std::vector<EvenementDeCombat> ticks = ticksPendant(partie, 5.0f);

    REQUIRE_FALSE(ticks.empty());
    int total = 0;
    for (const EvenementDeCombat& tick : ticks) {
        REQUIRE(std::holds_alternative<Place>(tick.cible));
        total += std::get<Soin>(tick.detail).montant;
    }
    REQUIRE(total == perte);  // plafonné : les ticks rendent ce que le guerrier avait perdu, pas un point de plus
}
