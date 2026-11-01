// ./tests/escouade/test_Escouade_energie.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Energie.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/ResultatDEffet.hpp"
#include "escouade/Escouade.hpp"
#include "monde/TileMap.hpp"

// Le coût en énergie des skills (étape 108.3). Les personnages de ce fichier ont des skills qui coûtent assez pour qu'on voie la
// réserve de 100 se vider : 60 pour les skills du guerrier et du prêtre (un seul à la fois), 30 pour l'explosion du mage.
// Placement comme dans test_Escouade_skills_degats.cpp : centre (50, 50), guerrier (place 1) en (85, 60), un monstre en (85, 60) est
// à portée de sa frappe.

namespace {
Escouade uneEscouadeQuiPaie(sf::Vector2f centre, int coutDeLExplosion = s_coutDeLExplosionDeTest) {
    return uneEscouadeDe(centre, {guerrierQuiPaie(), pretreQuiPaie(), mageQuiPaie(coutDeLExplosion)});
}

const std::vector<MonstreVu> s_unMonstreAPortee = {MonstreVu{IdMonstre{1}, cibleMonstre({85.0f, 60.0f})}};

int energieDe(const Escouade& escouade, Place place) {
    return escouade.energieDuPersonnage(place).value_or(-1);
}

std::size_t nombreDeRefus(const std::vector<ResultatDEffet>& resultats) {
    return static_cast<std::size_t>(std::ranges::count_if(
        resultats, [](const ResultatDEffet& resultat) { return std::holds_alternative<SkillSansEnergie>(resultat); }));
}

// Fait couler `secondes` secondes d'escouade seule : les skills, les recharges et l'énergie avancent.
void faireCouler(Escouade& escouade, const TileMap& carte, Generateur& generateur, float secondes) {
    for (int i = 0; i < nombreDeTics(secondes); ++i) {
        static_cast<void>(escouade.attaquer(s_dureeTic, {}, carte, generateur));
        escouade.mettreAJour(s_dureeTic, carte, {});
    }
}
}  // namespace

TEST_CASE("energie : les personnages commencent pleins") {
    const Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});

    REQUIRE(energieDe(escouade, Place{1}) == s_energieDeBase);
    REQUIRE(energieDe(escouade, Place{3}) == s_energieDeBase);
    REQUIRE(escouade.energieMaxDuPersonnage(Place{2}) == s_energieDeBase);
    REQUIRE_FALSE(escouade.energieDuPersonnage(Place{0}).has_value());
    REQUIRE_FALSE(escouade.energieDuPersonnage(Place{4}).has_value());
}

TEST_CASE("energie : le cout est paye au depart de l'effet, pas a l'engagement") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, s_unMonstreAPortee, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);
    REQUIRE(energieDe(escouade, Place{1}) == s_energieDeBase);  // en activation : rien n'est encore payé

    const std::vector<ResultatDEffet> resultats = attendreLeDepartDesSkillsTout(escouade, s_unMonstreAPortee, carteOuverte(), generateur);

    REQUIRE_FALSE(resultats.empty());
    REQUIRE(energieDe(escouade, Place{1}) < s_energieDeBase);
    REQUIRE(energieDe(escouade, Place{1}) >= s_energieDeBase - s_coutDeTest);  // payé, puis un peu de régénération pendant l'attente
}

TEST_CASE("energie : un skill pret mais trop cher est refuse avec son texte, sans attente ni recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});
    static_cast<void>(declencherEtAttendreTout(escouade, s_adresseFrappe, s_unMonstreAPortee, carteOuverte(), generateur));
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
    REQUIRE_FALSE(escouade.energieSuffit(s_adresseProvocation));  // 100 − 60 = 40 < 60

    const std::vector<ResultatDEffet> resultats = escouade.declencherSkill(s_adresseProvocation, s_unMonstreAPortee, carteOuverte(), generateur);

    REQUIRE(resultats.size() == 1);
    REQUIRE(nombreDeRefus(resultats) == 1);
    const SkillSansEnergie& refus = std::get<SkillSansEnergie>(resultats[0]);
    REQUIRE(refus.lanceur == Place{1});
    REQUIRE(refus.position == escouade.centreDuPersonnage(Place{1}));
    // Rien n'a eu lieu : le skill reste prêt, rien n'est en attente, rien n'est payé.
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseProvocation)->enActivation);
    const int energie = energieDe(escouade, Place{1});
    REQUIRE(attendreLeDepartDesSkillsTout(escouade, s_unMonstreAPortee, carteOuverte(), generateur).empty());
    REQUIRE(energieDe(escouade, Place{1}) >= energie);
}

TEST_CASE("energie : le skill refuse passe une fois l'energie remontee") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});
    static_cast<void>(declencherEtAttendreTout(escouade, s_adresseFrappe, s_unMonstreAPortee, carteOuverte(), generateur));
    const TileMap carte = carteOuverte();

    // 40 points restent, il en faut 60 : 10 s de régénération à 2 par seconde en rendent 19 ou 20.
    faireCouler(escouade, carte, generateur, 10.0f);
    REQUIRE(escouade.energieSuffit(s_adresseProvocation));
    const std::vector<ResultatDEffet> resultats = declencherEtAttendreTout(escouade, s_adresseProvocation, {}, carte, generateur);

    REQUIRE(nombreDeRefus(resultats) == 0);
    REQUIRE_FALSE(resultats.empty());  // le skill est parti
}

TEST_CASE("energie : l'energie regenere au debit de base, en combat comme ailleurs") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});
    static_cast<void>(declencherEtAttendreTout(escouade, s_adresseFrappe, s_unMonstreAPortee, carteOuverte(), generateur));
    const int apresLePaiement = energieDe(escouade, Place{1});

    faireCouler(escouade, carteOuverte(), generateur, 5.0f);

    const int gagne = energieDe(escouade, Place{1}) - apresLePaiement;
    REQUIRE(gagne >= 9);
    REQUIRE(gagne <= 10);  // 2 points par seconde pendant 5 s
}

