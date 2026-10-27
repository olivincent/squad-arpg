// ./tests/escouade/test_Escouade_skills.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "escouade/Specialite.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"
#include "base/Cercle.hpp"
#include "combat/Coup.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EtatSkill.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/Skill.hpp"
#include "monde/TileMap.hpp"
#include "combat/TypeSkill.hpp"
#include "combat/ModeDeCible.hpp"

TEST_CASE("etatDuSkill est vide pour une place invalide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE_FALSE(escouade.etatDuSkill(AdresseSkill{Place{0}, IdSkill::FrappePuissante}).has_value());
    REQUIRE_FALSE(escouade.etatDuSkill(AdresseSkill{Place{5}, IdSkill::FrappePuissante}).has_value());
}

TEST_CASE("etatDuSkill donne le type du skill de chaque place") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->type == TypeSkill::DegatsSurCible);
    REQUIRE(escouade.etatDuSkill(s_adresseProvocation)->type == TypeSkill::Provocation);
    REQUIRE(escouade.etatDuSkill(s_adresseSoin)->type == TypeSkill::Soin);
    REQUIRE(escouade.etatDuSkill(s_adresseExplosion)->type == TypeSkill::Zone);
    REQUIRE(escouade.etatDuSkill(s_adresseExplosion)->id == IdSkill::Explosion);
}

TEST_CASE("etatDuSkill d'un skill pret : progression 1, temps restant 0") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    const std::optional<EtatSkill> etat = escouade.etatDuSkill(s_adresseProvocation);

    REQUIRE(etat.has_value());
    REQUIRE(etat->progression == 1.0f);
    REQUIRE(etat->tempsRestant == 0.0f);
    REQUIRE(etat->lanceurVivant);
}

TEST_CASE("etatDuSkill juste apres un declenchement : recharge complete") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, {}, carteOuverte(), generateur).empty()); // la provocation part même sans monstre
    const std::optional<EtatSkill> etat = escouade.etatDuSkill(s_adresseProvocation);

    REQUIRE(etat.has_value());
    REQUIRE(etat->progression == 0.0f);
    REQUIRE(etat->tempsRestant > 0.0f);
}

TEST_CASE("etatDuSkill d'un lanceur mort : lanceurVivant est faux") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    for (int i = 0; i < 3; ++i) {
        infliger(escouade, IdMonstre{1}, {1000.0f, 1000.0f}, s_coupMortel);
    }

    const std::optional<EtatSkill> etat = escouade.etatDuSkill(s_adresseProvocation);

    REQUIRE(etat.has_value());
    REQUIRE_FALSE(etat->lanceurVivant);
}

// Temps d'activation des skills (étape 58) : dégâts 0,3 s, provocation 0,2 s,
// soin 0,6 s, zone 0,8 s. Escouade en (50, 50) : places en (85, 60),
// (35, 35) et (35, 85).

namespace {

// Appelle attaquer() par tics pendant cette durée (sans mettreAJour : les
// recharges ne courent pas) et renvoie tous les coups partis.
std::vector<Coup> attaquerPendant(Escouade& escouade, const std::vector<MonstreVu>& monstres, float duree, Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        const std::vector<Coup> coupsDuTic = escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur);
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
    }
    return coups;
}

// Nombre de coups d'exactement ce montant (écarte les attaques de base).
int compter(const std::vector<Coup>& coups, int degats) {
    return static_cast<int>(std::count_if(coups.begin(), coups.end(), [&](const Coup& coup) { return coup.attaque.degats.total() == degats; }));
}

// Position du personnage de cette place, lue par la proie d'un monstre qu'il provoque.
sf::Vector2f positionDeLaPlace(Escouade& escouade, Place place) {
    escouade.provoquer(IdMonstre{99}, place, 1000.0f);
    return escouade.proieDuMonstre(IdMonstre{99}, {0.0f, 0.0f})->cercle.centre;
}

