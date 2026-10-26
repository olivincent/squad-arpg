// ./tests/jeu/test_Partie_butin.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <set>
#include <variant>
#include <string>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/Progression.hpp"
#include "jeu/Expedition.hpp"
#include "jeu/ObjetAuSol.hpp"
#include "jeu/Partie.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Niveau.hpp"
#include "monde/TileMap.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "objets/CatalogueObjets.hpp"

// Le butin des monstres (étape 97.3) : un monstre qui meurt laisse des objets au sol.
namespace {
constexpr float s_tailleCase = 40.0f;

// Une salle de 7 x 3 cases : un monstre contre le mur du haut a des murs pour voisins.
const std::vector<std::string> s_plan = {
    "#########",
    "#.......#",
    "#.......#",
    "#.......#",
    "#########",
};

Niveau salleAvecMonstres(const std::vector<sf::Vector2f>& monstres) {
    std::vector<MonstreDuNiveau> deLaPartie;
    for (const sf::Vector2f position : monstres) {
        deLaPartie.push_back({position, static_cast<int>(deLaPartie.size())});
    }
    return {s_plan, s_tailleCase, {200.0f, 80.0f}, deLaPartie};
}

// Un monstre qui meurt au premier coup et laisse ce butin.
DefinitionDeMonstre monstreQuiLaisse(int chance, int nombre) {
    DefinitionDeMonstre definition = monstreSansHasard();
    definition.pointsDeVieMax = 1;
    definition.chanceDeButin = chance;
    definition.nombreDObjets = nombre;
    return definition;
}

Partie partieDeButin(const Niveau& niveau, int chance, int nombre) {
    return partieDeTest(niveau, compositionDeTest(), s_graineDeTest, monstreQuiLaisse(chance, nombre));
}

// Tue tous les monstres de la partie, un clic après l'autre.
void toutTuer(Partie& partie) {
    for (int i = 0; i < 3000 && !partie.getMonstres().empty(); ++i) {
        if (!partie.getEscouade().getCibleVerrouillee().has_value() && !partie.getMonstres().empty()) {
            static_cast<void>(partie.cliquer(partie.getMonstres().front().getCentre()));
        }
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.getMonstres().empty());
}
}  // namespace

TEST_CASE("butin : un monstre tue avec 100 % laisse le bon nombre d'objets valides") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 3);
    REQUIRE(partie.getSol().tous().empty());

    toutTuer(partie);

    REQUIRE(partie.getSol().tous().size() == 3);
    for (const ObjetAuSol& objet : partie.getSol().tous()) {
        REQUIRE(objetValide(objet.objet));
    }
}

TEST_CASE("experience : un monstre tue ajoute son experience a l'expedition, comme le butin, quel que soit le butin") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 0, 1);
    REQUIRE(partie.expedition().experience == 0);

    toutTuer(partie);

    REQUIRE(partie.expedition().experience == definitionDeMonstre(CategorieDeMonstre::Infanterie).experience);
}

TEST_CASE("experience : chaque monstre tue rapporte la sienne, l'experience s'additionne") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}, {200.0f, 60.0f}, {260.0f, 60.0f}}), 0, 1);

    toutTuer(partie);

    REQUIRE(partie.expedition().experience == 3 * definitionDeMonstre(CategorieDeMonstre::Infanterie).experience);
}

namespace {
// Un monstre qui meurt au premier coup et rapporte cette expérience.
Partie partieAvecExperience(int experience) {
    DefinitionDeMonstre definition = monstreQuiLaisse(0, 1);
    definition.experience = experience;
    return partieDeTest(salleAvecMonstres({{140.0f, 60.0f}}), compositionDeTest(), s_graineDeTest, definition);
}
}  // namespace

