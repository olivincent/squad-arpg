# Squad-ARPG — Écrans, vues et réglages

Depuis l'étape 65, `main()` ne fait que construire une `Application`. Le menu,
les options, le choix de l'escouade (étape 76) et la partie sont quatre écrans (`Ecran`) que
l'`Application` empile. Ce fichier explique les décisions ; le détail des interfaces est dans
la section « Référence des modules » à la fin de ce fichier, et celui d'une image de jeu dans `docs/boucle-et-entrees.md`.

## Pile d'écrans (patron State)

- **Une classe de base abstraite** `Ecran` (`traiterEvenement`, `mettreAJour`,
  `dessiner`, et `laisseVoirDessous`, non pure), cinq classes concrètes (`EcranMenu`,
  `EcranOptions`, `EcranChoixEscouade`, `EcranJeu`, `EcranInventaire`), manipulées seulement par
  `std::unique_ptr<Ecran>`. Copie interdite, destructeur virtuel.
- **Une pile** (`PileDEcrans`, un `std::vector<std::unique_ptr<Ecran>>`) plutôt qu'un écran
  courant : l'inventaire s'empile sur la partie sans réécriture, un futur menu de pause aussi.
  La pile a jusqu'à quatre écrans (menu, choix de l'escouade, jeu, inventaire ; ou menu et
  options). Seul l'écran du dessus reçoit les événements et est mis à jour : ceux du dessous sont
  conservés, donc **figés**, sans aucun code de pause.
- **Le dessin va plus bas que le dessus** (étape 84). `Ecran::laisseVoirDessous()` vaut `false`
  par défaut : l'écran remplit la fenêtre et cache ce qui est dessous. Un écran qui n'est qu'une
  couche (l'inventaire) la redéfinit à `true`. `premierEcranADessiner(pile)` (fonction pure,
  `interface/PileDEcrans`) renvoie l'indice du plus haut écran qui ne laisse pas voir dessous ;
  l'`Application` dessine de cet écran jusqu'à celui du dessus, de bas en haut. Le plus bas
  écran est toujours dessiné, même s'il laisse voir dessous.
- **Le `clear` de la fenêtre est dans l'`Application`**, une fois par image, avant de dessiner la
  pile. Aucun écran n'efface plus la fenêtre : c'est la condition pour en dessiner un par-dessus
  un autre.
- **Invariant de la pile : un écran vit toujours plus longtemps que ceux qui sont au-dessus de
  lui.** C'est ce qui permet à un écran du dessus de garder une référence vers une donnée d'un
  écran du dessous : `EcranInventaire` garde une `Escouade&` de l'`EcranJeu`. Ce n'est pas
  la règle du `Contexte`, jamais gardé parce que la fenêtre est recréée entre deux appels. Toute
  référence de ce genre se justifie par cet invariant ; un écran ne se retire de la pile que par
  une `Transition`, donc après tous ceux qui sont au-dessus de lui.
- **Un écran ne modifie jamais la pile lui-même** : il pourrait se détruire
  pendant l'exécution de sa propre fonction (`Depiler` dans
  `traiterEvenement`). Il renvoie une `Transition` (`std::variant<Aucune,
  Empiler, Depiler, Quitter, AppliquerReglages>`), que l'`Application` applique
  une fois l'appel terminé, avec `std::visit` et `Surcharges` (comme `Cible`).
  `mettreAJour` ne renvoie rien : aucun écran n'a encore besoin de changer
  d'écran en dehors d'un événement.
- **Pile vide = fin du programme** : `executer` s'arrête quand la fenêtre est
  fermée ou la pile vide. La croix de la fenêtre quitte quel que soit l'écran
  (l'`Application` la traite avant les écrans).
- **`Contexte`** (fenêtre, police, réglages en lecture seule) est construit par
  l'`Application` à chaque appel et jamais gardé par un écran : la fenêtre est
  recréée quand les réglages changent, une référence gardée serait périmée
  (même règle que la `Partie` dans le `Controleur`, C.12).
- **`Application` reste dans l'exécutable `main`**, les écrans et `Reglages`
  sont dans `squad_core` pour être testés sans fenêtre.
- **Logique testable sans fenêtre** : chaque écran à boutons a une fonction
  publique (`EcranMenu::cliquer`, `EcranOptions::cliquer`) qui reçoit un point
  en coordonnées d'interface et renvoie la `Transition`. `traiterEvenement` ne
  fait que convertir l'événement puis l'appeler. Le dessin n'est pas testé.
- **Entrées d'interface communes (D4, étape 84)** : deux fonctions libres de
  `interface/EntreeDInterface` remplacent le code répété dans les trois écrans de menu (pas de
  classe de base `EcranDeMenu`, les `mettreAJour` vides restent). `clicGaucheDInterface(evenement,
  contexte)` renvoie le point d'un clic gauche en coordonnées d'interface, ou rien ;
  `preparerLInterface(contexte)` pose la vue d'interface et renvoie le curseur converti. Toutes
  deux rendent un `PointDInterface { point, centre }` : les menus centrent leurs boutons sur le
  centre de la vue, donc il voyage avec le point. L'écran d'inventaire s'en sert aussi.
- **`EcranMenu` reçoit les résolutions disponibles** (P2, étape 84) : l'`Application` les calcule
  (`resolutionsDisponibles(getDesktopMode().size)`) et les passe au constructeur, qui les garde et
  les donne à l'`EcranOptions`. La branche « Options » de `cliquer` se teste sans bureau.
- **Flux** : menu → « Commencer partie » = `Empiler(EcranChoixEscouade)` ; choix →
  « Commencer » = `Empiler(EcranJeu)` (nouvelle `Partie`, nouvelle graine affichée) ;
  « Retour » = `Depiler` (menu) ; menu → « Options » = `Empiler(EcranOptions)` ;
  « Quitter » = `Quitter`. En jeu, R sur l'écran de fin = `Depiler` (retour à l'écran de
  choix, avec sa sélection), ignorée en cours de partie ; F5 génère une nouvelle carte à tout
  moment. Échap en jeu annule seulement la visée. I en jeu (hors écran de fin) =
  `Empiler(EcranInventaire)` ; I ou Échap dans l'inventaire = `Depiler`.
