// ./src/interface/DispositionInventaire.cpp
#include "interface/DispositionInventaire.hpp"
#include "combat/Caracteristiques.hpp"
#include "escouade/TailleDeLEscouade.hpp"

#include <algorithm>
#include <cassert>

namespace {

// Les positions verticales se comptent depuis le haut du panneau (`hautDuPanneau`).
constexpr float s_largeurDeColonne = 340.0f;
constexpr float s_ecartColonneEtSac = 24.0f;
constexpr float s_margeLaterale = 16.0f;  // entre le bord du panneau et son contenu
constexpr float s_hautDesOnglets = 58.0f;  // sous le titre
constexpr float s_hauteurOnglet = 32.0f;
constexpr float s_ecartEntreOnglets = 4.0f;
constexpr float s_hauteurArmure = 24.0f;
constexpr float s_ecartVertical = 6.0f;  // entre les onglets, l'armure et les emplacements
constexpr float s_largeurEmplacement = 160.0f;
constexpr float s_hauteurEmplacement = 40.0f;
constexpr float s_ecartEntreEmplacements = 6.0f;
constexpr std::size_t s_emplacementsParColonne = 5;

constexpr float s_cote = 44.0f;  // une case du sac
constexpr float s_ecartEntreCases = 4.0f;
constexpr float s_hauteurEtiquetteDuSac = 24.0f;
constexpr float s_hauteurDeLigneDeCaracteristique = 18.0f;
constexpr float s_largeurDuBoutonDeCaracteristique = 22.0f;  // « + » et « − », de la hauteur de la ligne
constexpr float s_largeurDuBoutonDeDepense = 64.0f;          // « Apply » et « Reset »
constexpr float s_ecartEntreBoutons = 4.0f;
constexpr float s_margeDesBoutons = 6.0f;  // entre le dernier bouton d'une ligne et le bord de la colonne
constexpr float s_margeBasse = 14.0f;  // entre le bas du contenu et le bord du panneau

constexpr float s_decalageInfobulle = 16.0f;  // entre le curseur et le coin de l'infobulle

static_assert(s_nombreEmplacements == 2 * s_emplacementsParColonne);
static_assert(s_casesDuSac % s_colonnesDuSac == 0);

// De haut en bas, la colonne du personnage ouvert (depuis le haut du panneau).
constexpr float s_hautDeLArmure = s_hautDesOnglets + s_hauteurOnglet + s_ecartVertical;
constexpr float s_hautDesEmplacements = s_hautDeLArmure + s_hauteurArmure + s_ecartVertical;
constexpr float s_hautDesCaracteristiques =
    s_hautDesEmplacements + static_cast<float>(s_emplacementsParColonne) * s_hauteurEmplacement +
    static_cast<float>(s_emplacementsParColonne - 1) * s_ecartEntreEmplacements + s_ecartVertical;
constexpr float s_hautDesStatistiques =
    s_hautDesCaracteristiques + static_cast<float>(s_lignesDeCaracteristiques) * s_hauteurDeLigneDeCaracteristique + s_ecartVertical;
constexpr float s_basDeLaColonne = s_hautDesStatistiques + static_cast<float>(s_lignesDeStatistiques) * s_hauteurDeLigneDeCaracteristique;

// À droite de la colonne, le sac : son étiquette, puis ses cases.
constexpr float s_largeurDuSac = static_cast<float>(s_colonnesDuSac) * (s_cote + s_ecartEntreCases) - s_ecartEntreCases;
constexpr float s_hautDuSac = s_hautDesOnglets + s_hauteurEtiquetteDuSac;
constexpr float s_rangeesDuSac = static_cast<float>(s_casesDuSac / s_colonnesDuSac);
constexpr float s_basDuSac = s_hautDuSac + s_rangeesDuSac * (s_cote + s_ecartEntreCases) - s_ecartEntreCases;

constexpr float s_largeurDuPanneau = s_largeurDeColonne + s_ecartColonneEtSac + s_largeurDuSac + 2.0f * s_margeLaterale;
constexpr float s_hauteurDuPanneau = std::max(s_basDeLaColonne, s_basDuSac) + s_margeBasse;

// Le haut du panneau, qui est centré dans la vue.
float hautDuPanneau(sf::Vector2f tailleDeVue) {
    return (tailleDeVue.y - s_hauteurDuPanneau) / 2.0f;
}

float gaucheDuPanneau(sf::Vector2f tailleDeVue) {
    return (tailleDeVue.x - s_largeurDuPanneau) / 2.0f;
}

// Le bord gauche de la colonne (et des onglets).
float xDeLaColonne(sf::Vector2f tailleDeVue) {
    return gaucheDuPanneau(tailleDeVue) + s_margeLaterale;
}

}  // namespace