TEST_CASE("niveau : une partie commence au niveau de l'experience de l'expedition") {
    REQUIRE(partieAvecExperience(10).getEscouade().getNiveau() == 1);

    Expedition expedition = expeditionDeTest();
    expedition.experience = experiencePourAtteindre(3);
    const Partie partie(salleAvecMonstres({}), expedition, s_graineDeTest, catalogueDeTest(monstreSansHasard()));

    REQUIRE(partie.getEscouade().getNiveau() == 3);
    REQUIRE(partie.getEscouade().pointsDeVieMaxDuPersonnage(1) == pointsDeVieMax(*partie.getEscouade().caracteristiquesDuPersonnage(1), 3));
}

TEST_CASE("niveau : l'experience d'un monstre tue fait monter l'escouade au seuil exact") {
    Partie partie = partieAvecExperience(experiencePourAtteindre(2) - 1);
    toutTuer(partie);
    REQUIRE(partie.getEscouade().getNiveau() == 1);

    Partie assez = partieAvecExperience(experiencePourAtteindre(2));
    toutTuer(assez);
    REQUIRE(assez.getEscouade().getNiveau() == 2);
}

TEST_CASE("niveau : plusieurs niveaux gagnes d'un coup sont tous appliques, a chaque personnage") {
    Partie partie = partieAvecExperience(experiencePourAtteindre(5));
    const std::optional<Caracteristiques> avant = partie.getEscouade().caracteristiquesDeBaseDuPersonnage(1);  // le guerrier de test : Puissance, Endurance
    const std::optional<int> vieMaxAvant = partie.getEscouade().pointsDeVieMaxDuPersonnage(1);

    toutTuer(partie);

    REQUIRE(partie.getEscouade().getNiveau() == 5);
    REQUIRE(partie.expedition().experience == experiencePourAtteindre(5));
    const Caracteristiques apres = *partie.getEscouade().caracteristiquesDeBaseDuPersonnage(1);
    REQUIRE(apres.valeur(Caracteristique::Puissance) == avant->valeur(Caracteristique::Puissance) + 2);   // niveaux 2 et 4
    REQUIRE(apres.valeur(Caracteristique::Endurance) == avant->valeur(Caracteristique::Endurance) + 2);   // niveaux 3 et 5
    REQUIRE(*partie.getEscouade().pointsDeVieMaxDuPersonnage(1) == *vieMaxAvant + 2 * s_pvParPointDEndurance + 4 * s_pvParNiveau);
}

namespace {
// Les niveaux annoncés par les événements de montée de niveau.
std::vector<int> niveauxAnnonces(const std::vector<EvenementDeCombat>& evenements) {
    std::vector<int> niveaux;
    for (const EvenementDeCombat& evenement : evenements) {
        if (const auto* atteint = std::get_if<NiveauAtteint>(&evenement.detail)) {
            REQUIRE(evenement.campCible == Camp::Escouade);
            REQUIRE(evenement.idCible == 0);  // l'escouade entière
            niveaux.push_back(atteint->niveau);
        }
    }
    return niveaux;
}
}  // namespace

TEST_CASE("niveau : un evenement par niveau gagne, dans l'ordre, y compris quand plusieurs viennent d'un coup") {
    Partie partie = partieAvecExperience(experiencePourAtteindre(5));

    toutTuer(partie);

    REQUIRE(niveauxAnnonces(partie.prendreEvenements()) == std::vector<int>{2, 3, 4, 5});
}

TEST_CASE("niveau : aucun evenement quand l'experience ne donne pas de niveau") {
    Partie partie = partieAvecExperience(experiencePourAtteindre(2) - 1);

    toutTuer(partie);

    REQUIRE(niveauxAnnonces(partie.prendreEvenements()).empty());
}

TEST_CASE("niveau : F9 fait gagner exactement un niveau, avec son evenement, et ainsi de suite") {
    Partie partie = partieAvecExperience(1);
    REQUIRE(partie.getNiveau() == 1);

    partie.gagnerUnNiveau();
    REQUIRE(partie.getNiveau() == 2);
    REQUIRE(partie.getExperience() == experiencePourAtteindre(2));
    REQUIRE(niveauxAnnonces(partie.prendreEvenements()) == std::vector<int>{2});

    partie.gagnerUnNiveau();
    REQUIRE(partie.getNiveau() == 3);
    REQUIRE(partie.getExperience() == experiencePourAtteindre(3));
    REQUIRE(niveauxAnnonces(partie.prendreEvenements()) == std::vector<int>{3});
}

