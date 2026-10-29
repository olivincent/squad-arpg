// ./src/interface/BarreDeRaccourcis.cpp
#include "interface/BarreDeRaccourcis.hpp"
#include <cassert>
#include "escouade/TailleDeLEscouade.hpp"

namespace {
constexpr std::size_t s_emplacementsParRangee = 4;  // aussi le nombre de colonnes : une par personnage
static_assert(s_tailleDeLEscouade <= s_emplacementsParRangee, "la barre a une colonne par personnage");
constexpr float s_coteDeLIcone = 48.0f;
constexpr float s_ecartEntreIcones = 4.0f;
constexpr float s_margeDuBas = 10.0f;
}  // namespace

BarreDeRaccourcis::BarreDeRaccourcis(std::size_t taille) : m_emplacements(taille) {}

std::size_t BarreDeRaccourcis::taille() const {
    return m_emplacements.size();
}

std::optional<AdresseSkill> BarreDeRaccourcis::adresse(std::size_t emplacement) const {
    assert(emplacement < taille());
    return m_emplacements[emplacement];
}

void BarreDeRaccourcis::placer(std::size_t emplacement, AdresseSkill adresse) {
    assert(emplacement < taille());
    m_emplacements[emplacement] = adresse;
}

void BarreDeRaccourcis::vider(std::size_t emplacement) {
    assert(emplacement < taille());
    m_emplacements[emplacement].reset();
}

BarreDeRaccourcis barreParDefaut(const std::vector<AdresseSkill>& adresses) {
    BarreDeRaccourcis barre(s_emplacementsPrincipaux);
    const std::size_t rangees = barre.taille() / s_emplacementsParRangee;
    std::vector<std::size_t> dejaPlaces(s_emplacementsParRangee, 0);  // par colonne (place - 1)
    for (const AdresseSkill& adresse : adresses) {
        if (adresse.place.numero < 1 || adresse.place.numero > s_emplacementsParRangee) {
            continue;
        }
        const std::size_t colonne = adresse.place.indice();
        if (dejaPlaces[colonne] >= rangees) {
            continue;
        }
        barre.placer(dejaPlaces[colonne] * s_emplacementsParRangee + colonne, adresse);
        ++dejaPlaces[colonne];
    }
    return barre;
}

sf::FloatRect rectangleDeLEmplacement(std::size_t emplacement, sf::Vector2f tailleVue) {
    assert(emplacement < s_emplacementsPrincipaux);
    const std::size_t rangees = s_emplacementsPrincipaux / s_emplacementsParRangee;
    const float pas = s_coteDeLIcone + s_ecartEntreIcones;
    const float largeur = static_cast<float>(s_emplacementsParRangee) * pas - s_ecartEntreIcones;
    const float hauteur = static_cast<float>(rangees) * pas - s_ecartEntreIcones;
    const float gauche = (tailleVue.x - largeur) / 2.0f;
    const float haut = tailleVue.y - s_margeDuBas - hauteur;
    const float colonne = static_cast<float>(emplacement % s_emplacementsParRangee);
    const float rangee = static_cast<float>(emplacement / s_emplacementsParRangee);
    return {{gauche + colonne * pas, haut + rangee * pas}, {s_coteDeLIcone, s_coteDeLIcone}};
}

std::optional<std::size_t> emplacementSous(sf::Vector2f point, sf::Vector2f tailleVue) {
    for (std::size_t i = 0; i < s_emplacementsPrincipaux; ++i) {
        if (rectangleDeLEmplacement(i, tailleVue).contains(point)) {
            return i;
        }
    }
    return std::nullopt;
}
