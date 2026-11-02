// ./tests/escouade/test_Personnage_controle.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Cible.hpp"
#include "combat/Controle.hpp"
#include "escouade/Escouade.hpp"
#include "escouade/Personnage.hpp"

using Catch::Approx;

// Le contrôle subi par un personnage (étape 113.3) : étourdi il ne fait rien, ralenti il avance moins vite, entravé son attaque de base est
// plus lente, en Disruption il ne lance plus de skill mais frappe encore. Guerrier de test (frappe puissante, provocation) en (100, 100) ;
// aucune résistance n'est supposée juste : on ne vérifie que des ordres de grandeur sauf là où la durée ne compte pas.

namespace {
Personnage unGuerrier() {
    return unPersonnage({100.0f, 100.0f}, guerrierDeTest());
}

void subirLeControle(Personnage& personnage, const EffetActif& controle) {
    Generateur generateur = generateurDeTest();
    static_cast<void>(personnage.recevoirDegats(attaqueQuiControle(controle), generateur));
}

// Le personnage essaie de rejoindre cette cible pendant `duree` secondes.
void marcherVers(Personnage& personnage, sf::Vector2f cible, float duree) {
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        personnage.mettreAJour(s_dureeTic, cible, carteOuverte(), {}, {});
    }
}

float distanceParcourue(Personnage& personnage, float duree) {
    const sf::Vector2f depart = personnage.getCentre();
    marcherVers(personnage, {1000.0f, 100.0f}, duree);
    return (personnage.getCentre() - depart).length();
}

const Cible s_unMonstre = Cible(CibleMonstre{IdMonstre{1}, Cercle{{110.0f, 100.0f}, 15.0f}});

int ticsAvantLeDepartDeLAttaque(Personnage& personnage) {
    Generateur generateur = generateurDeTest();
    for (int tic = 1; tic <= nombreDeTics(5.0f); ++tic) {
        if (personnage.avancerAttaque(s_dureeTic, generateur).has_value()) {
            return tic;
        }
    }
    return -1;
}
}  // namespace

TEST_CASE("controle personnage : etourdi, il ne bouge pas, puis il repart") {
    Personnage personnage = unGuerrier();
    subirLeControle(personnage, unControle(EtatInflige::Etourdissement, 1.0f));

    marcherVers(personnage, {1000.0f, 100.0f}, 0.8f);
    REQUIRE(personnage.getCentre() == sf::Vector2f(100.0f, 100.0f));
    REQUIRE_FALSE(personnage.estEnMouvement());

    marcherVers(personnage, {1000.0f, 100.0f}, 0.6f);  // l'étourdissement (au plus 1,1 s) a pris fin
    REQUIRE(personnage.getCentre().x > 100.0f);
}

TEST_CASE("controle personnage : ralenti, il avance proportionnellement moins vite") {
    Personnage libre = unGuerrier();
    Personnage ralenti = unGuerrier();
    subirLeControle(ralenti, unControle(EtatInflige::Ralentissement, 10.0f, 50));

    const float distanceLibre = distanceParcourue(libre, 0.5f);
    const float distanceRalentie = distanceParcourue(ralenti, 0.5f);

    REQUIRE(distanceLibre > 0.0f);
    REQUIRE(distanceRalentie > distanceLibre * 0.4f);
    REQUIRE(distanceRalentie < distanceLibre * 0.6f);
}

TEST_CASE("controle personnage : l'entrave ne ralentit pas son deplacement") {
    Personnage libre = unGuerrier();
    Personnage entrave = unGuerrier();
    subirLeControle(entrave, unControle(EtatInflige::Entrave, 10.0f, 50));

    REQUIRE(distanceParcourue(entrave, 0.5f) == distanceParcourue(libre, 0.5f));
}

TEST_CASE("controle personnage : etourdi, il ne peut ni attaquer ni lancer de skill") {
    Personnage personnage = unGuerrier();
    subirLeControle(personnage, unControle(EtatInflige::Etourdissement, 1.0f));

    REQUIRE_FALSE(personnage.peutAttaquer());
    REQUIRE_FALSE(personnage.peutUtiliserUnSkill());
    REQUIRE_FALSE(personnage.engagerAttaque(IdMonstre{1}));
    REQUIRE_FALSE(personnage.skillEstPret(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.peutEngagerSkill(IdSkill::FrappePuissante));
    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::FrappePuissante, s_unMonstre));

    marcherVers(personnage, personnage.getCentre(), 1.5f);  // fini
    REQUIRE(personnage.engagerAttaque(IdMonstre{1}));
}

