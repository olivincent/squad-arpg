// ./tests/combat/test_Controle.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "combat/Controle.hpp"

using Catch::Approx;

namespace {
// Un contrôle posé par ce skill (rang 0) pour cette durée.
EffetActif controle(IdSkill source, EtatInflige etat, int pourcentage, float duree = 5.0f, int rang = 0) {
    return {.source = source, .rang = rang, .dureeRestante = duree, .sorte = Controle{.etat = etat, .pourcentage = pourcentage}};
}

EffetActif etourdissement(IdSkill source = IdSkill::FrappeSacree, float duree = 5.0f) {
    return controle(source, EtatInflige::Etourdissement, 0, duree);
}

EffetActif disruption(IdSkill source = IdSkill::Explosion, float duree = 5.0f) {
    return controle(source, EtatInflige::Disruption, 0, duree);
}
}  // namespace

TEST_CASE("Controle : sans effet, tout est permis et rien n'est ralenti") {
    const EffetsActifs effets;

    REQUIRE(peutBouger(effets));
    REQUIRE(peutAttaquer(effets));
    REQUIRE(peutUtiliserUnSkill(effets));
    REQUIRE(facteurDeDeplacement(effets) == Approx(1.0f));
    REQUIRE(facteurDAttaque(effets) == Approx(1.0f));
}

TEST_CASE("Controle : l'etourdissement interdit de bouger, d'attaquer et d'utiliser un skill") {
    EffetsActifs effets;
    effets.poser(etourdissement());

    REQUIRE(estSousEtat(effets, EtatInflige::Etourdissement));
    REQUIRE_FALSE(peutBouger(effets));
    REQUIRE_FALSE(peutAttaquer(effets));
    REQUIRE_FALSE(peutUtiliserUnSkill(effets));
}

TEST_CASE("Controle : la Disruption interdit les skills et laisse l'attaque de base et le deplacement") {
    EffetsActifs effets;
    effets.poser(disruption());

    REQUIRE(estSousEtat(effets, EtatInflige::Disruption));
    REQUIRE_FALSE(peutUtiliserUnSkill(effets));
    REQUIRE(peutAttaquer(effets));
    REQUIRE(peutBouger(effets));
}

TEST_CASE("Controle : le ralentissement multiplie la vitesse de deplacement et seulement elle") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Ralentissement, 30));

    REQUIRE(facteurDeDeplacement(effets) == Approx(0.7f));
    REQUIRE(facteurDAttaque(effets) == Approx(1.0f));
    REQUIRE(peutBouger(effets));
    REQUIRE(peutAttaquer(effets));
    REQUIRE(peutUtiliserUnSkill(effets));
}

TEST_CASE("Controle : l'entrave multiplie la vitesse d'attaque et seulement elle") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Entrave, 20));

    REQUIRE(facteurDAttaque(effets) == Approx(0.8f));
    REQUIRE(facteurDeDeplacement(effets) == Approx(1.0f));
    REQUIRE(peutAttaquer(effets));
    REQUIRE(peutUtiliserUnSkill(effets));
}

TEST_CASE("Controle : deux etourdissements de skills differents durent tant qu'il en reste un") {
    EffetsActifs effets;
    effets.poser(etourdissement(IdSkill::FrappeSacree, 1.0f));
    effets.poser(etourdissement(IdSkill::FrappePuissante, 3.0f));

    static_cast<void>(effets.mettreAJour(2.0f));  // le premier a expiré, le second reste

    REQUIRE(effets.liste().size() == 1);
    REQUIRE_FALSE(peutBouger(effets));
    static_cast<void>(effets.mettreAJour(1.5f));
    REQUIRE(peutBouger(effets));
}

TEST_CASE("Controle : deux disruptions de skills differents durent tant qu'il en reste une") {
    EffetsActifs effets;
    effets.poser(disruption(IdSkill::Explosion, 1.0f));
    effets.poser(disruption(IdSkill::Contresort, 3.0f));

    static_cast<void>(effets.mettreAJour(2.0f));

    REQUIRE_FALSE(peutUtiliserUnSkill(effets));
    static_cast<void>(effets.mettreAJour(1.5f));
    REQUIRE(peutUtiliserUnSkill(effets));
}