// Les skills partis (prendreFaits), dans l'ordre ; les autres faits sont écartés.
std::vector<Escouade::SkillPart> skillsPartis(Escouade& escouade) {
    std::vector<Escouade::SkillPart> partis;
    for (const Escouade::FaitDEscouade& fait : escouade.prendreFaits()) {
        if (const auto* part = std::get_if<Escouade::SkillPart>(&fait)) {
            partis.push_back(*part);
        }
    }
    return partis;
}

}  // namespace

TEST_CASE("Un skill de degats part a la fin de son activation de 0,3 s, pas avant") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));
    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);

    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.25f, generateur), s_degatsFrappeDeTest) == 0);
    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.1f, generateur), s_degatsFrappeDeTest) == 1);
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);
    REQUIRE(escouade.etatDuSkill(s_adresseFrappe)->progression < 1.0f);  // la recharge a démarré au départ
}

TEST_CASE("Un skill en activation refuse un nouvel appui sur la meme touche") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());

    REQUIRE(compter(attaquerPendant(escouade, monstres, 1.0f, generateur), s_degatsFrappeDeTest) == 1);
}

TEST_CASE("Le lanceur reste sur place pendant l'activation, un ordre de deplacement ne l'annule pas") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    subirCoup(escouade, IdMonstre{1}, Place{1}, 30);  // un blessé à soigner
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carte, generateur).empty());
    const sf::Vector2f avant = positionDeLaPlace(escouade, Place{2});

    escouade.allerVers({300.0f, 300.0f}, carte);
    simulerAvecAttaque(escouade, carte, 0.4f, generateur);

    verifierPointProche(positionDeLaPlace(escouade, Place{2}), avant);   // le soigneur s'est arrêté
    REQUIRE(escouade.getCentre().x > 70.0f);                      // le reste de l'escouade est parti
    REQUIRE(placeAlliePourSoin(escouade) == Place{1});                   // pas encore soigné

    simulerAvecAttaque(escouade, carte, 0.5f, generateur);
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());      // soigné malgré l'ordre
}

TEST_CASE("La mort du lanceur annule l'activation, sans recharge consommee") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    subirCoup(escouade, IdMonstre{1}, Place{1}, 30);
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carteOuverte(), generateur).empty());

    subirCoup(escouade, IdMonstre{1}, Place{2}, s_coupMortel);
    static_cast<void>(attaquerPendant(escouade, {}, 1.0f, generateur));

    REQUIRE(placeAlliePourSoin(escouade) == Place{1});  // personne n'a été soigné
    REQUIRE(escouade.etatDuSkill(s_adresseSoin)->progression == 1.0f);
}

TEST_CASE("La disparition du monstre vise annule l'activation d'un skill de degats, sans recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, unSeulMonstre(cibleMonstre({85.0f, 60.0f})), carteOuverte(), generateur).empty());

    const std::vector<Coup> coups = attaquerPendant(escouade, {}, 1.0f, generateur);  // le monstre n'est plus dans la liste

    REQUIRE(compter(coups, s_degatsFrappeDeTest) == 0);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));
}

TEST_CASE("La mort de l'allie vise annule l'activation d'un soin, sans recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    subirCoup(escouade, IdMonstre{1}, Place{1}, 30);
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carteOuverte(), generateur).empty());

    subirCoup(escouade, IdMonstre{1}, Place{1}, s_coupMortel);
    static_cast<void>(attaquerPendant(escouade, {}, 1.0f, generateur));

    REQUIRE(escouade.skillEstPret(s_adresseSoin));
}

