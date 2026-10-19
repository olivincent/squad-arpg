// ./src/Reglages.cpp
#include "Reglages.hpp"

#include <algorithm>
#include <charconv>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>

namespace {

constexpr std::string_view s_cleLargeur = "largeur";
constexpr std::string_view s_cleHauteur = "hauteur";
constexpr std::string_view s_cleMode = "mode";
constexpr std::string_view s_valeurFenetre = "fenetre";
constexpr std::string_view s_valeurPleinEcran = "plein_ecran";
constexpr std::string_view s_espaces = " \t\r\n";

std::string_view sansEspaces(std::string_view texte) {
    const std::size_t debut = texte.find_first_not_of(s_espaces);
    if (debut == std::string_view::npos) {
        return {};
    }
    const std::size_t fin = texte.find_last_not_of(s_espaces);
    return texte.substr(debut, fin - debut + 1);
}

// Entier positif écrit en entier (aucun caractère de trop) ; vide sinon.
std::optional<unsigned> lireEntier(std::string_view texte) {
    unsigned valeur = 0;
    const char* fin = texte.data() + texte.size();
    const auto [arret, erreur] = std::from_chars(texte.data(), fin, valeur);
    if (erreur != std::errc{} || arret != fin) {
        return std::nullopt;
    }
    return valeur;
}

}  // namespace

const std::vector<Resolution>& resolutionsProposees() {
    static const std::vector<Resolution> resolutions = {
        {1200, 800},
        {1280, 720},
        {1600, 900},
        {1920, 1080},
    };
    return resolutions;
}

std::vector<Resolution> resolutionsDisponibles(sf::Vector2u tailleBureau) {
    std::vector<Resolution> disponibles;
    for (const Resolution& resolution : resolutionsProposees()) {
        if (resolution.largeur <= tailleBureau.x && resolution.hauteur <= tailleBureau.y) {
            disponibles.push_back(resolution);
        }
    }
    return disponibles;
}

std::optional<std::size_t> emplacementPourTouche(const TouchesDesRaccourcis& touches, sf::Keyboard::Scancode scancode) {
    const auto trouvee = std::find(touches.begin(), touches.end(), scancode);
    if (trouvee == touches.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(trouvee - touches.begin());
}

Reglages lireReglages(std::istream& flux) {
    const Reglages parDefaut;
    Reglages reglages;
    std::string ligne;
    while (std::getline(flux, ligne)) {
        const std::size_t egal = ligne.find('=');
        if (egal == std::string::npos) {
            continue;
        }
        const std::string_view cle = sansEspaces(std::string_view(ligne).substr(0, egal));
        const std::string_view valeur = sansEspaces(std::string_view(ligne).substr(egal + 1));
        if (cle == s_cleLargeur) {
            reglages.largeur = lireEntier(valeur).value_or(parDefaut.largeur);
        } else if (cle == s_cleHauteur) {
            reglages.hauteur = lireEntier(valeur).value_or(parDefaut.hauteur);
        } else if (cle == s_cleMode) {
            if (valeur == s_valeurPleinEcran) {
                reglages.mode = ModeAffichage::PleinEcran;
            } else {
                reglages.mode = parDefaut.mode;
            }
        }
    }

    const std::vector<Resolution>& proposees = resolutionsProposees();
    const Resolution choisie{reglages.largeur, reglages.hauteur};
    if (std::find(proposees.begin(), proposees.end(), choisie) == proposees.end()) {
        reglages.largeur = parDefaut.largeur;
        reglages.hauteur = parDefaut.hauteur;
    }
    return reglages;
}

void ecrireReglages(std::ostream& flux, const Reglages& reglages) {
    const std::string_view mode = reglages.mode == ModeAffichage::PleinEcran ? s_valeurPleinEcran : s_valeurFenetre;
    flux << s_cleLargeur << '=' << reglages.largeur << '\n';
    flux << s_cleHauteur << '=' << reglages.hauteur << '\n';
    flux << s_cleMode << '=' << mode << '\n';
}