- **Principe de conception** : c'est l'application du principe 1 révisé
  (« anticiper la structure, pas les fonctionnalités ») : la pile et la
  `Transition` accueillent la pause et l'inventaire sans réécriture, mais les alternatives
  `Remplacer` et une `mettreAJour` qui renvoie une `Transition` n'existent pas, car rien ne les
  utilise. Le dessin des écrans du dessous est arrivé avec l'inventaire, premier écran à en avoir
  besoin.

## `EcranInventaire` : l'inventaire (étapes 84 et 85)

- **Ouvert par I**, empilé par `EcranJeu` sur la commande `OuvrirInventaire` de
  `commandePourTouche`, sauf sur l'écran de fin (comme les touches de la barre). La touche I est
  écrite en dur, comme les touches F ; la configuration des touches viendra plus tard.
- **La partie est figée et visible dessous** (`laisseVoirDessous()` vrai), sous un voile sombre
  semi-transparent puis un panneau. Pas de code de pause : seul l'écran du dessus est mis à jour.
- **Avant d'empiler**, `EcranJeu` appelle `Controleur::interrompre()` (annule la visée et le
  déplacement continu). Sans cela, le relâchement du bouton gauche, reçu par l'inventaire,
  n'arriverait jamais au `Controleur` : l'escouade repartirait toute seule à la fermeture.
- **`Escouade&` non constante** (étape 85) : l'écran garde l'`Escouade` de l'`EcranJeu` (valide grâce
  à l'invariant de la pile ; `Partie::getEscouade()` a une surcharge non constante pour cela) et les
  noms des classes des places, que `EcranJeu` lui donne (il connaît ses `IdSpecialite`). Il n'appelle
  qu'une fonction de modification : `Escouade::deplacerObjet`. La croix de la fenêtre quitte, comme partout.
