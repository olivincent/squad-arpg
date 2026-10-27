// ./tests/interface/test_AffichageObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "combat/DescriptionDAttaque.hpp"
#include "interface/AffichageObjets.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"

TEST_CASE("affichage : chaque emplacement a un nom, tous differents") {
    std::set<std::string> noms;
    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        const std::string nom = nomDeLEmplacement(static_cast<Emplacement>(i));
        REQUIRE_FALSE(nom.empty());
        noms.insert(nom);
    }
    REQUIRE(noms.size() == s_nombreEmplacements);
}

TEST_CASE("affichage : chaque categorie a un nom") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE_FALSE(nomDeLaCategorie(definition.categorie).empty());
    }
}

TEST_CASE("affichage : chaque objet du catalogue a un sigle de deux lettres, tous differents") {
    std::set<std::string> sigles;
    for (const DefinitionObjet& definition : catalogueObjets()) {
        const std::string sigle = sigleDeLObjet(definition.id);
        REQUIRE(sigle.size() == 2);
        sigles.insert(sigle);
    }
    REQUIRE(sigles.size() == catalogueObjets().size());
}

TEST_CASE("affichage : l'infobulle donne le nom, la categorie, l'armure puis le seuil") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::CasqueDeFer});

    REQUIRE(lignes.size() == 4);
    REQUIRE(lignes[0] == definitionObjet(IdObjet::CasqueDeFer).nom);
    REQUIRE(lignes[1] == "Helmet");
    REQUIRE(lignes[2] == "Armor: +" + std::to_string(definitionObjet(IdObjet::CasqueDeFer).armure));
    REQUIRE(lignes[3] == ligneDeSeuil(seuilsDeLObjet(Objet{IdObjet::CasqueDeFer}).front()));
}

TEST_CASE("affichage : une ligne de seuil donne la caracteristique et la valeur exigee") {
    REQUIRE(ligneDeSeuil({Caracteristique::Puissance, 11}) == "Requires Might 11");
    REQUIRE(ligneDeSeuil({Caracteristique::Endurance, 12}) == "Requires Constitution 12");
}

TEST_CASE("affichage : l'infobulle donne les seuils de l'objet a son niveau, en dernier") {
    const Objet niveau5{IdObjet::EpeeADeuxMains, Rarete::Commun, std::nullopt, std::nullopt, 5};
    const std::vector<std::string> lignes = lignesDeLInfobulle(niveau5);

    REQUIRE(lignes[1] == "Weapon");
    REQUIRE(lignes.back() == ligneDeSeuil(seuilsDeLObjet(niveau5).back()));
    REQUIRE(lignes.back() == "Requires Might 14");  // 6 + 2 × (5 − 1)
}

TEST_CASE("affichage : un objet sans seuil n'a aucune ligne d'exigence") {
    for (const IdObjet libre : {IdObjet::GantsDeCuir, IdObjet::BottesDeCuir, IdObjet::AnneauDePuissance, IdObjet::CollierDEndurance}) {
        for (const std::string& ligne : lignesDeLInfobulle(Objet{libre})) {
            REQUIRE(ligne.find("Requires") == std::string::npos);
        }
    }
}

TEST_CASE("affichage : la ligne d'un objet inactif") {
    REQUIRE(ligneDObjetInactif() == "Inactive: requirement not met");
}

TEST_CASE("affichage : l'infobulle d'un objet sans armure n'a pas de ligne d'armure, mais une ligne par bonus") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::AnneauDePuissance});

    REQUIRE(lignes.size() == 3);
    REQUIRE(lignes[0] == definitionObjet(IdObjet::AnneauDePuissance).nom);
    REQUIRE(lignes[1] == "Ring");
    REQUIRE(lignes[2] == "Might: +" + std::to_string(definitionObjet(IdObjet::AnneauDePuissance).bonus.valeur(Caracteristique::Puissance)));
}

TEST_CASE("affichage : l'infobulle du collier d'endurance donne l'Endurance") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::CollierDEndurance});

    REQUIRE(lignes.size() == 3);
    REQUIRE(lignes[2] == "Constitution: +" + std::to_string(definitionObjet(IdObjet::CollierDEndurance).bonus.valeur(Caracteristique::Endurance)));
}

TEST_CASE("affichage : aucun objet du catalogue n'affiche une valeur nulle") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        for (const std::string& ligne : lignesDeLInfobulle(Objet{definition.id})) {
            REQUIRE(ligne.find("+0") == std::string::npos);
        }
    }
}

TEST_CASE("affichage : l'infobulle des bottes de cuir donne l'esquive contre la melee") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::BottesDeCuir});

    REQUIRE(lignes.back() == "Dodge vs Melee: +5 %");
}

TEST_CASE("affichage : l'infobulle du casque de cuir donne l'esquive contre les projectiles") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::CasqueDeCuir});

    REQUIRE(lignes.back() == "Dodge vs Projectiles: +5 %");
}

