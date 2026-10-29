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
constexpr Place s_guerrier{1};
constexpr Place s_mage{3};

sf::Vector2f centre(sf::FloatRect zone) {
    return zone.position + zone.size / 2.0f;
}

sf::Vector2f centreDeLaCase(std::size_t indice) {
    return centre(rectangleDeLaCaseDuSac(indice, s_vue));
}

// Le centre de cet emplacement dans la colonne du personnage de l'onglet ouvert.
sf::Vector2f centreDeLEmplacement(Emplacement emplacement) {
    return centre(rectangleDeLEmplacement(emplacement, s_vue));
}

// Ouvre l'onglet de cette place par un clic, comme le joueur.
void ouvrir(EcranInventaire& ecran, Place place) {
    ecran.saisir(centre(rectangleDeLOnglet(place, s_vue)), s_vue);
    REQUIRE(ecran.placeOuverte() == place);
}

// Une partie sans monstre, avec l'escouade de test et un sac vide.
Partie partieSansMonstre() {
    return unePartie(niveauDegage({}), s_graineDeTest);
}

// La même, avec des gants de cuir dans la case 0 du sac.
Partie partieAvecDesGants() {
    Partie partie = partieSansMonstre();
    REQUIRE(partie.getEscouade().ramasser(Objet{IdObjet::GantsDeTissu}));
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
    ouvrir(ecran, s_mage);
    ecran.saisir(centreDeLEmplacement(Emplacement::Gants), s_vue);  // emplacement vide
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionDansLeSac{0}});
}

TEST_CASE("glissement : rien ne bouge avant le relachement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});
}

TEST_CASE("glissement : relacher sur un emplacement compatible equipe l'objet") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ouvrir(ecran, s_mage);
    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(Emplacement::Gants), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants) == Objet{IdObjet::GantsDeTissu});
}

TEST_CASE("glissement : relacher sur une case du sac range l'objet") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLaCase(12), s_vue);

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.sac().a(12) == Objet{IdObjet::GantsDeTissu});
}

TEST_CASE("glissement : un emplacement incompatible refuse, rien ne bouge, le glissement se termine") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centreDeLEmplacement(Emplacement::Tete), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Tete) == Objet{IdObjet::HeaumeIntegral});
}

TEST_CASE("glissement : relacher dans un ecart du panneau ne bouge rien, ne jette rien et termine le glissement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher(centre(rectangleDeLArmure(s_vue)), s_vue);  // dans le panneau, sur aucun element

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});
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
    REQUIRE(partie.getSol().tous().front().objet == Objet{IdObjet::GantsDeTissu});
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("jeter : lacher un objet porte hors du panneau le pose au sol, l'emplacement se vide") {
    Partie partie = partieCapableDeToutPorter();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    const Exemplaires avant = exemplaires(partie);

    ecran.saisir(centreDeLEmplacement(Emplacement::MainDroite), s_vue);
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

    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeTissu});
    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("jeter : lacher hors du panneau sans glissement ne fait rien") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeTissu});
    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("glissement : relacher sans glissement ne fait rien") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.relacher(centreDeLaCase(3), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});
    REQUIRE_FALSE(escouade.sac().a(3).has_value());
}

TEST_CASE("glissement : un objet porte se saisit et s'echange avec un autre personnage") {
    Partie partie = partieCapableDeToutPorter();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLEmplacement(Emplacement::Jambes), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::Jambes}});
    ouvrir(ecran, s_mage);  // l'onglet s'ouvre, le glissement continue
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::Jambes}});
    ecran.relacher(centreDeLEmplacement(Emplacement::Jambes), s_vue);

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDePlaques});
}

TEST_CASE("glissement : Echap l'annule sans fermer, un deuxieme Echap ferme") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("glissement : I ferme l'ecran meme pendant un glissement, rien n'a bouge") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});
}

TEST_CASE("glissement : annulerLeGlissement abandonne sans rien deplacer") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ouvrir(ecran, s_mage);
    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.annulerLeGlissement();
    ecran.relacher(centreDeLEmplacement(Emplacement::Gants), s_vue);

    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Gants).has_value());
}

