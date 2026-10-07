# Squad-ARPG — Roadmap

## Méthode de travail

- **Conception dans Claude.ai, implémentation dans Claude Code.** On
  discute des options (génie logiciel et design de jeu) dans Claude.ai ;
  quand Olivier dit « fait moi un prompt », Claude.ai rédige un prompt qu'il
  colle dans Claude Code (VSCode). Les règles de travail de Claude Code
  sont dans `CLAUDE.md`.
- On avance une pièce à la fois : d'abord la structure de données (les
  paramètres/membres), puis les fonctions une par une. Déclaration dans le
  `.hpp` avant implémentation dans le `.cpp`.
- On ne construit que ce dont on a besoin maintenant, jamais en prévision
  d'une étape future.
- Après chaque petit ajout (même une seule fonction), on compile et on
  teste avant de continuer.
- Olivier teste visuellement et fait les commits lui-même.
- On confirme toujours où en est Olivier avant de proposer la suite.

## Découpage des étapes

### Étape 0 — Squelette de projet ✅ TERMINÉE

- [x] `main.cpp` minimal (`std::cout`, compile et s'exécute)
- [x] Validation manuelle de la chaîne d'outils (`g++` direct)
- [x] `CMakeLists.txt` minimal (C++20, cible `add_executable(main ...)`)
- [x] Configuration CMake + génération Ninja
- [x] Build et exécution via `cmake --build build`
- [x] `.gitignore` (`build/`)
- [x] Commits Git propres (main.cpp, puis CMakeLists.txt séparément)

### Étape 1 — Carré qui bouge au clic ✅ TERMINÉE

1. [x] Squelette de projet (fait à l'étape 0)
2. [x] Ouvrir une fenêtre SFML vide
3. [x] Dessiner un carré statique dedans
4. [x] Détecter un clic de souris (afficher les coordonnées en console,
       rien ne bouge encore)
5. [x] Déplacer le carré à la position du clic
6. [x] Afficher la position sous le carré (texte SFML)

### Étape 2 — Déplacement progressif ("tic" par "tic") ✅ TERMINÉE

1. [x] Position cible (`cible`) stockée séparément de la position du carré
2. [x] Déplacement d'un pas fixe par tic vers la cible (vecteur direction
       normalisé × vitesse)
3. [x] Arrêt exact sur la cible : si la distance restante ≤ pas, on pose
       le carré sur la cible (pas d'oscillation)
4. [x] Vitesse indépendante de la cadence : delta time via `sf::Clock`,
       vitesse en pixels par seconde

### Étape 3 — Salle avec murs ✅ TERMINÉE (remplacée en partie par l'étape 4)

1. [x] Dessiner un mur : un seul rectangle statique au milieu de la fenêtre
2. [x] Détecter le chevauchement avec le mur
3. [x] Bloquer le carré : on calcule la position suivante et on n'avance
       pas si elle entre dans le mur
4. [~] Glisser le long du mur → réalisé à l'étape 4.5, sur la grille
5. [~] Plusieurs murs (`std::vector`) → remplacé par la `TileMap` de
       l'étape 4

### Étape 4 — Carte en grille (`TileMap`) ✅ TERMINÉE

1. [x] `TileMap.hpp` : structure de données seule (`enum class Tile`,
       dimensions, taille de case, `std::vector<Tile>` 1D)
2. [x] Constructeur : sol partout, murs sur les bords ; ajout de
       `TileMap.cpp` au CMake
3. [x] `getTile(x, y)` (hors carte = mur) et fonction privée `indice(x, y)`
4. [x] `dessiner(sf::RenderTarget&)` : grille visible, sol brun, murs gris
5. [x] `chevaucheUnMur(sf::FloatRect)` : remplace le mur unique ;
       glissement le long des murs (axes X et Y testés séparément)
6. [x] Construction à partir d'un plan texte (`'#'` = mur), avec
       validation (plan vide, lignes de longueurs différentes) ; l'ancien
       constructeur "bordure" est supprimé

### Étape 5 — Caméra centrée sur le personnage ✅ TERMINÉE

1. [x] Carte agrandie au-delà de la fenêtre (59 × 40 cases), sans caméra
2. [x] `sf::View` centrée sur le carré à chaque image (après la mise à
       jour, avant le dessin)
3. [x] Clic converti en coordonnées du monde (`window.mapPixelToCoords`) ;
       le texte de position reste attaché au point cliqué sur la carte

### Étape 6 — Classe `Escouade` ✅ TERMINÉE

Le carré de `main.cpp` devient une classe, construite à côté de l'ancien
carré puis substituée à lui une fois validée.

1. [x] `Escouade.hpp` : structure de données seule (position, taille,
       cible, vitesse)
2. [x] Constructeur `explicit` (position de départ) ; `Escouade.cpp`
       ajouté au CMake
3. [x] `dessiner` : forme SFML créée à partir des données
4. [x] `allerVers(destination)` : le clic donne la cible
5. [x] `mettreAJour(dt, carte)` : déplacement et collision transférés
       sans modification (déclaration anticipée de `TileMap`) ; validé en
       comparant les deux carrés
6. [x] `getCentre()` pour la caméra ; suppression de l'ancien carré

### Étape 7 — Tests unitaires (Catch2) ✅ TERMINÉE

1. [x] `CMakeLists.txt` réorganisé : `TileMap.cpp` et `Escouade.cpp`
       regroupés dans une bibliothèque statique `squad_core` (SFML en
       `PUBLIC`, dossier `src` en `PUBLIC` include) ; `main` ne compile
       plus que `src/main.cpp` et se lie à `squad_core` en `PRIVATE`
2. [x] `find_package(Catch2 3 REQUIRED)` ; exécutable `tests` (lié à
       `squad_core` et `Catch2::Catch2WithMain`) avec un test trivial pour
       valider la chaîne d'outils
3. [x] Tests du constructeur et de `TileMap::getTile` : sol/mur,
       dimensions déduites du plan, coordonnée négative, plan vide
       (`std::out_of_range`), lignes de longueurs différentes
       (`std::invalid_argument`)
4. [x] Tests de `TileMap::chevaucheUnMur` : sol, chevauchement, contact
       exact sans chevauchement (convention `ceil - 1`), décalage de 1 px,
       hors carte en négatif

### Étape 8 — Module de recherche de chemin (A*) ✅ TERMINÉE

Module `Chemin` autonome, testé, mais pas encore branché sur l'escouade :
elle continue d'aller en ligne droite (branchement prévu à l'étape 9).

1. [x] Squelette : `Chemin.hpp`/`Chemin.cpp`, fonction libre
       `trouverChemin(const TileMap&, sf::Vector2i depart, sf::Vector2i
       arrivee)` (cas départ == arrivée seulement) ; ajout à `squad_core`
       et à l'exécutable `tests` (`test_Chemin.cpp`)
2. [x] Chemin vide si départ ou arrivée est un mur ou hors carte (y
       compris quand départ == arrivée sur un mur)
3. [x] `TileMap::getLargeur`/`getHauteur` (en cases), avec un test sur un
       plan non carré
4. [x] Algorithme A* complet (file de priorité, coûts, parents,
       reconstruction du chemin) ; tests : ligne droite, contournement
       d'un mur, arrivée inaccessible

### Étape 9 — L'escouade suit le chemin du A* ✅ TERMINÉE

1. [x] `TileMap::caseContenant(sf::Vector2f)` : conversion pixels → case
       (`std::floor`, comme `chevaucheUnMur`) ; tests (milieu de case,
       limite exacte entre deux cases, coordonnée négative)
2. [x] `TileMap::centreDeCase(sf::Vector2i)` : conversion case → pixels ;
       tests (case `(0,0)`, une autre case, aller-retour avec
       `caseContenant`)
3. [x] Le centre de l'escouade (et non plus son coin haut-gauche) vise le
       point visé dans `mettreAJour`, via un simple vecteur de
       déplacement appliqué à `m_position`
4. [x] `m_cible` remplacée par `m_chemin` (`std::vector<sf::Vector2f>`) et
       un indice de progression ; `allerVers` y place un seul point,
       `mettreAJour` avance l'indice quand un point est atteint
5. [x] Branchement du A* : nouvelle signature
       `allerVers(destination, const TileMap&)`, appel de
       `trouverChemin` depuis `Escouade` (`main.cpp` ne connaît pas le
       module `Chemin`) ; case de départ ignorée ; chemin vide = ordre
       ignoré
6. [x] Le dernier point du chemin est le point exact cliqué, pas le
       centre de la case d'arrivée

**Limite connue** : en cliquant près du coin d'une case alignée avec
l'escouade sur un seul axe, le centre de case visé peut être presque sur
la même ligne/colonne que l'escouade, ce qui produit parfois un léger
zigzag (l'escouade descend sur l'axe avant de remonter). Comportement
accepté pour l'instant, à revisiter plus tard. **Résolu à l'étape 11**
par le lissage du chemin.

### Étape 10 — Tests unitaires de l'escouade ✅ TERMINÉE

Couvre `Escouade` par des tests Catch2, sans modifier `src/` — ces tests
serviront de filet de sécurité pour le lissage de chemin à venir.

1. [x] `test_Escouade.cpp` créé et ajouté au CMake ; état initial (centre
       = position de départ + `(10, 10)`) et immobilité sans ordre
2. [x] Test de vitesse : un seul tic vers une destination lointaine en
       terrain dégagé avance le centre de `vitesse * dt`, quelle que soit
       la direction exacte
3. [x] Fonction d'aide de simulation (tics de 1/60 s pendant une durée
       donnée) ; tests d'arrivée : point exact hors centre de case en
       terrain dégagé, et contournement d'un mur