sf::FloatRect rectangleDuPanneau(sf::Vector2f tailleDeVue) {
    return {{gaucheDuPanneau(tailleDeVue), hautDuPanneau(tailleDeVue)}, {s_largeurDuPanneau, s_hauteurDuPanneau}};
}

sf::FloatRect rectangleDeLOnglet(Place place, sf::Vector2f tailleDeVue) {
    assert(place.numero >= 1 && place.numero <= s_tailleDeLEscouade);
    const float nombre = static_cast<float>(s_tailleDeLEscouade);
    const float largeur = (s_largeurDeColonne - (nombre - 1.0f) * s_ecartEntreOnglets) / nombre;
    return {{xDeLaColonne(tailleDeVue) + static_cast<float>(place.indice()) * (largeur + s_ecartEntreOnglets),
             hautDuPanneau(tailleDeVue) + s_hautDesOnglets},
            {largeur, s_hauteurOnglet}};
}

std::optional<Place> ongletSous(sf::Vector2f point, sf::Vector2f tailleDeVue) {
    for (const Place place : s_places) {
        if (rectangleDeLOnglet(place, tailleDeVue).contains(point)) {
            return place;
        }
    }
    return std::nullopt;
}

sf::FloatRect rectangleDeLArmure(sf::Vector2f tailleDeVue) {
    return {{xDeLaColonne(tailleDeVue), hautDuPanneau(tailleDeVue) + s_hautDeLArmure}, {s_largeurDeColonne, s_hauteurArmure}};
}

sf::FloatRect rectangleDeLEmplacement(Emplacement emplacement, sf::Vector2f tailleDeVue) {
    const std::size_t indice = static_cast<std::size_t>(emplacement);
    const std::size_t sousColonne = indice / s_emplacementsParColonne;
    const std::size_t rangee = indice % s_emplacementsParColonne;
    const float largeurDesDeux = 2.0f * s_largeurEmplacement + s_ecartEntreEmplacements;
    const float x = xDeLaColonne(tailleDeVue) + (s_largeurDeColonne - largeurDesDeux) / 2.0f +
                    static_cast<float>(sousColonne) * (s_largeurEmplacement + s_ecartEntreEmplacements);
    const float y = hautDuPanneau(tailleDeVue) + s_hautDesEmplacements + static_cast<float>(rangee) * (s_hauteurEmplacement + s_ecartEntreEmplacements);
    return {{x, y}, {s_largeurEmplacement, s_hauteurEmplacement}};
}

