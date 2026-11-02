// ./tests/jeu/test_Partie_controle.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Controle.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "jeu/Partie.hpp"
#include "monstres/Monstre.hpp"

// Le contrôle dans une partie (étape 113) : une frappe qui étourdit, le rendement décroissant et ses textes, le contresort du mage. Le monstre
// est à portée de la frappe du guerrier (comme dans test_Partie_effets.cpp), sans esquive ni armure, et indestructible.

using Catch::Approx;

namespace {
constexpr AdresseSkill s_adresseContresort{Place{3}, IdSkill::Contresort};

Partie partieAuMonstreEtourdi() {
    return partieDeTest(niveauDegage({{235.0f, 170.0f}}), {avecEnduranceEnorme(guerrierQuiEtourdit()), pretreDeTest(), mageQuiContreLesSorts()},
                        s_graineDeTest, monstreIndestructible());
}

const EffetsActifs& effetsDuMonstre(const Partie& partie) {
    return partie.getMonstres().front().getEffets();
}

// Avance de cette durée et rend les contrôles signalés sur des monstres, dans l'ordre.
std::vector<ControleSubi> controlesSurLesMonstresPendant(Partie& partie, float duree) {
    std::vector<ControleSubi> controles;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        partie.mettreAJour(s_dureeTic);
        for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
            if (const auto* controle = std::get_if<ControleSubi>(&evenement.detail); controle != nullptr && std::holds_alternative<IdMonstre>(evenement.cible)) {
                controles.push_back(*controle);
            }
        }
    }
    return controles;
}
}  // namespace

TEST_CASE("controle partie : une frappe qui touche etourdit le monstre et le dit") {
    Partie partie = partieAuMonstreEtourdi();
    REQUIRE(effetsDuMonstre(partie).liste().empty());

    partie.demanderSkill(s_adresseFrappe);
    const std::vector<ControleSubi> controles = controlesSurLesMonstresPendant(partie, 0.4f);  // l'activation dure 0,3 s

    REQUIRE(controles.size() == 1);
    REQUIRE(controles[0].etat == EtatInflige::Etourdissement);
    REQUIRE_FALSE(controles[0].immunise);
    REQUIRE(estSousEtat(effetsDuMonstre(partie), EtatInflige::Etourdissement));
    REQUIRE(effetsDuMonstre(partie).nombreDeControles() == 1);
    REQUIRE(effetsDuMonstre(partie).pilesDesMalus().empty());  // un contrôle n'est pas dans les marques rouges
    const EffetActif& effet = effetsDuMonstre(partie).liste()[0];
    REQUIRE(effet.source == IdSkill::FrappePuissante);
    REQUIRE(effet.dureeRestante > s_dureeDEtourdissementDeTest - 0.5f);
    REQUIRE(effet.dureeRestante <= s_dureeDEtourdissementDeTest);
}

TEST_CASE("controle partie : l'etourdissement expire apres sa duree") {
    Partie partie = partieAuMonstreEtourdi();
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));
    REQUIRE(estSousEtat(effetsDuMonstre(partie), EtatInflige::Etourdissement));

    avancer(partie, nombreDeTics(2.0f));

    REQUIRE_FALSE(estSousEtat(effetsDuMonstre(partie), EtatInflige::Etourdissement));
}

TEST_CASE("controle partie : le rendement decroissant raccourcit l'etourdissement puis dit Immune") {
    Partie partie = partieAuMonstreEtourdi();
    std::vector<ControleSubi> controles;
    for (int frappe = 0; frappe < 4; ++frappe) {
        partie.demanderSkill(s_adresseFrappe);
        const std::vector<ControleSubi> duTour = controlesSurLesMonstresPendant(partie, 0.4f);
        controles.insert(controles.end(), duTour.begin(), duTour.end());
        avancer(partie, nombreDeTics(5.0f));  // la recharge de la frappe
    }

    REQUIRE(controles.size() == 4);
    REQUIRE_FALSE(controles[0].immunise);
    REQUIRE_FALSE(controles[1].immunise);
    REQUIRE_FALSE(controles[2].immunise);
    REQUIRE(controles[3].immunise);
    REQUIRE(controles[3].etat == EtatInflige::Etourdissement);
}

TEST_CASE("controle partie : la duree pleine revient 15 s apres le dernier etourdissement") {
    Partie partie = partieAuMonstreEtourdi();
    partie.demanderSkill(s_adresseFrappe);
    static_cast<void>(controlesSurLesMonstresPendant(partie, 0.4f));
    avancer(partie, nombreDeTics(5.0f));
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.4f));
    REQUIRE(effetsDuMonstre(partie).rendements().applications(EtatInflige::Etourdissement) == 2);

    avancer(partie, nombreDeTics(15.0f));

    REQUIRE(effetsDuMonstre(partie).rendements().applications(EtatInflige::Etourdissement) == 0);
}

