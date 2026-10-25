// ./tests/escouade/test_Specialite.cpp
#include <catch2/catch_test_macros.hpp>
#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>

#include "interface/BarreDeRaccourcis.hpp"
#include "aides.hpp"
#include "aides_escouade.hpp"
#include "combat/Caracteristiques.hpp"
#include "combat/Defense.hpp"
#include "combat/Element.hpp"
#include "combat/TypeSkill.hpp"
#include "combat/TypeDArme.hpp"
#include "escouade/Composition.hpp"
#include "objets/CatalogueObjets.hpp"
#include "base/Eclaircir.hpp"
#include "escouade/Escouade.hpp"
#include "escouade/Personnage.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "escouade/Specialite.hpp"

namespace {
// Un personnage construit depuis la définition de la classe (son attaque de base et sa zone de contrôle en viennent).
Personnage deLaClasse(IdSpecialite id) {
    return Personnage({0.0f, 0.0f}, specialite(id).definition);
}

// Ce que la définition fait porter à cet emplacement, s'il y en a un.
std::optional<IdObjet> portePar(const DefinitionPersonnage& definition, Emplacement emplacement) {
    for (const ObjetDeDepart& objet : definition.equipementDeDepart) {
        if (objet.emplacement == emplacement) {
            return objet.objet;
        }
    }
    return std::nullopt;
}

// L'arme de main droite d'une définition (`assert` : elle en porte une).
IdObjet armeDe(const DefinitionPersonnage& definition) {
    const std::optional<IdObjet> arme = portePar(definition, Emplacement::MainDroite);
    assert(arme.has_value());
    return arme.value();
}
}  // namespace

TEST_CASE("catalogue : les specialites suivent l'ordre de IdSpecialite") {
    const std::vector<Specialite>& specialites = catalogue();

    REQUIRE(specialites.size() == 5);
    REQUIRE(specialites[0].id == IdSpecialite::Guerrier);
    REQUIRE(specialites[1].id == IdSpecialite::Pretre);
    REQUIRE(specialites[2].id == IdSpecialite::Mage);
    REQUIRE(specialites[3].id == IdSpecialite::Paladin);
    REQUIRE(specialites[4].id == IdSpecialite::Rodeur);
}

TEST_CASE("catalogue : chaque specialite a un nom, de la vision et des skills sans doublon") {
    for (const Specialite& specialite : catalogue()) {
        REQUIRE_FALSE(specialite.nom.empty());
        REQUIRE(specialite.definition.rayonVision == 320.0f);
        const std::vector<SkillConnu>& skills = specialite.definition.skills;
        for (std::size_t i = 0; i < skills.size(); ++i) {
            for (std::size_t j = i + 1; j < skills.size(); ++j) {
                REQUIRE(skills[i].id != skills[j].id);
            }
        }
    }
}

TEST_CASE("specialite : rend l'entree du catalogue de cet identifiant") {
    REQUIRE(specialite(IdSpecialite::Guerrier).nom == "Guerrier");
    REQUIRE(specialite(IdSpecialite::Pretre).definition.rang == Rang::Arriere);
    REQUIRE(specialite(IdSpecialite::Mage).definition.skills.front().id == IdSkill::Explosion);
}

TEST_CASE("composer : une definition par identifiant, dans l'ordre recu") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Mage, IdSpecialite::Guerrier, IdSpecialite::Pretre});

    REQUIRE(composition.size() == 3);
    REQUIRE(composition[0].rang == Rang::Arriere);
    REQUIRE(composition[0].skills.front().id == IdSkill::Explosion);
    REQUIRE(composition[1].rang == Rang::Avant);
    REQUIRE(composition[1].skills.front().id == IdSkill::FrappePuissante);
    REQUIRE(composition[2].skills.front().id == IdSkill::Soin);
}

TEST_CASE("composer : une liste vide donne une composition vide") {
    REQUIRE(composer({}).empty());
}

TEST_CASE("composer : des classes differentes gardent leur couleur de classe") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Pretre, IdSpecialite::Mage});

    for (std::size_t i = 0; i < composition.size(); ++i) {
        REQUIRE(composition[i].couleur == catalogue()[i].definition.couleur);
    }
}

