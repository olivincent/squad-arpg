// ./tests/monde/test_GenerateurDonjon.cpp
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "base/Aleatoire.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monde/Chemin.hpp"
#include "monde/CompositionDeGroupe.hpp"
#include "monde/GenerateurDonjon.hpp"
#include "monde/Niveau.hpp"
#include "monde/TileMap.hpp"

namespace {

constexpr Generateur::result_type s_graine = 12345;

bool memesSalles(const std::vector<Salle>& a, const std::vector<Salle>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].x != b[i].x || a[i].y != b[i].y || a[i].largeur != b[i].largeur ||
            a[i].hauteur != b[i].hauteur) {
            return false;
        }
    }
    return true;
}

// Des paramètres où chaque salle reçoit de `min` à `max` infanteries, et rien d'autre : ni nuée, ni mélange, ni créature.
ParametresDonjon parametresAvecInfanterie(int min, int max) {
    ParametresDonjon parametres;
    parametres.compositions = {{.effectifs = {{CategorieDeMonstre::Infanterie, min, max}}, .poids = 1}};
    parametres.compositionDeLaSalleLaPlusLoin.reset();
    return parametres;
}

// Des paramètres où chaque salle reçoit cette composition, sans créature.
ParametresDonjon parametresAvec(const CompositionDeGroupe& composition) {
    ParametresDonjon parametres;
    parametres.compositions = {composition};
    parametres.compositionDeLaSalleLaPlusLoin.reset();
    return parametres;
}

// Le nombre de monstres de chaque catégorie, par groupe.
std::map<int, std::map<CategorieDeMonstre, int>> categoriesParGroupe(const std::vector<MonstreDuNiveau>& monstres) {
    std::map<int, std::map<CategorieDeMonstre, int>> resultat;
    for (const MonstreDuNiveau& monstre : monstres) {
        ++resultat[monstre.groupe][monstre.categorie];
    }
    return resultat;
}

int nombreDe(CategorieDeMonstre categorie, const std::vector<MonstreDuNiveau>& monstres) {
    return static_cast<int>(std::ranges::count_if(monstres, [&](const MonstreDuNiveau& monstre) {
        return monstre.categorie == categorie;
    }));
}

}  // namespace

TEST_CASE("Salle centre est la case au milieu de l'emprise") {
    const Salle salle{3, 5, 6, 8};

    REQUIRE(salle.centre() == sf::Vector2i(6, 9));
}

TEST_CASE("Salle chevauche detecte deux salles qui se recouvrent") {
    const Salle a{2, 2, 6, 6};
    const Salle b{5, 5, 6, 6};

    REQUIRE(a.chevauche(b, 0));
    REQUIRE(b.chevauche(a, 0));
}

TEST_CASE("Salle chevauche ne detecte pas deux salles separees") {
    const Salle a{2, 2, 6, 6};
    const Salle b{20, 2, 6, 6};

    REQUIRE_FALSE(a.chevauche(b, 1));
    REQUIRE_FALSE(b.chevauche(a, 1));
}

TEST_CASE("Salle chevauche ne detecte pas deux salles qui se touchent sans marge") {
    const Salle a{2, 2, 6, 6};
    const Salle b{8, 2, 6, 6};

    REQUIRE_FALSE(a.chevauche(b, 0));
    REQUIRE_FALSE(b.chevauche(a, 0));
}

TEST_CASE("Salle chevauche exige la marge entre deux salles") {
    const Salle a{2, 2, 6, 6};
    const Salle touche{8, 2, 6, 6};
    const Salle uneCaseDeMur{9, 2, 6, 6};

    REQUIRE(a.chevauche(touche, 1));
    REQUIRE_FALSE(a.chevauche(uneCaseDeMur, 1));
    REQUIRE(a.chevauche(uneCaseDeMur, 2));
}

TEST_CASE("Salle chevauche tient compte de la verticale") {
    const Salle a{2, 2, 6, 6};
    const Salle dessous{2, 8, 6, 6};

    REQUIRE_FALSE(a.chevauche(dessous, 0));
    REQUIRE(a.chevauche(dessous, 1));
}

TEST_CASE("placerSalles place au moins une salle") {
    Generateur generateur(s_graine);

    REQUIRE_FALSE(placerSalles(generateur, ParametresDonjon{}).empty());
}

TEST_CASE("placerSalles ne chevauche aucune paire, marge comprise") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);
        const std::vector<Salle> salles = placerSalles(generateur, parametres);

        for (std::size_t i = 0; i < salles.size(); ++i) {
            for (std::size_t j = i + 1; j < salles.size(); ++j) {
                REQUIRE_FALSE(salles[i].chevauche(salles[j], parametres.marge));
            }
        }
    }
}

TEST_CASE("placerSalles garde les emprises dans la carte sans toucher le bord") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        for (const Salle& salle : placerSalles(generateur, parametres)) {
            REQUIRE(salle.x >= 1);
            REQUIRE(salle.y >= 1);
            REQUIRE(salle.x + salle.largeur <= parametres.largeurCarte - 1);
            REQUIRE(salle.y + salle.hauteur <= parametres.hauteurCarte - 1);
        }
    }
}