TEST_CASE("Zone : l'effet est evalue au depart, un monstre sorti de la zone n'est pas touche, un entre l'est") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const Cercle dedans = cibleMonstre({150.0f, 100.0f});
    const Cercle dehors = cibleMonstre({300.0f, 100.0f});
    REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, {150.0f, 100.0f}, {MonstreVu{IdMonstre{1}, dedans}, MonstreVu{IdMonstre{2}, dehors}}, carteOuverte(), generateur).empty());

    // Le monstre 1 était dans la zone à l'appui, le 2 y entre pendant l'activation.
    std::vector<Coup> coups = attaquerPendant(escouade, {MonstreVu{IdMonstre{1}, dedans}, MonstreVu{IdMonstre{2}, dehors}}, 0.7f, generateur);
    const std::vector<Coup> suite = attaquerPendant(escouade, {MonstreVu{IdMonstre{1}, dehors}, MonstreVu{IdMonstre{2}, dedans}}, 0.2f, generateur);
    coups.insert(coups.end(), suite.begin(), suite.end());

    REQUIRE(compter(coups, s_degatsZoneDeTest) == 1);
    REQUIRE(std::any_of(coups.begin(), coups.end(), [](const Coup& coup) { return coup.attaque.degats.total() == s_degatsZoneDeTest && coup.idMonstre == IdMonstre{2}; }));
}

TEST_CASE("Provocation : l'effet est evalue au depart, apres 0,2 s") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const Cercle dedans = cibleMonstre({100.0f, 60.0f});
    const Cercle dehors = cibleMonstre({600.0f, 60.0f});
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, {MonstreVu{IdMonstre{1}, dedans}, MonstreVu{IdMonstre{2}, dehors}}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());

    static_cast<void>(attaquerPendant(escouade, {MonstreVu{IdMonstre{1}, dedans}, MonstreVu{IdMonstre{2}, dehors}}, 0.1f, generateur));
    REQUIRE(escouade.getProvocations().empty());
    static_cast<void>(attaquerPendant(escouade, {MonstreVu{IdMonstre{1}, dehors}, MonstreVu{IdMonstre{2}, dedans}}, 0.15f, generateur));

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == IdMonstre{2});
}

TEST_CASE("Deux lanceurs peuvent activer en meme temps") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    subirCoup(escouade, IdMonstre{1}, Place{1}, 30);
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseSoin, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseSoin));

    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.4f, generateur), s_degatsFrappeDeTest) == 1);  // les dégâts sont partis
    REQUIRE(placeAlliePourSoin(escouade) == Place{1});                                         // pas encore le soin
    static_cast<void>(attaquerPendant(escouade, monstres, 0.3f, generateur));
    REQUIRE_FALSE(placeAlliePourSoin(escouade).has_value());
}

// Adresse invalide : place valide, mais skill inconnu de ce personnage.

TEST_CASE("Une adresse dont le skill est inconnu du personnage est traitee comme une place invalide") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const AdresseSkill inconnue{Place{1}, IdSkill::Soin};  // le guerrier de la place 1 n'a pas de soin

    REQUIRE_FALSE(escouade.skillEstPret(inconnue));
    REQUIRE_FALSE(escouade.etatDuSkill(inconnue).has_value());
    REQUIRE_FALSE(escouade.modeDuSkill(inconnue).has_value());
    REQUIRE(escouade.rayonZoneDuSkill(inconnue) == 0.0f);
    REQUIRE(declencherEtAttendre(escouade, inconnue, {MonstreVu{IdMonstre{1}, cibleMonstre({110.0f, 35.0f})}}, carteOuverte(), generateur).empty());
    REQUIRE(declencherZoneEtAttendre(escouade, inconnue, {235.0f, 235.0f}, {}, carteOuverte(), generateur).empty());
}

TEST_CASE("modeDuSkill donne le mode de cible du skill a l'adresse, vide pour une place invalide") {
    Escouade escouade = uneEscouade({50.0f, 50.0f});

    REQUIRE(escouade.modeDuSkill(s_adresseFrappe) == ModeDeCible::Monstre);
    REQUIRE(escouade.modeDuSkill(s_adresseExplosion) == ModeDeCible::Point);
    REQUIRE(escouade.modeDuSkill(s_adresseProvocation) == ModeDeCible::Soi);
    REQUIRE(escouade.modeDuSkill(s_adresseSoin) == ModeDeCible::Allie);
    REQUIRE_FALSE(escouade.modeDuSkill(AdresseSkill{Place{5}, IdSkill::FrappePuissante}).has_value());
}

