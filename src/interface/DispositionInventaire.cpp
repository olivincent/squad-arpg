// ./src/interface/DispositionInventaire.cpp
#include "interface/DispositionInventaire.hpp"
#include "combat/Caracteristiques.hpp"

#include <algorithm>
#include <cassert>

namespace {

// Les positions verticales se comptent depuis le haut du panneau (`hautDuPanneau`).
constexpr float s_largeurDeColonne = 340.0f;
constexpr float s_ecartDeColonnes = 12.0f;
constexpr float s_margeLaterale = 16.0f;  // entre le bord du panneau et les colonnes
constexpr float s_hautDesColonnes = 58.0f;  // sous le titre
constexpr float s_hauteurEnTete = 30.0f;
constexpr float s_hauteurArmure = 24.0f;
constexpr float s_ecartVertical = 6.0f;  // entre l'en-tête, l'armure et les emplacements
constexpr float s_largeurEmplacement = 160.0f;
constexpr float s_hauteurEmplacement = 40.0f;
constexpr float s_ecartEntreEmplacements = 6.0f;
constexpr std::size_t s_emplacementsParColonne = 5;

constexpr float s_cote = 44.0f;  // une case du sac
constexpr float s_ecartEntreCases = 4.0f;
constexpr float s_hauteurDeLigneDeCaracteristique = 18.0f;
constexpr float s_hautDuSac = 498.0f;  // sous le bloc des statistiques (fin à 468) et l'étiquette « Sac » (24)
constexpr float s_margeBasse = 14.0f;  // entre la dernière rangée du sac et le bord du panneau

constexpr float s_decalageInfobulle = 16.0f;  // entre le curseur et le coin de l'infobulle

static_assert(s_nombreEmplacements == 2 * s_emplacementsParColonne);
static_assert(s_casesDuSac % s_colonnesDuSac == 0);

constexpr float s_largeurDesColonnes = static_cast<float>(s_nombreDePlaces) * s_largeurDeColonne +
                                       static_cast<float>(s_nombreDePlaces - 1) * s_ecartDeColonnes;
constexpr float s_rangeesDuSac = static_cast<float>(s_casesDuSac / s_colonnesDuSac);
constexpr float s_largeurDuPanneau = s_largeurDesColonnes + 2.0f * s_margeLaterale;
constexpr float s_hauteurDuPanneau = s_hautDuSac + s_rangeesDuSac * (s_cote + s_ecartEntreCases) - s_ecartEntreCases + s_margeBasse;

// Le haut du panneau, qui est centré dans la vue.
float hautDuPanneau(sf::Vector2f tailleDeVue) {
    return (tailleDeVue.y - s_hauteurDuPanneau) / 2.0f;
}

// Le bord gauche de la colonne de cette place, les colonnes étant centrées dans la vue.
float xDeLaColonne(std::size_t place, sf::Vector2f tailleDeVue) {
    assert(place >= 1 && place <= s_nombreDePlaces);
    return (tailleDeVue.x - s_largeurDesColonnes) / 2.0f + static_cast<float>(place - 1) * (s_largeurDeColonne + s_ecartDeColonnes);
}

}  // namespace

sf::FloatRect rectangleDuPanneau(sf::Vector2f tailleDeVue) {
    return {{(tailleDeVue.x - s_largeurDuPanneau) / 2.0f, hautDuPanneau(tailleDeVue)}, {s_largeurDuPanneau, s_hauteurDuPanneau}};
}

sf::FloatRect rectangleDeLEnTete(std::size_t place, sf::Vector2f tailleDeVue) {
    return {{xDeLaColonne(place, tailleDeVue), hautDuPanneau(tailleDeVue) + s_hautDesColonnes}, {s_largeurDeColonne, s_hauteurEnTete}};
}

sf::FloatRect rectangleDeLArmure(std::size_t place, sf::Vector2f tailleDeVue) {
    return {{xDeLaColonne(place, tailleDeVue), hautDuPanneau(tailleDeVue) + s_hautDesColonnes + s_hauteurEnTete + s_ecartVertical},
            {s_largeurDeColonne, s_hauteurArmure}};
}

sf::FloatRect rectangleDeLEmplacement(std::size_t place, Emplacement emplacement, sf::Vector2f tailleDeVue) {
    const std::size_t indice = static_cast<std::size_t>(emplacement);
    const std::size_t sousColonne = indice / s_emplacementsParColonne;
    const std::size_t rangee = indice % s_emplacementsParColonne;
    const float largeurDesDeux = 2.0f * s_largeurEmplacement + s_ecartEntreEmplacements;
    const float x = xDeLaColonne(place, tailleDeVue) + (s_largeurDeColonne - largeurDesDeux) / 2.0f +
                    static_cast<float>(sousColonne) * (s_largeurEmplacement + s_ecartEntreEmplacements);
    const float y = hautDuPanneau(tailleDeVue) + s_hautDesColonnes + s_hauteurEnTete + s_hauteurArmure + 2.0f * s_ecartVertical +
                    static_cast<float>(rangee) * (s_hauteurEmplacement + s_ecartEntreEmplacements);
    return {{x, y}, {s_largeurEmplacement, s_hauteurEmplacement}};
}

