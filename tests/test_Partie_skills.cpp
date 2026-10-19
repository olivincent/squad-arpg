// ./tests/test_Partie_skills.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "Monstre.hpp"
#include "Partie.hpp"

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

    // 0,35 s : le skill est parti (activation de 0,3 s) et les tireurs ont eu
    // le temps d'exécuter leur coup (0,15 s), pas le mêlée, dont la recharge
    // de l'attaque de base a été relancée au complet (au moins 0,3 s) au départ.
    partie.demanderSkill(s_adresseFrappe);
    avancer(partie, nombreDeTics(0.35f));

    REQUIRE(partie.getMonstres().front().getPointsDeVie() == vieDeDepart - s_degatsFrappeDeTest - 2 * s_degatsBaseDeTest);
}
