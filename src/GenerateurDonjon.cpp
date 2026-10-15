// ./src/GenerateurDonjon.cpp
#include "GenerateurDonjon.hpp"
#include <cassert>
#include <random>

namespace {

constexpr char s_mur = '#';
constexpr char s_sol = '.';

// Tirage dans [min, max], bornes INCLUSES (comme std::uniform_int_distribution).
int tirerEntre(Generateur& generateur, int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generateur);
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

Niveau genererNiveau(Generateur& generateur, const ParametresDonjon& parametres) {
    const std::vector<Salle> salles = placerSalles(generateur, parametres);

    std::vector<std::string> plan(static_cast<std::size_t>(parametres.hauteurCarte),
                                  std::string(static_cast<std::size_t>(parametres.largeurCarte), s_mur));
    for (const Salle& salle : salles) {
        creuserSalle(plan, salle);
    }

    // Centre en pixels de la case centrale de la première salle.
    const sf::Vector2i caseDepart = salles.front().centre();
    const sf::Vector2f depart{(static_cast<float>(caseDepart.x) + 0.5f) * parametres.tailleCase,
                              (static_cast<float>(caseDepart.y) + 0.5f) * parametres.tailleCase};
    return {plan, parametres.tailleCase, depart, {}, 0};
}