TEST_CASE("controle personnage : l'etourdissement annule le coup engage, sans recharge consommee") {
    Personnage personnage = unGuerrier();
    REQUIRE(personnage.engagerAttaque(IdMonstre{1}));
    REQUIRE(personnage.estEnExecution());

    subirLeControle(personnage, unControle(EtatInflige::Etourdissement, 0.5f));

    REQUIRE_FALSE(personnage.estEnExecution());
    REQUIRE_FALSE(personnage.cibleEngagee().has_value());
    marcherVers(personnage, personnage.getCentre(), 0.7f);
    REQUIRE(personnage.engagerAttaque(IdMonstre{1}));  // prête tout de suite : aucune recharge n'a été tirée
}

TEST_CASE("controle personnage : l'etourdissement annule le skill en activation, sans recharge ni cout") {
    Personnage personnage = unGuerrier();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_unMonstre));
    REQUIRE(personnage.skillEngage().has_value());

    subirLeControle(personnage, unControle(EtatInflige::Etourdissement, 0.5f));

    REQUIRE_FALSE(personnage.skillEngage().has_value());
    REQUIRE(personnage.skillConnu(IdSkill::FrappePuissante)->getPhase() == PhaseSkill::Pret);
    marcherVers(personnage, personnage.getCentre(), 0.7f);
    REQUIRE(personnage.skillEstPret(IdSkill::FrappePuissante));
}

TEST_CASE("controle personnage : la Disruption annule le skill en activation et interdit les skills, pas l'attaque de base") {
    Personnage personnage = unGuerrier();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_unMonstre));

    subirLeControle(personnage, unControle(EtatInflige::Disruption, 2.0f));

    REQUIRE_FALSE(personnage.skillEngage().has_value());
    REQUIRE_FALSE(personnage.peutUtiliserUnSkill());
    REQUIRE_FALSE(personnage.engagerSkill(IdSkill::Provocation, std::nullopt));
    REQUIRE(personnage.peutAttaquer());
    REQUIRE(personnage.engagerAttaque(IdMonstre{1}));
}

TEST_CASE("controle personnage : la Disruption n'annule pas le coup de base engage") {
    Personnage personnage = unGuerrier();
    REQUIRE(personnage.engagerAttaque(IdMonstre{1}));

    subirLeControle(personnage, unControle(EtatInflige::Disruption, 2.0f));

    REQUIRE(personnage.estEnExecution());
}

TEST_CASE("controle personnage : les recharges continuent sous l'etourdissement") {
    Personnage personnage = unGuerrier();
    REQUIRE(personnage.engagerSkill(IdSkill::FrappePuissante, s_unMonstre));
    Generateur generateur = generateurDeTest();
    for (int i = 0; i < nombreDeTics(2.0f) && personnage.skillEngage().has_value(); ++i) {
        static_cast<void>(personnage.avancerSkill(s_dureeTic, generateur));
    }
    const float rechargeAuDepart = personnage.skillConnu(IdSkill::FrappePuissante)->getRechargeRestante();
    REQUIRE(rechargeAuDepart > 0.0f);

    subirLeControle(personnage, unControle(EtatInflige::Etourdissement, 5.0f));
    marcherVers(personnage, personnage.getCentre(), 1.0f);

    REQUIRE(personnage.skillConnu(IdSkill::FrappePuissante)->getRechargeRestante() < rechargeAuDepart - 0.9f);
}

TEST_CASE("controle personnage : entrave, le coup de base engage part plus tard") {
    Personnage libre = unGuerrier();
    Personnage entrave = unGuerrier();
    subirLeControle(entrave, unControle(EtatInflige::Entrave, 20.0f, 50));
    REQUIRE(libre.engagerAttaque(IdMonstre{1}));
    REQUIRE(entrave.engagerAttaque(IdMonstre{1}));

    const int ticsLibre = ticsAvantLeDepartDeLAttaque(libre);
    const int ticsEntrave = ticsAvantLeDepartDeLAttaque(entrave);

    REQUIRE(ticsLibre > 0);
    // La moitié de la vitesse : environ le double du temps (le dernier tic compte entier, d'où la marge).
    REQUIRE(ticsEntrave >= 2 * ticsLibre - 3);
    REQUIRE(ticsEntrave <= 2 * ticsLibre + 1);
}