4. [x] Gestion des ordres : ordre vers un mur à l'arrêt (ignoré), ordre
       vers un mur en cours de route (trajet d'origine conservé), second
       ordre valide en cours de route (remplace la destination)

Aucun comportement inattendu révélé : les 8 tests passent sans qu'aucune
modification de `src/` n'ait été nécessaire.

### Étape 11 — Lissage du chemin ✅ TERMINÉE

Retire du chemin calculé par le A* les points de passage inutiles :
l'escouade va en ligne droite vers le point le plus lointain qu'elle peut
atteindre sans toucher un mur. Corrige le zigzag de l'étape 9.

1. [x] `TileMap::trajetLibre(sf::FloatRect, sf::Vector2f)`, cas
       horizontal et vertical (enveloppe rectangulaire + `chevaucheUnMur`) ;
       tests (trajet dégagé, mur sur le trajet, contact exact, décalage
       de 1 px, déplacement nul, sortie de carte)
2. [x] `trajetLibre`, cas oblique : balayage par colonnes de cases,
       paramétré par `t` entre 0 (départ) et 1 (arrivée) ; tests (trajet
       oblique dégagé, mur en plein milieu dans les 4 sens obliques, coin
       qui accroche malgré une ligne des centres dégagée, trajet voisin
       libre, arrivée et départ dans un mur)
3. [x] `Chemin::lisserChemin` : algorithme glouton (recherche du point le
       plus lointain atteignable, en partant de la fin) ; tests (liste
       vide, un seul point, terrain dégagé → seul le dernier point
       reste, mur à contourner → moins de points, segments tous libres)
4. [x] Branchement dans `Escouade::allerVers`, entre la construction du
       chemin en pixels et son stockage ; les 8 tests d'`Escouade` de
       l'étape 10 passent sans modification, plus un test de vitesse
       (ligne droite lissée assez rapide pour un trajet diagonal, alors
       qu'un escalier non lissé aurait été trop lent pour la même durée
       simulée)

**Limite connue** : `lisserChemin` ne fait que retirer des points de la
liste donnée par le A* (centres de case) ; il ne peut pas en ajouter ni
en déplacer. Autour d'un pilier, dès qu'un saut direct est bloqué par le
mur, l'escouade retombe sur le centre de la case suivante tel quel,
plutôt que sur un point plus proche du coin du pilier qui raccourcirait
encore le trajet. Elle frôle donc moins les murs qu'un lissage « idéal »
(type *funnel algorithm* ou Theta*) ne le ferait, mais elle ne reste
jamais coincée. Accepté pour l'instant (cf. ARCHITECTURE.md).

### Étape 12 — Membres de l'escouade (`Personnage`, formation en carré) ✅ TERMINÉE

L'escouade possède 4 personnages qui se déplacent eux-mêmes pour tenir
une formation en carré autour d'elle, avec collision contre les murs, et
la formation s'oriente selon la direction de déplacement. Principe
retenu pour l'avenir (combat, actions) : un personnage sait seulement
« aller vers un point » ; c'est l'escouade qui décide de ce point.

1. [x] `Personnage.hpp`/`.cpp` : structure de données (`m_centre`,
       `m_taille`, `m_couleur`) et constructeur ; ajout à `squad_core`
2. [x] `Personnage::dessiner`
3. [x] `Escouade` : `m_decalages` (4 décalages de formation, orientation
       0 = vers +x) et `m_personnages`, créés à leur place dans le
       constructeur via la fonction privée `placePersonnage` (point de
       calcul unique) ; dessinés après l'ancre
4. [x] `Personnage::mettreAJour(dt, cible)` : ligne droite, pas fixe,
       sans collision à ce stade
5. [x] `Escouade::mettreAJour` met à jour l'ancre puis appelle celle de
       chaque personnage avec sa place courante
6. [x] Module `Deplacement` : extraction de `deplacerAvecGlissement`
       (collision/glissement axe par axe) depuis `Escouade::mettreAJour`,
       sans modification de comportement ; les 9 tests d'`Escouade`
       passent sans changement
7. [x] `tests/test_Deplacement.cpp` : terrain dégagé, mur qui bloque un
       axe, glissement oblique (composante parallèle conservée),
       déplacement nul
8. [x] `Personnage::mettreAJour` utilise `deplacerAvecGlissement` ; les
       personnages ne traversent plus les murs
9. [x] `Escouade::m_orientation` (`sf::Angle`, 0 au départ), utilisée
       dans `placePersonnage` via `rotatedBy`
10. [x] `Escouade::mettreAJour` met à jour `m_orientation` selon la
        direction **voulue** de l'ancre (avant collision), seulement si
        elle a un point à atteindre et que la distance est `> 0` ;
        rotation instantanée, dernière orientation conservée à l'arrêt

**Limite connue** : rotation instantanée de la formation (pas de
transition progressive) — un personnage peut donc traverser brusquement
sa place à chaque virage du chemin. Accepté pour l'instant (cf.
ARCHITECTURE.md). Autre limite héritée des décisions de conception : pas
de collision entre personnages ni entre un personnage et l'ancre ; un
personnage peut rester accroché derrière un coin de mur tant que
l'escouade ne repasse pas par là.

