// ./src/Controleur.cpp
#include "Controleur.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <array>
#include "Cercle.hpp"
#include "Partie.hpp"
#include "TypeSkill.hpp"

namespace {
// Touches provisoires 1 à 4 (remplacées par la barre de raccourcis) : une adresse par touche, dans l'ordre.
constexpr std::array<AdresseSkill, Controleur::s_nombreDeTouches> s_touches = {{
    {1, IdSkill::FrappePuissante},
    {1, IdSkill::Provocation},
    {2, IdSkill::Soin},
    {3, IdSkill::Explosion},
}};
}  // namespace

std::optional<AdresseSkill> Controleur::adresseDeLaTouche(std::size_t indiceTouche) {
    if (indiceTouche < 1 || indiceTouche > s_touches.size()) {
        return std::nullopt;
    }
    return s_touches[indiceTouche - 1];
}

void Controleur::debutImage(const Partie& partie) {
    // Lanceur mort ou escouade vaincue : on sort de la visée.
    if (m_adresseEnVisee.has_value() && !partie.getEscouade().skillEstPret(*m_adresseEnVisee)) {
        m_adresseEnVisee.reset();
    }
}

void Controleur::appuiGauche(Partie& partie, sf::Vector2f point) {
    if (m_adresseEnVisee.has_value()) {
        // En visée, le clic gauche lance le sort (même sur un monstre), réussi
        // ou refusé, et sort de la visée ; rien d'autre.
        partie.lancerSkillDeZone(*m_adresseEnVisee, point);
        m_adresseEnVisee.reset();
        return;
    }
    const ResultatClic resultat = partie.cliquer(point);
    if (resultat == ResultatClic::Monstre || resultat == ResultatClic::Objet) {
        m_deplacementContinu = false;
    } else if (resultat == ResultatClic::Sol) {
        m_deplacementContinu = true;
    }
}

void Controleur::appuiDroit() {
    m_adresseEnVisee.reset();
}

void Controleur::appuiTouche(Partie& partie, std::size_t indiceTouche) {
    const std::optional<AdresseSkill> adresse = adresseDeLaTouche(indiceTouche);
    if (!adresse.has_value()) {
        return;
    }
    // La visée dépend du type du skill, pas du numéro de la touche.
    if (partie.getEscouade().typeDuSkill(*adresse) == TypeSkill::Zone) {
        if (m_adresseEnVisee.has_value()) {
            m_adresseEnVisee.reset();
        } else if (partie.getEscouade().estVivante() && partie.getEscouade().skillEstPret(*adresse)) {
            m_adresseEnVisee = adresse;
            // Le déplacement continu est suspendu : il ne reprendra qu'à un
            // nouvel appui sur le sol.
            m_deplacementContinu = false;
        }
    } else {
        partie.demanderSkill(*adresse);
    }
}

void Controleur::appuiEchap() {
    m_adresseEnVisee.reset();
}

void Controleur::mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur) {
    if (boutonTenu && m_deplacementContinu) {
        partie.ordonnerDeplacement(curseur);
    }
}

void Controleur::dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const {
    if (!m_adresseEnVisee.has_value()) {
        return;
    }
    // Cercle de prévisualisation au rayon exact de la zone, sous le curseur.
    const float rayon = partie.getEscouade().rayonZoneDuSkill(*m_adresseEnVisee);
    sf::CircleShape apercu(rayon);
    apercu.setOrigin({rayon, rayon});
    apercu.setPosition(curseur);
    apercu.setFillColor(sf::Color::Transparent);
    apercu.setOutlineColor(sf::Color::Yellow);
    apercu.setOutlineThickness(2.0f);
    rendu.draw(apercu);

    // Cercle de portée de lancement autour du lanceur.
    const Cercle portee = partie.getEscouade().cercleDePorteeDuSkill(*m_adresseEnVisee);
    sf::CircleShape contourPortee(portee.rayon);
    contourPortee.setOrigin({portee.rayon, portee.rayon});
    contourPortee.setPosition(portee.centre);
    contourPortee.setFillColor(sf::Color::Transparent);
    contourPortee.setOutlineColor(sf::Color(255, 255, 255, 160));
    contourPortee.setOutlineThickness(2.0f);
    rendu.draw(contourPortee);
}

std::optional<AdresseSkill> Controleur::getAdresseEnVisee() const {
    return m_adresseEnVisee;
}