TEST_CASE("niveau : F9 donne l'experience qui manque, pas plus, quand on est deja a mi-chemin") {
    Expedition expedition = expeditionDeTest();
    expedition.experience = experiencePourAtteindre(3) + 5;
    Partie partie(salleAvecMonstres({}), expedition, s_graineDeTest, catalogueDeTest(monstreSansHasard()));
    REQUIRE(partie.getNiveau() == 3);

    partie.gagnerUnNiveau();

    REQUIRE(partie.getNiveau() == 4);
    REQUIRE(partie.getExperience() == experiencePourAtteindre(4));
}

TEST_CASE("niveau : la base d'un personnage ne vit que dans sa classe et l'experience, pas dans l'expedition") {
    Partie partie = partieAvecExperience(experiencePourAtteindre(4));
    const Expedition avant = partie.expedition();

    toutTuer(partie);
    const Expedition apres = partie.expedition();

    REQUIRE(apres.experience == experiencePourAtteindre(4));
    for (std::size_t i = 0; i < apres.membres.size(); ++i) {
        REQUIRE(apres.membres[i].definition.caracteristiques == avant.membres[i].definition.caracteristiques);  // la classe, jamais la croissance
    }
}

TEST_CASE("butin : avec 0 %, un monstre tue ne laisse rien") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 0, 3);

    toutTuer(partie);

    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("butin : les objets tombent sur le sol, jamais dans un mur, sur des cases differentes") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 5);
    const TileMap carte(s_plan, s_tailleCase);

    toutTuer(partie);

    REQUIRE(partie.getSol().tous().size() == 5);
    std::set<std::pair<int, int>> cases;
    for (const ObjetAuSol& objet : partie.getSol().tous()) {
        const sf::Vector2i coordCase = carte.caseContenant(objet.position);
        REQUIRE(carte.getTile(coordCase.x, coordCase.y) == Tile::Floor);
        cases.insert({coordCase.x, coordCase.y});
    }
    REQUIRE(cases.size() == 5);
    // Le premier objet tombe sur la case du monstre (la case (3, 1)).
    REQUIRE(carte.caseContenant(partie.getSol().tous().front().position) == sf::Vector2i(3, 1));
}

TEST_CASE("butin : les identifiants sont tous differents, d'un monstre a l'autre") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}, {180.0f, 100.0f}}), 100, 2);

    toutTuer(partie);

    REQUIRE(partie.getSol().tous().size() == 4);
    std::set<int> identifiants;
    for (const ObjetAuSol& objet : partie.getSol().tous()) {
        REQUIRE(objet.id >= 1);
        identifiants.insert(objet.id);
    }
    REQUIRE(identifiants.size() == 4);
}

TEST_CASE("butin : les monstres vivants ne laissent rien") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 3);

    avancer(partie, 5);

    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("butin : une partie neuve (F5, nouveau niveau, recommencement) n'a aucun objet au sol") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 3);
    toutTuer(partie);
    REQUIRE_FALSE(partie.getSol().tous().empty());

    partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 3);

    REQUIRE(partie.getSol().tous().empty());
}

TEST_CASE("butin : la meme graine laisse le meme butin") {
    Partie a = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 3);
    Partie b = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 3);

    toutTuer(a);
    toutTuer(b);

    REQUIRE(a.getSol().tous() == b.getSol().tous());
}

TEST_CASE("butin : les categories du jeu laissent ce que leur definition annonce") {
    // Les chances sont des données (« à tester ») : ce test ne les fige pas, il vérifie que la créature laisse toujours.
    const DefinitionDeMonstre& creature = definitionDeMonstre(CategorieDeMonstre::CreatureMonstrueuse);
    REQUIRE(creature.chanceDeButin == 100);
    REQUIRE(creature.nombreDObjets >= 1);
}