TEST_CASE("placerSalles respecte les bornes de taille") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        for (const Salle& salle : placerSalles(generateur, parametres)) {
            REQUIRE(salle.largeur >= parametres.coteMin);
            REQUIRE(salle.largeur <= parametres.coteMax);
            REQUIRE(salle.hauteur >= parametres.coteMin);
            REQUIRE(salle.hauteur <= parametres.coteMax);
        }
    }
}

TEST_CASE("placerSalles donne les memes salles pour la meme graine") {
    Generateur premier(s_graine);
    Generateur second(s_graine);

    REQUIRE(memesSalles(placerSalles(premier, ParametresDonjon{}), placerSalles(second, ParametresDonjon{})));
}

TEST_CASE("placerSalles remplit la carte jusqu'aux bornes quand elle est juste assez grande") {
    // Carte 8 x 8 : l'intérieur fait 6 x 6, une seule salle de côté 6 y tient.
    ParametresDonjon parametres;
    parametres.largeurCarte = 8;
    parametres.hauteurCarte = 8;
    parametres.coteMin = 6;
    parametres.coteMax = 6;
    Generateur generateur(s_graine);

    const std::vector<Salle> salles = placerSalles(generateur, parametres);

    REQUIRE(salles.size() == 1);
    REQUIRE(salles[0].x == 1);
    REQUIRE(salles[0].y == 1);
}

TEST_CASE("creuserSalle transforme l'emprise en sol et laisse le reste") {
    std::vector<std::string> plan(5, std::string(5, '#'));

    creuserSalle(plan, Salle{1, 1, 2, 3});

    REQUIRE(plan[0] == "#####");
    REQUIRE(plan[1] == "#..##");
    REQUIRE(plan[2] == "#..##");
    REQUIRE(plan[3] == "#..##");
    REQUIRE(plan[4] == "#####");
}

TEST_CASE("choisirLiaisons ne donne aucune liaison pour une seule salle") {
    REQUIRE(choisirLiaisons({Salle{1, 1, 6, 6}}).empty());
    REQUIRE(choisirLiaisons({}).empty());
}

TEST_CASE("choisirLiaisons donne n moins 1 liaisons, une par salle sauf la racine") {
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 6, 6}, {9, 1, 6, 6}, {1, 20, 6, 6}};

    const std::vector<Liaison> liaisons = choisirLiaisons(salles);

    REQUIRE(liaisons.size() == 3);
    for (std::size_t i = 0; i < liaisons.size(); ++i) {
        REQUIRE(liaisons[i].vers == i + 1);
        REQUIRE(liaisons[i].de < liaisons[i].vers);
    }
}

TEST_CASE("choisirLiaisons relie chaque salle a la plus proche parmi les precedentes") {
    // Centres (4, 4), (23, 4), (12, 4) : la salle 2 est plus proche de la 0 que de la 1.
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 6, 6}, {9, 1, 6, 6}};

    const std::vector<Liaison> liaisons = choisirLiaisons(salles);

    REQUIRE(liaisons[0].de == 0);
    REQUIRE(liaisons[1].de == 0);
}

TEST_CASE("choisirLiaisons garde la racine quand elle est la plus proche") {
    // Centres (4, 4), (44, 4), (14, 4) : la salle 2 est a 100 de la 0 et à 900 de la 1.
    const std::vector<Salle> salles{{1, 1, 6, 6}, {41, 1, 6, 6}, {11, 1, 6, 6}};

    const std::vector<Liaison> liaisons = choisirLiaisons(salles);

    REQUIRE(liaisons[0].de == 0);
    REQUIRE(liaisons[1].de == 0);
}

TEST_CASE("choisirLiaisons prefere la precedente la plus proche a la racine") {
    // Centres (4, 4), (14, 4), (24, 4) : la salle 2 est reliee a la 1, pas a la 0.
    const std::vector<Salle> salles{{1, 1, 6, 6}, {11, 1, 6, 6}, {21, 1, 6, 6}};

    REQUIRE(choisirLiaisons(salles)[1].de == 1);
}

TEST_CASE("choisirLiaisons choisit le plus petit indice a egalite") {
    // Centres (4, 4), (24, 4), (14, 4) : la salle 2 est a 10 des salles 0 et 1.
    const std::vector<Salle> salles{{1, 1, 6, 6}, {21, 1, 6, 6}, {11, 1, 6, 6}};

    REQUIRE(choisirLiaisons(salles)[1].de == 0);
}

TEST_CASE("creuserCouloir horizontal puis vertical dessine un L de deux cases") {
    std::vector<std::string> plan(10, std::string(10, '#'));

    creuserCouloir(plan, {2, 2}, {6, 6}, SensCouloir::HorizontalPuisVertical, 2);

    REQUIRE(plan[1] == "##########");
    REQUIRE(plan[2] == "##......##");
    REQUIRE(plan[3] == "##......##");
    REQUIRE(plan[4] == "######..##");
    REQUIRE(plan[5] == "######..##");
    REQUIRE(plan[6] == "######..##");
    REQUIRE(plan[7] == "######..##");
    REQUIRE(plan[8] == "##########");
}