sf::FloatRect rectangleDesCaracteristiques(std::size_t place, sf::Vector2f tailleDeVue) {
    const float haut = hautDuPanneau(tailleDeVue) + s_hautDesColonnes + s_hauteurEnTete + s_hauteurArmure + 2.0f * s_ecartVertical +
                       static_cast<float>(s_emplacementsParColonne) * s_hauteurEmplacement +
                       static_cast<float>(s_emplacementsParColonne - 1) * s_ecartEntreEmplacements + s_ecartVertical;
    return {{xDeLaColonne(place, tailleDeVue), haut},
            {s_largeurDeColonne, static_cast<float>(s_lignesDeCaracteristiques) * s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDeLaCellule(std::size_t place, std::size_t indice, sf::Vector2f tailleDeVue) {
    assert(indice <= s_nombreDeCaracteristiques);
    const sf::FloatRect bloc = rectangleDesCaracteristiques(place, tailleDeVue);
    const float largeurDeCellule = bloc.size.x / 2.0f;
    const std::size_t colonne = indice / s_lignesDeCaracteristiques;
    const std::size_t rangee = indice % s_lignesDeCaracteristiques;
    return {{bloc.position.x + static_cast<float>(colonne) * largeurDeCellule,
             bloc.position.y + static_cast<float>(rangee) * s_hauteurDeLigneDeCaracteristique},
            {largeurDeCellule, s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDesStatistiques(std::size_t place, sf::Vector2f tailleDeVue) {
    const sf::FloatRect caracteristiques = rectangleDesCaracteristiques(place, tailleDeVue);
    return {{caracteristiques.position.x, caracteristiques.position.y + caracteristiques.size.y + s_ecartVertical},
            {s_largeurDeColonne, static_cast<float>(s_lignesDeStatistiques) * s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDeLaStatistique(std::size_t place, std::size_t indice, sf::Vector2f tailleDeVue) {
    assert(indice < s_cellulesDeStatistiques);
    const sf::FloatRect bloc = rectangleDesStatistiques(place, tailleDeVue);
    const float largeurDeCellule = bloc.size.x / 2.0f;
    const std::size_t colonne = indice / s_lignesDeStatistiques;
    const std::size_t rangee = indice % s_lignesDeStatistiques;
    return {{bloc.position.x + static_cast<float>(colonne) * largeurDeCellule,
             bloc.position.y + static_cast<float>(rangee) * s_hauteurDeLigneDeCaracteristique},
            {largeurDeCellule, s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDeLaCaseDuSac(std::size_t indice, sf::Vector2f tailleDeVue) {
    assert(indice < s_casesDuSac);
    const float largeurDuSac = static_cast<float>(s_colonnesDuSac) * (s_cote + s_ecartEntreCases) - s_ecartEntreCases;
    const std::size_t colonne = indice % s_colonnesDuSac;
    const std::size_t rangee = indice / s_colonnesDuSac;
    const float x = (tailleDeVue.x - largeurDuSac) / 2.0f + static_cast<float>(colonne) * (s_cote + s_ecartEntreCases);
    const float y = hautDuPanneau(tailleDeVue) + s_hautDuSac + static_cast<float>(rangee) * (s_cote + s_ecartEntreCases);
    return {{x, y}, {s_cote, s_cote}};
}

std::optional<PositionDObjet> elementSous(sf::Vector2f point, sf::Vector2f tailleDeVue) {
    for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
        for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
            const Emplacement emplacement = static_cast<Emplacement>(i);
            if (rectangleDeLEmplacement(place, emplacement, tailleDeVue).contains(point)) {
                return PositionPortee{place, emplacement};
            }
        }
    }
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        if (rectangleDeLaCaseDuSac(indice, tailleDeVue).contains(point)) {
            return PositionDansLeSac{indice};
        }
    }
    return std::nullopt;
}

sf::FloatRect placerInfobulle(sf::Vector2f curseur, sf::Vector2f tailleInfobulle, sf::Vector2f tailleDeVue) {
    sf::Vector2f position = curseur + sf::Vector2f{s_decalageInfobulle, s_decalageInfobulle};
    if (position.x + tailleInfobulle.x > tailleDeVue.x) {
        position.x = curseur.x - s_decalageInfobulle - tailleInfobulle.x;
    }
    if (position.y + tailleInfobulle.y > tailleDeVue.y) {
        position.y = curseur.y - s_decalageInfobulle - tailleInfobulle.y;
    }
    position.x = std::clamp(position.x, 0.0f, tailleDeVue.x - tailleInfobulle.x);
    position.y = std::clamp(position.y, 0.0f, tailleDeVue.y - tailleInfobulle.y);
    return {position, tailleInfobulle};
}