### Étape 13 — Déplacement continu tant que le bouton est enfoncé ✅ TERMINÉE

Tant que le bouton gauche de la souris est enfoncé, l'escouade reçoit à
chaque image l'ordre d'aller vers le point du monde sous le curseur :
elle avance sans arrêt et ajuste sa trajectoire en continu, comme dans
un ARPG classique.

1. [x] Retrait du texte de position et de la police (`arial.ttf`)
       associée dans `main.cpp` : suppression pure, le déplacement au
       clic ne change pas
2. [x] Remplacement de l'événement `MouseButtonPressed` par la lecture
       de l'état (`sf::Mouse::isButtonPressed`, uniquement si
       `window.hasFocus()`) : à chaque image, la position du curseur est
       reconvertie en coordonnées du monde et redonnée à `allerVers`
       (A* + lissage, pas de second mode « direction du curseur »)

Seul `main.cpp` a été modifié ; `Escouade` et le reste de `squad_core`
sont inchangés, les 58 tests passent sans modification.

### Étape 14 — Rotation progressive de la formation ✅ TERMINÉE

L'orientation de la formation tourne progressivement vers la direction
voulue, à vitesse angulaire limitée, plutôt que de sauter d'un coup :
les places glissent en arc de cercle autour de l'ancre.

1. [x] Membre `m_orientationVoulue` (`sf::Angle`, 0 au départ) : reçoit
       la direction voulue de l'ancre (comme `m_orientation` avant),
       recopiée telle quelle dans `m_orientation`. Aucun changement de
       comportement à ce stade