TEST_CASE("creuserCouloir vertical puis horizontal dessine le L inverse") {
    std::vector<std::string> plan(10, std::string(10, '#'));

    creuserCouloir(plan, {2, 2}, {6, 6}, SensCouloir::VerticalPuisHorizontal, 2);

    REQUIRE(plan[1] == "##########");
    REQUIRE(plan[2] == "##..######");
    REQUIRE(plan[3] == "##..######");
    REQUIRE(plan[4] == "##..######");
    REQUIRE(plan[5] == "##..######");
    REQUIRE(plan[6] == "##......##");
    REQUIRE(plan[7] == "##......##");
    REQUIRE(plan[8] == "##########");
}

TEST_CASE("creuserCouloir va aussi vers la gauche et vers le haut") {
    std::vector<std::string> plan(10, std::string(10, '#'));

    creuserCouloir(plan, {6, 6}, {2, 2}, SensCouloir::HorizontalPuisVertical, 2);

    REQUIRE(plan[2] == "##..######");
    REQUIRE(plan[5] == "##..######");
    REQUIRE(plan[6] == "##......##");
    REQUIRE(plan[7] == "##......##");
    REQUIRE(plan[8] == "##########");
    REQUIRE(plan[1] == "##########");
}

TEST_CASE("creuserCouloir laisse du sol au depart et a l'arrivee, dans les deux sens") {
    for (const SensCouloir sens : {SensCouloir::HorizontalPuisVertical, SensCouloir::VerticalPuisHorizontal}) {
        std::vector<std::string> plan(10, std::string(10, '#'));

        creuserCouloir(plan, {2, 6}, {7, 1}, sens, 2);

        REQUIRE(plan[6][2] == '.');
        REQUIRE(plan[1][7] == '.');
    }
}

TEST_CASE("creuserCouloir a un coude plein") {
    std::vector<std::string> plan(10, std::string(10, '#'));

    creuserCouloir(plan, {2, 2}, {6, 6}, SensCouloir::HorizontalPuisVertical, 2);

    // Le coude est le carre de 2 x 2 en (6, 2).
    REQUIRE(plan[2][6] == '.');
    REQUIRE(plan[2][7] == '.');
    REQUIRE(plan[3][6] == '.');
    REQUIRE(plan[3][7] == '.');
}

TEST_CASE("creuserCouloir horizontal quand les centres sont alignes") {
    for (const SensCouloir sens : {SensCouloir::HorizontalPuisVertical, SensCouloir::VerticalPuisHorizontal}) {
        std::vector<std::string> plan(6, std::string(10, '#'));

        creuserCouloir(plan, {2, 2}, {6, 2}, sens, 2);

        REQUIRE(plan[1] == "##########");
        REQUIRE(plan[2] == "##......##");
        REQUIRE(plan[3] == "##......##");
        REQUIRE(plan[4] == "##########");
    }
}

TEST_CASE("creuserCouloir vertical quand les centres sont alignes") {
    for (const SensCouloir sens : {SensCouloir::HorizontalPuisVertical, SensCouloir::VerticalPuisHorizontal}) {
        std::vector<std::string> plan(8, std::string(6, '#'));

        creuserCouloir(plan, {2, 1}, {2, 5}, sens, 2);

        REQUIRE(plan[0] == "######");
        for (std::size_t ligne = 1; ligne <= 6; ++ligne) {
            REQUIRE(plan[ligne] == "##..##");
        }
        REQUIRE(plan[7] == "######");
    }
}

TEST_CASE("creuserCouloir respecte la largeur demandee") {
    std::vector<std::string> plan(8, std::string(8, '#'));

    creuserCouloir(plan, {1, 1}, {5, 1}, SensCouloir::HorizontalPuisVertical, 3);

    REQUIRE(plan[0] == "########");
    REQUIRE(plan[1] == "#.......");
    REQUIRE(plan[2] == "#.......");
    REQUIRE(plan[3] == "#.......");
    REQUIRE(plan[4] == "########");
}

TEST_CASE("genererNiveau relie le depart au centre de chaque salle") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 1; graine <= 5; ++graine) {
        Generateur generateur(graine);
        Generateur copie = generateur;

        const Niveau niveau = genererNiveau(generateur, parametres);

        const TileMap carte(niveau.plan, niveau.tailleCase);
        const sf::Vector2i depart = carte.caseContenant(niveau.departEscouade);
        for (const Salle& salle : placerSalles(copie, parametres)) {
            REQUIRE_FALSE(trouverChemin(carte, depart, salle.centre()).empty());
        }
    }
}

TEST_CASE("genererNiveau a des couloirs : plus de sol que les seules salles") {
    const ParametresDonjon parametres;
    Generateur generateur(s_graine);
    Generateur copie = generateur;

    const Niveau niveau = genererNiveau(generateur, parametres);

    std::vector<std::string> sallesSeules(static_cast<std::size_t>(parametres.hauteurCarte),
                                          std::string(static_cast<std::size_t>(parametres.largeurCarte), '#'));
    const std::vector<Salle> salles = placerSalles(copie, parametres);
    for (const Salle& salle : salles) {
        creuserSalle(sallesSeules, salle);
    }
    REQUIRE(salles.size() > 1);
    REQUIRE(niveau.plan != sallesSeules);
}

