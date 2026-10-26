// ./src/monde/GenerateurDonjon.cpp
#include "monde/GenerateurDonjon.hpp"
#include "base/Aleatoire.hpp"
#include "monde/TileMap.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <random>
#include <utility>

namespace {

constexpr char s_mur = '#';
constexpr char s_sol = '.';
constexpr int s_ecartMinEntreMonstres = 2; // en cases (Tchebychev) : une case libre entre deux monstres
constexpr int s_ecartMinDansUneGrappe = 1; // en cases : une case par monstre
constexpr int s_etalementDUneGrappe = 2;   // en cases : distance maximale (Tchebychev) au premier monstre du groupe

// Centre en pixels du monde d'une case de la grille.
sf::Vector2f centreEnPixels(sf::Vector2i coordCase, float tailleCase) {
    return {(static_cast<float>(coordCase.x) + 0.5f) * tailleCase, (static_cast<float>(coordCase.y) + 0.5f) * tailleCase};
}

// Distance au carré entre les centres de deux salles, en cases : suffisante
// pour comparer, sans racine carrée.
int distanceCarreeEntreCentres(const Salle& a, const Salle& b) {
    const sf::Vector2i ecart = a.centre() - b.centre();
    return ecart.x * ecart.x + ecart.y * ecart.y;
}

// Les cases où un monstre peut apparaître, bornes incluses.
struct ZoneDePlacement {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    sf::Vector2i min;
    sf::Vector2i max;
};

// L'intérieur de la salle (une case de marge contre les murs de l'emprise), réduit à l'entourage de `autourDe`
// (sans sortir de la salle) pour une grappe déjà commencée.
ZoneDePlacement zoneDePlacement(const Salle& salle, std::optional<sf::Vector2i> autourDe) {
    ZoneDePlacement zone{{salle.x + 1, salle.y + 1}, {salle.x + salle.largeur - 2, salle.y + salle.hauteur - 2}};
    if (autourDe.has_value()) {
        zone.min = {std::max(zone.min.x, autourDe->x - s_etalementDUneGrappe),
                    std::max(zone.min.y, autourDe->y - s_etalementDUneGrappe)};
        zone.max = {std::min(zone.max.x, autourDe->x + s_etalementDUneGrappe),
                    std::min(zone.max.y, autourDe->y + s_etalementDUneGrappe)};
    }
    return zone;
}

// Une composition utilisable : au moins un effectif, chacun avec 0 <= min <= max (lue seulement par des `assert`).
[[maybe_unused]] bool compositionValide(const CompositionDeGroupe& composition) {
    return !composition.effectifs.empty() &&
           std::ranges::all_of(composition.effectifs, [](const EffectifDeCategorie& effectif) {
               return effectif.min >= 0 && effectif.min <= effectif.max;
           });
}

// Une composition au hasard, selon les poids.
const CompositionDeGroupe& tirerComposition(Generateur& generateur, const std::vector<CompositionDeGroupe>& compositions) {
    int poidsTotal = 0;
    for (const CompositionDeGroupe& composition : compositions) {
        poidsTotal += composition.poids;
    }
    int reste = tirerEntre(generateur, 0, poidsTotal - 1);
    for (const CompositionDeGroupe& composition : compositions) {
        if (reste < composition.poids) {
            return composition;
        }
        reste -= composition.poids;
    }
    assert(false);  // le tirage est dans [0, poidsTotal)
    return compositions.back();
}

// Indice de la salle (hors la 0) la plus éloignée de la salle 0, centre à centre ; la plus petite à égalité.
// Vide s'il n'y a pas d'autre salle.
std::optional<std::size_t> salleLaPlusEloignee(const std::vector<Salle>& salles) {
    std::optional<std::size_t> plusLoin;
    int distanceMax = -1;
    for (std::size_t i = 1; i < salles.size(); ++i) {
        const int distance = distanceCarreeEntreCentres(salles[i], salles[0]);
        if (distance > distanceMax) {
            distanceMax = distance;
            plusLoin = i;
        }
    }
    return plusLoin;
}

// Transforme en sol le rectangle de cases [colonneMin, colonneMax] x [ligneMin, ligneMax], bornes incluses.
void creuserRectangle(std::vector<std::string>& plan, int colonneMin, int colonneMax, int ligneMin, int ligneMax) {
    for (int ligne = ligneMin; ligne <= ligneMax; ++ligne) {
        for (int colonne = colonneMin; colonne <= colonneMax; ++colonne) {
            plan[static_cast<std::size_t>(ligne)][static_cast<std::size_t>(colonne)] = s_sol;
        }
    }
}

// Un côté de l'anneau de cases qui entoure une salle, sans les coins.
struct CoteDAnneau {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    sf::Vector2i debut;    // première case du côté
    sf::Vector2i pas;      // d'une case à la suivante
    int longueur;          // nombre de cases
    sf::Vector2i sortant;  // direction qui s'éloigne de la salle
};

// Vrai pour une case de sol du plan ; hors du plan, ce n'est pas du sol.
bool estSol(const std::vector<std::string>& plan, sf::Vector2i coordCase) {
    return coordCase.y >= 0 && coordCase.y < static_cast<int>(plan.size()) && coordCase.x >= 0 &&
           coordCase.x < static_cast<int>(plan[static_cast<std::size_t>(coordCase.y)].size()) &&
           plan[static_cast<std::size_t>(coordCase.y)][static_cast<std::size_t>(coordCase.x)] == s_sol;
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

std::vector<MonstreDuNiveau> placerMonstres(Generateur& generateur, const std::vector<Salle>& salles,
                                            sf::Vector2f depart, const ParametresDonjon& parametres) {
    assert(!parametres.compositions.empty());
    assert(std::ranges::all_of(parametres.compositions, [](const CompositionDeGroupe& composition) {
        return composition.poids > 0 && compositionValide(composition);
    }));
    assert(!parametres.compositionDeLaSalleLaPlusLoin || compositionValide(*parametres.compositionDeLaSalleLaPlusLoin));
    assert(parametres.tentativesParMonstre >= 1);

    const std::optional<std::size_t> plusLoin = salleLaPlusEloignee(salles);
    std::vector<sf::Vector2i> casesPlacees;
    std::vector<MonstreDuNiveau> monstres;
    int prochainGroupe = 0;  // pas l'indice de la salle : seules les salles peuplées comptent
    for (std::size_t i = 1; i < salles.size(); ++i) {
        const std::size_t dejaPlaces = monstres.size();
        const Salle& salle = salles[i];
        // Une case de marge contre les murs de l'emprise.
        assert(salle.largeur >= 3 && salle.hauteur >= 3);
        const bool laPlusLoin = plusLoin == i && parametres.compositionDeLaSalleLaPlusLoin.has_value();
        const CompositionDeGroupe& composition = laPlusLoin ? *parametres.compositionDeLaSalleLaPlusLoin
                                                            : tirerComposition(generateur, parametres.compositions);
        const int ecartMin = composition.serre ? s_ecartMinDansUneGrappe : s_ecartMinEntreMonstres;
        std::optional<sf::Vector2i> premier;  // le premier monstre placé du groupe, autour de qui se forme une grappe
        for (const EffectifDeCategorie& effectif : composition.effectifs) {
            const int voulus = tirerEntre(generateur, effectif.min, effectif.max);
            for (int k = 0; k < voulus; ++k) {
                const ZoneDePlacement zone =
                    zoneDePlacement(salle, composition.serre ? premier : std::optional<sf::Vector2i>());
                for (int tentative = 0; tentative < parametres.tentativesParMonstre; ++tentative) {
                    const sf::Vector2i candidate{tirerEntre(generateur, zone.min.x, zone.max.x),
                                                 tirerEntre(generateur, zone.min.y, zone.max.y)};
                    const sf::Vector2f centre = centreEnPixels(candidate, parametres.tailleCase);
                    const sf::Vector2f versDepart = centre - depart;
                    const bool tropPresDuDepart =
                        versDepart.x * versDepart.x + versDepart.y * versDepart.y <
                        parametres.distanceMinDepart * parametres.distanceMinDepart;
                    const bool tropPresDUnMonstre =
                        std::any_of(casesPlacees.begin(), casesPlacees.end(), [&](sf::Vector2i placee) {
                            return std::max(std::abs(placee.x - candidate.x), std::abs(placee.y - candidate.y)) < ecartMin;
                        });
                    if (!tropPresDuDepart && !tropPresDUnMonstre) {
                        casesPlacees.push_back(candidate);
                        monstres.push_back({centre, prochainGroupe, effectif.categorie});
                        if (!premier.has_value()) {
                            premier = candidate;
                        }
                        break;
                    }
                }
            }
        }
        if (monstres.size() > dejaPlaces) {
            ++prochainGroupe;
        }
    }
    return monstres;
}

sf::Vector2i placerEscalier(Generateur& generateur, const std::vector<Salle>& salles,
                            const std::vector<MonstreDuNiveau>& monstres, float tailleCase) {
    assert(!salles.empty());
    const Salle& salle = salles[salleLaPlusEloignee(salles).value_or(0)];
    const ZoneDePlacement zone = zoneDePlacement(salle, std::nullopt);
    const auto caseDuMonstre = [&](const MonstreDuNiveau& monstre) {
        return sf::Vector2i{static_cast<int>(std::floor(monstre.position.x / tailleCase)),
                            static_cast<int>(std::floor(monstre.position.y / tailleCase))};
    };
    std::vector<sf::Vector2i> libres;
    for (int ligne = zone.min.y; ligne <= zone.max.y; ++ligne) {
        for (int colonne = zone.min.x; colonne <= zone.max.x; ++colonne) {
            const sf::Vector2i candidate{colonne, ligne};
            if (!std::ranges::any_of(monstres, [&](const MonstreDuNiveau& monstre) { return caseDuMonstre(monstre) == candidate; })) {
                libres.push_back(candidate);
            }
        }
    }
    if (libres.empty()) {
        return salle.centre();
    }
    return libres[static_cast<std::size_t>(tirerEntre(generateur, 0, static_cast<int>(libres.size()) - 1))];
}

std::vector<PorteDuNiveau> placerPortes(const std::vector<std::string>& plan, const std::vector<Salle>& salles,
                                        int largeurCouloir) {
    assert(largeurCouloir >= 1);
    std::vector<PorteDuNiveau> portes;
    for (const Salle& salle : salles) {
        const std::array<CoteDAnneau, 4> cotes = {{
            {{salle.x, salle.y - 1}, {1, 0}, salle.largeur, {0, -1}},
            {{salle.x, salle.y + salle.hauteur}, {1, 0}, salle.largeur, {0, 1}},
            {{salle.x - 1, salle.y}, {0, 1}, salle.hauteur, {-1, 0}},
            {{salle.x + salle.largeur, salle.y}, {0, 1}, salle.hauteur, {1, 0}},
        }};
        for (const CoteDAnneau& cote : cotes) {
            int i = 0;
            while (i < cote.longueur) {
                if (!estSol(plan, cote.debut + cote.pas * i)) {
                    ++i;
                    continue;
                }
                int fin = i;  // première case après la suite de sol
                while (fin < cote.longueur && estSol(plan, cote.debut + cote.pas * fin)) {
                    ++fin;
                }
                PorteDuNiveau porte;
                bool couloirContinue = true;
                for (int k = i; k < fin; ++k) {
                    const sf::Vector2i coordCase = cote.debut + cote.pas * k;
                    porte.cases.push_back(coordCase);
                    couloirContinue = couloirContinue && estSol(plan, coordCase + cote.sortant);
                }
                const bool bonneLargeur = fin - i == largeurCouloir;
                const bool encadree = !estSol(plan, cote.debut + cote.pas * (i - 1)) && !estSol(plan, cote.debut + cote.pas * fin);
                if (bonneLargeur && encadree && couloirContinue && std::ranges::find(portes, porte) == portes.end()) {
                    portes.push_back(porte);
                }
                i = fin;
            }
        }
    }
    return portes;
}

bool contientUnCoinDiagonal(const std::vector<std::string>& plan) {
    for (std::size_t ligne = 0; ligne + 1 < plan.size(); ++ligne) {
        const std::size_t largeur = std::min(plan[ligne].size(), plan[ligne + 1].size());
        for (std::size_t colonne = 0; colonne + 1 < largeur; ++colonne) {
            const bool hautGaucheMur = plan[ligne][colonne] == s_mur;
            const bool hautDroiteMur = plan[ligne][colonne + 1] == s_mur;
            const bool basGaucheMur = plan[ligne + 1][colonne] == s_mur;
            const bool basDroiteMur = plan[ligne + 1][colonne + 1] == s_mur;
            // Damier : chaque case a la nature opposée de ses deux voisines orthogonales du carré.
            if (hautGaucheMur == basDroiteMur && hautDroiteMur == basGaucheMur && hautGaucheMur != hautDroiteMur) {
                return true;
            }
        }
    }
    return false;
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
    // Les portes ne tirent rien. Les monstres en dernier : leurs tirages ne changent ni les salles ni les couloirs.
    std::vector<PorteDuNiveau> portes = placerPortes(plan, salles, parametres.largeurCouloir);
    std::vector<MonstreDuNiveau> monstres = placerMonstres(generateur, salles, depart, parametres);
    // L'escalier en tout dernier : son tirage ne change ni les salles, ni les couloirs, ni les monstres.
    const sf::Vector2i escalier = placerEscalier(generateur, salles, monstres, parametres.tailleCase);
    plan[static_cast<std::size_t>(escalier.y)][static_cast<std::size_t>(escalier.x)] = s_caractereDEscalier;
    return {plan, parametres.tailleCase, depart, std::move(monstres), std::move(portes)};
}