TEST_CASE("energie : un skill annule ne coute rien") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, s_unMonstreAPortee, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);

    // La cible disparaît pendant l'activation : le skill est annulé.
    REQUIRE(attendreLeDepartDesSkillsTout(escouade, {}, carteOuverte(), generateur).empty());

    REQUIRE(energieDe(escouade, Place{1}) == s_energieDeBase);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));  // et aucune recharge consommée
}

TEST_CASE("energie : un skill de zone paie son cout meme si la zone est vide") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouadeQuiPaie({200.0f, 200.0f});

    const std::vector<ResultatDEffet> resultats = declencherZoneEtAttendreTout(escouade, s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur);

    REQUIRE_FALSE(resultats.empty());  // le skill est parti, sans monstre à toucher
    REQUIRE(energieDe(escouade, Place{3}) <= s_energieDeBase - s_coutDeLExplosionDeTest + 2);  // payé, moins un peu de régénération
    REQUIRE(energieDe(escouade, Place{3}) >= s_energieDeBase - s_coutDeLExplosionDeTest);
}

TEST_CASE("energie : un skill de zone trop cher est refuse au clic, sans attente ni recharge") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouadeQuiPaie({200.0f, 200.0f}, s_energieDeBase + 1);

    const std::vector<ResultatDEffet> resultats = escouade.declencherSkillDeZone(s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur);

    REQUIRE(nombreDeRefus(resultats) == 1);
    REQUIRE(std::get<SkillSansEnergie>(resultats[0]).lanceur == Place{3});
    REQUIRE(escouade.skillEstPret(s_adresseExplosion));
    REQUIRE(energieDe(escouade, Place{3}) == s_energieDeBase);
    REQUIRE(attendreLeDepartDesSkillsTout(escouade, {}, carte, generateur).empty());
}

TEST_CASE("energie : un skill en attente dont le lanceur n'a plus de quoi payer est abandonne, sans cout, avec son texte") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});
    const TileMap carte = carteOuverte();

    // La frappe s'active ; la provocation, demandée pendant ce temps (100 ≥ 60 : acceptée), attend que le guerrier soit libre.
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, s_unMonstreAPortee, carte, generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, s_unMonstreAPortee, carte, generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseProvocation)->enActivation);

    // La frappe part et paie 60 : il reste 40, la provocation en coûterait 60.
    std::vector<ResultatDEffet> resultats;
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        const std::vector<ResultatDEffet> image = escouade.attaquer(s_dureeTic, s_unMonstreAPortee, carte, generateur);
        resultats.insert(resultats.end(), image.begin(), image.end());
    }

    REQUIRE(nombreDeRefus(resultats) == 1);
    REQUIRE(energieDe(escouade, Place{1}) < s_energieDeBase - s_coutDeTest + 3);  // seule la frappe a payé
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));                         // la provocation n'a ni recharge ni activation
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseProvocation)->enActivation);
    // Plus rien n'est en attente : une nouvelle demande est jugée seule (refusée ici, faute d'énergie), pas ignorée par une attente.
    REQUIRE(nombreDeRefus(escouade.declencherSkill(s_adresseProvocation, s_unMonstreAPortee, carte, generateur)) == 1);
}

TEST_CASE("energie : un skill en attente dont le lanceur a assez d'energie part normalement") {
    Generateur generateur = generateurDeTest();
    // La frappe est gratuite : une fois partie, il reste de quoi payer la provocation qui l'attendait.
    DefinitionPersonnage guerrier = guerrierQuiPaie();
    guerrier.skills.clear();
    guerrier.skills.push_back({IdSkill::FrappePuissante, skillExigeantDesArmes({})});  // gratuit
    guerrier.skills.push_back({IdSkill::Provocation, Skill({.mode = ModeDeCible::Soi,
                                                            .portee = 0.0f,
                                                            .rechargeMin = s_rechargeProvocationDeTest,
                                                            .rechargeMax = s_rechargeProvocationDeTest,
                                                            .effet = EffetProvocation{.duree = s_dureeProvocationDeTest},
                                                            .rayonZone = s_rayonProvocationDeTest,
                                                            .tempsExecution = s_activationProvocationDeTest,
                                                            .cout = s_coutDeTest})});
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {guerrier, pretreQuiPaie(), mageQuiPaie(s_coutDeLExplosionDeTest)});
    const TileMap carte = carteOuverte();
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, s_unMonstreAPortee, carte, generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, s_unMonstreAPortee, carte, generateur).empty());  // mise en attente

    std::vector<ResultatDEffet> resultats;
    for (int i = 0; i < nombreDeTics(1.5f); ++i) {
        const std::vector<ResultatDEffet> image = escouade.attaquer(s_dureeTic, s_unMonstreAPortee, carte, generateur);
        resultats.insert(resultats.end(), image.begin(), image.end());
    }

    REQUIRE(nombreDeRefus(resultats) == 0);
    REQUIRE(energieDe(escouade, Place{1}) <= s_energieDeBase - s_coutDeTest + 4);  // la provocation a payé
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));                   // elle est partie : en recharge
}

TEST_CASE("energie : un soin sans blesse ne coute rien") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeQuiPaie({50.0f, 50.0f});

    REQUIRE(declencherEtAttendreTout(escouade, s_adresseSoin, {}, carteOuverte(), generateur).empty());  // personne à soigner

    REQUIRE(energieDe(escouade, Place{2}) == s_energieDeBase);
    REQUIRE(escouade.skillEstPret(s_adresseSoin));
}