TEST_CASE("adressesDesSkills : par place, puis dans l'ordre de la liste du personnage") {
    const Escouade escouade = uneEscouade({200.0f, 200.0f});

    const std::vector<AdresseSkill> attendues = {s_adresseFrappe, s_adresseProvocation, s_adresseSoin, s_adresseExplosion};
    REQUIRE(escouade.adressesDesSkills() == attendues);
}

TEST_CASE("couleurDuPersonnage : la couleur de la composition, vide pour une place invalide") {
    const Escouade escouade = uneEscouade({200.0f, 200.0f});

    REQUIRE(escouade.couleurDuPersonnage(Place{1}) == guerrierDeTest().couleur);
    REQUIRE(escouade.couleurDuPersonnage(Place{3}) == mageDeTest().couleur);
    REQUIRE_FALSE(escouade.couleurDuPersonnage(Place{0}).has_value());
    REQUIRE_FALSE(escouade.couleurDuPersonnage(Place{4}).has_value());
}

// Un skill demandé pendant l'activation d'un autre skill du même lanceur (étape 80) : il
// est mis en attente (ou y reste) et part dès que le lanceur est libre, au lieu d'être perdu.
// Escouade en (50, 50) : guerrier en (85, 60).

namespace {

// Paladin et rôdeur de test : deux skills chacun, valeurs propres à ces tests (pas le catalogue).
constexpr int s_degatsFrappeSacreeDeTest = 20;
constexpr int s_puissanceImpositionDeTest = 25;
constexpr int s_degatsTirDeTest = 25;
constexpr int s_degatsPluieDeTest = 12;
constexpr AdresseSkill s_adresseFrappeSacree{Place{1}, IdSkill::FrappeSacree};
constexpr AdresseSkill s_adresseImposition{Place{1}, IdSkill::ImpositionDesMains};
constexpr AdresseSkill s_adresseTir{Place{1}, IdSkill::TirPercant};
constexpr AdresseSkill s_adressePluie{Place{1}, IdSkill::PluieDeFleches};

DefinitionPersonnage paladinDeTest() {
    DefinitionPersonnage paladin = guerrierDeTest();
    paladin.skills = {
        {IdSkill::FrappeSacree, Skill({.type = TypeSkill::DegatsSurCible, .mode = ModeDeCible::Monstre, .portee = s_porteeMeleeDeTest,
                                       .puissance = s_degatsFrappeSacreeDeTest, .rechargeMin = 5.0f,
                                       .rechargeMax = 5.0f, .tempsExecution = 0.3f, .nature = NatureDAttaque::Contact})},
        {IdSkill::ImpositionDesMains, Skill({.type = TypeSkill::Soin, .mode = ModeDeCible::Allie, .portee = s_porteeMeleeDeTest,
                                             .puissance = s_puissanceImpositionDeTest, .rechargeMin = 10.0f,
                                             .rechargeMax = 10.0f, .tempsExecution = 0.5f, .nature = NatureDAttaque::Contact})}};
    return paladin;
}

DefinitionPersonnage rodeurDeTest() {
    DefinitionPersonnage rodeur = mageDeTest();
    rodeur.skills = {
        {IdSkill::TirPercant, Skill({.type = TypeSkill::DegatsSurCible, .mode = ModeDeCible::Monstre, .portee = 200.0f, .puissance = s_degatsTirDeTest,
                                     .rechargeMin = 6.0f, .rechargeMax = 6.0f, .tempsExecution = 0.5f,
                                     .nature = NatureDAttaque::Projectile})},
        {IdSkill::PluieDeFleches, Skill({.type = TypeSkill::Zone, .mode = ModeDeCible::Point, .portee = 320.0f, .puissance = s_degatsPluieDeTest,
                                         .rechargeMin = 8.0f, .rechargeMax = 8.0f, .rayonZone = 45.0f,
                                         .tempsExecution = 0.8f, .nature = NatureDAttaque::Zone})}};
    return rodeur;
}

// Une image d'Escouade comme dans Partie, monstres immobiles : attaquer() puis mettreAJour().
// Renvoie tous les coups partis.
std::vector<Coup> combattrePendant(Escouade& escouade, const std::vector<MonstreVu>& monstres, float duree, Generateur& generateur) {
    std::vector<Coup> coups;
    for (int i = 0; i < nombreDeTics(duree); ++i) {
        const std::vector<Coup> coupsDuTic = escouade.attaquer(s_dureeTic, monstres, carteOuverte(), generateur);
        coups.insert(coups.end(), coupsDuTic.begin(), coupsDuTic.end());
        escouade.mettreAJour(s_dureeTic, carteOuverte(), {.physiques = monstres, .ciblables = monstres, .deControle = {}});
    }
    return coups;
}

}  // namespace