sf::FloatRect rectangleDesCaracteristiques(sf::Vector2f tailleDeVue) {
    return {{xDeLaColonne(tailleDeVue), hautDuPanneau(tailleDeVue) + s_hautDesCaracteristiques},
            {s_largeurDeColonne, static_cast<float>(s_lignesDeCaracteristiques) * s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDeLaCellule(std::size_t indice, sf::Vector2f tailleDeVue) {
    assert(indice <= s_nombreDeCaracteristiques);
    const sf::FloatRect bloc = rectangleDesCaracteristiques(tailleDeVue);
    return {{bloc.position.x, bloc.position.y + static_cast<float>(indice) * s_hauteurDeLigneDeCaracteristique},
            {bloc.size.x, s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDuBoutonPlus(Caracteristique caracteristique, sf::Vector2f tailleDeVue) {
    const sf::FloatRect ligne = rectangleDeLaCellule(static_cast<std::size_t>(caracteristique), tailleDeVue);
    return {{ligne.position.x + ligne.size.x - s_margeDesBoutons - s_largeurDuBoutonDeCaracteristique, ligne.position.y},
            {s_largeurDuBoutonDeCaracteristique, ligne.size.y}};
}

sf::FloatRect rectangleDuBoutonMoins(Caracteristique caracteristique, sf::Vector2f tailleDeVue) {
    const sf::FloatRect plus = rectangleDuBoutonPlus(caracteristique, tailleDeVue);
    return {{plus.position.x - s_ecartEntreBoutons - s_largeurDuBoutonDeCaracteristique, plus.position.y}, plus.size};
}

sf::FloatRect rectangleDuBoutonReinitialiser(sf::Vector2f tailleDeVue) {
    const sf::FloatRect ligne = rectangleDeLaCellule(s_nombreDeCaracteristiques, tailleDeVue);
    return {{ligne.position.x + ligne.size.x - s_margeDesBoutons - s_largeurDuBoutonDeDepense, ligne.position.y},
            {s_largeurDuBoutonDeDepense, ligne.size.y}};
}

sf::FloatRect rectangleDuBoutonAppliquer(sf::Vector2f tailleDeVue) {
    const sf::FloatRect reinitialiser = rectangleDuBoutonReinitialiser(tailleDeVue);
    return {{reinitialiser.position.x - s_ecartEntreBoutons - s_largeurDuBoutonDeDepense, reinitialiser.position.y}, reinitialiser.size};
}

sf::FloatRect rectangleDesStatistiques(sf::Vector2f tailleDeVue) {
    return {{xDeLaColonne(tailleDeVue), hautDuPanneau(tailleDeVue) + s_hautDesStatistiques},
            {s_largeurDeColonne, static_cast<float>(s_lignesDeStatistiques) * s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDeLaStatistique(std::size_t indice, sf::Vector2f tailleDeVue) {
    assert(indice < s_cellulesDeStatistiques);
    const sf::FloatRect bloc = rectangleDesStatistiques(tailleDeVue);
    const float largeurDeCellule = bloc.size.x / 2.0f;
    const std::size_t colonne = indice / s_lignesDeStatistiques;
    const std::size_t rangee = indice % s_lignesDeStatistiques;
    return {{bloc.position.x + static_cast<float>(colonne) * largeurDeCellule,
             bloc.position.y + static_cast<float>(rangee) * s_hauteurDeLigneDeCaracteristique},
            {largeurDeCellule, s_hauteurDeLigneDeCaracteristique}};
}

sf::FloatRect rectangleDeLaCaseDuSac(std::size_t indice, sf::Vector2f tailleDeVue) {
    assert(indice < s_casesDuSac);
    const std::size_t colonne = indice % s_colonnesDuSac;
    const std::size_t rangee = indice / s_colonnesDuSac;
    const float x = xDeLaColonne(tailleDeVue) + s_largeurDeColonne + s_ecartColonneEtSac + static_cast<float>(colonne) * (s_cote + s_ecartEntreCases);
    const float y = hautDuPanneau(tailleDeVue) + s_hautDuSac + static_cast<float>(rangee) * (s_cote + s_ecartEntreCases);
    return {{x, y}, {s_cote, s_cote}};
}

sf::FloatRect rectangleDeLEtiquetteDuSac(sf::Vector2f tailleDeVue) {
    return {{xDeLaColonne(tailleDeVue) + s_largeurDeColonne + s_ecartColonneEtSac, hautDuPanneau(tailleDeVue) + s_hautDesOnglets},
            {s_largeurDuSac, s_hauteurEtiquetteDuSac}};
}

std::optional<BoutonDePoints> boutonSous(sf::Vector2f point, Place ouverte, sf::Vector2f tailleDeVue) {
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (rectangleDuBoutonPlus(caracteristique, tailleDeVue).contains(point)) {
            return BoutonPlus{ouverte, caracteristique};
        }
        if (rectangleDuBoutonMoins(caracteristique, tailleDeVue).contains(point)) {
            return BoutonMoins{ouverte, caracteristique};
        }
    }
    if (rectangleDuBoutonAppliquer(tailleDeVue).contains(point)) {
        return BoutonAppliquer{ouverte};
    }
    if (rectangleDuBoutonReinitialiser(tailleDeVue).contains(point)) {
        return BoutonReinitialiser{ouverte};
    }
    return std::nullopt;
}

std::optional<CaracteristiqueSurvolee> caracteristiqueSous(sf::Vector2f point, Place ouverte, sf::Vector2f tailleDeVue) {
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (rectangleDeLaCellule(static_cast<std::size_t>(caracteristique), tailleDeVue).contains(point)) {
            return CaracteristiqueSurvolee{ouverte, caracteristique};
        }
    }
    return std::nullopt;
}

std::optional<PositionDObjet> elementSous(sf::Vector2f point, Place ouverte, sf::Vector2f tailleDeVue) {
    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        const Emplacement emplacement = static_cast<Emplacement>(i);
        if (rectangleDeLEmplacement(emplacement, tailleDeVue).contains(point)) {
            return PositionPortee{ouverte, emplacement};
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