2. [x] Membre `m_vitesseRotation` (360°/s, fixé dans le constructeur) ;
       la recopie directe est remplacée par une rotation bornée,
       exécutée à chaque `mettreAJour` (que l'ancre bouge ou non) :
       écart par `wrapSigned()` (toujours le plus court côté, entre
       -180° et 180°), pas limité à `vitesseRotation * dt`, arrêt exact
       si l'écart restant est plus petit que ce pas

**Limite connue** : la rotation progressive réduit nettement les
croisements entre personnages dans les virages, mais ne les élimine pas
complètement. Lors d'un demi-tour très rapide, un personnage peut couper
en ligne droite vers sa nouvelle place (de l'avant vers l'arrière ou
inversement) pendant qu'un autre fait le trajet inverse, et les deux
carrés se chevauchent brièvement en traversant le centre de la
formation — même si leurs places *finales* ne se chevauchent jamais.
Cause : chaque personnage poursuit sa cible en ligne droite
(`Personnage::mettreAJour`), pas en suivant l'arc de la rotation ; corriger
ça demanderait soit une vraie collision personnage-personnage, soit un
déplacement qui suit l'arc plutôt qu'une ligne droite — les deux hors de
portée de cette étape (cf. ARCHITECTURE.md).

### Étape 15 — Un monstre avec des points de vie ✅ TERMINÉE

Un monstre immobile (carré rouge, avec une barre de vie) est présent sur
la carte. Chaque clic gauche sur lui retire des points de vie ; à zéro,
il disparaît. Pas de classe de base commune avec `Personnage`, pas
d'héritage ; `main.cpp` possède le monstre (une simple variable), pas
`Escouade`.

1. [x] `Monstre.hpp`/`.cpp` : structure de données (`m_centre`,
       `m_taille`, `m_pointsDeVie`) et constructeur `explicit` (30 × 30,
       100 points de vie) ; ajout à `squad_core`
2. [x] `Monstre::dessiner` (carré rouge) ; création dans `main.cpp` à
       `(300, 100)`, dans la première pièce, vérifiée sans chevauchement
       de mur ; dessiné après la carte, avant l'escouade (pour qu'elle
       apparaisse par-dessus en lui passant dessus)
3. [x] `Monstre::contient(sf::Vector2f)` (convention `sf::FloatRect::
       contains` : bords gauche/haut inclus, droit/bas exclus) ;
       `tests/test_Monstre.cpp` créé et ajouté au CMake
4. [x] `Monstre::recevoirDegats(int)` (jamais sous zéro) et
       `Monstre::estVivant() const` ; tests (vivant à la création,
       encore vivant après 80 dégâts, mort après 100, mort après un coup
       qui dépasse le reste, toujours mort après un coup de plus)
5. [x] Branchement dans `main.cpp` : l'événement `MouseButtonPressed`
       (bouton gauche) revient dans la boucle d'événements, uniquement
       pour l'attaque (20 points de dégâts par clic, constante locale) —
       un coup par clic, pas par image ; la lecture d'état de l'étape 13
       reste inchangée pour le déplacement ; un clic sur le monstre fait
       les deux sans cas particulier ; le monstre mort n'est plus dessiné
       ni attaquable, mais continue d'exister
6. [x] Membre des points de vie maximum ; barre de vie dessinée dans
       `Monstre::dessiner` (fond sombre + rectangle vert proportionnel
       aux points de vie restants, au-dessus du monstre)

Aucune modification d'`Escouade`, `Personnage`, `Chemin`, `TileMap`,
`Deplacement`, ni des tests existants ; les 58 tests passent toujours,
plus 11 nouveaux sur `Monstre`.

### Étape 16 — Attaque continue et verrouillage de cible ✅ TERMINÉE

Quand le joueur appuie sur le monstre et garde le bouton enfoncé, la
cible se verrouille : le monstre reçoit un coup toutes les 0,5 seconde
et l'escouade ne reçoit plus d'ordre de déplacement, où que soit le
curseur, jusqu'au relâchement du bouton ou à la mort du monstre. Seul
`main.cpp` est modifié ; `Escouade` et `Monstre` ne changent pas.

1. [x] Variable locale `cibleVerrouillee` (`bool`) : posée par l'événement
       de clic si le monstre est vivant et contient le point ; retirée
       après la boucle d'événements si le bouton n'est plus enfoncé, si
       la fenêtre n'a plus le focus, ou si le monstre est mort (lecture
       d'état, pas d'événement `MouseButtonReleased`) ; `allerVers`
       conditionnée à l'absence de verrou