TEST_CASE("Lanceur occupe : une frappe demandee pendant sa provocation attend, puis part") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_adresseProvocation)->enActivation);

    // Monstre au contact, frappe prête : elle attend la fin de la provocation.
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));

    // La provocation part (0,2 s), puis la frappe s'active et part (0,3 s).
    REQUIRE(compter(attaquerPendant(escouade, monstres, 0.7f, generateur), s_degatsFrappeDeTest) == 1);
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseFrappe));  // sa recharge court
}

TEST_CASE("Lanceur occupe : une frappe en attente ne se perd pas si la cible arrive pendant la provocation") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    escouade.verrouiller(IdMonstre{1});
    const std::vector<MonstreVu> loin = unSeulMonstre(cibleMonstre({300.0f, 60.0f}));
    const std::vector<MonstreVu> pres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, loin, carteOuverte(), generateur).empty());  // hors de portée : en attente

    // Le monstre arrive au contact pendant l'activation de la provocation : la frappe attend encore.
    static_cast<void>(escouade.attaquer(s_dureeTic, pres, carteOuverte(), generateur));
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseFrappe)->enActivation);

    REQUIRE(compter(attaquerPendant(escouade, pres, 1.0f, generateur), s_degatsFrappeDeTest) == 1);
}

TEST_CASE("Lanceur occupe : sans verrou, une frappe en attente est abandonnee si sa cible n'est plus atteinte") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> pres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));
    const std::vector<MonstreVu> loin = unSeulMonstre(cibleMonstre({300.0f, 60.0f}));
    REQUIRE(escouade.declencherSkill(s_adresseProvocation, pres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseFrappe, pres, carteOuverte(), generateur).empty());

    // Le monstre s'éloigne avant la fin de la provocation : sans verrou, personne ne le suit.
    REQUIRE(compter(attaquerPendant(escouade, loin, 1.0f, generateur), s_degatsFrappeDeTest) == 0);
    REQUIRE(escouade.skillEstPret(s_adresseFrappe));  // sans recharge

    // Plus rien n'attend : un autre skill est de nouveau accepté.
    subirCoup(escouade, IdMonstre{1}, Place{1}, 30);
    REQUIRE(escouade.declencherSkill(s_adresseSoin, {}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_adresseSoin)->enActivation);
}

