# Squad-ARPG — Boucle de jeu et entrées

Depuis l'étape 52, trois acteurs se partagent ce qui tenait dans `main()`.
Depuis l'étape 65, le rôle de `main.cpp` décrit ci-dessous est tenu par
`EcranJeu` (partie, `Controleur`, caméra, traduction des événements,
compteur, dessin) et par `Application` (fenêtre, police, horloge, boucle
SFML) : partout où ce fichier écrit « `main.cpp` », lire `EcranJeu`, sauf pour
la fenêtre, la police, l'horloge et `setKeyRepeatEnabled`, qui sont à
`Application`. La pile d'écrans, les vues et les réglages sont dans
`docs/ecrans.md`.


| Acteur | Rôle | Connaît SFML Window/Event ? |
|---|---|---|
| `EcranJeu` (et `Application` pour la fenêtre, la police et l'horloge) | Caméra, traduction des événements SFML en appels au `Controleur`, retour à l'écran de choix de l'escouade (touche R après une défaite ou une victoire), nouvelle carte (F5), bascules de débogage (F3 points visés, chemins et monstres qui rentrent semi-transparents, F4 ancre, F6 brouillard de guerre ; `OptionsDeDebogage`), compteur de monstres, conversion du curseur en coordonnées du monde, enchaînement du dessin | Oui, seuls |
| `Controleur` | État d'interface : mode visée (`m_adresseEnVisee`) et déplacement continu (`m_deplacementContinu`), sans référence à la `Partie` (reçue à chaque appel) ; traduit les actions du joueur en demandes à la `Partie` ; dessine l'aperçu de visée | Non |
| `Partie` | Règles de jeu : possède la carte, l'`Escouade`, les `Monstre` et la `Vision` ; intermédiaire entre `Escouade` et `Monstre`, qui ne se connaissent pas | Non |

`Niveau` (`Niveau.hpp/.cpp`) est une structure de données (plan, taille de
case, départ de l'escouade, positions des monstres) ; `creerNiveau()` renvoie
le niveau du jeu et `Partie` se construit à partir de lui. Pas de lecture de
fichier : un seul niveau, codé dans `Niveau.cpp`.

## Boucle de jeu

- **Ordre d'une image** (tel que l'exécutent `main.cpp` et
  `Partie::mettreAJour`) :
  0. `EcranJeu::traiterEvenement` et `mettreAJour` : retour à la vue `m_camera`
     (l'image précédente a fini sur la vue de l'interface : sans cela,
     `mapPixelToCoords` convertirait les clics avec la mauvaise vue), taille de
     la caméra recalculée (`tailleDeVue`). `controleur.debutImage(partie)` (sortie
     de la visée si le lanceur ne peut plus lancer : mort, défaite) s'exécute
     depuis l'étape 65 à la fin de `mettreAJour`, donc avant les événements de
     l'image suivante ;
  1. événements, traduits en appels au `Controleur` : fermeture ;
     `appuiGauche(partie, point)` (ou `appuiInterface` si le clic tombe sur la barre de
     raccourcis, voir « Barre de raccourcis »), `appuiDroit()`,
     `activerEmplacement(partie, barre, emplacement)` (touche de la barre), `appuiEchap()`, et R
     (voir « Défaite de l'escouade »). Le `Controleur` les convertit en demandes à la `Partie` :
     `cliquer` (appliqué tout de suite : pose ou retire le verrou), et
     `demanderSkill`, `lancerSkillDeZone`, mémorisés ;
  2. `controleur.mettreAJourDeplacement(partie, boutonTenu, curseur)` : si le bouton
     est tenu et le déplacement continu armé, `partie.ordonnerDeplacement`
     (mémorisé) ;
  3. `partie.mettreAJour(dt)`, qui ne fait rien si la partie est perdue ou gagnée,
     sinon déroule, dans cet ordre exact :
     a. verrou levé si le monstre verrouillé n'est plus dans le vecteur ou n'est
        plus ciblable (il rentre à son poste, étape 69) ; liste des `EntiteVue`
        des **cibles** (monstres ciblables, une fois pour toute l'image) ;
     b. lancer du skill de zone demandé ;
     c. skill demandé (avant `attaquer` : un skill qui part relance la
        recharge de base de son lanceur, qui ne frappe donc pas deux fois) ;
     d. `attaquer`, si l'escouade est vivante, et application des `Coup` ;
        verrou levé si le monstre verrouillé est mort ;
     e. retrait des monstres morts ;
     f. ordre de déplacement, si l'escouade est vivante ;
     g. photo des camps pour les zones de contrôle (`photographierLesCamps`, avant tout
        déplacement), puis `escouade.mettreAJour`, qui reçoit une `VueDesMonstres` (étape 82.4) :
        les `EntiteVue` de **tous** les monstres (liste physique : obstacles bloquants des
        personnages, dont chacun contourne ceux qu'il n'approche pas), celles des monstres
        **ciblables** (verrou et « monstre à portée », recalculées après le retrait des morts en e)
        et les monstres de la première photo (dégagement des tireurs, étape 78), puis
        `cerclesDesVivants` et le tour de chaque monstre (proie, mise à jour
        avec ses obstacles bloquants et ses cercles à contourner, riposte) ;
     h. seconde photo des camps et coups gratuits des deux sens (`appliquerLesCoupsDOpportunite`,
        voir `docs/combat.md`) : ce sont les positions de fin de l'image qui comptent, après la mise
        à jour de l'escouade, la séparation douce et le tour des monstres ; monstres tués retirés,
        verrou levé ;
     i. `Vision` mise à jour (étape 72), en dernier pour que le dessin corresponde aux
        positions de la fin de l'image ;
     j. les demandes sont vidées ; les événements de combat de l'image restent dans la file de la
        `Partie` jusqu'à ce qu'`EcranJeu` les prenne (étape 79, voir « Événements de combat et
        textes flottants ») ;
  3 bis. `EcranJeu::mettreAJour`, juste après : `partie.prendreEvenements()`, chaque événement devient un
     texte flottant (sauf dans le brouillard), puis `m_textes.mettreAJour(dt)` ; aussi quand la partie est
     figée ;
  4. `main.cpp` : caméra centrée sur l'escouade, `partie.dessiner` (carte,
     monstres visibles avec le contour du verrou, semi-transparents s'ils rentrent à leur
     poste et que F3 est actif, escouade, voile de la `Vision` sauf si F6 l'a coupé), `controleur.
     dessinerApercu`, les débogages de F3, les textes flottants (vue du monde, après le voile du
     brouillard), puis, dans la vue d'interface, barre de raccourcis, compteur
     « Monstres : N » (haut gauche) et, si la partie est perdue ou gagnée,
     écran de fin (par-dessus la barre et le compteur), le tout avec la vue de
     l'interface (800 px de haut, voir `docs/ecrans.md`).
- **`attaquer` (d) et `escouade.mettreAJour` (g) ne se passent plus rien par des membres**
  (étape 82.4, R2 de `docs/audit.md`) : le verrou et « un monstre à portée » sont calculés par chacune
  sur les ciblables qu'elle reçoit. Leur ordre n'est donc plus une règle de correction ; `Partie`
  garde le sien. Seule la mémoire des agresseurs, actualisée par `attaquer`, aurait une image de
  retard si on les inversait (limite connue, `docs/formation.md`, « Intentions »).
- **Demandes mémorisées** : à part `cliquer`, les actions du joueur ne sont
  pas exécutées à l'événement mais à leur place dans `mettreAJour`. Un clic
  est appliqué tout de suite parce qu'il ne dépend que de l'état au moment
  du clic ; les autres doivent passer avant `attaquer` ou avant la mise à
  jour, dans un ordre qui compte. Les demandes ne survivent pas à l'image.
- **Invariant** : entre deux appels de `mettreAJour`, tout monstre du vecteur
  est vivant (documenté dans `Partie.hpp`) ; ni `cliquer` ni le dessin n'ont
  besoin de tester `estVivant()`.
- **Déplacement à pas fixe** plutôt que proportionnel à la distance :
  vitesse constante, comme un personnage d'ARPG.
- **Delta time** (`sf::Clock::restart()`) : vitesse en pixels par seconde.
  `setFramerateLimit(60)` limite l'usage du CPU, mais la logique n'en dépend
  pas. L'horloge est dans `Application::executer` (étape 65) ; `dt` est passé en paramètre.
- **`dt` borné à `s_dtMax` = 1/20 s** (étape 80, audit L1), à un seul endroit, dans
  `Application::executer` (`std::min`). Après un gel (fenêtre déplacée par sa barre de titre, point
  d'arrêt, mise en veille), l'horloge rendait plusieurs secondes d'un coup : le glissement ne teste
  que la position d'arrivée, donc un personnage à 240 px/s sautait un mur d'une case (40 px) dès
  `dt` ≈ 0,25 s, et les minuteries (activations, recharges, provocations) sautaient d'autant. Borné,
  le jeu ralentit pendant le gel au lieu de sauter. À 60 images par seconde, `dt` vaut 1/60 s : la
  borne n'a aucun effet en temps normal. Pas de pas fixe avec accumulateur : inutile tant que la
  simulation est déterministe par image (voir `docs/audit.md`, « Pistes écartées »). C'est, avec la
  règle du lanceur occupé (`docs/combat.md`), un des deux changements de comportement de l'étape 80.
- **Pas de `Camera`, de bus d'événements ni de hiérarchie d'états pour la
  visée** : la vue `camera` reste une variable de `main.cpp`, la visée est un
  `std::optional<std::size_t>` du `Controleur`.
- **Pas de texte de position** : rien n'affiche la position du curseur.
- **Générateur aléatoire (étape 56)** : `Partie` possède un `Generateur`
  initialisé par la graine de son constructeur et le passe par référence
  aux appels qui peuvent déclencher un skill (`attaquer`, `declencherSkill`,
  `declencherSkillDeZone`, `escouade.mettreAJour`, `Monstre::frapper`).
  `main.cpp` tire la graine avec `std::random_device` (variable locale de
  `main`) au démarrage et à chaque recommencement, et l'affiche dans la
  console (`Graine : …`) pour rejouer une partie à l'identique. L'ordre
  d'une image ne change pas.
- **`prochainId`** n'est pas un membre de `Partie` : il ne sert qu'à numéroter
  les monstres dans son constructeur.

## Entrée souris

- **Un seul événement d'appui décide de tout** : `MouseButtonPressed`
  (bouton gauche) décide, à l'instant du clic, ce que fait ce clic, peu
  importe où le curseur va ensuite. `Controleur::appuiGauche` : hors visée,
  `Partie::cliquer` renvoie `Monstre` (verrou posé, déplacement continu
  désarmé ; un monstre hors de vue n'est pas candidat, voir `docs/vision.md`), `Objet`
  (porte fermée cliquée, dont une case au moins n'est pas inconnue : verrou levé, interaction mémorisée,
  déplacement continu désarmé ; voir `docs/carte-et-collision.md`), `Sol` (verrou levé, déplacement continu armé) ou `Ignore`
  (escouade vaincue : rien ne change, y compris le déplacement continu).
- **Déplacement continu : lecture d'état, armée par l'événement d'appui** :
  `main.cpp` calcule `boutonTenu` = fenêtre avec le focus ET
  `sf::Mouse::isButtonPressed(Left)` et le passe à
  `mettreAJourDeplacement`, une fois par image. Tant que le bouton est tenu
  et le déplacement armé, la `Partie` rappelle `allerVers` avec le curseur
  (même A* + lissage qu'un clic ponctuel), si l'escouade est vivante.
  Cliquer sur un monstre puis glisser le curseur sur le sol ne déplace donc
  rien.
- **`window.hasFocus()`** est nécessaire : `isButtonPressed` lit l'état
  global de la souris, même sans focus.
- **Position du curseur** : convertie par `main.cpp` avec
  `window.mapPixelToCoords(...)` avec la vue de l'image précédente (celle
  effectivement affichée au moment du clic), avant la mise à jour de la
  caméra. L'aperçu de visée, lui, est converti avec la caméra de l'image.
- **Pas de limitation de fréquence** : un appel au A* par image, négligeable
  sur la carte actuelle.

## Combat engagé

- **Le verrou vit dans `Escouade`** (voir `docs/formation.md`) ; il stocke
  l'identifiant du monstre visé. `Partie` le retrouve par identifiant à
  chaque image, car un monstre peut être retiré du vecteur entre deux images.
- **Le verrou ne se rompt que dans quatre cas** : mort de la cible (monstre
  introuvable ou qui vient de mourir), abandon de la cible (elle n'est plus
  ciblable : elle rentre à son poste, étape 69), clic sur le sol, clic sur un
  autre monstre. Relâcher le bouton ou perdre le focus ne rompt pas le combat.
  Aucune zone de tolérance autour du monstre. Un clic sur un monstre qui rentre
  à son poste compte comme un clic au sol (`Sol`, déplacement continu armé).
- **L'attaque est dans `Escouade`** : ni `main.cpp` ni `Controleur` n'ont de
  recharge ni de dégât. `Partie` passe la liste des `EntiteVue` à
  `escouade.attaquer` et applique chaque `Coup` renvoyé au monstre qui porte
  l'identifiant. `attaquer` est appelé à chaque image, **verrou ou non** :
  avec un verrou elle décide si l'escouade s'approche ou s'arrête ; dans tous
  les cas chaque personnage vivant frappe de lui-même un monstre à sa portée
  (autodéfense, voir `docs/combat.md`).
- **Indicateur de cible** : `Partie::dessiner` donne à chaque monstre
  `escouade.getCibleVerrouillee() == m.getId()` (voir `docs/monstre.md`).

## Entrée clavier : le skill

- **Événement d'appui, pas lecture d'état** : un appui = une tentative.
  `EcranJeu` traduit la touche en emplacement (`emplacementPourTouche`) puis en
  `controleur.activerEmplacement(partie, barre, emplacement)` ; le
  `Controleur` lit l'adresse de l'emplacement et envoie les skills qui ne sont pas de type
  `Zone` en `partie.demanderSkill(adresse)`. Plusieurs
  appuis dans la même image : le dernier gagne (une seule demande
  mémorisée).
- **`window.setKeyRepeatEnabled(false)`** (dans `main.cpp`) : une touche
  tenue ne produit qu'un appui, sans quoi elle relancerait un skill
  rechargé.
- **Scancode plutôt que code de touche** : `sf::Keyboard::Scan::Q`
  désigne la touche physique en haut à gauche de la rangée des lettres, quelle que soit la
  disposition (en AZERTY elle est étiquetée « A »). L'icône affiche l'étiquette du
  clavier (`sf::Keyboard::getDescription`).
- **Skill demandé avant `attaquer`** : voir l'ordre d'une image.
  `declencherSkill` est appelée même sans verrou : le soin marche sans
  combat, et le skill de dégâts vise alors le monstre que son lanceur
  frapperait en autodéfense.
- **Correspondance des touches** : voir « Barre de raccourcis » ci-dessous. La table provisoire
  de quatre adresses de l'étape 74 (touches 1 à 4) a disparu. La visée est déduite du **type** du
  skill (`Zone`), jamais de la touche.

## Barre de raccourcis (étape 75)

- **Trois notions séparées** : les **emplacements** (le modèle, `BarreDeRaccourcis` : une
  liste d'`optional<AdresseSkill>`), les **touches** (une table touche → emplacement, une donnée,
  `Reglages::touchesDesRaccourcis`) et la **disposition** à l'écran (`rectangleDeLEmplacement`,
  calculée par la vue seulement). L'emplacement i reste l'emplacement i quelles que soient sa
  touche et sa place à l'écran : changer l'une des deux ne touche ni le modèle ni l'autre. Les
  futures fonctions (écran de configuration des touches, barre en ligne droite, barre d'auras,
  interface de placement) se brancheront sur l'une des trois sans réécrire les autres.
- **Indépendante des personnages** : la barre pointe vers n'importe quel skill de n'importe quel
  personnage (une `AdresseSkill`). Aucune limite par personnage ; une même adresse peut être
  placée sur deux emplacements (décision reportée, voir `docs/brainstorm.md`).
- **Propriétaire : `EcranJeu`** (état d'interface, comme la caméra). Ni `Partie` ni `Escouade`
  ne savent qu'une barre existe : `Escouade::adressesDesSkills()` fournit la liste ordonnée
  (place puis liste du personnage), `barreParDefaut` la dispose, `EcranJeu` reconstruit la barre
  à chaque nouvelle partie (au même endroit que `m_controleur = Controleur{}`). Le `Controleur`
  reçoit la barre à chaque appel, sans la garder.
- **Disposition par défaut** : une rangée de 4 emplacements par personnage, dans l'ordre des
  places. Avec la composition actuelle : Q = frappe puissante, W = provocation, A = soin,
  Z = explosion, le reste vide. Un 5ᵉ skill d'un personnage n'est pas placé (sans erreur).
- **Touches** : grille Q W E R / A S D F / Z X C V, scancodes (touches physiques). La table est
  dans `Reglages` mais n'est ni lue ni écrite dans `reglages.ini` : `EcranJeu` la lit déjà dans
  le `Contexte`, c'est le chemin que prendra la configuration. R est l'emplacement 3 en jeu ; sur
  l'écran de fin (perdue ou gagnée), R ramène au choix de l'escouade et les raccourcis sont ignorés. F3 à F6 et
  Échap ne sont pas des touches de la grille.
- **Une seule action pour la touche et le clic** : `Controleur::activerEmplacement`. Vide : rien.
  Skill de type `Zone` : entre en visée de cette adresse, ou en sort si c'est déjà elle (même
  placée sur deux emplacements) ; un autre skill de zone remplace la visée ; mêmes conditions
  qu'avant (escouade vivante, skill prêt) et désarme le déplacement continu. Autre type :
  `demanderSkill` ; en visée, la visée continue.
- **Règle « l'interface d'abord »** : à l'appui gauche, `EcranJeu` convertit le pixel avec la vue
  d'interface et teste la barre (`clicSurInterface`, qui s'appuie sur `emplacementSous`)
  **avant** tout traitement dans le monde. Un clic sur la barre est consommé : il n'atteint
  jamais `Controleur::appuiGauche` (ni verrou, ni porte, ni déplacement, ni point d'impact de
  visée) ; il appelle `Controleur::appuiInterface`. Sur l'écran de fin il est consommé sans effet.
  `ClicInterface { Barre barre; std::size_t emplacement; }` : l'enum `Barre` n'a qu'une valeur
  (`Raccourcis`) ; la barre d'auras s'y ajoutera, et le `switch` d'`appuiInterface` signalera le
  cas oublié.
- **Trois cas limites** : (1) un appui qui commence sur la barre ne lance pas le déplacement
  continu et désarme un déplacement continu déjà armé (sinon un ancien clic au sol ferait repartir
  l'escouade tant que le bouton est tenu) ; (2) un déplacement continu commencé au sol continue
  quand le curseur passe au-dessus de la barre (sa cible est le point du monde sous le curseur ;
  la décision est prise à l'appui seulement, `mettreAJourDeplacement` n'a pas changé) ; (3) en
  visée, un clic sur la barre est un clic d'interface, jamais le point d'impact.
- **Clic droit** inchangé (annule la visée, où qu'il soit). Pas de survol, pas d'infobulle.
- **Dessin** : `EcranJeu::dessiner` construit une `IconeDeRaccourci` par emplacement (état du
  skill demandé à l'escouade, nom de la touche, couleur du personnage lanceur par
  `Escouade::couleurDuPersonnage`, contour de visée) et `dessinerBarreDeSkills` les dessine aux
  rectangles de `rectangleDeLEmplacement` : grille de 4 colonnes × 3 rangées, icônes de 48 px,
  4 px d'écart, centrée en bas de la vue d'interface avec 10 px de marge. Le dessin et le clic
  partagent ce même calcul. Un emplacement vide est un cadre sombre avec seulement le nom de sa
  touche.
- **Limites connues** : touches non configurables ni sauvegardées ; une seule barre ; la barre ne
  montre pas le skill en attente ; pas de survol, d'infobulle ni de clic droit sur la barre ; pas
  d'interface pour placer les skills. Dans la grille par défaut, une 4ᵉ place n'a pas de rangée.

## Mode visée : le skill de zone (étape 42)

- **État d'interface, pas état de jeu** : `m_adresseEnVisee` (`std::optional<AdresseSkill>`)
  vit dans le
  `Controleur`. `Escouade` et `Partie` ne connaissent ni la souris ni le
  mode ; l'`Escouade` expose `skillEstPret`, `rayonZoneDuSkill`,
  `cercleDePorteeDuSkill` et `declencherSkillDeZone`.
- **Entrée** : activation de l'emplacement d'un skill de zone (touche ou clic) avec l'escouade
  vivante et le skill prêt ; sinon ignoré. **Sorties sans lancer** : clic droit, Échap, nouvelle
  activation de la même adresse,
  mort du lanceur ou défaite (`debutImage`, par `!skillEstPret`). Aucune
  recharge consommée.
- **Lancer** : uniquement sur l'événement d'appui gauche reçu pendant la
  visée. Le clic lance le sort même sur un monstre : il ne verrouille pas, ne
  déverrouille pas, ne déplace pas l'escouade. Après le lancer, réussi ou
  refusé, on sort de la visée. Le `Controleur` appelle
  `partie.lancerSkillDeZone`, qui n'agit que dans `mettreAJour`, avant le
  skill demandé et avant `attaquer`.
- **Écart minime avec l'avant-étape 52** : le lancer est transmis à la
  `Partie` au moment du clic. Autrefois, un Échap, un 4 ou un clic droit
  arrivant dans la même image après le clic l'annulait ; maintenant le sort
  part quand même.
- **Bouton déjà enfoncé** : entrer en visée désarme le déplacement continu ;
  il ne se réarme que sur un nouvel appui au sol. L'escouade ne suit donc pas
  le curseur ni pendant la visée ni après le lancer, tant que le bouton n'a
  pas été relâché puis réappuyé. Le drapeau joue le rôle d'un « attendre le
  relâchement » : pas de booléen de plus.
- **Visuel** : `Controleur::dessinerApercu(partie, rendu, curseur)` (curseur en
  coordonnées du monde, converti par `main.cpp` avec la caméra de l'image) :
  un `sf::CircleShape` jaune, sans remplissage, au rayon exact de la zone,
  sous le curseur, et un cercle blanc (portée de lancement) autour du
  lanceur. Rien hors visée, rien à l'impact.
- **Portée (étape 43)** : le clic lance toujours le sort et sort de la visée ;
  hors portée, `Escouade` met le sort en attente et le lanceur marche (voir
  `docs/combat.md`).
- **Les touches des skills qui ne sont pas de zone** restent actives pendant la visée.

## Défaite de l'escouade

- **`estVivante()` est consultée, jamais mise en cache** : `Partie::cliquer`
  la lit à chaque clic (`Ignore` si vaincue) ; le `Controleur` la lit pour
  entrer en visée. `Partie::estPerdue()` (`!estVivante()`) est la règle de
  défaite, dans le noyau et non dans `main.cpp`.
- **Le monde se fige (étape 54)** : `Partie::mettreAJour` sort tout de suite
  si la partie est perdue : plus aucun monstre ne bouge, ne frappe ni ne
  rentre à son poste. La boucle SFML continue (caméra, dessin, fermeture).
  Les garde-fous `estVivante()` internes à `mettreAJour` restent, inoffensifs.
- **Écran de fin** : `dessinerEcranDeFin` (module `EcranDeFin`, fonction
  libre sur le modèle de `BarreDeSkills`, titre en paramètre) est dessiné
  dans la vue par défaut, après la barre de skills : le voile
  semi-transparent la laisse visible dessous. `main.cpp` l'appelle avec
  « Défaite » si `partie.estPerdue()`, sinon avec « Victoire » si
  `partie.estGagnee()`.
- **Recommencer = remplacer l'objet** : depuis l'étape 65, R (seulement si la
  partie est perdue ou gagnée, ignorée en jeu) ramène à l'écran de choix de l'escouade (`Depiler`),
  et « Commencer » crée un `EcranJeu` neuf, donc une nouvelle `Partie` (même sélection de classes).
  F5 (touche de débogage, à tout moment, y compris en visée) appelle
  `EcranJeu::nouvelleCarte`, qui, comme l'ancienne lambda de `main.cpp`,
  fait `partie = nouvellePartie(tirage)` (nouvelle graine tirée et affichée,
  nouvelle carte générée) et `controleur = Controleur{}`. Le lancement
  utilise la même `nouvellePartie`. Pas de `reinitialiser()` : un état remis à
  zéro membre par membre risque d'oublier un membre. Une visée ou un
  déplacement continu en cours ne survivent pas. La caméra, recentrée à
  chaque image, suit l'escouade neuve sans code propre. R reste réservée aux
  écrans de fin.
- **Pourquoi `Controleur` reçoit la `Partie` à chaque appel** : une
  référence membre empêche la réaffectation (C++ Core Guidelines C.12).
  Avec une référence, `controleur = Controleur{}` était impossible, et
  remplacer la `Partie` par un `std::optional` aurait aussi demandé de
  reconstruire le `Controleur`. `Controleur` est maintenant un type valeur ;
  `Partie` reste une variable ordinaire de `main` et, remplacée par
  déplacement, garde son adresse.

## Victoire (étape 63)

- **Règle** : `Partie::estGagnee()` = aucun monstre dans le vecteur ET
  `!estPerdue()`. La défaite l'emporte : si le dernier monstre et le dernier
  personnage meurent dans la même image, la partie est perdue (gagner exige
  un survivant). La règle vit dans le noyau, comme `estPerdue`.
- **Même figement que la défaite** : `mettreAJour` sort tout de suite si
  `estPerdue() || estGagnee()`, un seul chemin de code. `cliquer` n'est pas
  bloqué après une victoire (il ne change rien d'utile : le monde est figé).
- **Compteur** : `Partie::monstresRestants()` plutôt qu'un comptage dans
  `main.cpp`, qui ne connaît pas `Monstre`. Dessiné par une fonction du
  namespace anonyme de `main.cpp` (un simple `sf::Text`, pas un module : une
  seule utilisation), en haut à gauche pour ne pas gêner la barre de skills
  en bas.
- **Carte sans monstre** : si le générateur n'en place aucun (très rare), la
  partie est gagnée dès le départ. Acceptée : F5 suffit, le générateur n'est
  pas modifié.
- **Tests** : les tests de `Controleur` qui font avancer la partie ont un
  monstre lointain (`s_monstreLointain`), sinon une partie sans monstre est
  gagnée donc figée.

## Les monstres dans `Partie::mettreAJour`

- **Deux listes d'`EntiteVue`** (étape 69) : `entitesCiblables()` (les monstres pour
  lesquels `estCiblable()` est vrai : pas ceux qui rentrent à leur poste) et
  `entitesPhysiques()` (tous). Les **cibles** vont à tout ce qui choisit un monstre
  à frapper, provoquer, viser ou verrouiller (`attaquer`, `declencherSkill`,
  `declencherSkillDeZone` : autodéfense, agresseurs, provocations et skills
  l'ignorent sans code propre ; les agresseurs et provocations déjà enregistrés
  sont oubliés par `attaquer`, un coup engagé sur lui est annulé) ; depuis l'étape 82.4,
  `escouade.mettreAJour` les reçoit aussi (`VueDesMonstres::ciblables`), pour le verrou et
  « un monstre à portée ». Les **physiques** vont aux obstacles : un monstre qui rentre bloque et
  est contourné comme avant. Les listes sont recalculées à chaque usage, jamais stockées.
- **Obstacles pour les personnages** : avant `escouade.mettreAJour`, la liste
  physique des `EntiteVue` de tous les monstres (à leur position de l'image précédente) ;
  `Escouade` en tire les obstacles bloquants et, par personnage, les cercles à
  contourner (tous sauf le monstre approché, voir `docs/formation.md`).
- **Obstacles pour les monstres** : après `escouade.mettreAJour`, les cercles
  des personnages vivants sont calculés une fois ; chaque monstre en part et y
  ajoute les cercles des autres monstres (identifiant différent). Ces derniers
  forment aussi, seuls, la liste `aContourner` du monstre (`autresMonstres`) :
  les monstres contournent les monstres, pas les personnages. Voir « Qui bloque
  qui » dans `docs/combat.md`.
- **Pour chaque monstre, dans l'ordre** :
  1. `proieDuMonstre(id, centre)` : le provocateur si le monstre est
     provoqué, sinon le personnage **vivant** le plus proche, sur la position
     du monstre *avant* qu'il ne bouge (vide si aucun n'est vivant) ; et
     `candidatsDeFrappe(id)` : le provocateur seul s'il est provoqué, sinon
     tous les vivants (étape 66) ;
  2. `m.mettreAJour(dt, proie, candidats, carte, obstacles, autresMonstres)` : le monstre décide
     seul de poursuivre, s'arrêter dès qu'il a une cible de frappe ou rentrer
     (voir `docs/monstre.md`) ;
  3. `faireFrapperLeMonstre` (étape 57) : un coup engagé avance, et part
     au bout de 0,3 s sur la place retenue (`escouade.recevoirCoup`), annulé
     si ce personnage est mort ; sinon, un monstre immobile avec une cible de
     frappe engage un coup (voir `docs/combat.md`).
- **Zones de contrôle** (étape 77) : deux photos des camps (`CampsDeControle`, privée à `Partie` :
  `EntiteDeControle` des monstres ciblables et des personnages vivants), l'une avant
  `escouade.mettreAJour`, l'autre après le tour des monstres. Un monstre qui passe en retour pendant
  l'image n'est donc pas dans la seconde photo, et un personnage ou un monstre mort pendant l'image non
  plus : aucun coup gratuit ne le concerne. Les coups des deux sens sont calculés avant d'en appliquer un.
- **Alerte du groupe** (étape 64) : avant la boucle, le statut « en poursuite »
  de chaque monstre est noté ; après, `alerterLesGroupes` alerte les membres
  des groupes dont un monstre vient d'entrer en poursuite (voir
  `docs/monstre.md`).
- **Indépendant du verrou** : un monstre poursuit et riposte dès qu'un
  personnage vivant est à sa portée.

## Événements de combat et textes flottants (étape 79)

- **Qui fait quoi** : `Partie` produit des `EvenementDeCombat` pendant `mettreAJour` (voir
  `docs/combat.md`) et les garde dans une file ; `EcranJeu` les lit une fois par image. `Partie` ne sait
  pas qui les lit : pas de callback.
- **Ordre dans `EcranJeu::mettreAJour`** : `Partie::mettreAJour(dt)`, puis `prendreEvenements()` (qui
  vide la file), puis pour chaque événement `TextesFlottants::ajouter` (filtre du brouillard : si
  `m_debogage.brouillard` et que `Partie::estVisible(position)` est faux, l'événement est ignoré),
  puis `TextesFlottants::mettreAJour(dt)`. Les textes ajoutés dans cette image ont donc un âge de `dt`
  au premier dessin, et partagent le même numéro d'image (empilement).
- **Partie figée** : le monde ne bouge plus, mais les textes continuent leur animation (la mise à jour
  des textes ne dépend pas de l'état de la partie) : le dernier coup d'une victoire s'efface
  normalement sous le voile de l'écran de fin.
- **Dessin** : dans la vue du monde (`m_camera`), après le monde, le voile du brouillard, l'aperçu de
  visée et les débogages de F3 ; avant la vue d'interface (barre de raccourcis, compteur, écran de fin).
- **Remise à zéro** : `nouvelleCarte` (F5, et l'entrée dans l'écran) remplace les textes par un
  `TextesFlottants` neuf, comme la barre et le contrôleur.

## Interface fixe à l'écran (étape 48)

- **Deux vues par image** : le monde est dessiné avec `camera`, puis la vue d'interface
  (`vueDInterface`, depuis l'étape 65) donne des coordonnées fixes pour la barre de skills,
  dessinée en dernier. La vue active
  en fin d'image n'est donc plus la caméra : l'image suivante commence par
  `window.setView(camera)` (sinon les clics seraient mal convertis).
- **Police** : DejaVu Sans, libre, committée avec sa licence dans
  `assets/fonts/`. Chargée une fois au départ de `main`, passée par
  `const sf::Font&` au dessin. Un `sf::Text` garde un pointeur vers sa
  police : elle doit vivre plus longtemps que lui. Échec de chargement :
  message sur `std::cerr`, `main` rend 1.
- **`EcranJeu` prépare les icônes** (depuis l'étape 75) : un `std::array<IconeDeRaccourci, 12>`
  rempli à partir de la `BarreDeRaccourcis` et de `escouade.etatDuSkill(adresse)` (voir « Barre
  de raccourcis »). Pas de classe `Hud` : le dessin n'a aucun état propre.
- **Limites connues** : la position se calcule depuis la taille de la vue par
  défaut (le redimensionnement n'est pas géré) ; le chemin de la police est
  relatif au dossier courant.

## Caméra

- **`sf::View` dans `main.cpp`** : une variable `camera`, copiée de
  `window.getDefaultView()`. Pas de classe `Camera` tant qu'il n'y a pas de
  comportement à y encapsuler.
- **Toujours centrée sur l'escouade** : à chaque image, après la mise à jour
  et avant le dessin, `camera.setCenter(partie.getEscouade().getCentre())`
  puis `window.setView(camera)`.
- **Conséquence acceptée** : près des bords de la carte, la caméra montre du
  noir au-delà des murs. L'ancre traversant les monstres, la caméra peut
  s'éloigner des personnages bloqués.

## Barre de skills et adresses (étapes 74 et 75)

- Depuis l'étape 75, `EcranJeu` construit 12 icônes, une par emplacement de la `BarreDeRaccourcis`
  (voir « Barre de raccourcis »). Le contour jaune est celui de l'icône dont l'adresse est
  `Controleur::getAdresseEnVisee()`. `BarreDeSkills` ne connaît ni la barre ni `Escouade` :
  elle reçoit des `IconeDeRaccourci`.
- `Partie` mémorise une `AdresseSkill` (`m_skillDemande`, `LancerDeZone::adresse`) ;
  la dernière demande de l'image gagne, comme avant.

## Qui possède quoi

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1) ; `ARCHITECTURE.md` en garde un résumé.

- `main.cpp` construit une `Application`, appelle `executer()` et affiche
  l'erreur (code 1) si la police manque.
- `Application` possède la fenêtre (`sf::RenderWindow`), la police
  (`sf::Font`), les `Reglages` et la pile d'écrans
  (`std::vector<std::unique_ptr<Ecran>>`). Elle charge `reglages.ini`, ouvre
  la fenêtre, fait tourner la boucle SFML (horloge, événements, mise à jour,
  dessin, `display`) et applique les `Transition` que renvoient les écrans.
  La fermeture de la fenêtre quitte, quel que soit l'écran. Seul l'écran du
  dessus reçoit les événements, est mis à jour et est dessiné.
- `EcranMenu`, `EcranOptions`, `EcranChoixEscouade` et `EcranJeu` sont les quatre `Ecran`. Ils
  reçoivent la fenêtre, la police et les réglages dans un `Contexte` à chaque
  appel et ne les gardent pas. `EcranChoixEscouade` possède la
  `SelectionDEscouade` ; `EcranJeu` reçoit la liste des `IdSpecialite` choisies, la garde, et possède une `Partie`, un
  `Controleur`, une `BarreDeRaccourcis`, des `TextesFlottants`, la caméra et le `std::random_device` (il appelle `composer` pour construire la `Partie`) ; il traduit les
  événements SFML (clic, touches de la barre de raccourcis, Échap, clic droit, F3, F4 et F6 de débogage) en appels au
  `Controleur`. Un clic gauche est d'abord testé sur l'interface (`clicSurInterface`, vue d'interface) : s'il touche la barre, il est consommé et n'atteint pas le monde. Une touche est traduite en emplacement par `emplacementPourTouche` (table des `Reglages`). (Touche R : retour à l'écran de choix de l'escouade si la partie est perdue ou gagnée, sinon c'est un raccourci ;
  F5 : nouvelle carte à tout moment ; F5, comme l'entrée dans l'écran,
  passe par `nouvellePartie`, qui tire la graine et génère le niveau),
  convertit le curseur en coordonnées du monde, centre la caméra sur
  l'escouade et enchaîne le dessin. Après chaque `Partie::mettreAJour`, il prend les `EvenementDeCombat` de la partie (`prendreEvenements`), en fait des textes flottants (sauf hors de vue quand le brouillard est actif) et les fait avancer. Son code n'utilise ni `Monstre`,
  ni `Coup`, ni `EntiteVue`, ni `Chemin`, ni `Personnage` (il les reçoit seulement par les inclusions de `Partie.hpp`).
- `Partie` possède la `TileMap`, l'`Escouade`, un `std::vector<Monstre>` et le
  `Generateur` aléatoire (graine reçue au constructeur),
  construits à partir d'un `Niveau`, ainsi que les `ObjetInteractif` (les portes) et la `Vision` (ce que le joueur sait de la carte). Elle fait l'intermédiaire entre
  `Escouade` et `Monstre`, qui ne se connaissent pas : elle leur donne la
  liste des monstres (`EntiteVue`, deux listes : les cibles, sans ceux qui rentrent à leur poste, et tous les monstres comme obstacles) ; pour chaque monstre, elle lui donne aussi sa proie et ses candidats de frappe (`EntiteVue` de personnages) et applique les `Coup` renvoyés. Elle prend deux photos des camps par image (`EntiteDeControle`) et applique les coups gratuits que `Engagement` en déduit, dans les deux sens. Elle tient la file des `EvenementDeCombat` : chaque dégât et chaque soin réellement appliqué y est inscrit (données seulement : elle ne connaît pas ceux qui la lisent).
- `Controleur` porte l'état d'interface : le mode visée et le déplacement
  continu. Il ne garde aucune référence à la `Partie` : celle-ci lui est
  passée à chaque appel. Il ne connaît pas les événements SFML.
- `Escouade` possède un `Mobile` (l'ancre), N `Personnage` (un par `DefinitionPersonnage` de la composition reçue ; 3 par défaut), des `Provocations` et des `Agresseurs`, et utilise `Encerclement` (géométrie des places d'attaque et de formation). Ces deux derniers ne connaissent ni `Personnage`, ni `Monstre`, ni `Escouade`.
- `Personnage` possède un `Mobile`, un `Skill` (son attaque de base) et une liste de `SkillConnu` (ses skills, chacun avec son `IdSkill`) ; `Monstre` possède un `Mobile` et un
  `Skill` (son attaque).
- `Mobile` utilise `Chemin` et `Deplacement` ; il reçoit en plus, pour chaque ordre, la liste des cercles que son chemin doit contourner (`aContourner`), distincte de ses obstacles bloquants (`docs/chemin.md`). `Personnage` utilise
  `Mobile` et le test de trajet de `TileMap`. `Skill`, `Personnage`, `Monstre` et `Escouade` échangent
  des `Cercle`. `Personnage` et `Monstre` possèdent chacun un `PointsDeVie` et dessinent leur barre de vie avec `BarreDeVie` ; leur `recevoirDegats` passe par `appliquerArmure` (`Defense`). `Escouade` et `Monstre` utilisent `choisirCibleAFrapper` (`Autodefense`) et
`replierVersOrigine` (`Chemin`).

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Architecture de la boucle** (étapes 52, 54 et 65) : `main.cpp` ne fait
  que lancer l'`Application`, qui possède la fenêtre, la police, les
  `Reglages` et une pile d'écrans (`EcranMenu`, `EcranOptions`, `EcranJeu`,
  patron State ; un écran renvoie une `Transition` que l'`Application`
  applique). `EcranJeu` garde la caméra et traduit les événements SFML en appels au
  `Controleur` (mode visée, déplacement continu, aperçu de visée ; type valeur, il reçoit la `Partie` à chaque appel) ; la
  `Partie` (carte, escouade, monstres) porte les règles de jeu et
  l'intermédiaire entre `Escouade` et `Monstre`, avec l'ordre d'une image
  conservé ; `Niveau` porte les données. Voir `docs/boucle-et-entrees.md`.
- **Barre de raccourcis** (étape 75) : en bas au centre de la fenêtre, fixe à l'écran (vue de
  l'interface, dessinée en dernier), une grille de 4 × 3 icônes carrées de 48 px (4 px d'écart,
  10 px de marge), les 12 emplacements de la `BarreDeRaccourcis` (propriété d'`EcranJeu`, ni
  `Partie` ni `Escouade` ne la connaissent). Chaque emplacement pointe vers un skill de n'importe
  quel personnage (une `AdresseSkill`) ; disposition par défaut : une rangée par personnage, dans
  l'ordre des places (rangée du haut : frappe puissante et provocation du guerrier ; milieu : soin
  du prêtre ; bas : explosion du mage), les autres cadres vides. Touches (scancodes, table dans
  `Reglages::touchesDesRaccourcis`, ni lue ni écrite dans `reglages.ini`) : Q W E R / A S D F /
  Z X C V, donc Q = frappe, W = provocation, A = soin, Z = explosion ; les touches 1 à 4 ne font
  plus rien. Une icône montre le sigle du skill (deux lettres : FP, PR, SO, EX…), le nom de sa touche dans un coin
  et une bande de la couleur du personnage lanceur ; un emplacement vide est un cadre sombre avec
  le nom de sa touche. **Clic** : cliquer une icône fait la même chose que sa touche
  (`Controleur::activerEmplacement`), avec la règle « l'interface d'abord » : le clic est testé
  sur la barre avant le monde et consommé (pas de verrou, de porte, de déplacement ni de point
  d'impact de visée) ; un appui sur la barre ne lance pas le déplacement continu et désarme
  celui d'un clic au sol précédent, mais un déplacement commencé au sol continue quand le
  curseur passe sur la barre. Sur l'écran de fin, R ramène au choix de l'escouade et les raccourcis (touche et
  clic) sont ignorés ; en jeu, R est l'emplacement 3. Voir `docs/boucle-et-entrees.md`.
  Pendant la recharge, un quartier noir semi-transparent suit les
  bords du carré (zone claire grandissante depuis midi, sens horaire) et
  le temps restant en secondes entières s'affiche au centre. Icône grisée,
  sans quartier ni chiffre, quand le lanceur est mort ; contour jaune sur
  l'icône du skill en cours de visée, contour blanc pendant son activation. Police DejaVu Sans dans
  `assets/fonts/`, copiée à côté de `main.exe` par CMake ; le jeu refuse
  de démarrer si elle manque.
- **Objectif et victoire** (étape 63) : la partie est gagnée quand il ne
  reste aucun monstre (`Partie::estGagnee`, faux si la partie est perdue : la
  défaite l'emporte si les derniers monstres et personnages meurent dans la
  même image). Le monde se fige comme après la défaite (même sortie
  anticipée de `mettreAJour`), l'écran de fin affiche « Victoire » (module
  `EcranDeFin`, titre en paramètre, partagé avec « Défaite ») et R ramène au
  choix de l'escouade, après une victoire comme après une défaite (F5 inchangée). Un
  compteur « Monstres : N » (`Partie::monstresRestants`), blanc à contour
  noir, est fixe en haut à gauche de l'écran (vue de l'interface).
- **Entrées** : un clic sur le sol envoie l'escouade vers ce point ; tant
  que le bouton reste enfoncé (fenêtre ayant le focus, escouade vivante),
  l'ordre est redonné à chaque image vers le curseur. Un clic bref sur un
  monstre engage un combat qui continue seul, sans tenir le bouton ; les
  raccourcis de la barre (clavier ou clic, voir « Barre de raccourcis ») déclenchent le
  skill de leur emplacement ; un skill de zone entre en mode visée (voir « Skills »).

### Limites connues

- Barre de raccourcis (étape 75) : elle ne montre pas le skill en attente ; les touches ne sont
  ni configurables ni sauvegardées (table dans `Reglages`, pas dans `reglages.ini`) ; une seule
  barre, pas de survol, d'infobulle ni de clic droit sur une icône, pas d'interface pour placer
  les skills ; une 4ᵉ place n'a pas de rangée dans la disposition par défaut, et un 5ᵉ skill d'un
  personnage n'est pas placé. Sa position se recalcule à chaque image depuis la vue d'interface.
  La police se charge par chemin relatif : le jeu doit être lancé depuis le dossier de `main.exe`.

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `Partie` (`jeu/`) — les règles de jeu

Possède par valeur la carte, l'escouade et les monstres. Orchestre
seulement : les règles propres à `Escouade` ou à `Monstre` y restent. Le
joueur agit par des demandes, mémorisées puis exécutées par `mettreAJour` à
leur place dans l'image ; l'ordre est décrit dans `docs/boucle-et-entrees.md`.
Invariant : entre deux appels, tout monstre du vecteur est vivant.
Une partie perdue ou gagnée est figée. Elle se recommence en la remplaçant par une
`Partie` neuve (réaffectation par déplacement : règle du zéro).

Données membres : `m_carte` (`TileMap`), `m_escouade` (`Escouade`),
`m_monstres` (`std::vector<Monstre>`), `m_objets` (`std::vector<ObjetInteractif>`), `m_interactionEnCours` (`std::optional<int>`, identifiant de l'objet que l'escouade va activer), `m_lancerDeZoneDemande`
(`std::optional<LancerDeZone>`, structure privée : `adresse`, `point`),
`m_skillDemande` (`std::optional<AdresseSkill>`), `m_deplacementOrdonne`
(`std::optional<sf::Vector2f>`), `m_generateur` (`Generateur`, le seul de
la partie : il descend en paramètre `Generateur&` jusqu'à `Skill`, que
personne ne copie ni ne garde), `m_vision` (`Vision`, mise à jour à la fin du
constructeur et de chaque `mettreAJour`, voir `docs/vision.md`), `m_evenements` (`std::vector<EvenementDeCombat>`, remplie par `mettreAJour`, vidée par `prendreEvenements`).

Deux listes d'`EntiteVue` sont recalculées à chaque usage (fonctions privées, rien de
stocké) : `entitesCiblables()` (les monstres pour lesquels `estCiblable()` est vrai) pour
tout ce qui choisit un monstre à frapper, provoquer, viser ou verrouiller (et, depuis l'étape 82.4,
le verrou et « un monstre à portée » de `Escouade::mettreAJour`) ; `entitesPhysiques()`
(tous) pour les obstacles bloquants et à contourner. Un monstre qui rentre à son poste
bloque donc comme avant mais n'est plus une cible, et le verrou posé sur lui est levé.

| Fonction | Rôle |
|---|---|
| `Partie(const Niveau& niveau, std::vector<DefinitionPersonnage> composition, Generateur::result_type graine)` | Construit la carte, l'escouade (avec cette composition : elle n'appartient pas au niveau) et les monstres (identifiants 1, 2, 3…, armure `niveau.armureMonstres`), ferme chaque case de `niveau.portes` (`placerPorte`) et crée un `ObjetInteractif` par porte (identifiants 1, 2, 3…, compteur distinct de celui des monstres) et initialise le générateur avec `graine` (`EcranJeu` la tire avec `std::random_device` et l'affiche ; les tests passent une graine fixe) |
| `ResultatClic cliquer(sf::Vector2f point)` | Appliqué tout de suite. `Monstre` : verrouille le monstre ciblable et **visible** (`m_vision.estVisible` de son centre) qui contient le point (un monstre qui rentre à son poste ou hors de vue compte comme le sol) ; `Objet` : un point dans la zone d'une porte fermée dont une case au moins n'est pas `Inconnue` (et pas sur un monstre cliquable) lève le verrou et annule les coups en exécution ; si un personnage vivant est déjà à portée (`unVivantPresDe`), la porte s'ouvre tout de suite (`ouvrir`), l'ordre de déplacement en attente est effacé, aucune interaction n'est mémorisée et la formation regarde la porte (`regarderVers`) ; sinon mémorise l'interaction et l'ordre d'aller au `pointDApproche` (`mettreAJour` appelle `regarderVers` juste après l'`allerVers` de cet ordre) ; sans point d'approche, c'est `Sol`. `Sol` : lève le verrou et annule les coups en exécution des personnages ; `Ignore` si l'escouade est vaincue, sans rien changer. Tout clic (sauf `Ignore`) annule l'interaction en cours. `enum class ResultatClic { Monstre, Objet, Sol, Ignore }` |
| `void demanderSkill(AdresseSkill adresse)` | Mémorise la demande de skill ; la dernière demande de l'image gagne |
| `void lancerSkillDeZone(AdresseSkill adresse, sf::Vector2f point)` | Mémorise le lancer du skill de zone |
| `void ordonnerDeplacement(sf::Vector2f point)` | Mémorise l'ordre de déplacement ; exécuté seulement si l'escouade est vivante ; annule l'interaction en cours |
| `bool estPerdue() const` | Vrai quand tous les personnages sont morts (`!m_escouade.estVivante()`) ; `[[nodiscard]]` |
| `bool estGagnee() const` | Vrai quand `m_monstres` est vide ET que la partie n'est pas perdue (la défaite l'emporte) ; `[[nodiscard]]` |
| `int monstresRestants() const` | Nombre de monstres vivants (`m_monstres.size()`), pour le compteur d'`EcranJeu` ; `[[nodiscard]]` |
| `std::vector<EvenementDeCombat> prendreEvenements()` | Les dégâts et soins réellement appliqués depuis le dernier appel, dans l'ordre où ils ont eu lieu ; la file est vidée (`std::exchange`). Un montant de 0 (cible morte, monstre en retour, soin à pleine vie) ne produit aucun événement ; une partie figée n'en produit plus ; `[[nodiscard]]` |
| `bool estVisible(sf::Vector2f point) const` | La case de ce point (pixels du monde) est-elle `Visible` à la fin de la dernière image ? (`Vision::estVisible`) ; sert à `EcranJeu` à ne pas montrer ce qui se passe dans le brouillard ; `[[nodiscard]]` |
| `void mettreAJour(float dt)` | Sans effet si la partie est perdue ou gagnée (le monde est figé). Sinon exécute, dans l'ordre : lancer de zone, skill demandé, `attaquer(dt, …)` et application des coups, déverrouillage si la cible est morte, retrait des monstres morts, ordre de déplacement, mise à jour (**photo des deux camps** pour les zones de contrôle, escouade avec sa `VueDesMonstres` — physiques, ciblables et monstres de la photo —, puis ouverture de la porte en cours d'interaction si `unVivantPresDe(zone, 20 px)`, tour de chaque monstre avec ses obstacles bloquants — personnages vivants et autres monstres — et ses cercles à contourner — les autres monstres seulement —, avec l'engagement et le départ de son coup, puis **seconde photo** et coups gratuits des deux sens : `coupsDOpportunite` ; dégâts de base de l'attaquant, `Escouade::recevoirCoup` côté personnage, `Monstre::recevoirDegats` côté monstre ; monstres tués retirés, verrou levé) ; vide les demandes. Chaque dégât appliqué (`appliquer`, `infligerAUnPersonnage`) et chaque soin pris à `Escouade::prendreSoins` passe par `signaler`, seul endroit où naît un événement ; les coups gratuits sont marqués `opportunite` |
| `void dessiner(sf::RenderTarget& rendu, bool afficherAncre, bool marquerLeRetour, bool brouillard) const` | Carte, monstres (contour jaune sur le monstre verrouillé ; `marquerLeRetour` est transmis à `Monstre::dessiner` ; un monstre non visible n'est pas dessiné si `brouillard`), escouade (`afficherAncre` est transmis à `Escouade::dessiner`), puis le voile de la `Vision` si `brouillard` |
| `void dessinerCheminsDesMonstres(sf::RenderTarget& rendu) const` | Débogage (F3) : le chemin restant de chaque monstre (`Monstre::dessinerChemin`) |
| `void dessinerZonesDeControle(sf::RenderTarget& rendu) const` | Débogage (F3) : un cercle fin magenta (rayon de l'entité + zone) autour de chaque personnage vivant et de chaque monstre qui ne rentre pas à son poste, qui a une zone > 0 |
| `const Escouade& getEscouade() const` | Lecture seule : caméra, barre de skills, requêtes du `Controleur` |
| `const std::vector<Monstre>& getMonstres() const` | Lecture seule (utilisée par les tests) |

### `Controleur` (`interface/`) — l'état d'interface

Ne connaît pas les événements SFML : `EcranJeu` les traduit en appels
simples. Points en pixels du monde.

Type valeur, constructible par défaut et réaffectable (règle du zéro) : aucune
référence membre (C++ Core Guidelines C.12), la `Partie` est reçue à chaque
appel (`const Partie&` si la fonction ne fait que lire).

Données membres : `m_deplacementContinu` (`bool`), `m_adresseEnVisee`
(`std::optional<AdresseSkill>`, vide = mode normal).

| Fonction | Rôle |
|---|---|
| `Controleur()` (par défaut) | Mode normal, déplacement continu désarmé ; `controleur = Controleur{}` remet l'état d'interface à zéro |
| `void debutImage(const Partie& partie)` | Sort de la visée si le lanceur ne peut plus lancer (`!skillEstPret`) |
| `void appuiGauche(Partie& partie, sf::Vector2f point)` | En visée : `lancerSkillDeZone` puis sortie de la visée (sans verrouiller ni déplacer). Sinon `cliquer` : `Monstre` et `Objet` désarment le déplacement continu, `Sol` l'arme, `Ignore` ne change rien |
| `void appuiDroit()` / `void appuiEchap()` | Annulent la visée sans recharge |
| `void activerEmplacement(Partie& partie, const BarreDeRaccourcis& barre, std::size_t emplacement)` | Une seule action pour la touche et le clic. Emplacement vide : rien. Skill de type `Zone` (`typeDuSkill`) : sort de la visée si cette même adresse y est ; sinon entre en visée de cette adresse (remplaçant la visée d'un autre skill de zone) si l'escouade est vivante et le skill prêt, et désarme le déplacement continu. Autre type : `demanderSkill(adresse)`, la visée en cours continue. `assert(emplacement < barre.taille())` |
| `void appuiInterface(Partie& partie, const BarreDeRaccourcis& barre, ClicInterface clic)` | Clic gauche consommé par l'interface : désarme le déplacement continu (un appui qui commence sur la barre ne le lance pas, et ne laisse pas un ancien clic au sol le relancer), puis `activerEmplacement` pour la barre `Raccourcis`. Jamais de verrou, de porte, de déplacement ni de point d'impact de visée |
| `void mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur)` | Si `boutonTenu` (bouton gauche enfoncé ET focus, calculé par `EcranJeu`) et déplacement armé : `ordonnerDeplacement(curseur)` |
| `void dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const` | En visée seulement : cercle jaune de la zone sous le curseur, cercle blanc de la portée autour du lanceur |
| `std::optional<AdresseSkill> getAdresseEnVisee() const` | Adresse en visée (`EcranJeu` en déduit l'icône à entourer de jaune) |

Le `Controleur` ne garde aucune référence à la barre : `EcranJeu` la lui passe à chaque appel, comme la `Partie`.

### `BarreDeRaccourcis` (`interface/`) — les emplacements de raccourcis

`BarreDeRaccourcis.hpp` / `.cpp`, sans SFML graphique. Trois notions séparées : les **emplacements** (le modèle, cette classe), les **touches** (une table touche → emplacement dans `Reglages`) et la **disposition** à l'écran (calculée par la vue seulement). L'emplacement i reste l'emplacement i quelles que soient sa touche et sa place. Voir `docs/boucle-et-entrees.md`. Constante `inline constexpr std::size_t s_emplacementsPrincipaux = 12`, dans l'en-tête neutre `interface/EmplacementsPrincipaux.hpp` depuis l'étape 81 (P1 : `Reglages.hpp` l'inclut sans dépendre du modèle de la barre).

Classe `BarreDeRaccourcis` : donnée membre `m_emplacements` (`std::vector<std::optional<AdresseSkill>>`). Possédée par `EcranJeu` (la barre d'auras à venir sera une autre instance).

| Fonction | Rôle |
|---|---|
| `explicit BarreDeRaccourcis(std::size_t taille)` | Tous les emplacements sont vides |
| `std::size_t taille() const` | Nombre d'emplacements ; `[[nodiscard]]` |
| `std::optional<AdresseSkill> adresse(std::size_t emplacement) const` | Vide si l'emplacement est vide ; `assert(emplacement < taille())` ; `[[nodiscard]]` |
| `void placer(std::size_t emplacement, AdresseSkill adresse)` | Aucune limite par personnage ; une même adresse peut être placée deux fois ; `assert` |
| `void vider(std::size_t emplacement)` | `assert` |

Trois fonctions libres :

| Fonction | Rôle |
|---|---|
| `BarreDeRaccourcis barreParDefaut(const std::vector<AdresseSkill>& adresses)` | Barre de `s_emplacementsPrincipaux` emplacements : une rangée de 4 par personnage, dans l'ordre des places (place 1 : emplacements 0 à 3…), ses skills dans l'ordre reçu. Le 5ᵉ skill d'un personnage, ou une place sans rangée, n'est pas placé (sans erreur). Ne connaît pas `Escouade` ; `[[nodiscard]]` |
| `sf::FloatRect rectangleDeLEmplacement(std::size_t emplacement, sf::Vector2f tailleVue)` | Grille de 4 colonnes × 3 rangées, icônes de 48 px, 4 px d'écart, centrée en bas de la vue d'interface avec 10 px de marge ; l'emplacement 0 en haut à gauche ; `assert` ; `[[nodiscard]]` |
| `std::optional<std::size_t> emplacementSous(sf::Vector2f point, sf::Vector2f tailleVue)` | Emplacement dont l'icône contient le point ; vide dans un écart ou hors barre. Construite sur `rectangleDeLEmplacement` : une seule source de vérité pour le dessin et le clic ; `[[nodiscard]]` |

### `ClicInterface` (`interface/`) — l'interface d'abord

`ClicInterface.hpp` / `.cpp`. `enum class Barre { Raccourcis }` (la barre d'auras s'y ajoutera), `struct ClicInterface { Barre barre; std::size_t emplacement; }` (comparable) et une fonction libre.

| Fonction | Rôle |
|---|---|
| `std::optional<ClicInterface> clicSurInterface(sf::Vector2f point, sf::Vector2f tailleVue)` | Ce que le point (coordonnées de la vue d'interface) touche dans l'interface ; vide s'il tombe sur le monde. Un clic qui touche l'interface est consommé par `EcranJeu` ; `[[nodiscard]]` |

### `BarreDeSkills` (`interface/`) — le dessin de la barre de raccourcis

Fonctions libres, sans état : tout est recalculé à chaque image. Ne connaît ni `Escouade` ni `Personnage` ni la barre : elle reçoit des `IconeDeRaccourci`. `struct IconeDeRaccourci { std::optional<EtatSkill> etat; sf::String touche; sf::Color couleurDuLanceur; std::size_t place; bool enVisee; }` (`place` : place du lanceur, 1 à N ; `etat` vide = emplacement vide ; construite par `EcranJeu`).

| Fonction | Rôle |
|---|---|
| `std::string sigleDuSkill(IdSkill id)` | Étiquette de deux lettres : FP, PR, SO, EX, FS, IM, TP, PF ; `switch` sans `default` (un `IdSkill` oublié est un avertissement du compilateur) ; `[[nodiscard]]` |
| `sf::Color couleurDuContour(bool enVisee, bool enActivation)` | Jaune en visée (prioritaire), blanc en activation, gris sinon |
| `int secondesAffichees(float tempsRestant)` | Secondes entières, arrondies vers le haut |
| `std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre)` | Sommets d'un `TriangleFan` : centre, aiguille, coins du carré dans l'intervalle, midi ; vide si la fraction est nulle |
| `void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police, const std::array<IconeDeRaccourci, s_emplacementsPrincipaux>& icones)` | Dessine les 12 icônes à la place de `rectangleDeLEmplacement` (à appeler avec la vue d'interface) : fond, sigle, nom de la touche dans un coin, bande de la couleur du lanceur (14 px) avec la place du lanceur, quartier sombre et chiffre en recharge, grisée si le lanceur est mort, contour jaune si `enVisee`, blanc si `enActivation`. Un emplacement vide est un cadre sombre avec seulement le nom de sa touche |

### `EcranDeFin` (`interface/`) — l'écran de fin de partie

Une fonction libre, sans état, sur le modèle de `BarreDeSkills`. Ne connaît ni `Partie` ni `Escouade`. Sert à la défaite et à la victoire.

| Fonction | Rôle |
|---|---|
| `void dessinerEcranDeFin(sf::RenderTarget& rendu, const sf::Font& police, const std::string& titre)` | Voile noir semi-transparent sur toute la vue par défaut, puis `titre` (« Victoire » ou « Défaite ») et « Appuyez sur R pour revenir au choix de l'escouade » au centre ; à appeler avec la vue par défaut, après la barre de skills |