// Les points libres et leur brouillon (étape 100.2). Une partie de niveau 2 : chaque personnage a 3 points libres.
namespace {
constexpr Place s_pretre{2};

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

    ouvrir(ecran, s_pretre);
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Volonte, s_vue)), s_vue);

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
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Puissance, s_vue)), s_vue);  // l'onglet 1 est ouvert

    REQUIRE(ecran.pointsEnAttente(s_guerrier) == unePuissance(3));
    REQUIRE(ecran.pointsRestants(s_guerrier) == 0);
}

TEST_CASE("points libres : au niveau 1, le + est sans effet") {
    Partie partie = partieSansMonstre();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.ajouterUnPoint(s_guerrier, Caracteristique::Puissance);
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Puissance, s_vue)), s_vue);

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

    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonMoins(Caracteristique::Puissance, s_vue)), s_vue);
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
    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonMoins(Caracteristique::Endurance, s_vue)), s_vue);

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

    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonAppliquer(s_vue)), s_vue);

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
    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonAppliquer(s_vue)), s_vue);

    REQUIRE(escouade.pointsDepensesDuPersonnage(s_mage) == Caracteristiques());
    REQUIRE(escouade.pointsLibresDuPersonnage(s_mage) == 3);
}

TEST_CASE("points libres : Reset vide le brouillon sans rien appliquer") {
    Partie partie = partieDeNiveau(2);
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Puissance);
    ecran.ajouterUnPoint(s_mage, Caracteristique::Adresse);

    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonReinitialiser(s_vue)), s_vue);

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

// Les onglets (étape 106.4).
TEST_CASE("onglets : l'ecran s'ouvre sur le premier personnage") {
    Partie partie = partieSansMonstre();
    const EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    REQUIRE(ecran.placeOuverte() == Place{1});
}

TEST_CASE("onglets : un clic sur un onglet l'ouvre, sans commencer de glissement") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    for (const Place place : s_places) {
        ouvrir(ecran, place);
        REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    }
    ouvrir(ecran, s_pretre);
    ouvrir(ecran, s_pretre);  // l'onglet ouvert, recliqué : rien ne change
}

TEST_CASE("onglets : les emplacements cliques sont ceux du personnage de l'onglet ouvert") {
    Partie partie = partieCapableDeToutPorter();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    REQUIRE(partie.getEscouade().equipementDuPersonnage(s_pretre).a(Emplacement::MainDroite).has_value());

    ecran.saisir(centreDeLEmplacement(Emplacement::MainDroite), s_vue);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::MainDroite}});
    ecran.annulerLeGlissement();
    ouvrir(ecran, s_pretre);
    ecran.saisir(centreDeLEmplacement(Emplacement::MainDroite), s_vue);

    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_pretre, Emplacement::MainDroite}});
}

TEST_CASE("onglets : le sac reste le meme, quel que soit l'onglet ouvert") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    for (const Place place : s_places) {
        ouvrir(ecran, place);
        ecran.saisir(centreDeLaCase(0), s_vue);
        REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionDansLeSac{0}});
        ecran.annulerLeGlissement();
    }
}

TEST_CASE("onglets : le brouillon des points libres survit au changement d'onglet et s'efface a la fermeture") {
    Partie partie = partieDeNiveau(2);
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Savoir, s_vue)), s_vue);
    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques::seule(Caracteristique::Savoir, 1));

    ouvrir(ecran, s_guerrier);
    ouvrir(ecran, s_mage);

    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques::seule(Caracteristique::Savoir, 1));
    REQUIRE(ecran.pointsRestants(s_mage) == 2);
    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(ecran.pointsEnAttente(s_mage) == Caracteristiques());
}

TEST_CASE("onglets : les boutons des points libres agissent sur le personnage de l'onglet ouvert") {
    Partie partie = partieDeNiveau(2);
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ouvrir(ecran, s_pretre);
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Puissance, s_vue)), s_vue);
    ouvrir(ecran, s_mage);
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Puissance, s_vue)), s_vue);
    ecran.saisir(centre(rectangleDuBoutonPlus(Caracteristique::Puissance, s_vue)), s_vue);

    REQUIRE(ecran.pointsEnAttente(s_pretre) == unePuissance(1));
    REQUIRE(ecran.pointsEnAttente(s_mage) == unePuissance(2));
    REQUIRE(ecran.pointsEnAttente(s_guerrier) == Caracteristiques());
}

