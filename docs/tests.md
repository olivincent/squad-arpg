# Squad-ARPG — Tests

## Tests

- **Catch2 3 adopté**, installé via MSYS2
  (`mingw-w64-ucrt-x86_64-catch`), trouvé par `find_package(Catch2 3
  REQUIRED)` comme SFML (pas de `FetchContent`).
- **Un seul exécutable `tests`**, six fichiers (`test_TileMap.cpp`,
  `test_Chemin.cpp`, `test_Escouade.cpp`, `test_Deplacement.cpp`,
  `test_Monstre.cpp`, `test_Personnage.cpp`), lancé directement
  (`./build/tests.exe`) — pas de CTest ni de `catch_discover_tests`.
- **Petits plans dédiés** : chaque test construit son propre plan texte
  (3×3, 4×4, 5×5…), jamais la carte de `main.cpp`, pour isoler exactement
  le cas testé.
- **Teste uniquement l'interface publique** de `TileMap`, `Chemin`,
  `Escouade`, `Personnage` et `Monstre` (`getTile`, `chevaucheUnMur`,
  `trajetLibre`, `getLargeur`/`getHauteur`,
  `caseContenant`/`centreDeCase`, le constructeur, `trouverChemin`,
  `lisserChemin`, `allerVers`, `attaquer`, `mettreAJour`, `getCentre`,
  `estAPortee`, `frapper`, `recevoirDegats`, `estVivant`,
  `personnageLePlusProche`) — aucune fonction n'a été ajoutée seulement
  pour faciliter les tests.
- **Propriétés plutôt que chemin exact** : pour `trouverChemin`, comme
  plusieurs plus courts chemins peuvent exister, les tests vérifient des
  propriétés (premier élément = départ, dernier = arrivée, cases
  consécutives voisines, aucun mur, longueur attendue) via une fonction
  d'aide partagée (`verifierCheminValide`), plutôt que de comparer à une
  liste de cases exacte. Même principe pour `lisserChemin` (moins de
  points, dernier point conservé, chaque segment validé par
  `trajetLibre`).
- **Chaque test `trajetLibre` porte un commentaire qui justifie ses
  chiffres** : les tests obliques en particulier (coin qui accroche un
  mur malgré une ligne des centres dégagée, même obstacle abordé dans
  les 4 sens obliques) ont été vérifiés à la main avant d'être écrits,
  pour s'assurer que le résultat attendu correspond bien à la géométrie
  décrite, pas à une intuition approximative.
- **`Escouade` : des résultats, pas des trajectoires**. Les tests
  vérifient où l'escouade finit et ce qu'elle fait d'un ordre, jamais sa
  position à un tic intermédiaire précis ni la forme du chemin suivi —
  pour rester valides même après un futur lissage du chemin. Pas de
  doublure (mock) : `dt` étant déjà un paramètre de `mettreAJour`, et
  `TileMap` se construisant à partir d'un plan texte, les tests utilisent
  les vraies classes.
  - **Comparaison avec tolérance** (`REQUIRE_THAT` + `Catch::Matchers::
    WithinAbs`, jamais `==`) : les positions accumulent des calculs en
    virgule flottante (vitesse × dt, racines carrées), donc deux calculs
    mathématiquement égaux peuvent différer d'une poussière d'arrondi.
    Fonction d'aide `verifierPointProche`.
  - **Fonction d'aide de simulation** `simuler(escouade, carte, durée)` :
    appelle `mettreAJour` en boucle avec un `dt` de 1/60 s (comme
    `setFramerateLimit(60)` dans `main.cpp`) pendant la durée donnée. Les
    tests simulent une durée généreuse par rapport au trajet attendu et
    ne vérifient que l'état final.
  - **`dt` en paramètre rend la classe testable** : si `mettreAJour`
    lisait le temps lui-même (horloge interne), un test n'aurait aucun
    moyen de contrôler le temps simulé à chaque tic ; en le recevant en
    paramètre, le comportement devient déterministe et reproductible
    sans horloge factice.