TEST_CASE("genererNiveau produit un plan aux bonnes dimensions") {
    const ParametresDonjon parametres;
    Generateur generateur(s_graine);

    const Niveau niveau = genererNiveau(generateur, parametres);

    REQUIRE(niveau.plan.size() == static_cast<std::size_t>(parametres.hauteurCarte));
    for (const std::string& ligne : niveau.plan) {
        REQUIRE(ligne.size() == static_cast<std::size_t>(parametres.largeurCarte));
    }
    REQUIRE(niveau.tailleCase == parametres.tailleCase);
}

TEST_CASE("genererNiveau creuse toutes les cases des salles") {
    const ParametresDonjon parametres;
    Generateur generateur(s_graine);
    Generateur copie = generateur;

    const Niveau niveau = genererNiveau(generateur, parametres);

    for (const Salle& salle : placerSalles(copie, parametres)) {
        for (int ligne = salle.y; ligne < salle.y + salle.hauteur; ++ligne) {
            for (int colonne = salle.x; colonne < salle.x + salle.largeur; ++colonne) {
                REQUIRE(niveau.plan[static_cast<std::size_t>(ligne)][static_cast<std::size_t>(colonne)] == '.');
            }
        }
    }
}

TEST_CASE("genererNiveau laisse le bord de la carte en mur") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);
        const Niveau niveau = genererNiveau(generateur, parametres);

        const std::size_t derniereLigne = niveau.plan.size() - 1;
        for (std::size_t colonne = 0; colonne < niveau.plan[0].size(); ++colonne) {
            REQUIRE(niveau.plan[0][colonne] == '#');
            REQUIRE(niveau.plan[derniereLigne][colonne] == '#');
        }
        for (const std::string& ligne : niveau.plan) {
            REQUIRE(ligne.front() == '#');
            REQUIRE(ligne.back() == '#');
        }
    }
}

TEST_CASE("genererNiveau place l'escouade au centre de la premiere salle") {
    const ParametresDonjon parametres;
    Generateur generateur(s_graine);
    Generateur copie = generateur;

    const Niveau niveau = genererNiveau(generateur, parametres);

    const sf::Vector2i centre = placerSalles(copie, parametres).front().centre();
    const TileMap carte(niveau.plan, niveau.tailleCase);
    REQUIRE(carte.caseContenant(niveau.departEscouade) == centre);
    REQUIRE(carte.getTile(centre.x, centre.y) == Tile::Floor);
    REQUIRE(niveau.departEscouade == carte.centreDeCase(centre));
}

TEST_CASE("genererNiveau place des monstres") {
    bool auMoinsUn = false;
    for (Generateur::result_type graine = 1; graine <= 5; ++graine) {
        Generateur generateur(graine);
        auMoinsUn = auMoinsUn || !genererNiveau(generateur, ParametresDonjon{}).monstres.empty();
    }

    REQUIRE(auMoinsUn);
}

TEST_CASE("genererNiveau place chaque monstre sur du sol, hors de la premiere salle, loin du depart") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);
        Generateur copie = generateur;

        const Niveau niveau = genererNiveau(generateur, parametres);

        const TileMap carte(niveau.plan, niveau.tailleCase);
        const Salle premiere = placerSalles(copie, parametres).front();
        for (const MonstreDuNiveau& monstre : niveau.monstres) {
            const sf::Vector2f position = monstre.position;
            const sf::Vector2i coord = carte.caseContenant(position);
            REQUIRE(carte.getTile(coord.x, coord.y) == Tile::Floor);
            REQUIRE(carte.centreDeCase(coord) == position);
            const bool dansPremiere = coord.x >= premiere.x && coord.x < premiere.x + premiere.largeur &&
                                      coord.y >= premiere.y && coord.y < premiere.y + premiere.hauteur;
            REQUIRE_FALSE(dansPremiere);
            const sf::Vector2f ecart = position - niveau.departEscouade;
            REQUIRE(ecart.x * ecart.x + ecart.y * ecart.y >=
                    parametres.distanceMinDepart * parametres.distanceMinDepart);
        }
    }
}

TEST_CASE("genererNiveau donne les memes monstres pour la meme graine") {
    Generateur premier(s_graine);
    Generateur second(s_graine);

    REQUIRE(genererNiveau(premier, ParametresDonjon{}).monstres ==
            genererNiveau(second, ParametresDonjon{}).monstres);
}

TEST_CASE("placerMonstres ne place aucun monstre dans la salle 0") {
    const std::vector<Salle> salles{{1, 1, 12, 12}};
    Generateur generateur(s_graine);

    REQUIRE(placerMonstres(generateur, salles, {0.0f, 0.0f}, ParametresDonjon{}).empty());
}