TEST_CASE("Controle : deux ralentissements gardent le plus fort, pas la somme") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Ralentissement, 30));
    effets.poser(controle(IdSkill::TirPercant, EtatInflige::Ralentissement, 50));

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(facteurDeDeplacement(effets) == Approx(0.5f));
}

TEST_CASE("Controle : deux entraves gardent la plus forte, pas la somme") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Entrave, 20));
    effets.poser(controle(IdSkill::TirPercant, EtatInflige::Entrave, 15));

    REQUIRE(facteurDAttaque(effets) == Approx(0.8f));
}

TEST_CASE("Controle : le ralentissement et l'entrave ne se confondent pas") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Ralentissement, 30, 5.0f, 0));
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Entrave, 20, 5.0f, 1));  // même skill, rang différent

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(facteurDeDeplacement(effets) == Approx(0.7f));
    REQUIRE(facteurDAttaque(effets) == Approx(0.8f));
}

TEST_CASE("Controle : le meme skill repose rafraichit la duree et garde le pourcentage le plus fort") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Ralentissement, 30, 2.0f));
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Ralentissement, 20, 3.0f));

    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste().front().dureeRestante == Approx(3.0f));
    REQUIRE(facteurDeDeplacement(effets) == Approx(0.7f));
}

TEST_CASE("Controle : un controle est un malus") {
    REQUIRE_FALSE(estUnBonus(etourdissement()));
}

TEST_CASE("Controle : un ralentissement total arrete sans devenir negatif") {
    EffetsActifs effets;
    effets.poser(controle(IdSkill::PluieDeFleches, EtatInflige::Ralentissement, 100));

    REQUIRE(facteurDeDeplacement(effets) == Approx(0.0f));
}

TEST_CASE("etatQuiResiste : l'entrave suit la resistance au ralentissement, les autres la leur") {
    REQUIRE(etatQuiResiste(EtatInflige::Etourdissement) == EtatDeControle::Etourdissement);
    REQUIRE(etatQuiResiste(EtatInflige::Ralentissement) == EtatDeControle::Ralentissement);
    REQUIRE(etatQuiResiste(EtatInflige::Entrave) == EtatDeControle::Ralentissement);
    REQUIRE(etatQuiResiste(EtatInflige::Disruption) == EtatDeControle::Disruption);
}

TEST_CASE("resistanceAppliquee : lit la resistance de l'etat qui resiste, bornee") {
    StatistiquesDeCombat statistiques;
    statistiques.resistancesAuControle[EtatDeControle::Ralentissement] = 12.0f;
    statistiques.resistancesAuControle[EtatDeControle::Etourdissement] = 250.0f;  // hors bornes
    statistiques.resistancesAuControle[EtatDeControle::Disruption] = -400.0f;

    REQUIRE(resistanceAppliquee(statistiques, EtatInflige::Ralentissement) == Approx(12.0f));
    REQUIRE(resistanceAppliquee(statistiques, EtatInflige::Entrave) == Approx(12.0f));
    REQUIRE(resistanceAppliquee(statistiques, EtatInflige::Etourdissement) == Approx(80.0f));
    REQUIRE(resistanceAppliquee(statistiques, EtatInflige::Disruption) == Approx(-100.0f));
}

TEST_CASE("dureePosee : duree x (1 - resistance) x rendement") {
    REQUIRE(dureePosee(2.0f, 0.0f, 1.0f) == Approx(2.0f));
    REQUIRE(dureePosee(2.0f, 25.0f, 1.0f) == Approx(1.5f));
    REQUIRE(dureePosee(2.0f, 0.0f, 0.5f) == Approx(1.0f));
    REQUIRE(dureePosee(2.0f, 50.0f, 0.25f) == Approx(0.25f));
    REQUIRE(dureePosee(2.0f, -100.0f, 1.0f) == Approx(4.0f));  // vulnérabilité maximale : le double
    REQUIRE(dureePosee(2.0f, 10.0f, 0.0f) == Approx(0.0f));    // immunité
}