2. [x] Variable locale `rechargeAttaque` (`float`, secondes, décrémentée
       de `dt`, jamais sous zéro) et constante `tempsRecharge` (0,5 s) ;
       les dégâts quittent l'événement de clic (qui ne pose plus que le
       verrou) pour une règle unique dans la phase de mise à jour : si la
       cible est verrouillée et que la recharge est à zéro, le monstre
       reçoit `degatsParClic` et la recharge repart à `tempsRecharge`.
       Ordre après la boucle d'événements : diminuer la recharge,
       attaquer si verrou + recharge à zéro, retirer le verrou si
       nécessaire, puis donner l'ordre de déplacement si le bouton est
       enfoncé et la cible non verrouillée — l'attaque passe avant le
       retrait du verrou pour qu'un clic très bref compte quand même pour
       un coup

Les 69 tests passent sans modification ; aucun nouveau test (tout se
passe dans `main.cpp`).

### Étape 17 — Portée d'attaque : les personnages frappent ✅ TERMINÉE

Quand la cible est verrouillée, l'escouade s'approche du monstre en
gardant sa formation, et ce sont les personnages qui frappent, chacun
avec sa propre portée, ses propres dégâts et sa propre recharge. `main.cpp`
n'inflige plus de dégâts lui-même.

1. [x] `Monstre::getCentre() const` ; un test dans `tests/test_Monstre.cpp`
2. [x] `Personnage` : membres `m_portee` (80 px), `m_degats` (5),
       `m_tempsRecharge` (0,5 s) et `m_recharge` (0 à la création),
       valeurs fixes identiques pour les 4, comme `m_vitesse`
3. [x] `Personnage::estAPortee(cible)` (distance au centre `<=` portée) ;
       `tests/test_Personnage.cpp` créé et ajouté au CMake
4. [x] Décrémentation de `m_recharge` dans `Personnage::mettreAJour`
       (jamais sous zéro), puis `Personnage::frapper(cible)` (dégâts si
       à portée et recharge écoulée, sinon 0, et relance la recharge)
5. [x] `Escouade::attaquer(cible, carte)` : approche (`allerVers`) tant
       qu'un personnage n'est pas à portée, arrêt sur place (chemin
       réduit au centre actuel) quand les 4 y sont, puis `frapper` sur
       chacun ; renvoie la somme des dégâts. `Escouade` et `Personnage`
       ne connaissent toujours pas `Monstre`, tout passe par
       `sf::Vector2f`
6. [x] Branchement dans `main.cpp` : le verrou de cible appelle
       `monstre.recevoirDegats(escouade.attaquer(monstre.getCentre(),
       carte))` à chaque image ; `rechargeAttaque`, `tempsRecharge` et
       `degatsParClic` de l'étape 16 supprimés

**Limite connue** : la portée ignore les murs (un personnage peut
frapper à travers) — acceptée pour l'instant.

### Étape 18 — Plusieurs monstres, cible désignée par identifiant ✅ TERMINÉE

Plusieurs monstres sur la carte, stockés dans un `std::vector<Monstre>`.
Cliquer sur l'un d'eux verrouille CE monstre, désigné par un identifiant
stable ; les monstres morts sont retirés du vecteur.

1. [x] `Monstre` : membre `m_id` et second paramètre du constructeur
       (`Monstre(sf::Vector2f centre, int id)`) ; `explicit` retiré.
       C'est `main.cpp` qui attribue les identifiants (variable locale
       `prochainId`, incrémentée à chaque création), pas un compteur
       `static` dans `Monstre`
2. [x] `int Monstre::getId() const` ; test sur deux monstres créés avec
       des identifiants différents
3. [x] Le verrou `bool cibleVerrouillee` devient `std::optional<int>
       idCible` (vide = pas de verrou), toujours avec un seul monstre
4. [x] La variable `monstre` devient `std::vector<Monstre> monstres` (un
       seul élément à ce stade) ; boucles pour le clic (premier monstre
       qui contient le point, `std::find_if`) et pour le dessin ;
       recherche de la cible par `std::find_if` sur l'`id`, l'itérateur
       recalculé à chaque image plutôt que conservé (il ne survit pas à
       un retrait du vecteur)
