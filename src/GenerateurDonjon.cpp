// ./src/GenerateurDonjon.cpp
#include "GenerateurDonjon.hpp"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <random>

namespace {

constexpr char s_mur = '#';
constexpr char s_sol = '.';
constexpr int s_ecartMinEntreMonstres = 2; // en cases (Tchebychev) : une case libre entre deux monstres

// Centre en pixels du monde d'une case de la grille.
sf::Vector2f centreEnPixels(sf::Vector2i coordCase, float tailleCase) {
    return {(static_cast<float>(coordCase.x) + 0.5f) * tailleCase, (static_cast<float>(coordCase.y) + 0.5f) * tailleCase};
}

// Tirage dans [min, max], bornes INCLUSES (comme std::uniform_int_distribution).
int tirerEntre(Generateur& generateur, int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generateur);
}

// Distance au carré entre les centres de deux salles, en cases : suffisante
// pour comparer, sans racine carrée.
int distanceCarreeEntreCentres(const Salle& a, const Salle& b) {
    const sf::Vector2i ecart = a.centre() - b.centre();
    return ecart.x * ecart.x + ecart.y * ecart.y;
}

// Transforme en sol le rectangle de cases [colonneMin, colonneMax] x [ligneMin, ligneMax], bornes incluses.
void creuserRectangle(std::vector<std::string>& plan, int colonneMin, int colonneMax, int ligneMin, int ligneMax) {
    for (int ligne = ligneMin; ligne <= ligneMax; ++ligne) {
        for (int colonne = colonneMin; colonne <= colonneMax; ++colonne) {
            plan[static_cast<std::size_t>(ligne)][static_cast<std::size_t>(colonne)] = s_sol;
        }
    }
}

}  // namespace

sf::Vector2i Salle::centre() const {
    return {x + largeur / 2, y + hauteur / 2};
}

bool Salle::chevauche(const Salle& autre, int marge) const {
    return x < autre.x + autre.largeur + marge && autre.x < x + largeur + marge &&
           y < autre.y + autre.hauteur + marge && autre.y < y + hauteur + marge;
}

std::vector<Salle> placerSalles(Generateur& generateur, const ParametresDonjon& parametres) {
    assert(parametres.tentatives >= 1);
    assert(parametres.coteMin >= 1 && parametres.coteMin <= parametres.coteMax);
    // Une salle de côté maximal doit tenir à l'intérieur de la bordure de mur.
    assert(parametres.coteMax <= parametres.largeurCarte - 2);
    assert(parametres.coteMax <= parametres.hauteurCarte - 2);

    std::vector<Salle> salles;
    for (int tentative = 0; tentative < parametres.tentatives; ++tentative) {
        const int largeur = tirerEntre(generateur, parametres.coteMin, parametres.coteMax);
        const int hauteur = tirerEntre(generateur, parametres.coteMin, parametres.coteMax);
        // Colonnes 1 à (largeurCarte - 1 - largeur) : la dernière case de la
        // salle est au plus à largeurCarte - 2, la bordure reste en mur.
        const Salle candidate{tirerEntre(generateur, 1, parametres.largeurCarte - 1 - largeur),
                              tirerEntre(generateur, 1, parametres.hauteurCarte - 1 - hauteur), largeur,
                              hauteur};
        bool libre = true;
        for (const Salle& gardee : salles) {
            if (candidate.chevauche(gardee, parametres.marge)) {
                libre = false;
                break;
            }
        }
        if (libre) {
            salles.push_back(candidate);
        }
    }
    return salles;
}

void creuserSalle(std::vector<std::string>& plan, const Salle& salle) {
    for (int ligne = salle.y; ligne < salle.y + salle.hauteur; ++ligne) {
        for (int colonne = salle.x; colonne < salle.x + salle.largeur; ++colonne) {
            plan[static_cast<std::size_t>(ligne)][static_cast<std::size_t>(colonne)] = s_sol;
        }
    }
}

std::vector<Liaison> choisirLiaisons(const std::vector<Salle>& salles) {
    std::vector<Liaison> liaisons;
    for (std::size_t i = 1; i < salles.size(); ++i) {
        std::size_t plusProche = 0;
        int distanceMin = distanceCarreeEntreCentres(salles[i], salles[0]);
        for (std::size_t j = 1; j < i; ++j) {
            const int distance = distanceCarreeEntreCentres(salles[i], salles[j]);
            if (distance < distanceMin) {  // strict : à égalité, le plus petit indice reste
                distanceMin = distance;
                plusProche = j;
            }
        }
        liaisons.push_back({plusProche, i});
    }
    return liaisons;
}

