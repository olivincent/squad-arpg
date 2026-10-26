// ./tests/interface/test_EcranInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "aides_escouade.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Progression.hpp"
#include "interface/DispositionInventaire.hpp"
#include "interface/EcranInventaire.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/Partie.hpp"
#include "objets/Objet.hpp"

namespace {

const std::vector<std::string> s_noms = {"Guerrier", "Pretre", "Mage"};
const sf::Vector2f s_vue{1200.0f, 800.0f};
constexpr std::size_t s_guerrier = 1;
constexpr std::size_t s_mage = 3;

sf::Vector2f centre(sf::FloatRect zone) {
    return zone.position + zone.size / 2.0f;
}

sf::Vector2f centreDeLaCase(std::size_t indice) {
    return centre(rectangleDeLaCaseDuSac(indice, s_vue));
}

sf::Vector2f centreDeLEmplacement(std::size_t place, Emplacement emplacement) {
    return centre(rectangleDeLEmplacement(place, emplacement, s_vue));
}

// Une partie sans monstre, avec l'escouade de test et un sac vide.
Partie partieSansMonstre() {
    return unePartie(niveauDegage({}), s_graineDeTest);
}

// La même, avec des gants de cuir dans la case 0 du sac.
Partie partieAvecDesGants() {
    Partie partie = partieSansMonstre();
    REQUIRE(partie.getEscouade().ramasser(Objet{IdObjet::GantsDeCuir}));
    return partie;
}

// La même, dont les trois personnages peuvent porter tous les objets du catalogue.
Partie partieCapableDeToutPorter() {
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition = capableDeToutPorter(std::move(definition));
    }
    Partie partie = partieDeTest(niveauDegage({}), std::move(composition));
    return partie;
}

}  // namespace

TEST_CASE("inventaire : I ferme l'ecran") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
}

TEST_CASE("inventaire : Echap ferme l'ecran quand rien n'est saisi") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("inventaire : les autres touches sont ignorees") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Q)));
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::F5)));
}

TEST_CASE("inventaire : laisse voir la partie dessous") {
    Partie partie = partieSansMonstre();
    const EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    REQUIRE(ecran.laisseVoirDessous());
}

TEST_CASE("glissement : saisir un objet commence le glissement, une case vide non") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(5), s_vue);  // case vide
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    ecran.saisir({1.0f, 1.0f}, s_vue);  // hors des elements
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    ecran.saisir(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);  // emplacement vide
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionDansLeSac{0}});
}

TEST_CASE("glissement : rien ne bouge avant le relachement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sur un emplacement compatible equipe l'objet") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : relacher sur une case du sac range l'objet") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLaCase(12), s_vue);

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.sac().a(12) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : un emplacement incompatible refuse, rien ne bouge, le glissement se termine") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(s_guerrier, Emplacement::Tete), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == Objet{IdObjet::CasqueDeFer});
}

TEST_CASE("glissement : relacher dans un ecart du panneau ne bouge rien, ne jette rien et termine le glissement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centre(rectangleDeLEnTete(s_guerrier, s_vue)), s_vue);  // dans le panneau, sur aucun element

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("jeter : lacher un objet du sac hors du panneau le pose au sol pres de l'ancre") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    const Exemplaires avant = exemplaires(partie);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher({1.0f, 1.0f}, s_vue);  // dans la marge, hors du panneau

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE_FALSE(partie.getEscouade().sac().a(0).has_value());
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(partie.getSol().tous().front().objet == Objet{IdObjet::GantsDeCuir});
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("jeter : lacher un objet porte hors du panneau le pose au sol, l'emplacement se vide") {
    Partie partie = partieCapableDeToutPorter();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    const Exemplaires avant = exemplaires(partie);

    ecran.saisir(centreDeLEmplacement(s_guerrier, Emplacement::MainDroite), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::MainDroite}});
    ecran.relacher({s_vue.x - 5.0f, s_vue.y - 5.0f}, s_vue);

    REQUIRE_FALSE(partie.getEscouade().equipementDuPersonnage(s_guerrier).a(Emplacement::MainDroite).has_value());
    REQUIRE(partie.getSol().tous().size() == 1);
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("jeter : annuler le glissement avant de lacher ne jette rien") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.annulerLeGlissement();
    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("jeter : lacher hors du panneau sans glissement ne fait rien") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("glissement : relacher sans glissement ne fait rien") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.relacher(centreDeLaCase(3), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE_FALSE(escouade.sac().a(3).has_value());
}