TEST_CASE("affichage : l'infobulle du bouclier rond donne son blocage") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::BouclierRond});

    const DonneesDeBouclier& bouclier = definitionObjet(IdObjet::BouclierRond).bouclier.value();
    REQUIRE(lignes[lignes.size() - 2] == "Block: " + std::to_string(bouclier.chanceDeBlocage) + " %, " +
                                 std::to_string(bouclier.montantBloque) + " points");
}

TEST_CASE("affichage : l'infobulle d'une arme a une main donne ses degats, sa recharge, sa portee et sa nature") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{IdObjet::EpeeLongue});
    const DescriptionDAttaque& attaque = definitionObjet(IdObjet::EpeeLongue).arme.value().attaque;

    REQUIRE(lignes == std::vector<std::string>{"Longsword", "Weapon", "Damage: " + std::to_string(attaque.degats.min.total()),
                                               "Cooldown: 0.3 to 0.7 s", "Range: 20 px", "Melee",
                                               ligneDeSeuil({Caracteristique::Puissance, s_seuilAuNiveau1})});
}

TEST_CASE("affichage : l'infobulle d'une arme a deux mains le dit, celle d'une arme de tir donne un projectile") {
    const std::vector<std::string> arc = lignesDeLInfobulle(Objet{IdObjet::Arc});
    const std::vector<std::string> sceptre = lignesDeLInfobulle(Objet{IdObjet::Sceptre});
    const std::vector<std::string> deuxMains = lignesDeLInfobulle(Objet{IdObjet::EpeeADeuxMains});

    REQUIRE(arc[arc.size() - 2] == "Two-handed");  // avant le seuil d'Adresse
    REQUIRE(arc[arc.size() - 3] == "Projectile");
    REQUIRE(arc[3] == "Cooldown: 0.4 to 0.6 s");
    REQUIRE(arc[4] == "Range: 120 px");
    REQUIRE(sceptre[sceptre.size() - 2] == "Projectile");  // une main : pas de « Two-handed »
    REQUIRE(std::ranges::find(sceptre, "Two-handed") == sceptre.end());
    REQUIRE(deuxMains[deuxMains.size() - 2] == "Two-handed");  // avant le seuil
    REQUIRE(deuxMains[3] == "Cooldown: 1.2 to 2.8 s");
}

TEST_CASE("affichage : seules les armes ont des lignes d'attaque, et les deux mains ne sont dites que des armes a deux mains") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        const std::vector<std::string> lignes = lignesDeLInfobulle(Objet{definition.id});
        const bool aDesDegats = std::ranges::any_of(lignes, [](const std::string& ligne) { return ligne.starts_with("Damage"); });
        const bool deuxMains = std::ranges::any_of(lignes, [](const std::string& ligne) { return ligne == "Two-handed"; });
        REQUIRE(aDesDegats == definition.arme.has_value());
        REQUIRE(deuxMains == estADeuxMains(definition.id));
    }
}

TEST_CASE("affichage : l'infobulle de la dague donne sa penetration, celle des autres armes non") {
    const std::vector<std::string> dague = lignesDeLInfobulle(Objet{IdObjet::Dague});

    REQUIRE(std::ranges::find(dague, "Penetration: 30 %") != dague.end());
    for (const IdObjet autre : {IdObjet::EpeeLongue, IdObjet::Masse, IdObjet::EpeeADeuxMains, IdObjet::Sceptre, IdObjet::Baton, IdObjet::Arc}) {
        for (const std::string& ligne : lignesDeLInfobulle(Objet{autre})) {
            REQUIRE(ligne.find("Penetration") == std::string::npos);
        }
    }
}

// Les affixes et la rareté (étape 95.5).
namespace {
const Objet s_arcAcereDuRenard{IdObjet::Arc, Rarete::Rare, Affixe{IdAffixe::Acere, 15}, Affixe{IdAffixe::DuRenard, 2}};
}  // namespace

TEST_CASE("affichage : l'infobulle d'un objet rare donne le nom compose, la categorie, l'attaque, une ligne par affixe, puis le seuil") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(s_arcAcereDuRenard);
    const DescriptionDAttaque& attaque = definitionObjet(IdObjet::Arc).arme.value().attaque;

    REQUIRE(lignes == std::vector<std::string>{"Keen Bow of the Fox", "Weapon", "Damage: " + std::to_string(attaque.degats.min.total()),
                                               "Cooldown: 0.4 to 0.6 s", "Range: 120 px", "Projectile", "Two-handed",
                                               "Penetration: +15 %", "Dexterity: +2",
                                               ligneDeSeuil({Caracteristique::Adresse, s_seuilAuNiveau1})});
}