// Le survol (étape 97.4) : `Partie::objetAuSolSous`.
namespace {
// Une partie où le mage, qui voit à 50 px seulement, tue d'un coup d'explosion un monstre hors de sa vue, à 240 px de
// l'escouade (hors de la détection du monstre, à portée du skill) : son butin tombe sur une case inconnue.
Partie partieOuLeButinTombeDansLInconnu(sf::Vector2f* positionDuMonstre) {
    constexpr float s_rayonDeVueCourt = 50.0f;
    std::vector<DefinitionPersonnage> composition = compositionDeTest();
    for (DefinitionPersonnage& definition : composition) {
        definition.rayonVision = s_rayonDeVueCourt;
    }
    *positionDuMonstre = {440.0f, 200.0f};
    // Un second monstre, loin dans un coin, pour que la partie ne soit pas gagnée (une partie gagnée se fige).
    Partie partie = partieDeTest(niveauDegage({*positionDuMonstre, {570.0f, 570.0f}}), std::move(composition), s_graineDeTest,
                                 monstreQuiLaisse(100, 1));
    partie.lancerSkillDeZone(s_adresseExplosion, *positionDuMonstre);
    avancer(partie, nombreDeTics(2.0f));
    return partie;
}
}  // namespace

TEST_CASE("objetAuSolSous : le curseur sur un objet explore le designe, un peu a cote aussi, loin non") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 1);
    toutTuer(partie);
    REQUIRE(partie.getSol().tous().size() == 1);
    const ObjetAuSol& objet = partie.getSol().tous().front();

    REQUIRE(partie.objetAuSolSous(objet.position) == &objet);
    REQUIRE(partie.objetAuSolSous(objet.position + sf::Vector2f(s_rayonDeSurvolDObjet - 1.0f, 0.0f)) == &objet);
    REQUIRE(partie.objetAuSolSous(objet.position + sf::Vector2f(0.0f, s_rayonDeSurvolDObjet + 1.0f)) == nullptr);
}

TEST_CASE("objetAuSolSous : entre deux objets, le plus proche du curseur") {
    Partie partie = partieDeButin(salleAvecMonstres({{140.0f, 60.0f}}), 100, 2);
    toutTuer(partie);
    REQUIRE(partie.getSol().tous().size() == 2);
    const ObjetAuSol& premier = partie.getSol().tous()[0];
    const ObjetAuSol& second = partie.getSol().tous()[1];
    // Deux cases voisines : leurs centres sont à 40 px, le curseur à 10 px du second les départage même si le premier est à 30 px.
    const sf::Vector2f pres = second.position + (premier.position - second.position) * 0.1f;

    REQUIRE(partie.objetAuSolSous(second.position) == &second);
    REQUIRE(partie.objetAuSolSous(pres) == &second);
    REQUIRE(partie.objetAuSolSous(premier.position) == &premier);
}

TEST_CASE("objetAuSolSous : un objet sur une case inconnue est cache, il se designe une fois la case explore, meme hors de vue") {
    sf::Vector2f positionDuMonstre;
    Partie partie = partieOuLeButinTombeDansLInconnu(&positionDuMonstre);
    REQUIRE(partie.getMonstres().size() == 1);
    REQUIRE(partie.getSol().tous().size() == 1);
    const sf::Vector2f positionDeLObjet = partie.getSol().tous().front().position;

    REQUIRE(partie.objetAuSolSous(positionDeLObjet) == nullptr);  // inconnu : caché

    partie.ordonnerDeplacement(positionDeLObjet);
    for (int i = 0; i < nombreDeTics(10.0f) && partie.objetAuSolSous(positionDeLObjet) == nullptr; ++i) {
        partie.mettreAJour(s_dureeTic);
    }
    REQUIRE(partie.objetAuSolSous(positionDeLObjet) != nullptr);  // vu : exploré

    partie.ordonnerDeplacement({100.0f, 200.0f});  // l'escouade s'éloigne : la case reste explorée, hors de vue
    avancer(partie, nombreDeTics(10.0f));
    REQUIRE(partie.objetAuSolSous(positionDeLObjet) != nullptr);
}