- **Glisser-déposer** (étape 85) : l'état est un `std::optional<PositionDObjet>`, l'origine. Rien ne
  bouge dans le modèle pendant le glissement ; tout se joue au relâchement.
  - Appui du bouton gauche sur une position occupée : début du glissement ; position vide ou ailleurs : rien.
  - Relâchement : si `elementSous` donne une position, `deplacerObjet(origine, cible)` (un résultat
    faux est normal : rien ne bouge, `static_cast<void>` commenté) ; ailleurs, rien. Le glissement
    se termine dans tous les cas. Le bouton droit ne fait rien.
  - Échap pendant un glissement l'annule seulement (un deuxième Échap ferme) ; I ferme l'écran (rien
    n'a bougé) ; `sf::Event::FocusLost` annule, parce que le relâchement hors de la fenêtre n'arrive pas
    (sans cela l'objet resterait collé au curseur).
  - Affichage : l'origine est recouverte d'un voile (`s_couleurAttenuation`), l'objet saisi (carré de la
    taille d'une case avec son sigle) suit le curseur et est dessiné en dernier, l'infobulle est masquée.
    Les destinations valides (`peutDeplacerObjet(origine, position)`, sauf l'origine) ont un contour doré :
    les emplacements des 3 personnages et les cases du sac. C'est la requête du relâchement, donc ce
    qui brille est permis.
  - Les fonctions `saisir`, `relacher`, `annulerLeGlissement` prennent des points en coordonnées
    d'interface : les tests n'ont besoin ni de fenêtre ni d'événement SFML. `traiterEvenement` ne fait que
    les traduire (`clicGaucheDInterface`, `relachementGaucheDInterface`, `FocusLost`).
- **Disposition en fonctions pures** (`interface/DispositionInventaire`), sans position stockée :
  tout est recalculé depuis la taille de la vue d'interface. En haut, une colonne par personnage
  (bande de la classe à la couleur du personnage, « Armure : N », « (mort) », 10 emplacements en
  deux colonnes de 5, puis, depuis l'étape 88, le bloc de ses cinq caractéristiques effectives — avec « (+N) »
  quand l'équipement en donne — et de ses points de vie « PV actuels / maximum », puis, depuis l'étape 89, le
  bloc de ses statistiques de combat : « Capacité offensive N » et « Capacité défensive N », lues par
  `Escouade::statistiquesDeCombatDuPersonnage`) ; en bas, le sac (4 rangées de 10 cases, à 550 px du haut). Le dessin, le survol et les tests
  appellent les mêmes rectangles (`rectangleDeLEmplacement`, `rectangleDeLaCaseDuSac`,
  `elementSous`), aussi utilisés par le glisser-déposer. Un test vérifie que tout
  tient dans 1200 × 800 (et en 16:9), dans le panneau, sans chevauchement.
- **Infobulle** : au survol d'un emplacement ou d'une case occupés, nom, catégorie et armure de
  l'objet (`lignesDeLInfobulle`), placés près du curseur et gardés entièrement dans la vue
  (`placerInfobulle`). Rien sur un emplacement ou une case vide.
- **Textes** : les noms des emplacements et des catégories et les sigles des objets sont dans
  l'interface (`interface/AffichageObjets`, `switch` sans `default` : le compilateur signale un
  oubli), en UTF-8 passé à `sf::String::fromUtf8`. Un texte plus large que son rectangle est
  rétréci.

## `EcranChoixEscouade` : le choix des 3 classes (étape 76)

- **Pourquoi un écran entre le menu et le jeu** : le joueur choisit sa composition avant chaque
  partie. L'écran de choix **reste sous** le jeu dans la pile : c'est la pile qui conserve la
  sélection, sans sauvegarde ni état global. R après une défaite ou une victoire dépile le jeu et
  retombe sur le choix, sélection intacte, pour rejouer la même composition ou en changer.
- **Le modèle est à part** : `SelectionDEscouade` (3 classes au plus, doublons permis, `retirer`
  fait remonter les suivantes) ne sait rien des écrans ; l'écran n'est que sa vue et sa traduction
  des clics. `EcranJeu` reçoit la liste d'`IdSpecialite` et la garde pour F5 ; c'est lui qui appelle
  `composer`, `Partie` ne reçoit toujours que des `DefinitionPersonnage`.
- **Trois colonnes, aucune position stockée** (même principe que `ListeDeBoutons`) : classes à
  gauche (`ListeDeBoutons`, un bouton par entrée du catalogue, dans son ordre : le bouton i est
  `catalogue()[i]`), cases de sélection à droite puis « Commencer » / « Retour »
  (`ListeDeBoutons`). `centreDesClasses`, `centreDesCases` et `centreDesActions` sont les seules
  sources des positions : dessin, clic et tests les appellent. La géométrie des cases est une
  fonction pure (`rectangleDeLaCase`, `caseSous`), comme la barre de raccourcis. À 1200 × 800 (vue de
  1200 px de large) les colonnes tiennent avec 40 px de marge.
- **Pas de `ListeDeBoutons` pour les cases** : une case vide n'est pas un bouton grisé mais une
  position de la sélection ; un clic sur une case remplie la retire, ce qui n'est pas une action de
  la liste. Les dimensions (520 × 60, 20 px) sont recopiées dans `SelectionDEscouade.cpp` pour que les
  colonnes s'alignent.
- **« Commencer » inactif** tant que la sélection n'est pas complète : `ListeDeBoutons::definirActif`,
  remis à jour après chaque ajout ou retrait ; un bouton inactif n'est jamais renvoyé par
  `boutonSous`, donc un clic dessus ne fait rien.
- **La bande de couleur d'une case** utilise `composer(sélection)` : le joueur voit déjà la nuance
  que le personnage aura en jeu.
- **Limites** : souris seulement ; pas de description de classe ni d'infobulle ; la sélection n'est
  pas écrite dans `reglages.ini` (elle repart vide à chaque lancement du jeu) ; au plus trois fois la
  même classe, parce qu'il n'y a que trois nuances et trois places.

## `EcranJeu` : le jeu de l'ancien `main.cpp`

- Code déplacé sans changement de comportement (étape 65, sous-étape 4) : un
  seul écart. `Controleur::debutImage` (sortie de la visée si le lanceur ne
  peut plus lancer) s'exécutait au début de l'image, avant les événements ; il
  n'y a plus de point d'entrée à cet endroit, il s'exécute donc à la fin de
  `mettreAJour`. Les événements de l'image suivante voient le même état ; seul
  l'affichage de la visée disparaît une image plus tôt.
- `EcranJeu` reçoit les `IdSpecialite` de la composition (étape 76) à sa construction et les garde
  (`m_specialites`, déclaré avant `m_partie` parce que la construction de la partie les lit) : F5
  régénère la carte avec la même composition.
- `EcranJeu` possède le `std::random_device` : la graine d'une nouvelle carte
  (F5) ou d'une nouvelle partie (nouvel `EcranJeu`) est tirée et affichée
  comme avant.

## Vues à hauteur fixe

- **Le monde visible fait toujours 800 px de haut** (`s_hauteurDeVue`) ; la
  largeur vaut `800 × largeur / hauteur de la fenêtre` (`tailleDeVue`, fonction
  pure). Un écran plus large montre plus de monde sur les côtés, jamais plus de
  haut : pas d'avantage de visibilité vertical, et l'interface garde les
  mêmes proportions. En 1200 × 800 tout est identique à avant l'étape 65.
- **Deux vues** : la caméra du jeu (`EcranJeu::m_camera`, centrée sur
  l'escouade, taille recalculée à chaque image) et la vue de l'interface
  (`vueDInterface` : 800 px de haut, coin haut gauche en (0, 0)), qui
  remplace la vue par défaut de SFML pour la barre de raccourcis, le compteur,
  l'écran de fin, le menu et les options. `dessinerBarreDeSkills` et
  `dessinerEcranDeFin` lisent la taille de la vue active (`getView`), pas
  celle de la vue par défaut ; la disposition de la barre ne dépend que de cette
  taille (`rectangleDeLEmplacement`), donc elle reste centrée en bas quel que soit
  le format de la fenêtre.
- **Clics** : convertis avec `mapPixelToCoords(pixel, vue)` : avec la caméra
  dans `EcranJeu`, avec la vue de l'interface dans les menus. Dans `EcranJeu`, le clic gauche
  est converti **deux fois** : d'abord avec la vue d'interface pour tester la barre de
  raccourcis (« l'interface d'abord », voir `docs/boucle-et-entrees.md`), puis, s'il n'a pas
  touché l'interface, avec la caméra pour le monde.

## Boutons

- `ListeDeBoutons` : une colonne de boutons (520 × 60 px, 20 px d'écart),
  **sans position stockée**. La disposition est recalculée à chaque appel à
  partir du `centre` reçu, car le menu reste sous l'écran des options pendant
  que celle-ci recrée la fenêtre : une position gardée serait périmée. Le
  `centre` est donc un paramètre de `cliquer` aussi.
- Un bouton **inactif** est grisé et `boutonSous` ne le renvoie jamais : ni
  survol ni clic (ligne de résolution en plein écran ou sans résolution
  disponible).
- Libellés écrits en UTF-8 dans le source, convertis par `sf::String::fromUtf8`
  (un `std::string` donné tel quel à `sf::Text` serait lu comme de l'ANSI :
  c'était le cas de « Défaite » dans `EcranDeFin`, dont les accents ne
  s'affichaient pas, corrigé après l'étape 65). Tout texte accentué passé à
  `sf::Text` doit passer par `fromUtf8`.
- Souris seulement : pas de navigation au clavier.

## Réglages et `reglages.ini`

- **`Reglages`** : `largeur`, `hauteur` (1200 × 800 par défaut) et un
  `ModeAffichage` (`Fenetre` ou `PleinEcran`). Le plein écran utilise toujours
  la résolution du bureau (`sf::VideoMode::getDesktopMode()`) ; la
  résolution des réglages ne sert qu'en fenêtré et est conservée pendant le
  plein écran.
- **Touches des raccourcis** (étape 75) : `Reglages::touchesDesRaccourcis`
  (`std::array<sf::Keyboard::Scancode, 12>`, index = emplacement de la barre), par défaut Q W E R /
  A S D F / Z X C V. Elle vit dans les réglages pour que l'écran de configuration des touches
  (à venir) la modifie sans toucher au reste, mais elle n'est **ni lue ni écrite** dans
  `reglages.ini` à cette étape : `lireReglages` la laisse à sa valeur par défaut et
  `ecrireReglages` ne l'écrit pas. `emplacementPourTouche` est la fonction pure qui traduit un
  scancode en emplacement.
- **Liste fixe de résolutions** (`resolutionsProposees` : 1200 × 800,
  1280 × 720, 1600 × 900, 1920 × 1080), filtrée par la taille du bureau
  (`resolutionsDisponibles`). Pas de modes tirés de `getFullscreenModes()`.
- **Format texte `cle=valeur`**, une clé par ligne ; lecture et écriture
  séparées du disque (`std::istream` / `std::ostream`), testées avec des flux
  en mémoire. **Jamais d'exception** : clé inconnue ou ligne mal formée
  ignorée, valeur invalide = défaut, résolution hors liste = 1200 × 800 (un
  fichier modifié à la main ne peut pas planter le jeu).
- **Fichier** `reglages.ini` à côté de `main.exe` (chemin relatif, comme la
  police : le jeu doit être lancé depuis son dossier), dans `.gitignore`.
  Absent ou illisible = réglages par défaut. Lu au lancement avant d'ouvrir
  la fenêtre ; écrit à chaque `AppliquerReglages`, après la recréation de la
  fenêtre. Un échec d'écriture affiche un message sur `cerr` et le jeu continue.
- **`ouvrirFenetre`** est la seule fonction qui crée la fenêtre (lancement et
  chaque changement de réglage). `create` fait perdre `setFramerateLimit(60)` et
  `setKeyRepeatEnabled(false)` : elle les réapplique toujours. Style fenêtré
  `Titlebar | Close` : plus de redimensionnement à la main.
- **Options** : chaque changement s'applique tout de suite (pas de bouton
  « Appliquer ») : `EcranOptions` modifie sa copie des réglages et renvoie
  `AppliquerReglages`. Ligne de résolution : tiers gauche = précédente, tiers
  droit = suivante, sans bouclage aux deux bouts.

## Limites connues

- Menu et options : pas de navigation au clavier ni à la manette ; pas de
  titre dans le menu.
- Résolution enregistrée valide mais plus grande que le bureau (changement
  d'écran) : la fenêtre s'ouvre trop grande ; les options permettent de
  corriger. Si aucune résolution ne tient dans le bureau, la ligne est inactive.
- `reglages.ini` est cherché dans le dossier courant, comme la police.
- Pas de menu de pause ni de retour au choix de l'escouade en cours de partie (R n'agit
  qu'après une défaite ou une victoire).
- Inventaire (étapes 84 et 85) : pas de clic droit pour équiper, pas de prise au clic puis dépôt au
  clic, pas de glisser vers le monde (`docs/brainstorm.md`) ; la touche I n'est pas configurable ; pas d'icônes d'objets (un sigle de deux lettres) ni de couleurs par rareté ;
  `s_casesDuSac` (disposition) doit égaler la capacité du sac, une constante de `Escouade.cpp`
  (un `assert` du dessin le vérifie) ; `EcranInventaire` suppose `s_nombreDePlaces` = 3 classes.
- Les boutons de `ListeDeBoutons` recopient le centrage de texte de
  `EcranDeFin.cpp` (à extraire dans un module commun, dans un commit à part).
- Le menu reste sous les options et n'est pas redessiné pendant le changement
  de fenêtre ; sa disposition est recalculée au prochain dessin.

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Menu et réglages** (étape 65) : au lancement, un menu à la souris
  (« Commencer partie », « Options », « Quitter ») ; « Commencer partie »
  ouvre le choix de l'escouade (voir « Choix de l'escouade »), puis lance une partie (nouvelle carte,
  graine dans la console). « Options » :
  mode fenêtré ou plein écran (à la résolution du bureau) et, en fenêtré,
  résolution parmi 1200 × 800, 1280 × 720, 1600 × 900 et 1920 × 1080 (celles
  qui tiennent dans le bureau) ; chaque changement s'applique tout de suite
  (la fenêtre est recréée) et est écrit dans `reglages.ini` à côté de
  `main.exe` (ignoré par Git ; absent ou invalide = 1200 × 800 fenêtré). Le
  monde visible fait toujours 800 px de haut (la largeur suit le format de la
  fenêtre) ; la barre de skills, le compteur, l'écran de fin et les menus
  suivent la même règle. Fenêtre sans redimensionnement à la main. Voir
  `docs/ecrans.md`.
- **Choix de l'escouade** (étape 76) : « Commencer partie » ouvre un écran à la souris. À gauche, un
  bouton par classe du catalogue (guerrier, prêtre, mage, paladin, rôdeur) ; à droite, trois cases
  « 1. Guerrier » / « 1. — » et les boutons « Commencer » (grisé tant que la sélection n'est pas
  complète) et « Retour » (menu). Cliquer une classe remplit la case suivante (la même classe peut
  être prise 1 à 3 fois ; une 4ᵉ ne fait rien), cliquer une case remplie la vide et les suivantes
  remontent. L'ordre de sélection donne les places 1 à 3, donc les rangées de la barre de raccourcis
  et la formation (selon le rang de chaque classe). La 2ᵉ et la 3ᵉ occurrence d'une même classe ont
  la couleur de la classe assombrie de 30 % et de 55 % ; la case montre déjà cette couleur. La
  sélection reste dans l'écran de choix : R après une défaite ou une victoire y revient avec la même
  sélection ; F5 en jeu garde la composition. **Classes** (`Specialite`, données, pas de
  hiérarchie) : *paladin* (devant, orange, armure 50, attaque de mêlée ; `FrappeSacree` : 20 dégâts,
  20 px, recharge 5 s, activation 0,3 s ; `ImpositionDesMains` : soin de 25, 20 px, recharge 10 s,
  activation 0,5 s) et *rôdeur* (derrière, vert, armure 0, attaque de tireur ; `TirPercant` : 25
  dégâts, 200 px, recharge 6 s, activation 0,5 s ; `PluieDeFleches` : zone de 12 dégâts, rayon 45 px,
  portée 320 px, recharge 8 s, activation 0,8 s) ; valeurs provisoires, à ajuster après test. Les
  icônes de la barre montrent un sigle de deux lettres par skill (FP, PR, SO, EX, FS, IM, TP, PF) et,
  dans leur bande de couleur, la place du lanceur (1 à 3). Voir `docs/ecrans.md`, `docs/formation.md`
  et `docs/combat.md`.

### Limites connues

- Menus (étape 65) : souris seulement ; « Options » et « Commencer partie »
  n'existent qu'au menu de départ (pas de pause ni de retour au menu en
  cours de partie) ; une résolution enregistrée plus grande que le bureau
  ouvre une fenêtre trop grande (les options la corrigent) ; `reglages.ini`
  est cherché dans le dossier courant, comme la police. Voir `docs/ecrans.md`.
- Choix de l'escouade (étape 76) : souris seulement ; pas de description des classes ni d'infobulle ;
  la sélection n'est pas gardée entre deux lancements du jeu ; pas d'arbre de skills ni de niveaux
  (deux personnages de la même classe ne diffèrent que par leur couleur) ; le paladin et le rôdeur
  ont des valeurs provisoires ; au plus trois fois la même classe (3 places).

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `Application` (`app/`) — la boucle et la pile d'écrans

`Application.hpp` / `.cpp`, dans l'exécutable `main` (pas dans `squad_core` :
elle possède la fenêtre). Voir `docs/ecrans.md`.

Données membres : `m_fenetre` (`sf::RenderWindow`), `m_police` (`sf::Font`),
`m_reglages` (`Reglages`), `m_pile` (`PileDEcrans`).

| Fonction | Rôle |
|---|---|
| `Application()` | Charge `reglages.ini` (absent ou illisible = réglages par défaut), ouvre la fenêtre, charge la police (`std::runtime_error` si elle manque), empile `EcranMenu` (avec `resolutionsDisponibles` du bureau) |
| `int executer()` | Boucle : `dt` (borné à `s_dtMax` = 1/20 s, seul endroit du bornage), événements (la croix ferme ; sinon `traiterEvenement` de l'écran du dessus puis `appliquer` de sa `Transition`), `mettreAJour` de l'écran du dessus, `clear` de la fenêtre (le seul), `dessiner` de chaque écran de `premierEcranADessiner(m_pile)` à celui du dessus, `display`. S'arrête quand la fenêtre est fermée ou la pile vide |
| `void ouvrirFenetre()` (privée) | Crée ou recrée la fenêtre à partir des réglages : plein écran à la résolution du bureau, ou fenêtré (`Titlebar` et `Close`) à celle des réglages ; réapplique 60 images par seconde et `setKeyRepeatEnabled(false)` |
| `void appliquer(Transition)` (privée) | `std::visit` : `Empiler` pousse l'écran, `Depiler` retire celui du dessus, `Quitter` ferme la fenêtre, `AppliquerReglages` copie les réglages, rouvre la fenêtre puis écrit `reglages.ini` |

### `Ecran`, `Transition`, `Contexte` (`interface/`) — le patron State

`Ecran.hpp`, `Transition.hpp` et `Contexte.hpp`, sans `.cpp`.

`class Ecran` : base abstraite (copie interdite, destructeur virtuel) ;
fonctions virtuelles pures `Transition traiterEvenement(const sf::Event&,
const Contexte&)`, `void mettreAJour(float dt, const Contexte&)` et `void
dessiner(const Contexte&) const`, et une fonction virtuelle non pure `[[nodiscard]] bool
laisseVoirDessous() const` (`false` par défaut : l'écran cache ce qui est dessous). Les écrans ne
sont manipulés que par `std::unique_ptr<Ecran>`. Aucun écran n'efface la fenêtre : c'est l'`Application`.

### `PileDEcrans` (`interface/`) — le choix des écrans à dessiner

`PileDEcrans.hpp` / `.cpp`. `using PileDEcrans = std::vector<std::unique_ptr<Ecran>>` et
`[[nodiscard]] std::size_t premierEcranADessiner(const PileDEcrans&)` : l'indice du plus haut écran
qui ne laisse pas voir dessous (`laisseVoirDessous()` faux), ou 0 ; `assert` : pile non vide.
Fonction pure, testée avec des écrans de test.

### `EntreeDInterface` (`interface/`) — les entrées des écrans de menu

`EntreeDInterface.hpp` / `.cpp`. `struct PointDInterface { sf::Vector2f point; sf::Vector2f centre; }`.

| Fonction | Rôle |
|---|---|
| `std::optional<PointDInterface> clicGaucheDInterface(const sf::Event&, const Contexte&)` | Le point d'un clic gauche en coordonnées de la vue d'interface et le centre de cette vue ; vide pour tout autre événement ; `[[nodiscard]]` |
| `std::optional<PointDInterface> relachementGaucheDInterface(const sf::Event&, const Contexte&)` | Le point du relâchement du bouton gauche, même conversion ; vide pour tout autre événement ; utilisée par le glisser-déposer de l'inventaire ; `[[nodiscard]]` |
| `PointDInterface preparerLInterface(const Contexte&)` | Pose `vueDInterface` sur la fenêtre et rend la position du curseur dans cette vue, avec le centre ; ne vide pas la fenêtre ; `[[nodiscard]]` |

Non testées (elles ont besoin d'une `RenderWindow`).

`Transition` : `std::variant<Aucune, Empiler, Depiler, Quitter,
AppliquerReglages>` ; `Empiler` porte un `std::unique_ptr<Ecran>`,
`AppliquerReglages` des `Reglages`. Un écran ne modifie jamais la pile lui-même :
l'`Application` applique la transition une fois l'appel terminé.

`struct Contexte { sf::RenderWindow& fenetre; const sf::Font& police; const
Reglages& reglages; }` : construit par l'`Application` à chaque appel, jamais
stocké par un écran.

### `EcranJeu` (`interface/`) — la partie

`EcranJeu.hpp` / `.cpp`. Reprend ce que faisait `main()` pour le jeu.

Données membres : `m_tirageDeGraine` (`std::random_device`), `m_specialites` (`std::vector<IdSpecialite>`, les classes de l'escouade, dans l'ordre des places), `m_partie`
(`Partie`), `m_controleur` (`Controleur`), `m_barre` (`BarreDeRaccourcis`, reconstruite par `barreParDefaut` à chaque `nouvelleCarte`), `m_textes` (`TextesFlottants`, remis à zéro par `nouvelleCarte`), `m_camera` (`sf::View`),
`m_debogage` (`OptionsDeDebogage { bool pointsVises = false; bool ancre = true; bool brouillard = true; }`,
état d'interface remis à sa valeur de départ par `nouvelleCarte`).

| Fonction | Rôle |
|---|---|
| `explicit EcranJeu(std::vector<IdSpecialite> specialites)` | Garde les classes (`m_specialites`, déclaré avant `m_partie`), tire une graine, la génère et construit la partie (`nouvellePartie`, qui appelle `composer(specialites)`) ; F5 réutilise les mêmes classes |
| `Transition traiterEvenement(...)` | Clic gauche (d'abord `clicSurInterface` : sur la barre, `Controleur::appuiInterface`, sauf partie perdue ou gagnée ; sinon `appuiGauche`), clic droit, puis, pour une touche, `executer(commandePourTouche(...))` (voir `docs/boucle-et-entrees.md`) ; sinon `Aucune` |
| `Transition executer(const CommandeDeTouche&)` (privée) | `std::visit` de la commande : Échap annule la visée, F5 nouvelle carte, F3 bascule les points visés et les chemins restants des personnages et des monstres, les zones de contrôle, et la semi-transparence des monstres qui rentrent à leur poste, F4 le carré vert de l'ancre, F6 le brouillard de guerre (visuel seulement), un emplacement de la barre est activé ; `RetourAuChoix` (R sur l'écran de fin) renvoie `Depiler` ; `OuvrirInventaire` appelle `Controleur::interrompre()` puis renvoie `Empiler(EcranInventaire(escouade, noms des classes))` ; les autres : `Aucune` |
| `void mettreAJour(float, const Contexte&)` | Taille de la caméra (`tailleDeVue`), déplacement continu, `Partie::mettreAJour`, puis `prendreEvenements` : chaque événement devient un texte (`TextesFlottants::ajouter`), sauf si le brouillard est actif et que `Partie::estVisible(position)` est faux ; `TextesFlottants::mettreAJour(dt)` (aussi après une défaite ou une victoire : le monde est figé, pas les textes) ; `Controleur::debutImage` (sortie de la visée), caméra centrée sur l'escouade |
| `void dessiner(const Contexte&) const` | Monde avec la caméra (ancre cachée si `!m_debogage.ancre`, brouillard selon `m_debogage.brouillard`), aperçu de visée, points visés et chemins restants si `m_debogage.pointsVises`, textes flottants (vue du monde, après le brouillard), puis, avec `vueDInterface` : barre de raccourcis (`iconesDeLaBarre`), compteur « Monstres : N » et écran de fin |

### `EcranMenu` (`interface/`) — le menu de départ

`EcranMenu.hpp` / `.cpp`. Données membres : `m_resolutions` (`std::vector<Resolution>`, reçues à la
construction, `explicit EcranMenu(std::vector<Resolution>)`), `m_boutons` (`ListeDeBoutons`) :
« Commencer partie », « Options », « Quitter ».

| Fonction | Rôle |
|---|---|
| `Transition cliquer(sf::Vector2f point, sf::Vector2f centre, const Reglages&) const` | Point en coordonnées de la vue d'interface. Commencer : `Empiler(EcranChoixEscouade)` ; Options : `Empiler(EcranOptions)` avec les réglages en cours et les résolutions reçues (`m_resolutions`) ; Quitter : `Quitter` ; ailleurs : `Aucune` |

### `EcranOptions` (`interface/`) — les options d'affichage

`EcranOptions.hpp` / `.cpp`. Données membres : `m_reglages` (copie), `m_resolutions`
(`std::vector<Resolution>` disponibles), `m_indiceResolution`, `m_boutons`
(trois lignes : mode, résolution, Retour).

| Fonction | Rôle |
|---|---|
| `EcranOptions(const Reglages&, std::vector<Resolution>)` | Part des réglages en cours ; l'indice est celui de la résolution courante dans la liste (0 si absente) |
| `Transition cliquer(sf::Vector2f point, sf::Vector2f centre)` | Mode : bascule, `AppliquerReglages`. Résolution : tiers gauche = précédente, tiers droit = suivante, `AppliquerReglages` (`Aucune` aux bouts de la liste, au milieu, ou ligne inactive : plein écran ou liste vide). Retour : `Depiler` |

### `EcranChoixEscouade` (`interface/`) — le choix des 3 classes

`EcranChoixEscouade.hpp` / `.cpp`. Entre le menu et la partie. Données membres : `m_selection` (`SelectionDEscouade`), `m_classes` (`ListeDeBoutons`, un bouton par entrée du `catalogue()`, dans son ordre), `m_actions` (`ListeDeBoutons` : « Commencer », « Retour »). Trois colonnes sans position stockée, recalculées à partir du `centre` de la vue d'interface : les classes à gauche, les cases de sélection puis les actions à droite. Voir `docs/ecrans.md`.

| Fonction | Rôle |
|---|---|
| `sf::Vector2f centreDesClasses(sf::Vector2f centre)` / `centreDesCases(...)` / `centreDesActions(...)` | Centre de chaque colonne (décalages de −300, +300 et +300 px en x ; 0, −80 et +140 en y) ; une seule source pour le dessin, le clic et les tests ; `[[nodiscard]]` |
| `EcranChoixEscouade()` | Sélection vide, « Commencer » inactif |
| `Transition cliquer(sf::Vector2f point, sf::Vector2f centre)` | Classe : l'ajoute à la sélection (sans effet si elle est complète). Case remplie : la retire. « Commencer » (actif seulement si la sélection est complète) : `Empiler(EcranJeu(sélection))`, la sélection restant dans cet écran. « Retour » : `Depiler`. Case vide ou ailleurs : `Aucune` ; `[[nodiscard]]` |
| `const SelectionDEscouade& getSelection() const` | La sélection en cours (tests) ; `[[nodiscard]]` |
| `void dessiner(const Contexte&) const` | Titre, boutons des classes, trois cases « 1. Guerrier » ou « 1. — » (bande de la couleur que le personnage aura en jeu, doublons assombris), actions |

### `EcranInventaire` (`interface/`) — l'inventaire

`EcranInventaire.hpp` / `.cpp`. Données membres : `m_escouade` (`Escouade&`, celle de l'`EcranJeu` du dessous : valide grâce à l'invariant de la pile), `m_nomsDesClasses` (`std::vector<std::string>`, un nom par place), `m_origine` (`std::optional<PositionDObjet>`, l'origine du glissement en cours).

| Fonction | Rôle |
|---|---|
| `EcranInventaire(Escouade&, std::vector<std::string>)` | `assert` : `s_nombreDePlaces` noms |
| `Transition appuyer(sf::Keyboard::Scancode)` | I : `Depiler` ; Échap : annule le glissement en cours (`Aucune`), sinon `Depiler` ; autre touche : `Aucune` ; `[[nodiscard]]` |
| `void saisir(sf::Vector2f point, sf::Vector2f tailleDeVue)` | Commence un glissement si un objet est sous le point |
| `void relacher(sf::Vector2f point, sf::Vector2f tailleDeVue)` | `deplacerObjet(origine, position sous le point)` s'il y en a une ; termine toujours le glissement ; sans glissement, ne fait rien |
| `void annulerLeGlissement()` | Efface l'origine sans rien déplacer |
| `const std::optional<PositionDObjet>& origineDuGlissement() const` | L'origine du glissement, vide sinon ; lue par les tests ; `[[nodiscard]]` |
| `Transition traiterEvenement(...)` | Touche : `appuyer` ; appui gauche : `saisir` ; relâchement gauche : `relacher` ; `FocusLost` : `annulerLeGlissement` ; le reste : `Aucune` |
| `void mettreAJour(float, const Contexte&)` | Ne fait rien (le monde dessous est figé) |
| `bool laisseVoirDessous() const` | `true` |
| `void dessiner(const Contexte&) const` | Vue d'interface, voile sombre, panneau (marge de 40 px), titre, les 3 colonnes (en-tête à la couleur du personnage, armure lue sur son `Equipement`, « (mort) » si ses points de vie sont 0, 10 emplacements, caractéristiques effectives et points de vie, puis capacités offensive et défensive, lus sur l'`Escouade`), le sac (sigles), puis : sans glissement, l'infobulle de l'objet sous le curseur ; pendant un glissement, le contour des destinations valides, le voile sur l'origine et l'objet saisi sous le curseur (pas d'infobulle) |

### `DispositionInventaire` (`interface/`) — où est chaque élément

`DispositionInventaire.hpp` / `.cpp`, fonctions pures en coordonnées de la vue d'interface, `tailleDeVue` en paramètre. Constantes : `s_nombreDePlaces` (3), `s_colonnesDuSac` (10), `s_casesDuSac` (40, la capacité du sac), `s_lignesDeCaracteristiques` et `s_lignesDeStatistiques` (3), `s_cellulesDeStatistiques` (6). Colonnes de 340 px centrées (20 px d'écart), emplacements de 160 × 40 px, cases du sac de 44 px.

| Fonction | Rôle |
|---|---|
| `rectangleDeLEnTete(place, tailleDeVue)` / `rectangleDeLArmure(...)` | Bande de la classe et ligne « Armure » de la colonne de cette place (de 1 à `s_nombreDePlaces`) ; `[[nodiscard]]` |
| `rectangleDeLEmplacement(place, Emplacement, tailleDeVue)` | Rectangle de l'emplacement ; les 5 premiers de l'enum à gauche, les 5 suivants à droite ; `[[nodiscard]]` |
| `rectangleDesCaracteristiques(place, tailleDeVue)` / `rectangleDeLaCellule(place, indice, tailleDeVue)` | Étape 88 : le bloc sous les emplacements (340 × 54 px, `s_lignesDeCaracteristiques` = 3 lignes de 18 px) et ses 6 cellules en deux colonnes remplies colonne par colonne : Puissance, Endurance, Adresse, puis Savoir, Volonté, points de vie ; `assert(indice <= s_nombreDeCaracteristiques)` ; `[[nodiscard]]` |
| `rectangleDesStatistiques(place, tailleDeVue)` / `rectangleDeLaStatistique(place, indice, tailleDeVue)` | Étape 89 : le bloc sous celui des caractéristiques (340 × 54 px, `s_lignesDeStatistiques` = 3 lignes de 18 px) et ses `s_cellulesDeStatistiques` (6) cellules en deux colonnes remplies colonne par colonne ; la première colonne porte les capacités offensive et défensive, le reste est libre pour les statistiques de l'étape 90 ; `assert(indice < s_cellulesDeStatistiques)` ; le sac a été descendu de 500 à 550 px pour lui faire place ; `[[nodiscard]]` |
| `rectangleDeLaCaseDuSac(indice, tailleDeVue)` | Rectangle de la case (`assert(indice < s_casesDuSac)`). Nommée ainsi parce que `rectangleDeLaCase` existe dans `SelectionDEscouade` ; `[[nodiscard]]` |
| `std::optional<PositionDObjet> elementSous(point, tailleDeVue)` | `PositionPortee {place, emplacement}` ou `PositionDansLeSac {caseDuSac}` (`objets/PositionDObjet`) ; vide dans un écart, sur un en-tête ou hors des éléments ; `[[nodiscard]]` |
| `sf::FloatRect placerInfobulle(curseur, tailleInfobulle, tailleDeVue)` | Sous et à droite du curseur (16 px), de l'autre côté s'il sortirait de la vue, puis gardée dans la vue ; `[[nodiscard]]` |

### `SelectionDEscouade` (`escouade/`) — les classes choisies

`SelectionDEscouade.hpp` / `.cpp`. Logique pure : une classe et deux fonctions libres. Donnée membre : `m_specialites` (`std::vector<IdSpecialite>`) ; constante `static constexpr std::size_t s_taille = 3`.

| Fonction | Rôle |
|---|---|
| `void ajouter(IdSpecialite id)` | À la fin s'il reste de la place, sans effet sinon ; les doublons sont permis |
| `void retirer(std::size_t indice)` | Retire cette case, les suivantes remontent ; `assert(indice < liste().size())` |
| `bool estComplete() const` | Trois classes choisies ; `[[nodiscard]]` |
| `const std::vector<IdSpecialite>& liste() const` | Dans l'ordre de sélection, c'est l'ordre des places 1 à 3 ; `[[nodiscard]]` |
| `sf::FloatRect rectangleDeLaCase(std::size_t indice, sf::Vector2f centre)` | Case de sélection `indice` : colonne de 3 cases de 520 × 60 px, 20 px d'écart, centrée sur `centre` ; `assert` ; `[[nodiscard]]` |
| `std::optional<std::size_t> caseSous(sf::Vector2f point, sf::Vector2f centre)` | Case qui contient le point, vide dans un écart ou hors des cases ; construite sur `rectangleDeLaCase` ; `[[nodiscard]]` |

### `ListeDeBoutons` (`interface/`) — une colonne de boutons

`ListeDeBoutons.hpp` / `.cpp`. Données membres : `m_libelles`, `m_actifs`. Aucune
position stockée : la colonne (boutons de 520 × 60 px, 20 px d'écart) est
recalculée à partir du `centre` reçu.

| Fonction | Rôle |
|---|---|
| `explicit ListeDeBoutons(std::vector<std::string> libelles)` | Tous les boutons sont actifs |
| `sf::FloatRect rectangleDuBouton(std::size_t indice, sf::Vector2f centre) const` | Rectangle du bouton, la colonne centrée sur `centre` |
| `void definirActif(std::size_t indice, bool actif)` | Un bouton inactif est grisé et jamais « sous » le curseur |
| `std::optional<std::size_t> boutonSous(sf::Vector2f point, sf::Vector2f centre) const` | Indice du bouton actif qui contient le point |
| `void dessiner(sf::RenderTarget&, const sf::Font&, sf::Vector2f centre, std::optional<std::size_t> survole) const` | Dessine fond, contour et libellé (UTF-8) ; le bouton survolé est plus clair |

### `Reglages` (`interface/`) — les réglages d'affichage

`Reglages.hpp` / `.cpp` : logique pure, fonctions libres sans état.
`enum class ModeAffichage { Fenetre, PleinEcran }`, `struct Resolution {
unsigned largeur, hauteur; }` (comparable), `struct Reglages { unsigned largeur =
1200; unsigned hauteur = 800; ModeAffichage mode = Fenetre; TouchesDesRaccourcis touchesDesRaccourcis; }`, où `using TouchesDesRaccourcis = std::array<sf::Keyboard::Scancode, s_emplacementsPrincipaux>` (index = emplacement ; touches physiques ; par défaut Q W E R / A S D F / Z X C V). Les touches ne sont ni lues ni écrites dans `reglages.ini` (l'écran de configuration viendra plus tard).

| Fonction | Rôle |
|---|---|
| `const std::vector<Resolution>& resolutionsProposees()` | 1200 × 800, 1280 × 720, 1600 × 900, 1920 × 1080 |
| `std::vector<Resolution> resolutionsDisponibles(sf::Vector2u tailleBureau)` | Celles qui tiennent dans le bureau |
| `Reglages lireReglages(std::istream&)` | Format `cle=valeur` (`largeur`, `hauteur`, `mode` = `fenetre` ou `plein_ecran`). Jamais d'exception : clé inconnue ou ligne mal formée ignorée, valeur invalide = défaut, résolution hors liste = 1200 × 800 |
| `void ecrireReglages(std::ostream&, const Reglages&)` | Écrit les trois lignes lues par `lireReglages` |
| `std::optional<std::size_t> emplacementPourTouche(const TouchesDesRaccourcis&, sf::Keyboard::Scancode)` | Emplacement de cette touche dans la table, vide si elle n'y est pas ; `[[nodiscard]]` |

### `TailleDeVue` (`interface/`) — la vue à hauteur fixe

`TailleDeVue.hpp` / `.cpp`, constante `s_hauteurDeVue` (800 px).

| Fonction | Rôle |
|---|---|
| `sf::Vector2f tailleDeVue(sf::Vector2u tailleFenetre, float hauteur)` | `hauteur` en hauteur, largeur = `hauteur × largeur / hauteur de la fenêtre` |
| `sf::View vueDInterface(sf::Vector2u tailleFenetre)` | Vue de 800 px de haut, coin haut gauche en (0, 0) : barre de skills, compteur, écran de fin, menus |
