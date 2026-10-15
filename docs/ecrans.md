# Squad-ARPG — Écrans, vues et réglages

Depuis l'étape 65, `main()` ne fait que construire une `Application`. Le menu,
les options et la partie sont trois écrans (`Ecran`) que l'`Application`
empile. Ce fichier explique les décisions ; le détail des interfaces est dans
`ARCHITECTURE.md`, et celui d'une image de jeu dans `docs/boucle-et-entrees.md`.

## Pile d'écrans (patron State)

- **Une classe de base abstraite** `Ecran` (`traiterEvenement`, `mettreAJour`,
  `dessiner`), trois classes concrètes (`EcranMenu`, `EcranOptions`,
  `EcranJeu`), manipulées seulement par `std::unique_ptr<Ecran>`. Copie
  interdite, destructeur virtuel.
- **Une pile** (`std::vector<std::unique_ptr<Ecran>>`) plutôt qu'un écran
  courant : un futur menu de pause s'empilera sur la partie sans réécriture.
  Aujourd'hui la pile n'a jamais plus de deux écrans (menu, puis options ou
  jeu). Seul l'écran du dessus reçoit les événements, est mis à jour et est
  dessiné ; celui du dessous est conservé, mais figé et invisible.
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
  fait que convertir l'événement puis l'appeler. La conversion et le dessin ne
  sont pas testés.
- **Flux** : menu → « Commencer partie » = `Empiler(EcranJeu)` (nouvelle
  `Partie`, nouvelle graine affichée) ; « Options » = `Empiler(EcranOptions)` ;
  « Quitter » = `Quitter`. En jeu, R sur l'écran de fin = `Depiler` (retour au
  menu), ignorée en cours de partie ; F5 génère une nouvelle carte à tout
  moment. Échap en jeu annule seulement la visée.
- **Principe de conception** : c'est l'application du principe 1 révisé
  (« anticiper la structure, pas les fonctionnalités ») : la pile et la
  `Transition` accueilleront la pause sans réécriture, mais les alternatives
  `Remplacer`, une `mettreAJour` qui renvoie une `Transition` et le dessin des
  écrans du dessous n'existent pas, car rien ne les utilise.

## `EcranJeu` : le jeu de l'ancien `main.cpp`

- Code déplacé sans changement de comportement (étape 65, sous-étape 4) : un
  seul écart. `Controleur::debutImage` (sortie de la visée si le lanceur ne
  peut plus lancer) s'exécutait au début de l'image, avant les événements ; il
  n'y a plus de point d'entrée à cet endroit, il s'exécute donc à la fin de
  `mettreAJour`. Les événements de l'image suivante voient le même état ; seul
  l'affichage de la visée disparaît une image plus tôt.
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
  remplace la vue par défaut de SFML pour la barre de skills, le compteur,
  l'écran de fin, le menu et les options. `dessinerBarreDeSkills` et
  `dessinerEcranDeFin` lisent la taille de la vue active (`getView`), pas
  celle de la vue par défaut.
- **Clics** : convertis avec `mapPixelToCoords(pixel, vue)` : avec la caméra
  dans `EcranJeu`, avec la vue de l'interface dans les menus.

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
- Pas de menu de pause ni de retour au menu en cours de partie (R n'agit
  qu'après une défaite ou une victoire).
- Les boutons de `ListeDeBoutons` recopient le centrage de texte de
  `EcranDeFin.cpp` (à extraire dans un module commun, dans un commit à part).
- Le menu reste sous les options et n'est pas redessiné pendant le changement
  de fenêtre ; sa disposition est recalculée au prochain dessin.