- **`Monstre` testé par son interface publique** (`getCentre`, `getId`,
  `contient`, `recevoirDegats`, `estVivant`, `mettreAJour`, `frapper`) :
  bords d'inclusion/exclusion de `contient` vérifiés séparément sur
  chaque côté (gauche/haut inclus, droit/bas exclus) ; `recevoirDegats`/
  `estVivant` couvrent le cas d'un coup qui dépasse le reste des points
  de vie et celui d'un coup supplémentaire une fois déjà mort ; `getId`
  vérifié sur deux monstres créés avec des identifiants différents, qui
  renvoient chacun le leur (étape 18). Le branchement dans `main.cpp`
  (verrouillage de cible par identifiant, vecteur de monstres,
  `std::erase_if`, contour sur la cible, riposte) n'est pas testé —
  tout se passe dans `main.cpp`, hors de portée de `tests/`.
  - **`frapper`/`mettreAJour` (étape 20)** : mêmes six cas que
    `Personnage::frapper` ci-dessous, avec les valeurs du monstre
    (portée 80 px, dégâts 10, recharge 1 s). Fonction d'aide
    `ecoulerTemps(monstre, duree)`, même principe que celle de
    `test_Personnage.cpp` mais sans cible ni carte, puisque
    `Monstre::mettreAJour` ne prend que `dt` (le monstre ne se déplace
    pas). **Noms de `TEST_CASE` distincts de ceux de
    `test_Personnage.cpp`** (préfixés `Monstre.`) : Catch2 exige des
    noms de cas de test uniques dans tout l'exécutable `tests`, pas
    seulement dans un fichier.
- **`Personnage` (`test_Personnage.cpp`, étape 17)** :
  - `estAPortee` : point proche à portée, point lointain hors portée,
    distance exactement égale à la portée (cas limite, `<=`).
  - `frapper` : renvoie les dégâts à portée ; 0 hors portée ; 0 au
    second appel immédiat (recharge en cours) ; 0 après une durée
    simulée trop courte, de nouveau les dégâts après une durée simulée
    généreuse (**durées qui évitent les égalités strictes en virgule
    flottante** : 0,3 s et 0,6 s plutôt que 0,25 s ou 0,5 s pile, pour
    ne pas dépendre de l'arrondi exact de l'accumulation de `dt`) ; un
    coup tenté hors de portée ne déclenche pas la recharge.
  - **Simuler le temps sans déplacer le personnage** : fonction d'aide
    `ecoulerTemps` qui appelle `mettreAJour` par tics de 1/60 s avec le
    centre du personnage comme cible (il reste immobile, seule la
    recharge évolue), sur un petit plan dégagé dédié — même principe
    que `simuler` pour `Escouade`.
  - **Points de vie (étape 20)** : `estVivant` à la création, encore
    vivant après 80 dégâts, mort après 100, mort après un coup qui
    dépasse le reste — mêmes quatre cas que `Monstre`. `getCentre`
    vérifié directement sur le centre donné au constructeur.
- **`Escouade::attaquer` (`test_Escouade.cpp`)** : comme pour les autres
  fonctions d'`Escouade`, des résultats plutôt que des trajectoires
  exactes.
  - Fonction d'aide `simulerAttaque` : appelle `attaquer` puis
    `mettreAJour` à chaque tic, et cumule les dégâts renvoyés par
    `attaquer` sur toute la durée simulée.
  - Cible lointaine en terrain dégagé : le premier appel renvoie 0 et
    l'escouade s'en rapproche (distance réduite après un tic) ; après
    une durée généreuse, le total des dégâts cumulés est `> 0`, mais
    l'escouade ne finit pas sur la cible (elle s'arrête à portée).
  - Cible déjà à portée des 4 personnages dès le départ : le premier
    appel renvoie 20 (4 × 5) et l'escouade ne bouge pas.
- **`Escouade::personnageLePlusProche` (étape 20)** : test par
  propriété plutôt que par position de formation exacte, comme les
  autres tests d'`Escouade` — pour un point lointain, la distance entre
  le centre renvoyé et ce point est strictement inférieure à la
  distance entre le centre de l'escouade et ce point (un personnage en
  formation est forcément plus proche d'un point extérieur que le
  centre de l'escouade lui-même, quelle que soit l'orientation exacte
  de la formation). `recevoirDegats` n'a pas de test unitaire : son
  effet n'est observable par aucune fonction publique, et on n'ajoute
  pas d'accesseur juste pour le tester (vérifié visuellement en jeu).