TEST_CASE("Lanceur occupe : le paladin soigne apres sa frappe sacree") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {paladinDeTest()});  // seul, devant : en (85, 60)
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({110.0f, 60.0f}));
    subirCoup(escouade, IdMonstre{1}, Place{1}, 4 * s_puissanceImpositionDeTest);  // blessé (sans mourir) : il est la cible de son soin
    const int vieBlessee = *escouade.pointsDeVieDuPersonnage(Place{1});

    REQUIRE(escouade.declencherSkill(s_adresseFrappeSacree, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkill(s_adresseImposition, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adresseImposition)->enActivation);

    // La frappe part (0,3 s), puis le soin s'active et part (0,5 s).
    REQUIRE(compter(combattrePendant(escouade, monstres, 1.2f, generateur), s_degatsFrappeSacreeDeTest) == 1);
    REQUIRE(*escouade.pointsDeVieDuPersonnage(Place{1}) == vieBlessee + s_puissanceImpositionDeTest);
}

TEST_CASE("Lanceur occupe : le rodeur lance sa pluie de fleches apres son tir percant") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouadeDe({50.0f, 50.0f}, {rodeurDeTest()});  // seul, derrière : en (35, 60)
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({135.0f, 60.0f}));

    REQUIRE(escouade.declencherSkill(s_adresseTir, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.declencherSkillDeZone(s_adressePluie, {135.0f, 60.0f}, monstres, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_adressePluie)->enActivation);
    REQUIRE(escouade.skillEstPret(s_adressePluie));

    // Le tir part (0,5 s), puis la pluie s'active et part (0,8 s).
    const std::vector<Coup> coups = attaquerPendant(escouade, monstres, 1.6f, generateur);
    REQUIRE(compter(coups, s_degatsTirDeTest) == 1);
    REQUIRE(compter(coups, s_degatsPluieDeTest) == 1);
}

// Faits « skill parti » (étape 86) : un par skill de la barre, au départ de l'effet.

TEST_CASE("Un skill de degats signale son depart a la fin de l'activation, pas avant") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    static_cast<void>(escouade.declencherSkill(s_adresseFrappe, monstres, carteOuverte(), generateur));
    static_cast<void>(attaquerPendant(escouade, monstres, 0.25f, generateur));
    REQUIRE(skillsPartis(escouade).empty());

    static_cast<void>(attaquerPendant(escouade, monstres, 0.1f, generateur));
    const std::vector<Escouade::SkillPart> partis = skillsPartis(escouade);
    REQUIRE(partis.size() == 1);
    REQUIRE(partis[0].place == Place{1});
    REQUIRE(partis[0].skill == IdSkill::FrappePuissante);
    verifierPointProche(partis[0].position, *escouade.centreDuPersonnage(Place{1}));
}

TEST_CASE("Provocation, soin et zone signalent leur depart, meme sans monstre touche") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    infliger(escouade, IdMonstre{1}, {185.0f, 235.0f}, 20);  // le mage est blessé

    static_cast<void>(declencherEtAttendre(escouade, s_adresseProvocation, {}, carteOuverte(), generateur));
    std::vector<Escouade::SkillPart> partis = skillsPartis(escouade);
    REQUIRE(partis.size() == 1);
    REQUIRE(partis[0].skill == IdSkill::Provocation);

    static_cast<void>(declencherEtAttendre(escouade, s_adresseSoin, {}, carteOuverte(), generateur));
    partis = skillsPartis(escouade);
    REQUIRE(partis.size() == 1);
    REQUIRE(partis[0].skill == IdSkill::Soin);
    REQUIRE(partis[0].place == Place{2});

    // Zone vide : le sort part quand même.
    static_cast<void>(declencherZoneEtAttendre(escouade, s_adresseExplosion, {300.0f, 300.0f}, {}, carte, generateur));
    partis = skillsPartis(escouade);
    REQUIRE(partis.size() == 1);
    REQUIRE(partis[0].skill == IdSkill::Explosion);
    REQUIRE(partis[0].place == Place{3});
}

TEST_CASE("L'attaque de base ne signale aucun skill parti") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({50.0f, 50.0f});
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({85.0f, 60.0f}));

    REQUIRE_FALSE(attaquerPendant(escouade, monstres, 1.0f, generateur).empty());  // l'attaque de base a frappé
    REQUIRE(skillsPartis(escouade).empty());
}

