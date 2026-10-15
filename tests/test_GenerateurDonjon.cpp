// ./tests/test_GenerateurDonjon.cpp
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "Aleatoire.hpp"
#include "Chemin.hpp"
#include "GenerateurDonjon.hpp"
#include "TileMap.hpp"

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

TEST_CASE("genererNiveau ne place aucun monstre") {
    Generateur generateur(s_graine);

    REQUIRE(genererNiveau(generateur, ParametresDonjon{}).positionsMonstres.empty());
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