TEST_CASE("placerMonstres place chaque monstre au centre d'une case interieure d'une salle") {
    const ParametresDonjon parametres;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 8, 7}, {1, 20, 12, 12}, {30, 20, 6, 6}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        for (const MonstreDuNiveau& monstre : placerMonstres(generateur, salles, {100.0f, 100.0f}, parametres)) {
            const sf::Vector2f position = monstre.position;
            const float colonne = position.x / parametres.tailleCase - 0.5f;
            const float ligne = position.y / parametres.tailleCase - 0.5f;
            REQUIRE(colonne == std::floor(colonne));
            REQUIRE(ligne == std::floor(ligne));
            bool dansUneSalle = false;
            for (std::size_t i = 1; i < salles.size(); ++i) {
                const Salle& s = salles[i];
                dansUneSalle = dansUneSalle || (colonne >= static_cast<float>(s.x + 1) &&
                                                colonne <= static_cast<float>(s.x + s.largeur - 2) &&
                                                ligne >= static_cast<float>(s.y + 1) &&
                                                ligne <= static_cast<float>(s.y + s.hauteur - 2));
            }
            REQUIRE(dansUneSalle);
        }
    }
}

TEST_CASE("placerMonstres place au plus max monstres par salle") {
    ParametresDonjon parametres = parametresAvecInfanterie(1, 2);
    const Salle salle{20, 20, 12, 12};
    const std::vector<Salle> salles{{1, 1, 6, 6}, salle};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        REQUIRE(placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres).size() <= 2);
    }
}

TEST_CASE("placerMonstres laisse au moins une case libre entre deux monstres") {
    ParametresDonjon parametres = parametresAvecInfanterie(6, 6);
    parametres.tentativesParMonstre = 50;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        for (std::size_t i = 0; i < monstres.size(); ++i) {
            for (std::size_t j = i + 1; j < monstres.size(); ++j) {
                const float ecartX = std::abs(monstres[i].position.x - monstres[j].position.x);
                const float ecartY = std::abs(monstres[i].position.y - monstres[j].position.y);
                REQUIRE(std::max(ecartX, ecartY) >= 2.0f * parametres.tailleCase);
            }
        }
    }
}

TEST_CASE("placerMonstres abandonne les monstres qui ne tiennent pas") {
    // Salle 6 x 6 : l'intérieur fait 4 x 4 cases, donc 4 monstres au plus à 2 cases d'écart.
    ParametresDonjon parametres = parametresAvecInfanterie(10, 10);
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 6, 6}};
    Generateur generateur(s_graine);

    const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

    REQUIRE(monstres.size() <= 4);
}

TEST_CASE("placerMonstres tient les monstres a la distance minimale du depart") {
    const ParametresDonjon parametres;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {10, 1, 12, 12}, {30, 1, 12, 12}};
    const sf::Vector2f depart{100.0f, 100.0f};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        for (const MonstreDuNiveau& monstre : placerMonstres(generateur, salles, depart, parametres)) {
            const sf::Vector2f position = monstre.position;
            const sf::Vector2f ecart = position - depart;
            REQUIRE(ecart.x * ecart.x + ecart.y * ecart.y >=
                    parametres.distanceMinDepart * parametres.distanceMinDepart);
        }
    }
}

TEST_CASE("placerMonstres ne place rien dans une salle entierement proche du depart") {
    // Les centres des cases intérieures de la salle 1 sont à moins de 100 px du départ.
    const std::vector<Salle> salles{{1, 1, 6, 6}, {18, 8, 6, 6}};
    const sf::Vector2f depart{840.0f, 440.0f};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        REQUIRE(placerMonstres(generateur, salles, depart, ParametresDonjon{}).empty());
    }
}

TEST_CASE("placerMonstres donne les memes positions pour la meme graine") {
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}, {40, 5, 10, 10}};
    Generateur premier(s_graine);
    Generateur second(s_graine);

    REQUIRE(placerMonstres(premier, salles, {0.0f, 0.0f}, ParametresDonjon{}) ==
            placerMonstres(second, salles, {0.0f, 0.0f}, ParametresDonjon{}));
}

TEST_CASE("genererNiveau donne le meme plan pour la meme graine") {
    Generateur premier(s_graine);
    Generateur second(s_graine);

    REQUIRE(genererNiveau(premier, ParametresDonjon{}).plan == genererNiveau(second, ParametresDonjon{}).plan);
}

TEST_CASE("genererNiveau produit un plan que TileMap accepte") {
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);
        const Niveau niveau = genererNiveau(generateur, ParametresDonjon{});

        REQUIRE_NOTHROW(TileMap(niveau.plan, niveau.tailleCase));
    }
}

TEST_CASE("placerMonstres donne le meme groupe aux monstres d'une meme salle") {
    ParametresDonjon parametres = parametresAvecInfanterie(3, 3);
    parametres.tentativesParMonstre = 50;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        REQUIRE(monstres.size() >= 2);
        for (const MonstreDuNiveau& monstre : monstres) {
            REQUIRE(monstre.groupe == monstres.front().groupe);
        }
    }
}