TEST_CASE("Provocation : un fait par monstre provoque, aucun pour un monstre hors de la zone") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{IdMonstre{1}, cibleMonstre({335.0f, 210.0f})},
        MonstreVu{IdMonstre{2}, cibleMonstre({235.0f, 310.0f})},
        MonstreVu{IdMonstre{3}, cibleMonstre({800.0f, 210.0f})},
    };

    static_cast<void>(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur));

    std::vector<IdMonstre> provoques;
    for (const Escouade::FaitDEscouade& fait : escouade.prendreFaits()) {
        if (const auto* provoque = std::get_if<Escouade::MonstreProvoque>(&fait)) {
            provoques.push_back(provoque->idMonstre);
        }
    }
    REQUIRE(provoques == std::vector<IdMonstre>{IdMonstre{1}, IdMonstre{2}});

    // Relancée après la recharge, elle provoque de nouveau les mêmes monstres : de nouveaux faits.
    escouade.mettreAJour(10.1f, carteOuverte(), {});
    static_cast<void>(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur));
    int nouveaux = 0;
    for (const Escouade::FaitDEscouade& fait : escouade.prendreFaits()) {
        nouveaux += std::holds_alternative<Escouade::MonstreProvoque>(fait) ? 1 : 0;
    }
    REQUIRE(nouveaux == 2);
}

// Les armes exigées (étape 91.5), avec les vraies classes : le guerrier a besoin d'une épée, d'une masse, d'une dague
// ou d'une épée à deux mains pour sa frappe puissante. Escouade en (50, 50) : le guerrier en (85, 60).
namespace {
constexpr AdresseSkill s_frappeDuGuerrierReel{Place{1}, IdSkill::FrappePuissante};
constexpr AdresseSkill s_provocationDuGuerrierReel{Place{1}, IdSkill::Provocation};

Escouade unGuerrierReel() {
    // Capable de tout porter : les armes qu'on lui donne ne se heurtent pas à ses seuils.
    return uneEscouadeDe({50.0f, 50.0f}, {capableDeToutPorter(composer({IdSpecialite::Guerrier})[0])});
}

void retirerLArme(Escouade& escouade) {
    REQUIRE(escouade.deplacerObjet(PositionPortee{1, Emplacement::MainDroite}, PositionDansLeSac{0}));
}
}  // namespace

TEST_CASE("Armes exigees : sans l'arme, le skill est refuse comme celui d'un lanceur mort, sans recharge ni attente") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = unGuerrierReel();
    const std::vector<MonstreVu> monstres = unSeulMonstre(cibleMonstre({110.0f, 60.0f}));
    escouade.verrouiller(IdMonstre{1});
    retirerLArme(escouade);

    REQUIRE_FALSE(escouade.skillEstPret(s_frappeDuGuerrierReel));
    REQUIRE(escouade.etatDuSkill(s_frappeDuGuerrierReel)->armeAbsente);
    REQUIRE(escouade.declencherSkill(s_frappeDuGuerrierReel, monstres, carteOuverte(), generateur).empty());

    REQUIRE_FALSE(escouade.etatDuSkill(s_frappeDuGuerrierReel)->enActivation);
    REQUIRE(escouade.etatDuSkill(s_frappeDuGuerrierReel)->tempsRestant == 0.0f);   // pas de recharge
    // Pas de mise en attente non plus : un autre skill est accepté tout de suite.
    REQUIRE(escouade.declencherSkill(s_provocationDuGuerrierReel, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->enActivation);
}

TEST_CASE("Armes exigees : la provocation, sans exigence, se lance aux mains nues") {
    Escouade escouade = unGuerrierReel();
    retirerLArme(escouade);

    REQUIRE(escouade.skillEstPret(s_provocationDuGuerrierReel));
    REQUIRE_FALSE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->armeAbsente);
}