TEST_CASE("affichage : un objet magique n'a qu'une ligne d'affixe, un objet commun aucune") {
    const std::vector<std::string> magique = lignesDeLInfobulle(Objet{IdObjet::Masse, Rarete::Magique, Affixe{IdAffixe::Flamboyant, 6}});
    const std::vector<std::string> commun = lignesDeLInfobulle(Objet{IdObjet::Masse});

    REQUIRE(magique.front() == "Blazing Mace");
    REQUIRE(std::ranges::find(magique, "Fire Damage: +6") != magique.end());
    REQUIRE(magique.size() == commun.size() + 1);
    REQUIRE(commun.front() == "Mace");
}

TEST_CASE("affichage : l'infobulle d'une cuirasse rare garde l'armure du modele et celle de l'affixe sur deux lignes") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(
        Objet{IdObjet::CuirasseDeMailles, Rarete::Rare, Affixe{IdAffixe::Renforce, 3}, Affixe{IdAffixe::DEsquive, 3}});

    REQUIRE(lignes == std::vector<std::string>{"Reinforced Chain Mail of Evasion", "Body Armor",
                                               "Armor: +" + std::to_string(definitionObjet(IdObjet::CuirasseDeMailles).armure),
                                               "Armor: +3", "Dodge vs Melee: +3 %",
                                               ligneDeSeuil({Caracteristique::Endurance, s_seuilAuNiveau1})});
}

TEST_CASE("ligneDAffixe : une ligne par effet, avec sa valeur") {
    REQUIRE(ligneDAffixe({IdAffixe::Flamboyant, 6}) == "Fire Damage: +6");
    REQUIRE(ligneDAffixe({IdAffixe::Sanctifie, 4}) == "Holy Damage: +4");
    REQUIRE(ligneDAffixe({IdAffixe::Acere, 15}) == "Penetration: +15 %");
    REQUIRE(ligneDAffixe({IdAffixe::Renforce, 3}) == "Armor: +3");
    REQUIRE(ligneDAffixe({IdAffixe::DuBoeuf, 2}) == "Might: +2");
    REQUIRE(ligneDAffixe({IdAffixe::DeLOurs, 2}) == "Constitution: +2");
    REQUIRE(ligneDAffixe({IdAffixe::DuRenard, 2}) == "Dexterity: +2");
    REQUIRE(ligneDAffixe({IdAffixe::DeLaChouette, 2}) == "Intellect: +2");
    REQUIRE(ligneDAffixe({IdAffixe::DuLion, 2}) == "Resolve: +2");
    REQUIRE(ligneDAffixe({IdAffixe::DEsquive, 3}) == "Dodge vs Melee: +3 %");
    REQUIRE(ligneDAffixe({IdAffixe::DeDeviation, 4}) == "Dodge vs Projectiles: +4 %");
    REQUIRE(ligneDAffixe({IdAffixe::DeBlocage, 5}) == "Block: +5 %");
}

TEST_CASE("ligneDAffixe : tout affixe du catalogue a une ligne non vide") {
    for (const DefinitionDAffixe& definition : catalogueAffixes()) {
        REQUIRE_FALSE(ligneDAffixe({definition.id, 1}).empty());
    }
}

namespace {
// Luminance relative (WCAG) d'une couleur, entre 0 et 1.
double luminance(sf::Color couleur) {
    const auto canal = [](unsigned char valeur) {
        const double c = valeur / 255.0;
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * canal(couleur.r) + 0.7152 * canal(couleur.g) + 0.0722 * canal(couleur.b);
}

double contraste(sf::Color a, sf::Color b) {
    const double claire = std::max(luminance(a), luminance(b));
    const double sombre = std::min(luminance(a), luminance(b));
    return (claire + 0.05) / (sombre + 0.05);
}
}  // namespace

TEST_CASE("couleurDeLaRarete : blanc, jaune et vert, trois couleurs distinctes") {
    REQUIRE(couleurDeLaRarete(Rarete::Commun) == sf::Color::White);
    const sf::Color magique = couleurDeLaRarete(Rarete::Magique);
    const sf::Color rare = couleurDeLaRarete(Rarete::Rare);
    REQUIRE(magique != rare);
    REQUIRE(magique != sf::Color::White);
    REQUIRE(rare != sf::Color::White);
    REQUIRE((magique.r > magique.b && magique.g > magique.b));  // jaune : rouge et vert dominent
    REQUIRE((rare.g > rare.r && rare.g > rare.b));              // vert
}

TEST_CASE("couleurDeLaRarete : lisible sur le fond le plus clair de l'inventaire (une case occupee, gris a 70)") {
    const sf::Color fondLePlusClair(70, 70, 70);  // `s_couleurCaseOccupee` de l'écran d'inventaire
    for (const Rarete rarete : {Rarete::Commun, Rarete::Magique, Rarete::Rare}) {
        REQUIRE(contraste(couleurDeLaRarete(rarete), fondLePlusClair) >= 4.5);  // seuil AA du texte courant
    }
}