5. [x] Trois monstres dans la première pièce, identifiants attribués par
       `prochainId` : `(300, 100)` (id 1, position d'origine),
       `(100, 300)` (id 2), `(300, 500)` (id 3) — tous espacés d'au
       moins 100 px
6. [x] Retrait des monstres morts avec `std::erase_if`, après le retrait
       du verrou et avant l'ordre de déplacement ; suppression des
       vérifications `estVivant()` devenues inutiles dans la boucle du
       clic et celle du dessin, l'invariant « tout monstre du vecteur
       est vivant » tenant désormais entre deux appels d'`erase_if`

### Étape 19 — Indicateur visuel de la cible verrouillée ✅ TERMINÉE

Le monstre verrouillé (`idCible`) est entouré d'un contour jaune tant
que le verrou est posé sur lui.

1. [x] `Monstre::dessiner` reçoit le paramètre `bool estCible`, sans
       encore s'en servir ; `main.cpp` passe `idCible == monstre.getId()`
       dans la boucle de dessin (comparaison directe de l'`optional`
       avec l'identifiant, sans tester `has_value()` au préalable)
2. [x] `Monstre::dessiner` applique un contour jaune de 3 px vers
       l'intérieur (`setOutlineThickness(-3.0f)`) au carré du monstre
       quand `estCible` est vrai — ne touche pas la barre de vie, ne
       change pas l'empreinte du monstre à l'écran

`idCible` reste l'unique source de vérité sur le monstre visé ;
`Monstre` ne stocke rien de cette information lui-même. Aucune
modification d'`Escouade`, `Personnage`, `Chemin`, `TileMap` ni
`Deplacement`.

### Étape 20 — Les monstres ripostent ✅ TERMINÉE

Chaque monstre frappe le personnage le plus proche de lui dès que
celui-ci est à sa portée, qu'il soit lui-même attaqué ou non. Les
personnages ont maintenant des points de vie et une barre de vie.

1. [x] `Personnage` : membres `m_pointsDeVie` et `m_pointsDeVieMax`
       (100 chacun)
2. [x] `Personnage::recevoirDegats` et `estVivant` ; tests dans
       `test_Personnage.cpp`
3. [x] `Personnage::dessiner` : barre de vie (même apparence que celle
       de `Monstre`)
4. [x] `Personnage::getCentre() const` ; un test
5. [x] `Escouade` : fonction privée `indicePersonnageLePlusProche`, puis
       `personnageLePlusProche` ; test par propriété (centre renvoyé
       plus proche d'un point lointain que ne l'est le centre de
       l'escouade)
6. [x] `Escouade::recevoirDegats(origine, quantite)` (applique les
       dégâts au personnage le plus proche d'`origine`) ; pas de test
       unitaire, effet vérifié visuellement
7. [x] `Monstre` : membres `m_portee` (80 px), `m_degats` (10),
       `m_tempsRecharge` (1 s), `m_recharge`
8. [x] `Monstre::mettreAJour(dt)` (décrémente la recharge, le monstre
       reste immobile) et `Monstre::frapper(cible)` (même contrat que
       `Personnage::frapper`) ; tests dans `test_Monstre.cpp`, calqués
       sur ceux de `Personnage::frapper`
9. [x] Branchement dans `main.cpp` : pour chaque monstre, à chaque
       image, `m.mettreAJour(dt)` puis `escouade.recevoirDegats(origine,
       m.frapper(escouade.personnageLePlusProche(origine)))`, après
       `escouade.mettreAJour(dt, carte)`

Duplication assumée entre `Monstre` et `Personnage` (portée/recharge
d'un côté, points de vie/barre de vie de l'autre) — aucune
factorisation à cette étape, elle reste une option pour plus tard.

**Limite connue** : un personnage à 0 point de vie ne meurt pas — il
reste en formation, continue de frapper et de recevoir des coups.
Traiter la mort d'un personnage est hors de portée de cette étape.

### Étape 21 — Un monstre change de cible quand un personnage meurt ✅ TERMINÉE

Limite connue de l'étape 20 : un personnage à 0 point de vie restait
ciblé par les monstres et continuait de frapper. Un monstre ignore
maintenant les personnages morts et frappe le vivant le plus proche de
lui ; un personnage mort ne frappe plus.

1. [x] `Personnage::frapper` renvoie 0, sans relancer la recharge, si
       le personnage n'est pas vivant ; test dans `test_Personnage.cpp`
2. [x] Passage aux `std::optional`, sans encore filtrer les morts
       (comportement inchangé) : `indicePersonnageLePlusProche` renvoie
       un `std::optional<std::size_t>`, `personnageLePlusProche` un
       `std::optional<sf::Vector2f>`, `recevoirDegats` ne fait rien si
       l'indice est vide ; `main.cpp` adapté (riposte conditionnelle,
       `if (auto cible = ...)`) ; test existant de
       `personnageLePlusProche` adapté
3. [x] `indicePersonnageLePlusProche` ignore les personnages non
       vivants et renvoie un optional vide s'il n'en reste aucun ;
       nouveaux tests dans `test_Escouade.cpp` (dégâts insuffisants
       pour tuer, changement de cible une fois le plus proche mort,
       plus aucun vivant)

**Limite connue** : un personnage mort reste en formation, se déplace
et est dessiné comme avant (barre de vie vide) ; seule sa participation
au combat change. Le retirer de la formation ou du vecteur, le griser,
et une condition de défaite de l'escouade restent hors de portée de
cette étape (cf. ARCHITECTURE.md).

### Étape 22 — Classe `Attaque`, partagée par `Personnage` et `Monstre` ✅ TERMINÉE

Extrait dans une classe `Attaque` (composition, pas héritage) la
logique de portée/dégâts/recharge dupliquée entre `Personnage` et
`Monstre` depuis l'étape 20, sans aucun changement de comportement ni
d'interface publique de ces deux classes.

1. [x] `Attaque.hpp`/`.cpp` : structure de données (`m_portee`,
       `m_degats`, `m_tempsRecharge`, `m_recharge`) et constructeur ;
       ajout à `squad_core`
2. [x] `Attaque::estAPortee` ; `tests/test_Attaque.cpp` créé et ajouté
       au CMake (point à portée, hors portée, distance exactement
       égale à la portée)
3. [x] `Attaque::mettreAJour` puis `Attaque::frapper` ; tests calqués
       sur les six cas existants de `Personnage::frapper`
4. [x] Substitution dans `Personnage` : les quatre membres remplacés
       par `m_attaque` ; `estAPortee`, `frapper` et la décrémentation
       dans `mettreAJour` délèguent à `m_attaque`. La règle « un
       personnage mort ne frappe pas » reste dans `Personnage::frapper`
5. [x] Substitution dans `Monstre`, de la même façon, sans règle
       supplémentaire
6. [x] Ménage dans `test_Personnage.cpp` et `test_Monstre.cpp` : ne
       gardent que les cas prouvant le branchement et les valeurs
       propres à chaque classe, les six cas génériques restant
       couverts par `test_Attaque.cpp`

Aucune modification d'`Escouade` ni de `main.cpp` ; les interfaces
publiques de `Personnage` et de `Monstre` sont inchangées.

### Étape 23 — Classe `Mobile` : le déplacement par chemin sort d'`Escouade` ✅ TERMINÉE

Extrait dans une classe `Mobile` (composition, comme `Attaque`) le
déplacement par chemin qui vivait directement dans `Escouade` :
position, taille, vitesse, chemin, suivi d'indice, A* + lissage,
collision/glissement. Prépare l'étape 24, où `Monstre` aura besoin du
même déplacement.

1. [x] `Mobile.hpp`/`.cpp` : structure de données (`m_position`,
       `m_taille`, `m_chemin`, `m_indexChemin`, `m_vitesse`) et
       constructeur (en centre) ; ajout à `squad_core`
2. [x] `Mobile::getCentre`/`getTaille` ; `tests/test_Mobile.cpp` créé et
       ajouté au CMake
3. [x] `Mobile::allerVers` : code déplacé tel quel depuis
       `Escouade::allerVers`
4. [x] `Mobile::mettreAJour` : partie « ancre » copiée depuis
       `Escouade::mettreAJour` (sans orientation ni personnages) ;
       tests calqués sur ceux d'`Escouade`
5. [x] `Mobile::arreter` (chemin réduit au centre actuel)
6. [x] `Mobile::directionVoulue` (vecteur du centre vers le point
       courant du chemin, vide si plus de point ou distance nulle)
7. [x] Substitution dans `Escouade` : `m_position`, `m_taille`,
       `m_vitesse`, `m_chemin`, `m_indexChemin` remplacés par
       `m_mobile` ; `allerVers`, `getCentre`, l'arrêt dans `attaquer`
       et la partie « ancre » de `mettreAJour` délèguent à `m_mobile` ;
       l'orientation de la formation est désormais calculée à partir
       de `m_mobile.directionVoulue()`, lue au même moment du tic
       qu'avant (avant la collision). Les tests d'`Escouade` passent
       sans modification

Aucune modification de `Personnage`, `Monstre`, `Attaque` ni
`main.cpp` ; interface publique d'`Escouade` inchangée.

### Étape 24 — À définir

Pistes discutées :
- Donner un `Mobile` à `Monstre`, pour qu'il se déplace (ils restent
  immobiles pour l'instant)
- Mort ou mise hors combat d'un personnage à 0 point de vie (retrait de
  la formation, apparence), condition de défaite de l'escouade
- Collision entre personnages, ou déplacement suivant l'arc de rotation
  plutôt qu'une ligne droite, pour éliminer le chevauchement résiduel
  lors d'un demi-tour rapide (voir limite connue de l'étape 14)
- Factorisation des points de vie et de la barre de vie entre
  `Monstre` et `Personnage`, si la duplication devient gênante
- Effet visuel au coup ou à la mort d'un monstre ou d'un personnage
- Un lissage plus serré autour des coins (*funnel algorithm* ou Theta*),
  si le contournement actuel des piliers s'avère insuffisant
- Finitions : caméra bloquée aux bords de la carte, affichage fixe à
  l'écran, police committée dans le dépôt
- Limitation de fréquence des appels à `allerVers`, si un problème de
  performance est mesuré sur une carte plus grande
- Types de personnages, statistiques, armes, capacités, ligne de vue
  pour la portée (hors de portée explicite des étapes 12 à 17, 20 et 22)

## État actuel (à mettre à jour à chaque session)

- Dépôt Git : `squad-arpg`, poussé sur GitHub
- Outils : conception dans Claude.ai, implémentation avec Claude Code dans
  VSCode (règles dans `CLAUDE.md`)
- Bibliothèque graphique : SFML 3.0.2, intégrée à CMake
- Fenêtre 1200 × 800 ; carte de 59 × 40 cases de 40 px (2360 × 1600 px),
  plusieurs pièces et couloirs, définie en texte dans `main.cpp`
- L'escouade (classe `Escouade`, carré vert de 20 × 20, l'ancre) démarre
  en `(100, 100)`, se déplace à 180 px/s en suivant le chemin calculé par
  le A* puis lissé (trajets directs en terrain dégagé, plus de zigzag,
  coins de murs contournés au plus court sans les frôler exactement),
  s'arrête exactement sur le point cliqué, reste bloquée par les murs et
  glisse le long de ceux-ci (filet de sécurité conservé). Depuis l'étape
  23, ce déplacement par chemin vit dans un module séparé, `Mobile`
  (possédé par valeur par `Escouade`, `m_mobile`), en vue de l'étape 24
  où `Monstre` en aura besoin aussi