TEST_CASE("Armes exigees : remettre une arme qui convient (la dague) rend le skill pret") {
    Escouade escouade = unGuerrierReel();
    retirerLArme(escouade);
    REQUIRE_FALSE(escouade.skillEstPret(s_frappeDuGuerrierReel));

    REQUIRE(escouade.ramasser(Objet{IdObjet::Dague}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{1}, PositionPortee{1, Emplacement::MainDroite}));

    REQUIRE(escouade.skillEstPret(s_frappeDuGuerrierReel));
    REQUIRE_FALSE(escouade.etatDuSkill(s_frappeDuGuerrierReel)->armeAbsente);
}

TEST_CASE("Armes exigees : un arc ne convient pas a la frappe puissante") {
    Escouade escouade = unGuerrierReel();
    retirerLArme(escouade);
    REQUIRE(escouade.ramasser(Objet{IdObjet::Arc}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{1}, PositionPortee{1, Emplacement::MainDroite}));

    REQUIRE(escouade.etatDuSkill(s_frappeDuGuerrierReel)->armeAbsente);
}

TEST_CASE("Armes exigees : un skill en attente est abandonne quand son lanceur perd l'arme") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = unGuerrierReel();
    const std::vector<MonstreVu> loin = unSeulMonstre(cibleMonstre({300.0f, 60.0f}));
    escouade.verrouiller(IdMonstre{1});
    REQUIRE(escouade.declencherSkill(s_frappeDuGuerrierReel, loin, carteOuverte(), generateur).empty());  // hors de portée : en attente
    REQUIRE_FALSE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->enActivation);
    REQUIRE(escouade.declencherSkill(s_provocationDuGuerrierReel, loin, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->enActivation);  // refusée : un skill attend

    retirerLArme(escouade);

    REQUIRE(escouade.declencherSkill(s_provocationDuGuerrierReel, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->enActivation);  // plus rien n'attend
}

// Retirer l'arme sans la ranger (étape 97.6, jeter un objet) : mêmes règles que le déplacement vers le sac.
TEST_CASE("Armes exigees : retirerObjet sur l'arme portee rend le skill refuse, et rend l'arme") {
    Escouade escouade = unGuerrierReel();
    REQUIRE(escouade.skillEstPret(s_frappeDuGuerrierReel));

    const std::optional<Objet> arme = escouade.retirerObjet(PositionPortee{1, Emplacement::MainDroite});

    REQUIRE(arme.has_value());
    REQUIRE(definitionObjet(arme->modele).arme.has_value());
    REQUIRE_FALSE(escouade.equipementDuPersonnage(Place{1}).a(Emplacement::MainDroite).has_value());
    REQUIRE_FALSE(escouade.skillEstPret(s_frappeDuGuerrierReel));
    REQUIRE(escouade.etatDuSkill(s_frappeDuGuerrierReel)->armeAbsente);
}

TEST_CASE("Armes exigees : un skill en attente est abandonne quand retirerObjet retire l'arme de son lanceur") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = unGuerrierReel();
    const std::vector<MonstreVu> loin = unSeulMonstre(cibleMonstre({300.0f, 60.0f}));
    escouade.verrouiller(IdMonstre{1});
    REQUIRE(escouade.declencherSkill(s_frappeDuGuerrierReel, loin, carteOuverte(), generateur).empty());  // hors de portée : en attente
    REQUIRE(escouade.declencherSkill(s_provocationDuGuerrierReel, loin, carteOuverte(), generateur).empty());
    REQUIRE_FALSE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->enActivation);  // refusée : un skill attend

    REQUIRE(escouade.retirerObjet(PositionPortee{1, Emplacement::MainDroite}).has_value());

    REQUIRE(escouade.declencherSkill(s_provocationDuGuerrierReel, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.etatDuSkill(s_provocationDuGuerrierReel)->enActivation);  // plus rien n'attend
}
