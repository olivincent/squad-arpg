// ./tests/escouade/test_Escouade_degagement.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Coup.hpp"
#include "combat/Engagement.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/Skill.hpp"
#include "monde/TileMap.hpp"

namespace {

// Composition par défaut des tests : guerrier (place 1, devant), prêtre (2), mage (3). Escouade au
// repos en (300, 500) : le centre de l'ancre est en (310, 510), le guerrier en (335, 510), le prêtre en
// (285, 485) et le mage en (285, 535). Seul le guerrier a une zone de contrôle (20 px).
constexpr std::size_t s_guerrier = 1;
constexpr std::size_t s_pretre = 2;
constexpr std::size_t s_mage = 3;

constexpr float s_zoneDuMonstre = 20.0f;
// Le mage est d'une Puissance énorme : son attaque de base (le bâton, portée de tireur) fait quatre fois plus que
// celle des autres, donc reconnaissable.
const Caracteristiques s_caracteristiquesDuMage({110, 10, 10, 10, 10});
const int s_degatsDuMage = montantEffectif(s_degatsBaseDeTest, Coefficients::seul(Caracteristique::Puissance, 1.0f),
                                           s_caracteristiquesDuMage);

// Le mage tire des coups reconnaissables (exécution de 0,15 s).
Escouade uneEscouadeAuMageReconnaissable() {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    composition[s_mage - 1].caracteristiques = s_caracteristiquesDuMage;
    return Escouade({300.0f, 500.0f}, std::move(composition));
}

// Les deux listes que Partie donne à l'escouade pour un monstre (id 1) immobile à ce centre.
struct UnMonstre {
    Cercle cercle;
    std::vector<EntiteVue> vues;
    std::vector<EntiteDeControle> controle;
};

UnMonstre unMonstreEn(sf::Vector2f centre) {
    const Cercle cercle = cibleMonstre(centre);
    return {cercle, {EntiteVue{1, cercle}}, {EntiteDeControle{1, cercle, s_zoneDuMonstre}}};
}

// Le monstre engage-t-il le personnage de cette place ?
bool engageLaPlace(const Escouade& escouade, const UnMonstre& monstre, std::size_t place) {
    return engage(monstre.controle.front(), escouade.entitesDeControle()[place - 1]);
}

// Une image de Partie sans bouger les monstres : attaquer() puis mettreAJour(), avec la photo des
// monstres. Renvoie les coups partis.
std::vector<Coup> image(Escouade& escouade, const TileMap& carte, const std::vector<EntiteVue>& vues,
                        const std::vector<EntiteDeControle>& controle, Generateur& generateur) {
    std::vector<Coup> coups = escouade.attaquer(s_dureeTic, vues, carte, generateur);
    escouade.mettreAJour(s_dureeTic, carte, {.physiques = vues, .ciblables = vues, .deControle = controle});
    return coups;
}

// Ce que mettreAJour reçoit pour ce monstre dans une image sans attaquer avant (le premier mettreAJour
// d'un test) : aucun ciblable, donc personne n'a de monstre « à portée ». C'est ce que voyait l'escouade
// dans ces tests avant l'étape 82.4, quand seul attaquer le calculait.
Escouade::VueDesMonstres sansAttaquer(const UnMonstre& monstre) {
    return {.physiques = monstre.vues, .ciblables = {}, .deControle = monstre.controle};
}

int coupsDuMage(const std::vector<Coup>& coups) {
    int total = 0;
    for (const Coup& coup : coups) {
        total += coup.attaque.montant == s_degatsDuMage ? 1 : 0;
    }
    return total;
}

// Le monstre a frappé le guerrier : il est un agresseur, comme dans une vraie partie. Sans cela, un mage
// qui a fini de se dégager retournerait vers sa place de formation, au contact.
void lAgresseEstConnu(Escouade& escouade) {
    subirCoup(escouade, 1, s_guerrier, 1);
}

// Le monstre est entre le guerrier et le mage : il engage les deux (pas le prêtre).
const sf::Vector2f s_entreGuerrierEtMage{310.0f, 530.0f};

}  // namespace