TEST_CASE("composer : la meme classe prise plusieurs fois s'assombrit de 0, 30 puis 55 pour cent") {
    const sf::Color couleurDeClasse = specialite(IdSpecialite::Guerrier).definition.couleur;

    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Guerrier, IdSpecialite::Guerrier, IdSpecialite::Guerrier});

    REQUIRE(composition[0].couleur == couleurDeClasse);
    REQUIRE(composition[1].couleur == assombrir(couleurDeClasse, 0.30f));
    REQUIRE(composition[2].couleur == assombrir(couleurDeClasse, 0.55f));
    REQUIRE(composition[1].couleur != composition[0].couleur);
    REQUIRE(composition[2].couleur != composition[1].couleur);
}

TEST_CASE("composer : le rang d'occurrence se compte par classe, pas sur toute la liste") {
    const sf::Color couleurDuMage = specialite(IdSpecialite::Mage).definition.couleur;

    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Mage, IdSpecialite::Guerrier, IdSpecialite::Mage});

    REQUIRE(composition[0].couleur == couleurDuMage);
    REQUIRE(composition[1].couleur == specialite(IdSpecialite::Guerrier).definition.couleur);
    REQUIRE(composition[2].couleur == assombrir(couleurDuMage, 0.30f));
}

TEST_CASE("composer : seule la couleur change entre deux occurrences d'une classe") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Pretre, IdSpecialite::Pretre});

    REQUIRE(composition[1].equipementDeDepart == composition[0].equipementDeDepart);
    REQUIRE(composition[1].rang == composition[0].rang);
    REQUIRE(composition[1].skills.size() == composition[0].skills.size());
}

TEST_CASE("le paladin est une melee qui frappe et soigne au contact") {
    const DefinitionPersonnage& paladin = specialite(IdSpecialite::Paladin).definition;
    const DefinitionPersonnage& guerrier = specialite(IdSpecialite::Guerrier).definition;

    REQUIRE(specialite(IdSpecialite::Paladin).nom == "Paladin");
    REQUIRE(paladin.rang == Rang::Avant);
    // La même armure et le même bouclier que le guerrier, mais la masse à la place de l'épée longue.
    REQUIRE(armureDe(paladin.equipementDeDepart) == armureDe(guerrier.equipementDeDepart));
    REQUIRE(armeDe(paladin) == IdObjet::Masse);
    REQUIRE(deLaClasse(IdSpecialite::Paladin).getPorteeAttaqueDeBase() == deLaClasse(IdSpecialite::Guerrier).getPorteeAttaqueDeBase());
    REQUIRE(paladin.skills.size() == 2);

    REQUIRE(paladin.skills[0].id == IdSkill::FrappeSacree);
    REQUIRE(paladin.skills[0].skill.getType() == TypeSkill::DegatsSurCible);
    REQUIRE(paladin.skills[0].skill.getPuissance() == 60);
    REQUIRE(paladin.skills[0].skill.getPortee() == 20.0f);

    REQUIRE(paladin.skills[1].id == IdSkill::ImpositionDesMains);
    REQUIRE(paladin.skills[1].skill.getType() == TypeSkill::Soin);
    REQUIRE(paladin.skills[1].skill.getPuissance() == 75);
    REQUIRE(paladin.skills[1].skill.getPortee() == 20.0f);
}

TEST_CASE("le rodeur est un tireur avec un tir de loin et un skill de zone") {
    const DefinitionPersonnage& rodeur = specialite(IdSpecialite::Rodeur).definition;

    REQUIRE(specialite(IdSpecialite::Rodeur).nom == "Rôdeur");
    REQUIRE(rodeur.rang == Rang::Arriere);
    REQUIRE(rodeur.equipementDeDepart == std::vector<ObjetDeDepart>{{Emplacement::MainDroite, IdObjet::Arc}});
    REQUIRE(deLaClasse(IdSpecialite::Rodeur).getPorteeAttaqueDeBase() == deLaClasse(IdSpecialite::Mage).getPorteeAttaqueDeBase());
    REQUIRE(rodeur.skills.size() == 2);

    REQUIRE(rodeur.skills[0].id == IdSkill::TirPercant);
    REQUIRE(rodeur.skills[0].skill.getType() == TypeSkill::DegatsSurCible);
    REQUIRE(rodeur.skills[0].skill.getPuissance() == 75);
    REQUIRE(rodeur.skills[0].skill.getPortee() == 200.0f);

    REQUIRE(rodeur.skills[1].id == IdSkill::PluieDeFleches);
    REQUIRE(rodeur.skills[1].skill.getType() == TypeSkill::Zone);
    REQUIRE(rodeur.skills[1].skill.getPuissance() == 36);
    REQUIRE(rodeur.skills[1].skill.getRayonZone() == 45.0f);
    REQUIRE(rodeur.skills[1].skill.getPortee() == 320.0f);
}