TEST_CASE("onglets : jeter un objet hors du panneau marche depuis n'importe quel onglet") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_mage);

    ecran.saisir(centreDeLaCase(0), s_vue);
    ecran.relacher({1.0f, 1.0f}, s_vue);

    REQUIRE_FALSE(partie.getEscouade().sac().a(0).has_value());
    REQUIRE(partie.getSol().tous().size() == 1);
}

// Le survol d'un onglet pendant un glissement (étape 106.5).
namespace {
constexpr Emplacement s_tete = Emplacement::Tete;

void survoler(EcranInventaire& ecran, Place place) {
    ecran.survoler(centre(rectangleDeLOnglet(place, s_vue)), s_vue);
}

// La partie sans monstre où le mage porte un capuchon (sans seuil) ; le guerrier porte un heaume intégral, que le mage ne peut pas porter.
Partie partieAuHeaumeEtAuCapuchon() {
    Partie partie = partieSansMonstre();
    Escouade& escouade = partie.getEscouade();
    REQUIRE(escouade.ramasser(Objet{IdObjet::Capuchon}));
    REQUIRE(escouade.deplacerObjet(PositionDansLeSac{0}, PositionPortee{s_mage, s_tete}));
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(s_tete) == Objet{IdObjet::HeaumeIntegral});
    return partie;
}
}  // namespace

TEST_CASE("survoler : sans glissement, un onglet survole ne s'ouvre pas") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    survoler(ecran, s_mage);

    REQUIRE(ecran.placeOuverte() == s_guerrier);
}

TEST_CASE("survoler : pendant un glissement, un onglet survole s'ouvre aussitot, le glissement continue") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ecran.saisir(centreDeLaCase(0), s_vue);

    survoler(ecran, s_mage);
    REQUIRE(ecran.placeOuverte() == s_mage);
    survoler(ecran, s_pretre);
    REQUIRE(ecran.placeOuverte() == s_pretre);

    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionDansLeSac{0}});
}

TEST_CASE("survoler : pendant un glissement, un point hors des onglets ne change pas l'onglet ouvert") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_pretre);
    ecran.saisir(centreDeLaCase(0), s_vue);

    ecran.survoler(centreDeLaCase(3), s_vue);
    ecran.survoler({1.0f, 1.0f}, s_vue);

    REQUIRE(ecran.placeOuverte() == s_pretre);
}

TEST_CASE("survoler : l'origine d'un emplacement reste valide quand son onglet n'est plus affiche") {
    Partie partie = partieCapableDeToutPorter();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLEmplacement(Emplacement::Jambes), s_vue);  // les jambes du guerrier
    survoler(ecran, s_mage);
    REQUIRE(ecran.origineDuGlissement() == PositionDObjet{PositionPortee{s_guerrier, Emplacement::Jambes}});
    ecran.relacher(centreDeLEmplacement(Emplacement::Jambes), s_vue);  // les jambes du mage

    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_guerrier).a(Emplacement::Jambes).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(Emplacement::Jambes) == Objet{IdObjet::JambieresDePlaques});
}

TEST_CASE("survoler : un objet du sac glisse sur l'emplacement d'un autre personnage") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    survoler(ecran, s_pretre);
    ecran.relacher(centreDeLEmplacement(Emplacement::Gants), s_vue);

    REQUIRE_FALSE(escouade.sac().a(0).has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_pretre).a(Emplacement::Gants) == Objet{IdObjet::GantsDeTissu});
}