TEST_CASE("Degagement : le mage au contact d'un monstre tenu par le guerrier se degage vers sa place d'attaque") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);
    REQUIRE(engageLaPlace(escouade, monstre, s_guerrier));
    REQUIRE(engageLaPlace(escouade, monstre, s_mage));
    REQUIRE_FALSE(engageLaPlace(escouade, monstre, s_pretre));
    lAgresseEstConnu(escouade);

    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));

    REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);
    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_guerrier).has_value());  // la mêlée tient le monstre
    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_pretre).has_value());    // pas engagé

    int coupsPendantLeDegagement = 0;
    int coupsApres = 0;
    for (int i = 0; i < nombreDeTics(2.0f); ++i) {
        const bool enDegagement = escouade.degagementDuPersonnage(s_mage).has_value();
        const int coups = coupsDuMage(image(escouade, carte, monstre.vues, monstre.controle, generateur));
        (enDegagement ? coupsPendantLeDegagement : coupsApres) += coups;
    }

    REQUIRE(coupsPendantLeDegagement == 0);  // il marche, il ne tire pas
    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());  // arrivé
    const std::vector<Cercle> cercles = escouade.cerclesDesVivants();
    const float ecart = distanceBordABord(cercles[s_mage - 1], monstre.cercle);
    REQUIRE_THAT(ecart, Catch::Matchers::WithinAbs(0.9f * s_porteeTireurDeTest, 1.0f));  // à 90 % de sa portée
    REQUIRE_FALSE(engageLaPlace(escouade, monstre, s_mage));
    REQUIRE(engageLaPlace(escouade, monstre, s_guerrier));  // le guerrier tient toujours le monstre
    REQUIRE(coupsApres > 0);                                // arrivé, il tire sur place
}

TEST_CASE("Degagement : seul au contact du monstre, le mage reste et tire") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn({255.0f, 535.0f});
    REQUIRE(engageLaPlace(escouade, monstre, s_mage));
    REQUIRE_FALSE(engageLaPlace(escouade, monstre, s_guerrier));

    int coups = 0;
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        coups += coupsDuMage(image(escouade, carte, monstre.vues, monstre.controle, generateur));
        REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
    }

    REQUIRE(coups > 0);
}

TEST_CASE("Degagement : le pretre se degage comme le mage") {
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn({312.0f, 482.0f});
    REQUIRE(engageLaPlace(escouade, monstre, s_guerrier));
    REQUIRE(engageLaPlace(escouade, monstre, s_pretre));
    REQUIRE_FALSE(engageLaPlace(escouade, monstre, s_mage));

    escouade.mettreAJour(s_dureeTic, carteOuverte(), sansAttaquer(monstre));

    REQUIRE(escouade.degagementDuPersonnage(s_pretre) == 1);
    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
}

TEST_CASE("Degagement : engage par deux monstres dont le guerrier n'en tient qu'un, le mage reste") {
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre premier = unMonstreEn(s_entreGuerrierEtMage);
    const Cercle second = cibleMonstre({255.0f, 545.0f});  // sur le mage seulement
    const std::vector<EntiteVue> vues{premier.vues.front(), EntiteVue{2, second}};
    const std::vector<EntiteDeControle> controle{premier.controle.front(), EntiteDeControle{2, second, s_zoneDuMonstre}};
    REQUIRE(engage(controle[1], escouade.entitesDeControle()[s_mage - 1]));
    REQUIRE_FALSE(engage(controle[1], escouade.entitesDeControle()[s_guerrier - 1]));

    for (int i = 0; i < nombreDeTics(0.5f); ++i) {
        escouade.mettreAJour(s_dureeTic, carteOuverte(), {.physiques = vues, .ciblables = {}, .deControle = controle});
        REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
    }
}

TEST_CASE("Degagement : sans liste de monstres de controle, personne ne se degage") {
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);

    escouade.mettreAJour(s_dureeTic, carteOuverte(), {.physiques = monstre.vues, .ciblables = {}, .deControle = {}});

    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
}

TEST_CASE("Degagement : un coup en execution part d'abord, le degagement commence apres") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);
    lAgresseEstConnu(escouade);

    // Première image : le mage, immobile et à portée, engage son coup ; la règle de départ l'attend.
    int coups = coupsDuMage(image(escouade, carte, monstre.vues, monstre.controle, generateur));
    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
    for (int i = 0; i < nombreDeTics(0.1f); ++i) {  // 0,1 s < 0,15 s d'exécution
        coups += coupsDuMage(image(escouade, carte, monstre.vues, monstre.controle, generateur));
    }
    REQUIRE(coups == 0);
    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());

    for (int i = 0; i < nombreDeTics(0.2f); ++i) {  // le coup part, puis le départ
        coups += coupsDuMage(image(escouade, carte, monstre.vues, monstre.controle, generateur));
    }

    REQUIRE(coups == 1);
    REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);
}

