// ./src/escouade/Escouade.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/ActionEngagee.hpp"
#include "combat/AdresseSkill.hpp"
#include "combat/Attaque.hpp"
#include "escouade/Agresseurs.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Cible.hpp"
#include "base/Cercle.hpp"
#include "escouade/Composition.hpp"
#include "escouade/Formation.hpp"
#include "escouade/Intention.hpp"
#include "combat/Coup.hpp"
#include "combat/ResultatDEffet.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "combat/Engagement.hpp"
#include "combat/EtatSkill.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/IdSkill.hpp"
#include "combat/ModeDeCible.hpp"
#include "combat/Place.hpp"
#include "monde/Mobile.hpp"
#include "combat/EntiteVue.hpp"
#include "escouade/Personnage.hpp"
#include "escouade/PlacesDAttaque.hpp"
#include "objets/DeplacementDObjets.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Equipement.hpp"
#include "objets/Inventaire.hpp"
#include "objets/Objet.hpp"
#include "objets/PositionDObjet.hpp"
#include "escouade/Provocations.hpp"
#include "combat/ResolutionDuCoup.hpp"

class TileMap;
class Escouade {
public:
    static constexpr std::size_t s_capaciteDuSac = 40;  // cases du sac de l'escouade

    // `membres` : un personnage par membre (sa définition et son équipement), aux places 1 à N dans l'ordre (assert : non
    // vide). Le sac est vide, le niveau est 1.
    Escouade(sf::Vector2f positionDepart, std::vector<MembreDEscouade> membres);
    // Avec ce sac, tel quel (ses objets restent à leur case), et ce niveau (>= 1), le même pour tous les personnages : ceux
    // d'une expédition qui continue (`jeu/Expedition`). `assert` : la capacité du sac est `s_capaciteDuSac`.
    Escouade(sf::Vector2f positionDepart, std::vector<MembreDEscouade> membres, Inventaire sac, int niveau);
    // Le niveau de l'escouade : celui de chacun de ses personnages.
    [[nodiscard]] int getNiveau() const;
    // Tous les personnages, morts compris, passent à ce niveau (`Personnage::monterAuNiveau`). `assert` : au moins le niveau
    // actuel.
    void monterAuNiveau(int niveau);
    // `afficherAncre` faux : le carré vert de l'ancre n'est pas dessiné (débogage).
    void dessiner(sf::RenderTarget& rendu, bool afficherAncre) const;
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    // Une fois l'ancre arrêtée, la formation se tourne vers ce point. Tout nouvel ordre à
    // l'ancre (allerVers, approche du verrou, verrouiller) l'efface.
    void regarderVers(sf::Vector2f point);
    void annulerAttaquesEngagees();
    // Tout ce que l'escouade produit passe par ces trois fonctions et par elles seules, en `ResultatDEffet` (dans l'ordre où ils
    // surviennent) : le départ d'un skill, les dégâts sur un monstre (`Coup` : l'attaque de base, un skill de dégâts, de frappe ou de
    // zone), un soin rendu, une provocation posée. Les soins et les provocations ont déjà eu lieu dans l'escouade ; les `Coup` sont à
    // infliger par `Partie`, qui traduit le tout en événements.
    [[nodiscard]] std::vector<ResultatDEffet> attaquer(float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] std::vector<ResultatDEffet> declencherSkill(AdresseSkill adresse, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] bool skillEstPret(AdresseSkill adresse) const;
    [[nodiscard]] std::optional<EtatSkill> etatDuSkill(AdresseSkill adresse) const;
    // Comment le skill à cette adresse vise ; vide si la place est invalide ou le skill inconnu de ce personnage.
    [[nodiscard]] std::optional<ModeDeCible> modeDuSkill(AdresseSkill adresse) const;
    // Les adresses de tous les skills de l'escouade : par place croissante, puis dans l'ordre de la
    // liste de chaque personnage.
    [[nodiscard]] std::vector<AdresseSkill> adressesDesSkills() const;
    // Couleur du personnage de cette place ; vide si la place est invalide.
    [[nodiscard]] std::optional<sf::Color> couleurDuPersonnage(Place place) const;
    // Points de vie du personnage de cette place ; vide si la place est invalide. Lu par les tests et par
    // l'écran d'inventaire (le jeu les montre sinon par la barre de vie dessinée).
    [[nodiscard]] std::optional<int> pointsDeVieDuPersonnage(Place place) const;
    // Points de vie maximum du personnage de cette place ; vide si la place est invalide.
    [[nodiscard]] std::optional<int> pointsDeVieMaxDuPersonnage(Place place) const;
    // Caractéristiques effectives (base plus équipement) du personnage de cette place ; vide si la place est invalide.
    [[nodiscard]] std::optional<Caracteristiques> caracteristiquesDuPersonnage(Place place) const;
    // Caractéristiques de base (celles de sa classe, sans l'équipement) du personnage de cette place ; vide si la place est
    // invalide. Ce que l'expédition garde d'un étage à l'autre.
    [[nodiscard]] std::optional<Caracteristiques> caracteristiquesDeBaseDuPersonnage(Place place) const;
    // Les points libres que le personnage de cette place a dépensés (ce que l'expédition garde), et ceux qui lui restent à
    // dépenser (calculés : `Personnage::getPointsLibres`) ; vide si la place est invalide.
    [[nodiscard]] std::optional<Caracteristiques> pointsDepensesDuPersonnage(Place place) const;
    [[nodiscard]] std::optional<int> pointsLibresDuPersonnage(Place place) const;
    // Au moins un personnage, mort compris (il garde ses points), a des points libres à dépenser : le signal « Points to spend ».
    [[nodiscard]] bool aDesPointsLibres() const;
    // Fait dépenser ces points au personnage de cette place (`Personnage::depenserDesPoints`), tout ou rien ; faux, sans rien
    // changer, si la place est invalide ou si la dépense est refusée (valeur négative, plus que ses points libres). Permis à
    // tout moment, même en combat, et pour un mort.
    [[nodiscard]] bool depenserDesPoints(Place place, const Caracteristiques& depense);
    // Statistiques de combat du personnage de cette place (tirées de ses caractéristiques effectives) ; vide
    // si la place est invalide. Lues par l'écran d'inventaire.
    [[nodiscard]] std::optional<StatistiquesDeCombat> statistiquesDeCombatDuPersonnage(Place place) const;
    [[nodiscard]] float rayonZoneDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] std::vector<ResultatDEffet> declencherSkillDeZone(AdresseSkill adresse, sf::Vector2f point, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    // Les monstres tels que mettreAJour les reçoit, en trois listes nommées (on ne peut pas en
    // inverser deux). Aucun champ n'a de valeur par défaut : en omettre un dans une initialisation
    // désignée est signalé par le compilateur (-Wmissing-field-initializers). Si Partie oubliait
    // `deControle`, plus aucun personnage ne se dégagerait, sans aucune erreur.
    struct VueDesMonstres {
        // Tous les monstres, même ceux qui rentrent à leur poste : ils bloquent les personnages, et
        // chacun contourne en plus tous ceux qu'il n'approche pas.
        std::vector<MonstreVu> physiques;
        // Ceux qu'on peut viser (pas ceux qui rentrent à leur poste) : le verrou n'est vu, et un
        // monstre n'est « à portée », que s'il y est.
        std::vector<MonstreVu> ciblables;
        // Ceux que la règle des zones de contrôle voit, avec leur zone ; Partie y met la photo du
        // début de l'image. Vide : aucun personnage ne se dégage.
        std::vector<MonstreDeControle> deControle;
    };
    void mettreAJour(float dt, const TileMap& carte, const VueDesMonstres& monstres);
    [[nodiscard]] sf::Vector2f getCentre() const;
    // Personnage que ce monstre poursuit : l'identifiant est sa place (1 à N), le cercle le sien.
    [[nodiscard]] std::optional<PersonnageVu> proieDuMonstre(IdMonstre idMonstre, sf::Vector2f positionMonstre) const;
    // Personnages que ce monstre peut frapper (identifiant = place) : seulement son
    // provocateur s'il est provoqué et vivant, sinon tous les personnages vivants.
    [[nodiscard]] std::vector<PersonnageVu> candidatsDeFrappe(IdMonstre idMonstre) const;
    [[nodiscard]] bool placeEstVivante(Place place) const;
    // Une attaque du monstre `idMonstre` sur le personnage de cette place. Renvoie le résultat du coup
    // (`resoudreCoup`, qui tire dans `generateur`) : les dégâts réellement subis (après armure) ; rien si la
    // place est invalide ou morte. Une attaque qui a des dégâts désigne son auteur comme agresseur.
    ResultatDuCoup recevoirCoup(IdMonstre idMonstre, Place place, const Attaque& attaque, Generateur& generateur);
    [[nodiscard]] bool estVivante() const;
    [[nodiscard]] std::vector<Cercle> cerclesDesVivants() const;
    // Une entité de contrôle (identifiant = place de 1 à N, cercle, zone de contrôle) par personnage
    // vivant : les morts n'engagent personne et ne sont pas engagés.
    [[nodiscard]] std::vector<PersonnageDeControle> entitesDeControle() const;
    // Attaque de base du personnage de cette place (dégâts tirés dans la plage de son arme, avant l'armure de la cible,
    // capacité offensive) : un coup d'opportunité la lit ici, avec le générateur de la partie. Vide si la place est invalide.
    [[nodiscard]] std::optional<Attaque> attaqueDeBase(Place place, Generateur& generateur) const;
    // Un cercle (centre du personnage, rayon de vision) par personnage vivant : la vision de
    // l'escouade est l'union de leurs champs.
    [[nodiscard]] std::vector<Cercle> cerclesDeVision() const;
    // Un personnage vivant est-il à `ecart` ou moins (bord à bord) de cette zone ?
    [[nodiscard]] bool unVivantPresDe(sf::FloatRect zone, float ecart) const;
    // Lu par les tests : le choix de la cible, avant tout temps d'activation (declencherSkill l'appelle aussi).
    [[nodiscard]] std::optional<Cible> choisirCible(ModeDeCible mode, AdresseSkill adresse, const std::vector<MonstreVu>& monstres, const TileMap& carte) const;
    void verrouiller(IdMonstre idCible);
    void deverrouiller();
    [[nodiscard]] std::optional<IdMonstre> getCibleVerrouillee() const;
    // Les places de formation du dernier mettreAJour. Lu par les tests : la Formation est extraite, mais
    // l'assemblage (Escouade la met à jour avec la direction de marche et les obstacles) ne s'observe que là.
    [[nodiscard]] const std::vector<sf::Vector2f>& getPlacesDeFormation() const;
    // Identifiant du monstre dont le personnage de cette place se dégage (voir degagementVoulu) ;
    // vide s'il ne se dégage pas ou si la place est invalide. Lu par les tests : la règle de départ et de
    // fin du dégagement (degagementVoulu) n'est pas extraite, son résultat ne se voit que là.
    [[nodiscard]] std::optional<IdMonstre> degagementDuPersonnage(Place place) const;
    // Centre du personnage de cette place (de 1 à N), mort ou vivant ; vide si la place est invalide.
    [[nodiscard]] std::optional<sf::Vector2f> centreDuPersonnage(Place place) const;
    // Point vers lequel le personnage de cette place (de 1 à N) marche, calculé par le dernier
    // mettreAJour : place de formation, place d'attaque, cible du skill en attente, ou sa
    // propre position s'il reste sur place. Vide si la place est invalide. Lu par les tests : la
    // résolution intention -> point est l'assemblage d'Escouade, que ni Intention ni PlacesDAttaque ne voient.
    [[nodiscard]] std::optional<sf::Vector2f> pointVise(Place place) const;
    // Débogage : un trait fin du centre de chaque personnage vivant à son point visé, et un petit
    // cercle à ce point.
    void dessinerPointsVises(sf::RenderTarget& rendu) const;
    // Lu par les tests : le contenu de la mémoire (identifiant et minuterie) ; par le comportement, il
    // exigerait des simulations de plusieurs secondes.
    [[nodiscard]] const std::vector<Agresseurs::Agresseur>& getAgresseurs() const;
    // Le sac de l'escouade (cases à position fixe) et l'équipement du personnage de cette place
    // (`assert` : place valide). Lus par l'écran d'inventaire.
    [[nodiscard]] const Inventaire& sac() const;
    [[nodiscard]] const Equipement& equipementDuPersonnage(Place place) const;
    // Le même, sans ses objets inactifs (seuil non atteint) : celui dont l'armure et les bonus ont un effet (étape 92.2).
    [[nodiscard]] const Equipement& equipementActifDuPersonnage(Place place) const;
    // Les opérations sont transactionnelles : vraies, tout a été fait ; fausses, rien n'a bougé.
    // Un objet n'est jamais à deux endroits, ni nulle part. L'armure du personnage est recalculée.
    // Équiper ou déséquiper un personnage mort est permis. `assert` : place valide, case du sac < capacité.
    // Met l'objet dans la première case libre du sac ; faux si le sac est plein.
    [[nodiscard]] bool ramasser(Objet objet);
    // Met tous ces objets dans le sac, dans l'ordre, ou aucun : faux, sans rien changer, s'il n'y a pas assez de cases
    // libres (même esprit que `ramasser`, transactionnel). Sert à la touche de débogage F7.
    [[nodiscard]] bool ramasserTout(const std::vector<Objet>& objets);
    // L'objet à cette position, s'il y en a un (`assert` : place valide, case du sac < capacité).
    [[nodiscard]] std::optional<Objet> objetA(const PositionDObjet& position) const;
    // Un objet peut-il aller de `de` à `vers` (case du sac ou emplacement d'un personnage) ? Applique la règle
    // des déplacements (`deplacerLesObjets`, dont les deux mains) sur une copie. L'écran s'en sert pour la surbrillance.
    [[nodiscard]] bool peutDeplacerObjet(const PositionDObjet& de, const PositionDObjet& vers) const;
    // Les seuils que le personnage qui recevrait l'objet de `de` à `vers` n'atteindrait pas (`seuilsManques`) ; vide si
    // rien ne manque ou si le déplacement est refusé pour une autre raison. L'écran les montre en rouge (étape 92.3).
    [[nodiscard]] std::vector<SeuilDeCaracteristique> seuilsManques(const PositionDObjet& de, const PositionDObjet& vers) const;
    // L'objet porté à cet emplacement est-il inactif (porté, mais son seuil n'est plus atteint) ? Faux sans objet.
    [[nodiscard]] bool objetPorteEstInactif(Place place, Emplacement emplacement) const;
    // L'objet de `de` va à `vers` ; si `vers` est occupé, les deux objets s'échangent ; une arme à deux mains range
    // l'objet de la main gauche dans le sac (et un bouclier une arme à deux mains). Faux, sans rien changer, si
    // `peutDeplacerObjet` est faux ; vrai sans effet si `de == vers`.
    [[nodiscard]] bool deplacerObjet(const PositionDObjet& de, const PositionDObjet& vers);
    // Retire l'objet de cette position (case du sac ou emplacement) et le rend ; vide, sans rien changer, si la position est
    // vide. Un retrait ordinaire : sa case reste vide, l'armure, les caractéristiques et les défenses du personnage sont
    // recalculées, et un skill (en activation ou en attente) qui exigeait l'arme retirée est abandonné. L'objet n'est plus
    // nulle part : celui qui l'a retiré doit le poser ailleurs (le sol, `Partie::jeterUnObjet`).
    [[nodiscard]] std::optional<Objet> retirerObjet(const PositionDObjet& position);
    // Pose une provocation sans lancer de skill. L'effet d'un skill de provocation la pose lui-même ; cette fonction sert aux tests, qui
    // ont besoin d'un monstre provoqué sans en passer par le skill.
    void provoquer(IdMonstre idMonstre, Place place, float duree);
    // Lu par les tests : le provocateur, l'identifiant et la durée restante, que candidatsDeFrappe ne montre pas.
    [[nodiscard]] const std::vector<Provocations::Provocation>& getProvocations() const;

private:
    // Le sac et l'équipement de chaque personnage, copiés : l'état sur lequel se décide un déplacement.
    [[nodiscard]] EtatDesObjets etatDesObjets() const;
    // Cette place est celle d'un personnage de l'escouade (de 1 à N).
    [[nodiscard]] bool placeValide(Place place) const;
    [[nodiscard]] Personnage& personnageDeLaPlace(Place place);              // place valide exigée
    [[nodiscard]] const Personnage& personnageDeLaPlace(Place place) const;  // place valide exigée
    [[nodiscard]] bool skillEnAttenteDeDegats() const;
    void annulerSkillDeDegatsEnAttente();
    // Abandonne le skill en attente dont le lanceur n'a plus l'arme qu'il exige (sans recharge consommée).
    void abandonnerLeSkillEnAttenteSansArme();
    [[nodiscard]] std::optional<Place> placeDuVivantLePlusProche(sf::Vector2f point) const;
    [[nodiscard]] std::optional<Place> placeDeLaProie(IdMonstre idMonstre, sf::Vector2f positionMonstre) const;
    // Avec quoi un personnage atteint une cible : son attaque de base ou son skill.
    // `skill` vide : l'attaque de base.
    [[nodiscard]] bool peutAtteindre(const Personnage& personnage, std::optional<IdSkill> skill, Cercle cible, const TileMap& carte) const;
    [[nodiscard]] bool peutLancerAuPoint(AdresseSkill adresse, sf::Vector2f point, const TileMap& carte) const;
    std::vector<ResultatDEffet> avancerSkillDe(Place place, float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    // Le skill part : son départ, puis son effet (`std::visit` sur l'effet) appliqué à chaque entité que `entitesTouchees` désigne. Les
    // soins et les provocations ont lieu ici ; les dégâts sortent en `Coup`.
    std::vector<ResultatDEffet> appliquerLEffet(Place place, IdSkill id, const std::optional<Cible>& cible, const Attaque& attaque, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    // Les entités qu'un skill de ce lanceur touche : sans rayon de zone, la cible (le lanceur lui-même pour un skill sans cible) ; avec
    // un rayon, toutes celles du camp que touche son effet (`campTouche`) qui sont dans le cercle de ce rayon centré sur la cible, le
    // point visé ou le lanceur, et en vue de ce centre (`ligneDeVue` : le sort ne traverse pas les murs).
    struct EntitesTouchees {
        std::vector<IdMonstre> monstres;
        std::vector<Place> allies;
    };
    [[nodiscard]] EntitesTouchees entitesTouchees(const Skill& skill, Place lanceur, const std::optional<Cible>& cible, const std::vector<MonstreVu>& monstres, const TileMap& carte) const;
    // L'intention que le skill en attente donne à son lanceur.
    struct IntentionDuSkill {
        Place lanceur;
        Intention intention;
    };
    // `verrou` : le monstre verrouillé, s'il est ciblable (vide sinon).
    std::optional<IntentionDuSkill> avancerSkillEnAttente(const TileMap& carte, const std::optional<MonstreVu>& verrou);
    // Étapes d'attaquer(), dans l'ordre où elles sont appelées.
    void actualiserMemoires(const std::vector<MonstreVu>& monstres);
    void engagerSkillDeZoneEnAttente(const TileMap& carte);
    [[nodiscard]] bool tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const;
    void approcherVerrou(const std::vector<MonstreVu>& monstres, const TileMap& carte);
    void engagerSkillDeDegatsEnAttente(const std::vector<MonstreVu>& monstres, const TileMap& carte);
    std::vector<ResultatDEffet> avancerSkills(float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    std::vector<ResultatDEffet> autodefense(float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] float porteeDe(const Personnage& personnage, std::optional<IdSkill> skill) const;
    // Places d'attaque (m_placesDAttaque) : ce qu'elles doivent savoir du personnage de cet indice
    // qui approche ce monstre avec une action de cette portée.
    [[nodiscard]] PlacesDAttaque::Approche approcheDe(std::size_t indice, MonstreVu monstre, float portee) const;
    // Par personnage : son centre s'il est vivant et immobile (il occupe l'endroit où il se tient), vide sinon.
    [[nodiscard]] std::vector<std::optional<sf::Vector2f>> occupants() const;
    // Étapes de mettreAJour().
    void ecoulerMemoires(float dt);
    void separerPersonnages(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles);
    // Ce qui vaut pour tous les personnages dans un mettreAJour, calculé avant la boucle des personnages.
    struct ContexteDesIntentions {
        bool ancreArretee;
        std::optional<MonstreVu> verrou;         // le monstre verrouillé, s'il est ciblable
        std::vector<MonstreVu> agresseurs;       // la mémoire des agresseurs qui sont ciblables, avec leur cercle
        std::optional<IntentionDuSkill> skill;   // l'intention du lanceur du skill en attente
    };
    [[nodiscard]] std::optional<MonstreVu> confirmerDegagement(std::size_t indice,
                                                               const std::optional<PlacesDAttaque::Place>& ancienne,
                                                               const std::vector<MonstreVu>& monstres,
                                                               const TileMap& carte);
    [[nodiscard]] SituationDuPersonnage situationDe(std::size_t indice,
                                                    const std::optional<PlacesDAttaque::Place>& ancienne,
                                                    const std::optional<MonstreVu>& degagement,
                                                    const ContexteDesIntentions& contexte,
                                                    const std::vector<MonstreVu>& ciblables,
                                                    const TileMap& carte) const;
    // Le point visé par une intention et, pour une approche, si une place d'attaque a été trouvée.
    struct PointVise {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
        sf::Vector2f point;
        bool placeTrouvee;
    };
    PointVise pointDeLIntention(std::size_t indice, const Intention& intention,
                                const std::optional<PlacesDAttaque::Place>& ancienne, const TileMap& carte);
    // Dégagement : ces deux fonctions décident, avec m_degagements, qui se dégage de quel monstre ;
    // choisirIntention en fait sa règle 2 (le point de branchement des futurs gambits).
    void mettreAJourDegagements(bool ancreArretee, const std::vector<MonstreDeControle>& monstresDeControle);
    [[nodiscard]] std::optional<IdMonstre> degagementVoulu(std::size_t indice, bool ancreArretee, const std::vector<PersonnageDeControle>& personnages,
                                       const std::vector<MonstreDeControle>& monstresDeControle) const;

    struct SkillEnAttente {
        AdresseSkill adresse;  // du skill en attente (place du lanceur et identifiant)
        Cible cible;        // fixée à l'appui (pour un monstre, le cercle lu en route est celui du verrou, à jour)
    };

    Mobile m_mobile;
    Formation m_formation;  // décalages, orientation et places de formation autour de l'ancre
    std::vector<Personnage> m_personnages;
    std::optional<IdMonstre> m_cibleVerrouillee;
    std::optional<SkillEnAttente> m_skillEnAttente; // un seul skill en attente pour toute l'escouade
    PlacesDAttaque m_placesDAttaque;  // par personnage : place d'attaque retenue autour du monstre qu'il approche
    std::vector<sf::Vector2f> m_pointsVises;  // par personnage, calculé par mettreAJour
    // Par personnage : identifiant du monstre dont il se dégage vers sa place d'attaque (vide = il ne se dégage pas).
    std::vector<std::optional<IdMonstre>> m_degagements;
    Agresseurs m_agresseurs; // monstres qui ont frappé l'escouade récemment
    Provocations m_provocations; // monstres provoqués par un personnage
    Inventaire m_sac;            // les objets que personne ne porte
    int m_niveau;                // le même pour tous les personnages, >= 1
};