TEST_CASE("trois rodeurs : tous derriere, en colonne, avec trois nuances de vert") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Rodeur, IdSpecialite::Rodeur, IdSpecialite::Rodeur});

    std::vector<Rang> rangs;
    for (const DefinitionPersonnage& definition : composition) {
        rangs.push_back(definition.rang);
    }
    using Decalages = std::vector<sf::Vector2f>;
    REQUIRE(decalagesDeFormation(rangs) == Decalages{{-25.0f, -50.0f}, {-25.0f, 0.0f}, {-25.0f, 50.0f}});
    REQUIRE(composition[0].couleur != composition[1].couleur);
    REQUIRE(composition[1].couleur != composition[2].couleur);
    REQUIRE(composition[0].couleur != composition[2].couleur);
}

TEST_CASE("trois rodeurs : la barre par defaut a une rangee par rodeur") {
    const Escouade escouade({100.0f, 100.0f},
                            composer({IdSpecialite::Rodeur, IdSpecialite::Rodeur, IdSpecialite::Rodeur}));

    const BarreDeRaccourcis barre = barreParDefaut(escouade.adressesDesSkills());

    for (std::size_t place = 1; place <= 3; ++place) {
        const std::size_t debut = (place - 1) * 4;
        REQUIRE(barre.adresse(debut) == AdresseSkill{place, IdSkill::TirPercant});
        REQUIRE(barre.adresse(debut + 1) == AdresseSkill{place, IdSkill::PluieDeFleches});
        REQUIRE_FALSE(barre.adresse(debut + 2).has_value());
        REQUIRE_FALSE(barre.adresse(debut + 3).has_value());
    }
}

TEST_CASE("une escouade de paladin, de rodeur et de guerrier connait les skills de chacun") {
    const Escouade escouade({100.0f, 100.0f},
                            composer({IdSpecialite::Paladin, IdSpecialite::Rodeur, IdSpecialite::Guerrier}));

    REQUIRE(escouade.typeDuSkill({1, IdSkill::ImpositionDesMains}) == TypeSkill::Soin);
    REQUIRE(escouade.typeDuSkill({2, IdSkill::PluieDeFleches}) == TypeSkill::Zone);
    REQUIRE(escouade.typeDuSkill({3, IdSkill::FrappePuissante}) == TypeSkill::DegatsSurCible);
    REQUIRE_FALSE(escouade.typeDuSkill({1, IdSkill::TirPercant}).has_value());
}

TEST_CASE("nature des attaques : contact pour les melees, projectile pour les tireurs, zone pour les zones") {
    const auto nature = [](IdSpecialite id) { return deLaClasse(id).getAttaqueDeBase().nature; };

    REQUIRE(nature(IdSpecialite::Guerrier) == NatureDAttaque::Contact);
    REQUIRE(nature(IdSpecialite::Paladin) == NatureDAttaque::Contact);
    REQUIRE(nature(IdSpecialite::Pretre) == NatureDAttaque::Projectile);
    REQUIRE(nature(IdSpecialite::Mage) == NatureDAttaque::Projectile);
    REQUIRE(nature(IdSpecialite::Rodeur) == NatureDAttaque::Projectile);

    const auto natureDuSkill = [](IdSpecialite id, std::size_t rang) { return specialite(id).definition.skills[rang].skill.getNature(); };
    REQUIRE(natureDuSkill(IdSpecialite::Guerrier, 0) == NatureDAttaque::Contact);   // frappe puissante
    REQUIRE(natureDuSkill(IdSpecialite::Mage, 0) == NatureDAttaque::Zone);          // explosion
    REQUIRE(natureDuSkill(IdSpecialite::Paladin, 0) == NatureDAttaque::Contact);    // frappe sacree
    REQUIRE(natureDuSkill(IdSpecialite::Rodeur, 0) == NatureDAttaque::Projectile);  // tir percant
    REQUIRE(natureDuSkill(IdSpecialite::Rodeur, 1) == NatureDAttaque::Zone);        // pluie de fleches
}

TEST_CASE("zone de controle : 20 px pour le guerrier et le paladin (arme de contact), 0 pour les trois autres") {
    REQUIRE(deLaClasse(IdSpecialite::Guerrier).getZoneDeControle() == 20.0f);
    REQUIRE(deLaClasse(IdSpecialite::Paladin).getZoneDeControle() == 20.0f);
    REQUIRE(deLaClasse(IdSpecialite::Pretre).getZoneDeControle() == 0.0f);
    REQUIRE(deLaClasse(IdSpecialite::Mage).getZoneDeControle() == 0.0f);
    REQUIRE(deLaClasse(IdSpecialite::Rodeur).getZoneDeControle() == 0.0f);
}