TEST_CASE("glissement : un objet porte se saisit et s'echange avec un autre personnage") {
    Partie partie = partieCapableDeToutPorter();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLEmplacement(s_guerrier, Emplacement::Jambes), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::Jambes}});
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Jambes), s_vue);

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDeFer});
}

TEST_CASE("glissement : Echap l'annule sans fermer, un deuxieme Echap ferme") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("glissement : I ferme l'ecran meme pendant un glissement, rien n'a bouge") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
}

TEST_CASE("glissement : annulerLeGlissement abandonne sans rien deplacer") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.annulerLeGlissement();
    ecran.relacher(centreDeLEmplacement(s_mage, Emplacement::Gants), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeCuir});
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants).has_value());
}

// Les points libres et leur brouillon (étape 100.2). Une partie de niveau 2 : chaque personnage a 3 points libres.
namespace {
constexpr std::size_t s_pretre = 2;

Partie partieDeNiveau(int niveau) {
    Expedition expedition = expeditionDeTest();
    expedition.experience = experiencePourAtteindre(niveau);
    return Partie(niveauDegage({}), std::move(expedition), s_graineDeTest, catalogueDeTest(monstreSansHasard()));
}

Caracteristiques unePuissance(int valeur = 1) {
    return Caracteristiques::seule(Caracteristique::Puissance, valeur);
}
}  // namespace

TEST_CASE("points libres : un + met un point en attente sans toucher a l'escouade") {
    Partie partie = partieDeNiveau(2);
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    const Caracteristiques baseAvant = escouade.caracteristiquesDeBaseDuPersonnage(s_mage).value();
    const int maximumAvant = escouade.pointsDeVieMaxDuPersonnage(s_mage).value();
    REQUIRE(ecran.pointsRestants(s_mage) == 3);

    ecran.ajouterUnPoint(s_mage, Caracteristique::Endurance);

    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques::seule(Caracteristique::Endurance, 1));
    REQUIRE(ecran.pointsRestants(s_mage) == 2);
    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques());  // rien n'est dépensé
    REQUIRE(escouade.pointsLibresDuPersonnage(s_mage) == 3);
    REQUIRE(escouade.caracteristiquesDeBaseDuPersonnage(s_mage) == baseAvant);
    REQUIRE(escouade.pointsDeVieMaxDuPersonnage(s_mage) == maximumAvant);  // les valeurs dérivées ne montrent pas le brouillon
}

TEST_CASE("points libres : cliquer sur le + d'une caracteristique met un point en attente") {
    Partie partie = partieDeNiveau(2);
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centre(rectangleDuBoutonPlus(s_pretre, Caracteristique::Volonte, s_vue)), s_vue);

    REQUIRE(ecran.pointsEnAttente(s_pretre) == Caracteristiques::seule(Caracteristique::Volonte, 1));
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());  // un bouton ne commence aucun glissement
}

TEST_CASE("points libres : le + est sans effet quand il ne reste aucun point a repartir") {
    Partie partie = partieDeNiveau(2);
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    for (int i = 0; i < 3; ++i) {
        ecran.ajouterUnPoint(s_guerrier, Caracteristique::Puissance);
    }
    REQUIRE(ecran.pointsRestants(s_guerrier) == 0);

    ecran.ajouterUnPoint(s_guerrier, Caracteristique::Adresse);
    ecran.saisir(centre(rectangleDuBoutonPlus(s_guerrier, Caracteristique::Puissance, s_vue)), s_vue);

    REQUIRE(ecran.pointsEnAttente(s_guerrier) == unePuissance(3));
    REQUIRE(ecran.pointsRestants(s_guerrier) == 0);
}

TEST_CASE("points libres : au niveau 1, le + est sans effet") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.ajouterUnPoint(s_guerrier, Caracteristique::Puissance);
    ecran.saisir(centre(rectangleDuBoutonPlus(s_guerrier, Caracteristique::Puissance, s_vue)), s_vue);

    REQUIRE(ecran.pointsEnAttente(s_guerrier) == Caracteristiques());
    REQUIRE(ecran.pointsRestants(s_guerrier) == 0);
}

TEST_CASE("points libres : le - retire un point en attente, pas plus") {
    Partie partie = partieDeNiveau(2);
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Puissance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Puissance);

    ecran.retirerUnPoint(s_mage, Caracteristique::Puissance);
    REQUIRE(ecran.pointsEnAttente(s_mage) == unePuissance());
    REQUIRE(ecran.pointsRestants(s_mage) == 2);

    ecran.saisir(centre(rectangleDuBoutonMoins(s_mage, Caracteristique::Puissance, s_vue)), s_vue);
    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());

    ecran.retirerUnPoint(s_mage, Caracteristique::Puissance);  // plus rien en attente : pas de valeur negative
    ecran.retirerUnPoint(s_mage, Caracteristique::Savoir);
    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());
    REQUIRE(ecran.pointsRestants(s_mage) == 3);
}