void creuserCouloir(std::vector<std::string>& plan, sf::Vector2i depart, sf::Vector2i arrivee, SensCouloir sens,
                    int largeur) {
    assert(largeur >= 1);
    const int colonneMin = std::min(depart.x, arrivee.x);
    const int colonneMax = std::max(depart.x, arrivee.x) + largeur - 1;
    const int ligneMin = std::min(depart.y, arrivee.y);
    const int ligneMax = std::max(depart.y, arrivee.y) + largeur - 1;

    // Deux bandes qui se croisent au coude, qui est donc plein.
    if (sens == SensCouloir::HorizontalPuisVertical) {
        creuserRectangle(plan, colonneMin, colonneMax, depart.y, depart.y + largeur - 1);
        creuserRectangle(plan, arrivee.x, arrivee.x + largeur - 1, ligneMin, ligneMax);
    } else {
        creuserRectangle(plan, depart.x, depart.x + largeur - 1, ligneMin, ligneMax);
        creuserRectangle(plan, colonneMin, colonneMax, arrivee.y, arrivee.y + largeur - 1);
    }
}

std::vector<sf::Vector2f> placerMonstres(Generateur& generateur, const std::vector<Salle>& salles,
                                         sf::Vector2f depart, const ParametresDonjon& parametres) {
    assert(parametres.monstresParSalleMin >= 0 && parametres.monstresParSalleMin <= parametres.monstresParSalleMax);
    assert(parametres.tentativesParMonstre >= 1);

    std::vector<sf::Vector2i> casesPlacees;
    std::vector<sf::Vector2f> positions;
    for (std::size_t i = 1; i < salles.size(); ++i) {
        const Salle& salle = salles[i];
        // Une case de marge contre les murs de l'emprise.
        assert(salle.largeur >= 3 && salle.hauteur >= 3);
        const int voulus = tirerEntre(generateur, parametres.monstresParSalleMin, parametres.monstresParSalleMax);
        for (int k = 0; k < voulus; ++k) {
            for (int tentative = 0; tentative < parametres.tentativesParMonstre; ++tentative) {
                const sf::Vector2i candidate{tirerEntre(generateur, salle.x + 1, salle.x + salle.largeur - 2),
                                             tirerEntre(generateur, salle.y + 1, salle.y + salle.hauteur - 2)};
                const sf::Vector2f centre = centreEnPixels(candidate, parametres.tailleCase);
                const sf::Vector2f versDepart = centre - depart;
                const bool tropPresDuDepart =
                    versDepart.x * versDepart.x + versDepart.y * versDepart.y <
                    parametres.distanceMinDepart * parametres.distanceMinDepart;
                const bool tropPresDUnMonstre =
                    std::any_of(casesPlacees.begin(), casesPlacees.end(), [&](sf::Vector2i placee) {
                        return std::max(std::abs(placee.x - candidate.x), std::abs(placee.y - candidate.y)) <
                               s_ecartMinEntreMonstres;
                    });
                if (!tropPresDuDepart && !tropPresDUnMonstre) {
                    casesPlacees.push_back(candidate);
                    positions.push_back(centre);
                    break;
                }
            }
        }
    }
    return positions;
}

Niveau genererNiveau(Generateur& generateur, const ParametresDonjon& parametres) {
    const std::vector<Salle> salles = placerSalles(generateur, parametres);

    std::vector<std::string> plan(static_cast<std::size_t>(parametres.hauteurCarte),
                                  std::string(static_cast<std::size_t>(parametres.largeurCarte), s_mur));
    for (const Salle& salle : salles) {
        creuserSalle(plan, salle);
    }
    // Les couloirs après les salles ; un tirage du sens par liaison.
    for (const Liaison& liaison : choisirLiaisons(salles)) {
        const SensCouloir sens = tirerEntre(generateur, 0, 1) == 0 ? SensCouloir::HorizontalPuisVertical
                                                                   : SensCouloir::VerticalPuisHorizontal;
        creuserCouloir(plan, salles[liaison.de].centre(), salles[liaison.vers].centre(), sens,
                       parametres.largeurCouloir);
    }

    // Centre en pixels de la case centrale de la première salle.
    const sf::Vector2f depart = centreEnPixels(salles.front().centre(), parametres.tailleCase);
    // Les monstres en dernier : leurs tirages ne changent ni les salles ni les couloirs.
    return {plan, parametres.tailleCase, depart, placerMonstres(generateur, salles, depart, parametres), 0};
}