TEST_CASE("placerMonstres donne des groupes differents aux monstres de salles differentes") {
    ParametresDonjon parametres = parametresAvecInfanterie(2, 2);
    parametres.tentativesParMonstre = 50;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}, {40, 5, 10, 10}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        // Deux monstres par salle, deux salles : les deux premiers vont ensemble, pas avec les deux suivants.
        REQUIRE(monstres.size() == 4);
        REQUIRE(monstres[0].groupe == monstres[1].groupe);
        REQUIRE(monstres[2].groupe == monstres[3].groupe);
        REQUIRE(monstres[0].groupe != monstres[2].groupe);
    }
}

TEST_CASE("placerMonstres ne saute aucun numero de groupe quand une salle reste vide") {
    // La salle 1 est entierement proche du depart : elle n'a aucun monstre, la salle 2 reçoit le groupe 0.
    const std::vector<Salle> salles{{1, 1, 6, 6}, {18, 8, 6, 6}, {40, 20, 12, 12}};
    const sf::Vector2f depart{840.0f, 440.0f};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        for (const MonstreDuNiveau& monstre : placerMonstres(generateur, salles, depart, ParametresDonjon{})) {
            REQUIRE(monstre.groupe == 0);
        }
    }
}
TEST_CASE("placerPortes : un couloir perpendiculaire a une salle recoit une porte") {
    const std::vector<std::string> plan = {
        "############",
        "#...########",
        "#.........##",
        "#.........##",
        "############",
    };

    const std::vector<PorteDuNiveau> portes = placerPortes(plan, {Salle{1, 1, 3, 3}}, 2);

    REQUIRE(portes == std::vector<PorteDuNiveau>{PorteDuNiveau{{{4, 2}, {4, 3}}}});
}

TEST_CASE("placerPortes : une ouverture de 3 cases n'a pas de porte") {
    const std::vector<std::string> plan = {
        "############",
        "#..........#",
        "#..........#",
        "#..........#",
        "############",
    };

    REQUIRE(placerPortes(plan, {Salle{1, 1, 3, 3}}, 2).empty());
}

TEST_CASE("placerPortes : un couloir qui traverse une salle a une porte a l'entree et une a la sortie") {
    const std::vector<std::string> plan = {
        "############",
        "####...#####",
        "#..........#",
        "#..........#",
        "####...#####",
        "############",
    };

    const std::vector<PorteDuNiveau> portes = placerPortes(plan, {Salle{4, 1, 3, 4}}, 2);

    REQUIRE(portes == std::vector<PorteDuNiveau>{PorteDuNiveau{{{3, 2}, {3, 3}}}, PorteDuNiveau{{{7, 2}, {7, 3}}}});
}

TEST_CASE("placerPortes : une porte vue des deux salles n'est donnee qu'une fois") {
    const std::vector<std::string> plan = {
        "#########",
        "#...#...#",
        "#.......#",
        "#.......#",
        "#########",
    };

    const std::vector<PorteDuNiveau> portes = placerPortes(plan, {Salle{1, 1, 3, 3}, Salle{5, 1, 3, 3}}, 2);

    REQUIRE(portes == std::vector<PorteDuNiveau>{PorteDuNiveau{{{4, 2}, {4, 3}}}});
}

TEST_CASE("genererNiveau : portes sur du sol, alignees, et salles toutes reliees portes ouvertes") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 1; graine <= 5; ++graine) {
        Generateur generateur(graine);
        Generateur copie = generateur;

        const Niveau niveau = genererNiveau(generateur, parametres);

        TileMap carte(niveau.plan, niveau.tailleCase);
        for (const PorteDuNiveau& porte : niveau.portes) {
            REQUIRE(static_cast<int>(porte.cases.size()) == parametres.largeurCouloir);
            for (const sf::Vector2i coordCase : porte.cases) {
                REQUIRE(carte.getTile(coordCase.x, coordCase.y) == Tile::Floor);
                carte.placerPorte(coordCase);
                carte.ouvrirPorte(coordCase);
            }
        }
        const sf::Vector2i depart = carte.caseContenant(niveau.departEscouade);
        for (const Salle& salle : placerSalles(copie, parametres)) {
            REQUIRE_FALSE(trouverChemin(carte, depart, salle.centre()).empty());
        }
    }
}

TEST_CASE("genererNiveau : une graine redonne les memes portes") {
    const ParametresDonjon parametres;
    Generateur premier(s_graine);
    Generateur second(s_graine);

    REQUIRE(genererNiveau(premier, parametres).portes == genererNiveau(second, parametres).portes);
}

TEST_CASE("genererNiveau : place des portes") {
    bool auMoinsUne = false;
    for (Generateur::result_type graine = 1; graine <= 5; ++graine) {
        Generateur generateur(graine);
        auMoinsUne = auMoinsUne || !genererNiveau(generateur, ParametresDonjon{}).portes.empty();
    }

    REQUIRE(auMoinsUne);
}

TEST_CASE("contientUnCoinDiagonal detecte le damier dans les deux orientations") {
    REQUIRE(contientUnCoinDiagonal({
        "....",
        ".#..",
        "..#.",
        "....",
    }));
    REQUIRE(contientUnCoinDiagonal({
        "....",
        "..#.",
        ".#..",
        "....",
    }));
}

