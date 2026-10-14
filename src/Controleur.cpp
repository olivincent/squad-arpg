// ./src/Controleur.cpp
#include "Controleur.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include "Cercle.hpp"
#include "Partie.hpp"

namespace {
// Place du lanceur du skill de zone, qui se vise avant de se lancer.
constexpr std::size_t s_placeDuSkillDeZone = 4;
}  // namespace

void Controleur::debutImage(const Partie& partie) {
    // Lanceur mort ou escouade vaincue : on sort de la visée.
    if (m_placeEnVisee.has_value() && !partie.getEscouade().skillEstPret(*m_placeEnVisee)) {
        m_placeEnVisee.reset();
    }
}

void Controleur::appuiGauche(Partie& partie, sf::Vector2f point) {
    if (m_placeEnVisee.has_value()) {
        // En visée, le clic gauche lance le sort (même sur un monstre), réussi
        // ou refusé, et sort de la visée ; rien d'autre.
        partie.lancerSkillDeZone(*m_placeEnVisee, point);
        m_placeEnVisee.reset();
        return;
    }
    const ResultatClic resultat = partie.cliquer(point);
    if (resultat == ResultatClic::Monstre) {
        m_deplacementContinu = false;
    } else if (resultat == ResultatClic::Sol) {
        m_deplacementContinu = true;
    }
}

void Controleur::appuiDroit() {
    m_placeEnVisee.reset();
}

void Controleur::appuiTouche(Partie& partie, std::size_t place) {
    if (place >= 1 && place <= 3) {
        partie.demanderSkill(place);
    } else if (place == s_placeDuSkillDeZone) {
        if (m_placeEnVisee.has_value()) {
            m_placeEnVisee.reset();
        } else if (partie.getEscouade().estVivante() && partie.getEscouade().skillEstPret(place)) {
            m_placeEnVisee = place;
            // Le déplacement continu est suspendu : il ne reprendra qu'à un
            // nouvel appui sur le sol.
            m_deplacementContinu = false;
        }
    }
}

void Controleur::appuiEchap() {
    m_placeEnVisee.reset();
}

void Controleur::mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur) {
    if (boutonTenu && m_deplacementContinu) {
        partie.ordonnerDeplacement(curseur);
    }
}

void Controleur::dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const {
    if (!m_placeEnVisee.has_value()) {
        return;
    }
    // Cercle de prévisualisation au rayon exact de la zone, sous le curseur.
    const float rayon = partie.getEscouade().rayonZoneDuSkill(*m_placeEnVisee);
    sf::CircleShape apercu(rayon);
    apercu.setOrigin({rayon, rayon});
    apercu.setPosition(curseur);
    apercu.setFillColor(sf::Color::Transparent);
    apercu.setOutlineColor(sf::Color::Yellow);
    apercu.setOutlineThickness(2.0f);
    rendu.draw(apercu);

    // Cercle de portée de lancement autour du lanceur.
    const Cercle portee = partie.getEscouade().cercleDePorteeDuSkill(*m_placeEnVisee);
    sf::CircleShape contourPortee(portee.rayon);
    contourPortee.setOrigin({portee.rayon, portee.rayon});
    contourPortee.setPosition(portee.centre);
    contourPortee.setFillColor(sf::Color::Transparent);
    contourPortee.setOutlineColor(sf::Color(255, 255, 255, 160));
    contourPortee.setOutlineThickness(2.0f);
    rendu.draw(contourPortee);
}

std::optional<std::size_t> Controleur::getPlaceEnVisee() const {
    return m_placeEnVisee;
}
