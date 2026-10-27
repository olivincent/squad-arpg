// ./tests/jeu/test_Partie_skills.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <utility>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Attaque.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/ResolutionDuCoup.hpp"
#include "monstres/Monstre.hpp"
#include "jeu/Partie.hpp"

TEST_CASE("un skill demande part dans mettreAJour et sa recharge demarre") {
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseFrappe));

    partie.demanderSkill(s_adresseFrappe);
    REQUIRE(partie.getEscouade().skillEstPret(s_adresseFrappe)); // rien ne part avant mettreAJour
    partie.mettreAJour(s_dureeTic);

    REQUIRE_FALSE(partie.getEscouade().skillEstPret(s_adresseFrappe));
}

TEST_CASE("un skill et attaquer dans la meme image ne frappent pas deux fois le meme lanceur") {
    // Monstre à portée du guerrier (frappe puissante) et des deux
    // tireurs (attaques de base). Le guerrier qui lance son skill ne
    // frappe pas avec son attaque de base pendant l'activation ni juste après.
    Partie partie = unePartie(niveauDegage({{235.0f, 170.0f}}), s_graineDeTest);
    const int vieDeDepart = partie.getMonstres().front().getPointsDeVie();
    const Defenses monstre{.armure = 0, .statistiques = monstreSansHasard().statistiques};
    const Attaque frappe{.degats = degatsPhysiques(s_degatsFrappeDeTest),
                         .capaciteOffensive = partie.getEscouade().statistiquesDeCombatDuPersonnage(Place{1})->capaciteOffensive,
                         .nature = NatureDAttaque::Contact};
    // Les attaques de base des deux tireurs, par les deux bornes de la plage de leur arme (le sceptre et le bâton, de Puissance de
    // référence) : un coup du tireur est entre les deux.
    const auto bornes = [&](Place place, IdObjet arme) {
        const DescriptionDAttaque description = descriptionDeLArme(Objet{arme});
        Attaque basse = *attaqueDeBaseDe(partie.getEscouade(), place);
        Attaque haute = basse;
        basse.degats = description.degats.min;
        haute.degats = description.degats.max;
        return std::pair{basse, haute};
    };
    const auto [bassePretre, hautePretre] = bornes(Place{2}, IdObjet::Sceptre);
    const auto [basseMage, hauteMage] = bornes(Place{3}, IdObjet::Baton);

    // 0,35 s : le skill est parti (activation de 0,3 s) et les tireurs ont eu
    // le temps d'exécuter leur coup (0,15 s), pas le mêlée, dont la recharge
    // de l'attaque de base a été relancée au complet (au moins 0,3 s) au départ.
    partie.demanderSkill(s_adresseFrappe);
    std::vector<EvenementDeCombat> evenements;
    for (int i = 0; i < nombreDeTics(0.35f); ++i) {
        partie.mettreAJour(s_dureeTic);
        const std::vector<EvenementDeCombat> duTic = partie.prendreEvenements();
        evenements.insert(evenements.end(), duTic.begin(), duTic.end());
    }

    // Le critique est au tirage de la partie : chaque événement dit s'il l'était. Un coup de la frappe, un de
    // chaque tireur, et aucun du guerrier avec son attaque de base (qui ferait un quatrième coup).
    int total = 0;
    int frappes = 0;
    int basesDesTireurs = 0;
    for (const EvenementDeCombat& evenement : evenements) {
        const DegatsSubis* coup = std::get_if<DegatsSubis>(&evenement.detail);
        if (coup == nullptr || !std::holds_alternative<IdMonstre>(evenement.cible)) {
            continue;
        }
        total += coup->montant;
        if (coup->montant == degatsAttendus(frappe, monstre, coup->critique)) {
            ++frappes;
        } else if ((coup->montant >= degatsAttendus(bassePretre, monstre, coup->critique) &&
                    coup->montant <= degatsAttendus(hautePretre, monstre, coup->critique)) ||
                   (coup->montant >= degatsAttendus(basseMage, monstre, coup->critique) &&
                    coup->montant <= degatsAttendus(hauteMage, monstre, coup->critique))) {
            ++basesDesTireurs;
        }
    }
    REQUIRE(frappes == 1);
    REQUIRE(basesDesTireurs == 2);
    REQUIRE(partie.getMonstres().front().getPointsDeVie() == vieDeDepart - total);
}