TEST_CASE("controle personnage : entrave, la recharge de l'attaque de base est plus lente") {
    Personnage libre = unGuerrier();
    Personnage entrave = unGuerrier();
    REQUIRE(libre.engagerAttaque(IdMonstre{1}));
    REQUIRE(entrave.engagerAttaque(IdMonstre{1}));
    static_cast<void>(ticsAvantLeDepartDeLAttaque(libre));
    static_cast<void>(ticsAvantLeDepartDeLAttaque(entrave));
    subirLeControle(entrave, unControle(EtatInflige::Entrave, 30.0f, 80));  // après le coup : seule la recharge en pâtit

    // Le libre a fini sa recharge (au plus s_rechargeMeleeMaxDeTest), l'entravé n'en a écoulé que le cinquième.
    const float attente = s_rechargeMeleeMaxDeTest + 0.2f;
    marcherVers(libre, libre.getCentre(), attente);
    marcherVers(entrave, entrave.getCentre(), attente);

    REQUIRE(libre.engagerAttaque(IdMonstre{1}));
    REQUIRE_FALSE(entrave.engagerAttaque(IdMonstre{1}));
}

TEST_CASE("controle personnage : un controle pose est signale une fois") {
    Personnage personnage = unGuerrier();

    subirLeControle(personnage, unControle(EtatInflige::Etourdissement, 1.0f));

    const std::vector<ControleApplique> poses = personnage.prendreLesControles();
    REQUIRE(poses.size() == 1);
    REQUIRE(poses[0].etat == EtatInflige::Etourdissement);
    REQUIRE_FALSE(poses[0].immunise);
    REQUIRE(personnage.prendreLesControles().empty());
}

TEST_CASE("controle escouade : un skill en attente est abandonne quand son lanceur est etourdi") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{IdMonstre{4}, cibleMonstre({85.0f, 60.0f})}};
    static_cast<void>(escouade.declencherSkill(s_adresseProvocation, monstres, carteOuverte(), generateur));  // le guerrier active
    static_cast<void>(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur));      // occupé : la frappe attend

    Generateur autre = generateurDeTest();
    static_cast<void>(escouade.recevoirCoup(IdMonstre{4}, Place{1}, attaqueQuiControle(unControle(EtatInflige::Etourdissement, 1.0f)), autre));
    int skillsPartis = 0;
    for (int i = 0; i < nombreDeTics(4.0f); ++i) {  // bien plus longtemps que l'étourdissement
        for (const ResultatDEffet& resultat : escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur)) {
            skillsPartis += std::holds_alternative<SkillParti>(resultat) ? 1 : 0;
        }
        escouade.mettreAJour(s_dureeTic, carteOuverte(), {});
    }

    REQUIRE(skillsPartis == 0);
}

TEST_CASE("controle escouade : un skill demande par un lanceur etourdi est refuse") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    Generateur autre = generateurDeTest();
    static_cast<void>(escouade.recevoirCoup(IdMonstre{4}, Place{1}, attaqueQuiControle(unControle(EtatInflige::Etourdissement, 2.0f)), autre));

    const std::vector<ResultatDEffet> resultats = escouade.declencherSkill(s_adresseProvocation, {}, carteOuverte(), generateur);

    REQUIRE(resultats.empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));
}

TEST_CASE("controle personnage : la Constitution raccourcit un etourdissement (0,3 % par point au-dessus de 10), la Resolve une Disruption") {
    Personnage robuste = unPersonnage({100.0f, 100.0f}, avecCaracteristique(guerrierDeTest(), Caracteristique::Endurance, 110));
    Personnage volontaire = unPersonnage({100.0f, 100.0f}, avecCaracteristique(guerrierDeTest(), Caracteristique::Volonte, 110));
    Personnage ordinaire = unPersonnage({100.0f, 100.0f}, avecCaracteristique(guerrierDeTest(), Caracteristique::Endurance, 10));

    subirLeControle(robuste, unControle(EtatInflige::Etourdissement, 1.0f));
    subirLeControle(ordinaire, unControle(EtatInflige::Etourdissement, 1.0f));
    subirLeControle(volontaire, unControle(EtatInflige::Etourdissement, 1.0f));

    const auto duree = [](const Personnage& personnage) { return personnage.getEffets().liste().front().dureeRestante; };
    REQUIRE(duree(ordinaire) == Approx(1.0f));
    REQUIRE(duree(robuste) == Approx(0.7f));      // Constitution 110 : 30 % de résistance
    REQUIRE(duree(volontaire) > 0.99f);           // la Resolve ne protège pas de l'étourdissement
    subirLeControle(volontaire, unControle(EtatInflige::Disruption, 1.0f, 0, IdSkill::Explosion));
    REQUIRE(volontaire.getEffets().liste().back().dureeRestante == Approx(0.7f));
}