TEST_CASE("composer garde l'arme, donc la zone de controle, de la classe, quelle que soit la nuance") {
    const std::vector<DefinitionPersonnage> composition =
        composer({IdSpecialite::Paladin, IdSpecialite::Paladin, IdSpecialite::Mage});

    REQUIRE(Personnage({0.0f, 0.0f}, composition[0]).getZoneDeControle() == 20.0f);
    REQUIRE(Personnage({0.0f, 0.0f}, composition[1]).getZoneDeControle() == 20.0f);
    REQUIRE(Personnage({0.0f, 0.0f}, composition[2]).getZoneDeControle() == 0.0f);
}

TEST_CASE("equipement de depart : les melees portent armure, bouclier et arme, les tireurs seulement leur arme") {
    for (const IdSpecialite melee : {IdSpecialite::Guerrier, IdSpecialite::Paladin}) {
        const DefinitionPersonnage& definition = specialite(melee).definition;
        REQUIRE(armureDe(definition.equipementDeDepart) > 0);
        REQUIRE(portePar(definition, Emplacement::MainGauche) == IdObjet::BouclierRond);
    }
    for (const IdSpecialite tireur : {IdSpecialite::Pretre, IdSpecialite::Mage, IdSpecialite::Rodeur}) {
        const DefinitionPersonnage& definition = specialite(tireur).definition;
        REQUIRE(definition.equipementDeDepart.size() == 1);
        REQUIRE(armureDe(definition.equipementDeDepart) == 0);
    }
}

TEST_CASE("equipement de depart : l'arme de chaque classe") {
    REQUIRE(armeDe(specialite(IdSpecialite::Guerrier).definition) == IdObjet::EpeeLongue);
    REQUIRE(armeDe(specialite(IdSpecialite::Paladin).definition) == IdObjet::Masse);
    REQUIRE(armeDe(specialite(IdSpecialite::Pretre).definition) == IdObjet::Sceptre);
    REQUIRE(armeDe(specialite(IdSpecialite::Mage).definition) == IdObjet::Baton);
    REQUIRE(armeDe(specialite(IdSpecialite::Rodeur).definition) == IdObjet::Arc);
}

TEST_CASE("equipement de depart : une arme a deux mains ne cote pas de bouclier a qui n'en porte pas") {
    for (const Specialite& classe : catalogue()) {
        const IdObjet arme = armeDe(classe.definition);
        if (estADeuxMains(arme)) {
            REQUIRE_FALSE(portePar(classe.definition, Emplacement::MainGauche).has_value());
        }
    }
}

TEST_CASE("equipement de depart : chaque objet va a son emplacement, sans emplacement en double") {
    for (const Specialite& specialite : catalogue()) {
        const std::vector<ObjetDeDepart>& depart = specialite.definition.equipementDeDepart;
        for (std::size_t i = 0; i < depart.size(); ++i) {
            REQUIRE(peutAller(definitionObjet(depart[i].objet).categorie, depart[i].emplacement));
            for (std::size_t j = i + 1; j < depart.size(); ++j) {
                REQUIRE(depart[i].emplacement != depart[j].emplacement);
            }
        }
    }
}

TEST_CASE("armure des classes : une escouade du catalogue encaisse selon l'equipement de sa classe") {
    // Le guerrier sans son bouclier, qui bloquerait au hasard : son armure est celle du reste de son équipement.
    std::vector<DefinitionPersonnage> composition = composer({IdSpecialite::Guerrier, IdSpecialite::Mage});
    std::erase_if(composition[0].equipementDeDepart,
                  [](const ObjetDeDepart& objet) { return objet.emplacement == Emplacement::MainGauche; });
    const int armureDuGuerrier = armureDe(composition[0].equipementDeDepart);
    Escouade escouade({100.0f, 100.0f}, std::move(composition));

    REQUIRE(subirCoup(escouade, 7, 1, 40) == appliquerArmure(40, armureDuGuerrier));
    REQUIRE(subirCoup(escouade, 7, 2, 40) == 40);
}