TEST_CASE("controle partie : le contresort met un monstre en Disruption, sans lui faire de degats") {
    Partie partie = partieAuMonstreEtourdi();
    const int vieDeDepart = partie.getMonstres().front().getPointsDeVie();

    partie.demanderSkill(s_adresseContresort);
    const std::vector<ControleSubi> controles = controlesSurLesMonstresPendant(partie, 0.6f);  // l'activation dure 0,4 s

    REQUIRE(controles.size() == 1);
    REQUIRE(controles[0].etat == EtatInflige::Disruption);
    REQUIRE(estSousEtat(effetsDuMonstre(partie), EtatInflige::Disruption));
    REQUIRE_FALSE(estSousEtat(effetsDuMonstre(partie), EtatInflige::Etourdissement));
    const EffetActif& effet = effetsDuMonstre(partie).liste()[0];
    REQUIRE(effet.source == IdSkill::Contresort);
    REQUIRE(effet.dureeRestante > s_dureeDeDisruptionDeTest - 0.7f);
    REQUIRE(effet.dureeRestante <= s_dureeDeDisruptionDeTest * 1.5f);  // majorée au plus de 50 % : coefficient de durée 0,5 et plafond de +100 %
    // Les attaques de base des tireurs ont pu toucher : on ne vérifie que l'absence de dégâts du sort (le monstre n'a qu'une barre de vie énorme).
    REQUIRE(partie.getMonstres().front().getPointsDeVie() <= vieDeDepart);
}

TEST_CASE("controle partie : un controle de monstre sur un personnage s'affiche au-dessus de lui") {
    Partie partie = partieAuMonstreEtourdi();
    Generateur generateur = generateurDeTest();

    static_cast<void>(partie.getEscouade().recevoirCoup(IdMonstre{1}, Place{2}, attaqueQuiControle(unControle(EtatInflige::Etourdissement, 1.0f)), generateur));
    partie.mettreAJour(s_dureeTic);

    bool vu = false;
    for (const EvenementDeCombat& evenement : partie.prendreEvenements()) {
        const auto* controle = std::get_if<ControleSubi>(&evenement.detail);
        if (controle != nullptr && std::holds_alternative<Place>(evenement.cible) && std::get<Place>(evenement.cible) == Place{2}) {
            vu = controle->etat == EtatInflige::Etourdissement && !controle->immunise;
        }
    }
    REQUIRE(vu);
}

TEST_CASE("controle partie : un skill de zone qui ralentit et entrave pose deux effets de rangs differents sur chaque monstre touche") {
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}, {250.0f, 185.0f}}), {guerrierDeTest(), pretreDeTest(), mageQuiRalentit()},
                                 s_graineDeTest, monstreIndestructible());

    partie.lancerSkillDeZone(s_adresseExplosion, {242.0f, 177.0f});
    const std::vector<ControleSubi> controles = controlesSurLesMonstresPendant(partie, 1.0f);  // l'activation dure 0,8 s

    REQUIRE(controles.size() == 4);  // « Slowed » et « Hampered » sur chacun des deux monstres
    for (const Monstre& monstre : partie.getMonstres()) {
        const EffetsActifs& effets = monstre.getEffets();
        REQUIRE(effets.nombreDeControles() == 2);
        REQUIRE(effets.liste()[0].rang == 1);
        REQUIRE(effets.liste()[1].rang == 2);
        REQUIRE(facteurDeDeplacement(effets) == Approx(0.7f));
        REQUIRE(facteurDAttaque(effets) == Approx(0.8f));
        REQUIRE(peutBouger(effets));
        REQUIRE(peutAttaquer(effets));
    }
}

TEST_CASE("controle partie : les deux controles d'un meme skill expirent ensemble apres leur duree") {
    Partie partie = partieDeTest(niveauDegage({{235.0f, 170.0f}}), {guerrierDeTest(), pretreDeTest(), mageQuiRalentit()}, s_graineDeTest,
                                 monstreIndestructible());
    partie.lancerSkillDeZone(s_adresseExplosion, {235.0f, 170.0f});
    avancer(partie, nombreDeTics(1.0f));
    REQUIRE(effetsDuMonstre(partie).nombreDeControles() == 2);

    avancer(partie, nombreDeTics(3.5f));

    REQUIRE(effetsDuMonstre(partie).nombreDeControles() == 0);
    REQUIRE(facteurDeDeplacement(effetsDuMonstre(partie)) == Approx(1.0f));
}