TEST_CASE("contientUnCoinDiagonal ne detecte rien dans un plan sans damier") {
    // Mur plein, mur en L, couloir : aucun carré de 2 x 2 n'est un damier.
    REQUIRE_FALSE(contientUnCoinDiagonal({
        "#####",
        "#...#",
        "#.#.#",
        "#...#",
        "#####",
    }));
    REQUIRE_FALSE(contientUnCoinDiagonal({
        "###.",
        "##..",
        "#...",
        "....",
    }));
    REQUIRE_FALSE(contientUnCoinDiagonal({"..", ".."}));
    REQUIRE_FALSE(contientUnCoinDiagonal({"##", "##"}));
    REQUIRE_FALSE(contientUnCoinDiagonal({}));
    REQUIRE_FALSE(contientUnCoinDiagonal({"#.#."}));  // une seule ligne : aucun carré
}

TEST_CASE("contientUnCoinDiagonal detecte le damier sur le bord du plan") {
    REQUIRE(contientUnCoinDiagonal({
        "#.",
        ".#",
    }));
    REQUIRE(contientUnCoinDiagonal({
        "...",
        ".#.",
        "#.#",
    }));  // coin bas-gauche : le carré (0,1)-(1,2) est un damier
    REQUIRE(contientUnCoinDiagonal({
        "..#.",
        "...#",
    }));  // coin haut-droit
}

// Échantillon, pas garantie : un sondage de 20 000 graines (hors tests) en trouve 145 qui contiennent
// le motif, la première étant la 391. Les graines 1 à 50 n'en ont pas ; voir docs/vision.md.
TEST_CASE("genererNiveau ne contient aucun coin diagonal pour les graines 1 a 50") {
    int avecCoin = 0;
    Generateur::result_type premiere = 0;
    for (Generateur::result_type graine = 1; graine <= 50; ++graine) {
        Generateur generateur(graine);
        const Niveau niveau = genererNiveau(generateur, ParametresDonjon{});
        if (contientUnCoinDiagonal(niveau.plan)) {
            if (avecCoin == 0) {
                premiere = graine;
            }
            ++avecCoin;
        }
    }

    INFO(avecCoin << " graines sur 50 contiennent un coin diagonal, la premiere est " << premiere);
    REQUIRE(avecCoin == 0);
}

TEST_CASE("placerMonstres : chaque groupe suit l'une des compositions, aucune categorie hors d'elles") {
    const ParametresDonjon parametres;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 12, 12}, {1, 20, 12, 12}, {40, 30, 12, 8}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {100.0f, 100.0f}, parametres);

        for (const auto& [groupe, categories] : categoriesParGroupe(monstres)) {
            std::vector<CompositionDeGroupe> candidates = parametres.compositions;
            candidates.push_back(*parametres.compositionDeLaSalleLaPlusLoin);
            const bool uneComposition = std::ranges::any_of(candidates, [&](const CompositionDeGroupe& composition) {
                return std::ranges::all_of(categories, [&](const auto& entree) {
                    const auto effectif = std::ranges::find_if(composition.effectifs, [&](const EffectifDeCategorie& e) {
                        return e.categorie == entree.first;
                    });
                    return effectif != composition.effectifs.end() && entree.second <= effectif->max;
                });
            });
            INFO("groupe " << groupe);
            REQUIRE(uneComposition);
        }
    }
}

TEST_CASE("placerMonstres : un melange donne une infanterie lourde et un ou deux infanteries par salle") {
    const CompositionDeGroupe melange = compositionsOrdinaires()[2];
    const ParametresDonjon parametres = parametresAvec(melange);
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        REQUIRE(nombreDe(CategorieDeMonstre::InfanterieLourde, monstres) == 1);
        REQUIRE(nombreDe(CategorieDeMonstre::Infanterie, monstres) >= 1);
        REQUIRE(nombreDe(CategorieDeMonstre::Infanterie, monstres) <= 2);
        REQUIRE(monstres.size() == static_cast<std::size_t>(nombreDe(CategorieDeMonstre::Infanterie, monstres) + 1));
    }
}

TEST_CASE("placerMonstres : une nuee est groupee autour de son premier monstre, une case par monstre") {
    const CompositionDeGroupe nuee = compositionsOrdinaires()[0];
    const ParametresDonjon parametres = parametresAvec(nuee);
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        REQUIRE(monstres.size() >= 1);
        for (const MonstreDuNiveau& monstre : monstres) {
            REQUIRE(monstre.categorie == CategorieDeMonstre::Nuee);
        }
        const EffectifDeCategorie& effectif = nuee.effectifs.front();
        REQUIRE(monstres.size() <= static_cast<std::size_t>(effectif.max));
        for (std::size_t i = 0; i < monstres.size(); ++i) {
            const float ecartAuPremierX = std::abs(monstres[i].position.x - monstres[0].position.x);
            const float ecartAuPremierY = std::abs(monstres[i].position.y - monstres[0].position.y);
            REQUIRE(std::max(ecartAuPremierX, ecartAuPremierY) <= 2.0f * parametres.tailleCase);
            for (std::size_t j = i + 1; j < monstres.size(); ++j) {
                REQUIRE(monstres[i].position != monstres[j].position);
            }
        }
    }
}