- L'escouade possède 4 personnages (`Personnage`, carrés colorés de
  20 × 20, 240 px/s) qui tiennent une formation en carré autour de
  l'ancre, bloqués par les murs et glissant le long d'eux comme elle
  (fonction partagée `deplacerAvecGlissement`, module `Deplacement`) ;
  la formation pivote progressivement vers la direction de marche
  (360°/s, toujours par le plus court côté). Chaque personnage a aussi
  une attaque (portée 80 px, dégâts 5, recharge 0,5 s, module `Attaque`
  partagé avec `Monstre` depuis l'étape 22), 100 points de vie et une
  barre de vie au-dessus de lui ; un personnage à 0 point de vie ne
  meurt pas encore (limite connue, étape 20), mais depuis l'étape 21 il
  ne frappe plus et n'est plus ciblé par les monstres
- Caméra (`sf::View`) toujours centrée sur l'escouade ; du noir apparaît
  au-delà des bords de la carte
- Tant que le bouton gauche de la souris est enfoncé et que la fenêtre a
  le focus, l'escouade reçoit l'ordre d'aller vers le point sous le
  curseur à chaque image (lecture d'état, pas d'événement) ; un clic
  bref reste un simple aller-vers ponctuel. Plus de texte de position ni
  de police chargée dans `main.cpp` (retirés à l'étape 13)
- Trois monstres immobiles (classe `Monstre`, carrés rouges de 30 × 30,
  100 points de vie chacun, barre de vie au-dessus d'eux, identifiant
  stable `m_id` attribué par `main.cpp`) sont présents dans la première
  pièce, à `(300, 100)` (id 1), `(100, 300)` (id 2) et `(300, 500)`
  (id 3) ; `main.cpp` les possède directement dans un
  `std::vector<Monstre>` (pas `Escouade`) ; pas de collision avec eux
  (l'escouade et les personnages leur passent dessus). Chaque monstre a
  aussi une attaque (portée 80 px, dégâts 10, recharge 1 s, module
  `Attaque`) : il frappe le personnage **vivant** de l'escouade le plus
  proche de lui dès qu'il est à portée, qu'il soit lui-même attaqué ou
  non (`Escouade::personnageLePlusProche`/`recevoirDegats`, étape 20 ;
  filtrage des morts, étape 21) ; s'il n'en reste aucun, il ne frappe
  plus
- Cliquer sur un monstre verrouille CE monstre (son `id`, dans
  `std::optional<int> idCible`) : l'escouade s'approche en formation
  jusqu'à ce que ses 4 personnages soient à portée (80 px), puis
  s'arrête et chacun frappe pour 5 points de dégâts dès que sa propre
  recharge (0,5 s) est écoulée — la barre de vie descend par petits
  crans, plus vite une fois tout le monde à portée, mort en 2 à
  3 secondes. Le monstre verrouillé est entouré d'un contour jaune
  (étape 19). Le verrou tombe au relâchement du bouton, à la perte du
  focus, ou à la mort du monstre visé ; l'escouade ne reçoit alors plus
  d'ordre de déplacement tant qu'il est posé, et en reçoit un de nouveau
  (vers le curseur) dès qu'il retombe. Un monstre mort est retiré du
  vecteur (`std::erase_if`, à chaque image) ; les autres restent
  attaquables, quel que soit l'ordre dans lequel ils meurent
- Logique de jeu (`TileMap`, `Escouade`, `Mobile`, `Chemin`,
  `Personnage`, `Deplacement`, `Monstre`, `Attaque`) dans une
  bibliothèque `squad_core`, testée avec Catch2 3
  (`./build/tests.exe`, 112 tests : 32 sur `TileMap`, 13 sur `Chemin`,
  16 sur `Escouade`, 4 sur `Deplacement`, 16 sur `Monstre`, 11 sur
  `Personnage`, 9 sur `Attaque`, 11 sur `Mobile`)
- Depuis l'étape 22, `Personnage` et `Monstre` possèdent chacun un
  `m_attaque` (`Attaque`) plutôt que leurs propres membres de portée,
  dégâts et recharge — composition, pas héritage ; aucun changement de
  comportement ni d'interface publique
- Depuis l'étape 23, `Escouade` possède un `m_mobile` (`Mobile`) plutôt
  que ses propres membres de position/chemin/vitesse — composition,
  comme `Attaque` ; aucun changement de comportement ni d'interface
  publique
- Près des piliers, le contournement n'est pas toujours le plus court
  possible (voir étape 11, limite connue) — accepté pour l'instant
- Lors d'un demi-tour rapide, deux personnages peuvent encore se
  chevaucher brièvement en traversant le centre de la formation (voir
  étape 14, limite connue) — accepté pour l'instant
- La portée d'attaque des personnages (et désormais des monstres)
  ignore les murs (voir étape 17, limite connue) — accepté pour
  l'instant
- Un personnage à 0 point de vie ne meurt pas : il reste en formation,
  se déplace et continue de recevoir des coups ; depuis l'étape 21, il
  ne frappe plus et n'est plus ciblé par les monstres, qui reportent
  leur attaque sur un autre personnage vivant, ou cessent de frapper si
  aucun ne l'est (voir étape 20, limite connue) — accepté pour l'instant
- Étape en cours : **Étape 24** — à choisir