TEST_CASE("points libres : le - ne retire jamais un point deja depense") {
    Partie partie = partieDeNiveau(2);
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Endurance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Endurance);
    ecran.appliquer(s_mage);
    const Caracteristiques baseApres = escouade.caracteristiquesDeBaseDuPersonnage(s_mage).value();

    ecran.retirerUnPoint(s_mage, Caracteristique::Endurance);
    ecran.saisir(centre(rectangleDuBoutonMoins(s_mage, Caracteristique::Endurance, s_vue)), s_vue);

    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques::seule(Caracteristique::Endurance, 2));
    REQUIRE(escouade.caracteristiquesDeBaseDuPersonnage(s_mage) == baseApres);
    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());
}

TEST_CASE("points libres : Apply depense pour de bon, puis vide le brouillon") {
    Partie partie = partieDeNiveau(2);
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    const int maximumAvant = escouade.pointsDeVieMaxDuPersonnage(s_mage).value();
    const int puissanceAvant = escouade.caracteristiquesDeBaseDuPersonnage(s_mage).value().valeur(Caracteristique::Puissance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Puissance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Endurance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Endurance);

    ecran.saisir(centre(rectangleDuBoutonAppliquer(s_mage, s_vue)), s_vue);

    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques({1, 2, 0, 0, 0}));
    REQUIRE(escouade.pointsLibresDuPersonnage(s_mage) == 0);
    REQUIRE(escouade.caracteristiquesDeBaseDuPersonnage(s_mage).value().valeur(Caracteristique::Puissance) == puissanceAvant + 1);
    REQUIRE(escouade.pointsDeVieMaxDuPersonnage(s_mage) == maximumAvant + 2 * s_pvParPointDEndurance);
    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());
    REQUIRE(ecran.pointsRestants(s_mage) == 0);
}

TEST_CASE("points libres : Apply sans rien en attente ne fait rien") {
    Partie partie = partieDeNiveau(2);
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.appliquer(s_mage);
    ecran.saisir(centre(rectangleDuBoutonAppliquer(s_mage, s_vue)), s_vue);

    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques());
    REQUIRE(escouade.pointsLibresDuPersonnage(s_mage) == 3);
}

TEST_CASE("points libres : Reset vide le brouillon sans rien appliquer") {
    Partie partie = partieDeNiveau(2);
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Puissance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Adresse);

    ecran.saisir(centre(rectangleDuBoutonReinitialiser(s_mage, s_vue)), s_vue);

    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());
    REQUIRE(ecran.pointsRestants(s_mage) == 3);
    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques());
}

TEST_CASE("points libres : fermer l'ecran efface le brouillon sans rien appliquer") {
    for (const sf::Keyboard::Scancode touche : {sf::Keyboard::Scan::I, sf::Keyboard::Scan::Escape}) {
        Partie partie = partieDeNiveau(2);
        Escouade& escouade = partie.getEscouade();
        EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
        ecran.ajouterUnPoint(s_mage, Caracteristique::Puissance);
        ecran.ajouterUnPoint(s_guerrier, Caracteristique::Endurance);

        REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(touche)));

        REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());
        REQUIRE(ecran.pointsEnAttente(s_guerrier) == Caracteristiques());
        REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques());
        REQUIRE(escouade.pointsDepensesDuPersonnage(s_guerrier) == Caracteristiques());
    }
}

TEST_CASE("points libres : un brouillon par personnage, independants") {
    Partie partie = partieDeNiveau(3);  // 6 points libres chacun
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ecran.ajouterUnPoint(s_guerrier, Caracteristique::Puissance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Savoir);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Savoir);

    ecran.appliquer(s_mage);

    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques::seule(Caracteristique::Savoir, 2));
    REQUIRE(escouade.pointsDepensesDuPersonnage(s_guerrier) == Caracteristiques());
    REQUIRE(ecran.pointsEnAttente(s_guerrier) == unePuissance());  // le brouillon du guerrier est intact
    REQUIRE(ecran.pointsRestants(s_guerrier) == 5);
    REQUIRE(ecran.pointsRestants(s_mage) == 4);
}
