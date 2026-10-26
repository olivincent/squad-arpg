// ./src/interface/Controleur.cpp
#include "interface/Controleur.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include "interface/BarreDeRaccourcis.hpp"
#include "base/Cercle.hpp"
#include "jeu/Partie.hpp"
#include "combat/TypeSkill.hpp"

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
    if (resultat == ResultatClic::Monstre || resultat == ResultatClic::Porte || resultat == ResultatClic::ObjetAuSol) {
        m_deplacementContinu = false;
    } else if (resultat == ResultatClic::Sol) {
        m_deplacementContinu = true;
    }
}

void Controleur::appuiInterface(Partie& partie, const BarreDeRaccourcis& barre, ClicInterface clic) {
    // Un appui qui commence sur l'interface ne lance pas le déplacement continu, et désarme
    // celui d'un ancien clic au sol (sinon l'escouade repartirait tant que le bouton est tenu).
    m_deplacementContinu = false;
    switch (clic.barre) {
        case Barre::Raccourcis:
            activerEmplacement(partie, barre, clic.emplacement);
            break;
    }
}

void Controleur::appuiDroit() {
    m_adresseEnVisee.reset();
}

void Controleur::activerEmplacement(Partie& partie, const BarreDeRaccourcis& barre, std::size_t emplacement) {
    const std::optional<AdresseSkill> adresse = barre.adresse(emplacement);
    if (!adresse.has_value()) {
        return;
    }
    // La visée dépend du type du skill, pas de l'emplacement.
    if (partie.getEscouade().typeDuSkill(*adresse) == TypeSkill::Zone) {
        if (m_adresseEnVisee == adresse) {
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

void Controleur::interrompre() {
    m_adresseEnVisee.reset();
    m_deplacementContinu = false;
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