TEST_CASE("placerMonstres : une nuee a toujours son nombre complet dans une salle assez grande") {
    const CompositionDeGroupe nuee = compositionsOrdinaires()[0];
    ParametresDonjon parametres = parametresAvec(nuee);
    parametres.tentativesParMonstre = 200;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 12, 12}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        REQUIRE(monstres.size() >= static_cast<std::size_t>(nuee.effectifs.front().min));
    }
}

TEST_CASE("placerMonstres : une salle trop petite pour toute une nuee en recoit autant qu'elle en tient") {
    // Salle 4 x 3 : l'intérieur fait 2 x 1 cases.
    const CompositionDeGroupe nuee = compositionsOrdinaires()[0];
    ParametresDonjon parametres = parametresAvec(nuee);
    parametres.tentativesParMonstre = 200;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 20, 4, 3}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        REQUIRE(monstres.size() == 2);
        REQUIRE(monstres[0].position != monstres[1].position);
    }
}

TEST_CASE("placerMonstres : la creature est seule, dans la salle la plus eloignee de la salle 0") {
    const ParametresDonjon parametres;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 8, 7}, {1, 20, 12, 12}, {40, 30, 8, 8}};  // la 3 est la plus loin
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres);

        REQUIRE(nombreDe(CategorieDeMonstre::CreatureMonstrueuse, monstres) == 1);
        const auto creature = std::ranges::find(monstres, CategorieDeMonstre::CreatureMonstrueuse, &MonstreDuNiveau::categorie);
        const float colonne = creature->position.x / parametres.tailleCase - 0.5f;
        const float ligne = creature->position.y / parametres.tailleCase - 0.5f;
        REQUIRE(colonne >= 41.0f);  // dans la salle 3
        REQUIRE(ligne >= 31.0f);
        for (const MonstreDuNiveau& monstre : monstres) {
            if (monstre.groupe == creature->groupe) {
                REQUIRE(monstre.categorie == CategorieDeMonstre::CreatureMonstrueuse);
            }
        }
    }
}

TEST_CASE("placerMonstres : a egalite de distance, la creature va dans la plus petite salle") {
    // Les salles 1 et 2 sont à la même distance de la salle 0 (19 cases, l'une à droite, l'autre en dessous).
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 6, 6}, {1, 20, 6, 6}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, {0.0f, 0.0f}, ParametresDonjon{});

        const auto creature = std::ranges::find(monstres, CategorieDeMonstre::CreatureMonstrueuse, &MonstreDuNiveau::categorie);
        REQUIRE(creature != monstres.end());
        REQUIRE(creature->position.x > creature->position.y);  // la salle 1 est à droite de la salle 0
    }
}

TEST_CASE("placerMonstres : sans composition pour la salle la plus loin, aucune creature") {
    ParametresDonjon parametres;
    parametres.compositionDeLaSalleLaPlusLoin.reset();
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 8, 7}, {1, 20, 12, 12}, {40, 30, 8, 8}};
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        REQUIRE(nombreDe(CategorieDeMonstre::CreatureMonstrueuse,
                         placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres)) == 0);
    }
}

TEST_CASE("placerMonstres : les trois compositions ordinaires sortent, dans les salles ordinaires") {
    const ParametresDonjon parametres;
    const std::vector<Salle> salles{{1, 1, 6, 6}, {20, 1, 12, 12}, {1, 20, 12, 12}, {40, 30, 12, 8}};
    int nuees = 0;
    int infanteries = 0;
    int melanges = 0;
    for (Generateur::result_type graine = 0; graine < 200; ++graine) {
        Generateur generateur(graine);

        for (const auto& [groupe, categories] : categoriesParGroupe(placerMonstres(generateur, salles, {0.0f, 0.0f}, parametres))) {
            if (categories.contains(CategorieDeMonstre::Nuee)) {
                ++nuees;
            } else if (categories.contains(CategorieDeMonstre::InfanterieLourde)) {
                ++melanges;
            } else if (categories.contains(CategorieDeMonstre::Infanterie)) {
                ++infanteries;
            }
        }
    }

    // 400 salles ordinaires tirées à 30 %, 40 % et 30 % : en voir moins d'une de chaque est de l'ordre de 0,7^400.
    REQUIRE(nuees > 0);
    REQUIRE(infanteries > 0);
    REQUIRE(melanges > 0);
}

TEST_CASE("genererNiveau : au plus une creature") {
    const ParametresDonjon parametres;
    for (Generateur::result_type graine = 0; graine < 50; ++graine) {
        Generateur generateur(graine);

        const Niveau niveau = genererNiveau(generateur, parametres);

        REQUIRE(nombreDe(CategorieDeMonstre::CreatureMonstrueuse, niveau.monstres) <= 1);
    }
}
