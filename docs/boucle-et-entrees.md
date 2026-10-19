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
     g. `EntiteVue` de **tous** les monstres, liste physique (obstacles bloquants
        des personnages, dont chacun contourne ceux qu'il n'approche pas),
        `escouade.mettreAJour`, puis
        `cerclesDesVivants` et le tour de chaque monstre (proie, mise à jour
        avec ses obstacles bloquants et ses cercles à contourner, riposte) ;
     h. `Vision` mise à jour (étape 72), en dernier pour que le dessin corresponde aux
        positions de la fin de l'image ;
     i. les demandes sont vidées ;
  4. `main.cpp` : caméra centrée sur l'escouade, `partie.dessiner` (carte,
     monstres visibles avec le contour du verrou, semi-transparents s'ils rentrent à leur
     poste et que F3 est actif, escouade, voile de la `Vision` sauf si F6 l'a coupé), `controleur.
     dessinerApercu`, puis, dans la vue d'interface, barre de raccourcis, compteur
     « Monstres : N » (haut gauche) et, si la partie est perdue ou gagnée,
     écran de fin (par-dessus la barre et le compteur), le tout avec la vue de
     l'interface (800 px de haut, voir `docs/ecrans.md`).
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
  pas. L'horloge reste dans `main.cpp` ; `dt` est passé en paramètre.
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
  sont oubliés par `attaquer`, un coup engagé sur lui est annulé). Les
  **physiques** vont aux obstacles : un monstre qui rentre bloque et est contourné
  comme avant. Les listes sont recalculées à chaque usage, jamais stockées.
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
- **Alerte du groupe** (étape 64) : avant la boucle, le statut « en poursuite »
  de chaque monstre est noté ; après, `alerterLesGroupes` alerte les membres
  des groupes dont un monstre vient d'entrer en poursuite (voir
  `docs/monstre.md`).
- **Indépendant du verrou** : un monstre poursuit et riposte dès qu'un
  personnage vivant est à sa portée.

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