TEST_CASE("caracteristiques des classes : somme de 50, et les points de vie de depart ne dependent que de l'Endurance") {
    for (const Specialite& classe : catalogue()) {
        const DefinitionPersonnage& definition = classe.definition;
        int somme = 0;
        for (const Caracteristique caracteristique : s_caracteristiques) {
            somme += definition.caracteristiques.valeur(caracteristique);
        }
        REQUIRE(somme == 50);
        REQUIRE(pointsDeVieMax(definition.caracteristiques) ==
                s_pointsDeVieDeBase + s_pvParPointDEndurance * definition.caracteristiques.valeur(Caracteristique::Endurance));
    }
}

TEST_CASE("caracteristiques des classes : le guerrier a plus d'Endurance que le mage, le mage plus de Savoir") {
    const Caracteristiques& guerrier = specialite(IdSpecialite::Guerrier).definition.caracteristiques;
    const Caracteristiques& mage = specialite(IdSpecialite::Mage).definition.caracteristiques;

    REQUIRE(guerrier.valeur(Caracteristique::Endurance) > mage.valeur(Caracteristique::Endurance));
    REQUIRE(mage.valeur(Caracteristique::Savoir) > guerrier.valeur(Caracteristique::Savoir));
}

TEST_CASE("armes exigees : frappe puissante, frappe sacree et skills de l'archer") {
    using Types = std::vector<TypeDArme>;
    const auto exigees = [](IdSpecialite id, std::size_t rang) { return specialite(id).definition.skills[rang].skill.getArmesExigees(); };

    REQUIRE(exigees(IdSpecialite::Guerrier, 0) ==
            Types{TypeDArme::EpeeLongue, TypeDArme::Masse, TypeDArme::Dague, TypeDArme::EpeeADeuxMains});  // frappe puissante
    REQUIRE(exigees(IdSpecialite::Paladin, 0) ==
            Types{TypeDArme::EpeeLongue, TypeDArme::Masse, TypeDArme::EpeeADeuxMains});                    // frappe sacree
    REQUIRE(exigees(IdSpecialite::Rodeur, 0) == Types{TypeDArme::Arc});                                      // tir percant
    REQUIRE(exigees(IdSpecialite::Rodeur, 1) == Types{TypeDArme::Arc});                                      // pluie de fleches
}

TEST_CASE("armes exigees : les autres skills n'exigent rien") {
    REQUIRE(specialite(IdSpecialite::Guerrier).definition.skills[1].skill.getArmesExigees().empty());   // provocation
    REQUIRE(specialite(IdSpecialite::Pretre).definition.skills[0].skill.getArmesExigees().empty());     // soin
    REQUIRE(specialite(IdSpecialite::Mage).definition.skills[0].skill.getArmesExigees().empty());       // explosion
    REQUIRE(specialite(IdSpecialite::Paladin).definition.skills[1].skill.getArmesExigees().empty());    // imposition des mains
}

TEST_CASE("armes exigees : chaque classe porte au depart l'arme que ses skills exigent") {
    for (const Specialite& classe : catalogue()) {
        const Personnage personnage({0.0f, 0.0f}, classe.definition);
        for (const SkillConnu& connu : classe.definition.skills) {
            REQUIRE(personnage.skillEstPret(connu.id));
            REQUIRE_FALSE(personnage.etatDuSkill(connu.id)->armeAbsente);
        }
    }
}

TEST_CASE("seuils : chaque classe atteint les seuils de son equipement de depart avec ses caracteristiques de base") {
    for (const Specialite& classe : catalogue()) {
        for (const ObjetDeDepart& objet : classe.definition.equipementDeDepart) {
            REQUIRE(seuilsAtteints(objet.objet, classe.definition.caracteristiques));
        }
    }
}

TEST_CASE("seuils : aucune classe ne peut porter l'epee a deux mains au depart") {
    for (const Specialite& classe : catalogue()) {
        REQUIRE_FALSE(seuilsAtteints(IdObjet::EpeeADeuxMains, classe.definition.caracteristiques));
    }
}

TEST_CASE("l'explosion du mage est du feu, les autres skills de degats et de zone sont physiques") {
    for (const Specialite& classe : catalogue()) {
        for (const SkillConnu& connu : classe.definition.skills) {
            const bool explosion = connu.id == IdSkill::Explosion;
            INFO(classe.nom << " : skill " << static_cast<int>(connu.id));
            if (connu.skill.getType() == TypeSkill::DegatsSurCible || connu.skill.getType() == TypeSkill::Zone) {
                REQUIRE(connu.skill.getElement() == (explosion ? Element::Feu : Element::Physique));
            }
        }
    }
}