TEST_CASE("survoler : l'echange avec un personnage qui ne peut pas reprendre l'objet envoie celui-ci au sac") {
    Partie partie = partieAuHeaumeEtAuCapuchon();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_mage);

    ecran.saisir(centreDeLEmplacement(s_tete), s_vue);  // le capuchon du mage
    survoler(ecran, s_guerrier);
    ecran.relacher(centreDeLEmplacement(s_tete), s_vue);  // sur le heaume du guerrier

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(s_tete) == Objet{IdObjet::Capuchon});
    REQUIRE_FALSE(escouade.equipementDuPersonnage(s_mage).a(s_tete).has_value());  // le mage ne porte pas le heaume
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::HeaumeIntegral});
}

TEST_CASE("survoler : sac plein, l'emplacement du heaume n'est pas une destination et rien ne bouge") {
    Partie partie = partieAuHeaumeEtAuCapuchon();
    Escouade& escouade = partie.getEscouade();
    while (escouade.ramasser(Objet{IdObjet::GantsDeTissu})) {
    }
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_mage);
    const Exemplaires avant = exemplaires(partie);

    ecran.saisir(centreDeLEmplacement(s_tete), s_vue);
    REQUIRE_FALSE(escouade.peutDeplacerObjet(*ecran.origineDuGlissement(), PositionPortee{s_guerrier, s_tete}));
    survoler(ecran, s_guerrier);
    ecran.relacher(centreDeLEmplacement(s_tete), s_vue);

    REQUIRE(escouade.equipementDuPersonnage(s_guerrier).a(s_tete) == Objet{IdObjet::HeaumeIntegral});
    REQUIRE(escouade.equipementDuPersonnage(s_mage).a(s_tete) == Objet{IdObjet::Capuchon});
    REQUIRE(exemplaires(partie) == avant);
}

TEST_CASE("survoler : relacher sur un onglet ne deplace rien et termine le glissement") {
    Partie partie = partieAvecDesGants();
    Escouade& escouade = partie.getEscouade();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    survoler(ecran, s_mage);
    ecran.relacher(centre(rectangleDeLOnglet(s_mage, s_vue)), s_vue);

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(escouade.sac().a(0) == Objet{IdObjet::GantsDeTissu});  // rien n'a bougé…
    REQUIRE(partie.getSol().tous().empty());                         // …ni n'a été jeté (le point est dans le panneau)
}

TEST_CASE("survoler : Echap annule le glissement apres le changement d'onglet, sans rien deplacer") {
    Partie partie = partieAvecDesGants();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    ecran.saisir(centreDeLaCase(0), s_vue);
    survoler(ecran, s_mage);
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Escape)));

    REQUIRE_FALSE(ecran.origineDuGlissement().has_value());
    REQUIRE(partie.getEscouade().sac().a(0) == Objet{IdObjet::GantsDeTissu});
    REQUIRE(ecran.placeOuverte() == s_mage);  // l'onglet reste ouvert
}

TEST_CASE("ongletsDeDestination : vide sans glissement") {
    Partie partie = partieAuHeaumeEtAuCapuchon();
    const EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);

    REQUIRE(ecran.ongletsDeDestination().empty());
}

TEST_CASE("ongletsDeDestination : les onglets dont le personnage peut recevoir l'objet saisi, d'apres la meme requete que le relachement") {
    Partie partie = partieAuHeaumeEtAuCapuchon();
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_mage);

    ecran.saisir(centreDeLEmplacement(s_tete), s_vue);  // le capuchon du mage

    // Le guerrier (l'échange, le heaume allant au sac) et le prêtre (tête vide) ; pas le mage, dont le capuchon n'a pas d'autre place.
    REQUIRE(ecran.ongletsDeDestination() == std::vector<Place>{s_guerrier, s_pretre});
}

TEST_CASE("ongletsDeDestination : sac plein, l'onglet du guerrier n'y est plus") {
    Partie partie = partieAuHeaumeEtAuCapuchon();
    Escouade& escouade = partie.getEscouade();
    while (escouade.ramasser(Objet{IdObjet::GantsDeTissu})) {
    }
    EcranInventaire ecran(partie.getEscouade(), partie.getSol(), partie.getCarte(), s_noms);
    ouvrir(ecran, s_mage);

    ecran.saisir(centreDeLEmplacement(s_tete), s_vue);

    REQUIRE(ecran.ongletsDeDestination() == std::vector<Place>{s_pretre});
}