TEST_CASE("Degagement : fin si le monstre disparait ou rentre a son poste") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));
    REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);

    SECTION("mort ou retire : absent des deux listes") {
        escouade.mettreAJour(s_dureeTic, carte, {});
    }
    SECTION("rentre a son poste : encore un obstacle, plus une cible") {
        escouade.mettreAJour(s_dureeTic, carte, {.physiques = monstre.vues, .ciblables = {}, .deControle = {}});
    }

    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
}

TEST_CASE("Degagement : fin si l'ancre se met en mouvement") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));
    REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);

    escouade.allerVers({800.0f, 500.0f}, carte);
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));

    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
}

TEST_CASE("Degagement : pas de depart quand l'ancre est deja en mouvement") {
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);

    escouade.allerVers({800.0f, 500.0f}, carte);
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));

    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
}

TEST_CASE("Degagement : un skill en attente pour le mage empeche le depart et met fin au degagement") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    const sf::Vector2f loin{900.0f, 500.0f};  // hors de portée de l'explosion (250 px) : le skill est mis en attente
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);

    SECTION("avant le depart") {
        Escouade escouade = uneEscouadeAuMageReconnaissable();
        REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, loin, monstre.vues, carte, generateur).empty());

        escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));

        REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
    }
    SECTION("pendant le degagement") {
        Escouade escouade = uneEscouadeAuMageReconnaissable();
        escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));
        REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);
        REQUIRE(escouade.declencherSkillDeZone(s_adresseExplosion, loin, monstre.vues, carte, generateur).empty());

        escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));

        REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
    }
}

TEST_CASE("Degagement : un skill du joueur a portee met fin au degagement, le mage reste sur place pendant l'activation") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    const UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));
    REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);

    static_cast<void>(escouade.declencherSkillDeZone(s_adresseExplosion, monstre.cercle.centre, monstre.vues, carte, generateur));
    const sf::Vector2f avant = escouade.cerclesDesVivants()[s_mage - 1].centre;
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));

    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
    verifierPointProche(*escouade.pointVise(s_mage), avant, 0.01f);
}

TEST_CASE("Degagement : fin si le porteur s'en va, le mage est de nouveau seul au contact") {
    Generateur generateur = generateurDeTest();
    const TileMap& carte = carteOuverte();
    Escouade escouade = uneEscouadeAuMageReconnaissable();
    UnMonstre monstre = unMonstreEn(s_entreGuerrierEtMage);
    lAgresseEstConnu(escouade);
    escouade.mettreAJour(s_dureeTic, carte, sansAttaquer(monstre));
    REQUIRE(escouade.degagementDuPersonnage(s_mage) == 1);
    for (int i = 0; i < nombreDeTics(0.05f); ++i) {  // quelques pas : le mage a commencé à reculer
        static_cast<void>(image(escouade, carte, monstre.vues, monstre.controle, generateur));
    }
    const sf::Vector2f mage = escouade.cerclesDesVivants()[s_mage - 1].centre;
    REQUIRE(mage != sf::Vector2f(285.0f, 535.0f));

    // Le monstre se retrouve sur le mage seulement (le guerrier est à plus de 45 px de lui).
    monstre = unMonstreEn(mage + sf::Vector2f(-30.0f, 0.0f));
    REQUIRE(engageLaPlace(escouade, monstre, s_mage));
    REQUIRE_FALSE(engageLaPlace(escouade, monstre, s_guerrier));
    escouade.mettreAJour(s_dureeTic, carte, {.physiques = monstre.vues, .ciblables = monstre.vues, .deControle = monstre.controle});

    REQUIRE_FALSE(escouade.degagementDuPersonnage(s_mage).has_value());
}

TEST_CASE("Degagement : place invalide") {
    Escouade escouade = uneEscouadeAuMageReconnaissable();

    REQUIRE_FALSE(escouade.degagementDuPersonnage(0).has_value());
    REQUIRE_FALSE(escouade.degagementDuPersonnage(4).has_value());
}
