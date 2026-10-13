# Squad-ARPG — Historique des étapes

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

### Étape 24 — Les monstres détectent, poursuivent et abandonnent ✅ TERMINÉE

Un monstre qui voit un personnage vivant entrer dans son rayon de
détection le poursuit (A* + lissage, via `Mobile`), s'arrête dès qu'il
est à portée d'attaque, et abandonne si sa proie s'éloigne trop.

1. [x] Six nouveaux emplacements de monstres (`(660, 380)`, `(500, 580)`,
       `(900, 220)`, `(1940, 180)`, `(1500, 540)`, `(1620, 540)`),
       remplaçant les trois de l'étape 18, hors de la salle de départ
2. [x] `Monstre` : `m_centre` et `m_taille` remplacés par `m_mobile`
       (`Mobile`, 30 × 30, 120 px/s), sans encore le faire bouger
       (signature de `mettreAJour` inchangée) ; `getCentre`, `contient`,
       `dessiner` et `frapper` délèguent à `m_mobile`
3. [x] Détection sans état : `m_rayonDetection` (200 px) ; nouvelle
       signature `mettreAJour(dt, std::optional<sf::Vector2f> cible,
       const TileMap&)` ; cible dans le rayon → `allerVers`, sinon
       `arreter()` ; `main.cpp` adapté
4. [x] Arrêt à portée : `arreter()` aussi si la cible est à portée de
       `m_attaque`, pour que le monstre cherche à frapper plutôt qu'à
       rejoindre sa proie
5. [x] État et abandon : `m_enPoursuite` et `m_rayonAbandon` (400 px) ;
       une cible détectée (`<=` 200 px) n'est abandonnée qu'au-delà de
       400 px — hystérésis entre les deux rayons, pour éviter les
       oscillations poursuite/abandon près d'un seuil unique

**Limites connues** : un monstre qui abandonne reste sur place plutôt
que de retourner à son poste (pas de patrouille) ; plusieurs monstres
qui poursuivent la même cible peuvent se superposer (pas de collision
entre monstres). La détection, comme la portée d'attaque (étape 17),
ignore les murs. Les trois sont acceptées pour l'instant (cf.
ARCHITECTURE.md).

### Étape 25 — Statut du monstre et retour au poste ✅ TERMINÉE

Le monstre a un statut explicite à trois valeurs (en garde, poursuite,
retour) et un territoire autour de son poste : il abandonne quand
lui-même s'éloigne trop de son poste, puis y retourne.

1. [x] `m_enPoursuite` devient `m_etat` (`enum class Etat { EnGarde,
       Poursuite }`, privé) ; aucun changement de comportement
2. [x] Membre `m_poste` (le centre donné au constructeur), pas encore
       utilisé (enchaînés avec la sous-étape précédente)
3. [x] Statut `Retour` : au lieu de s'arrêter sur place, le monstre
       retourne à son poste (`arreter()` puis `allerVers(m_poste,
       carte)`, une seule fois à l'entrée en `Retour`), puis repasse
       `EnGarde` une fois arrivé (`directionVoulue()` vide) ; règle
       d'abandon encore inchangée (proie trop loin du monstre, ou plus
       de proie) ; pendant le retour, toute proie est encore ignorée
4. [x] Territoire : `m_rayonAbandon` renommé `m_rayonTerritoire` ;
       l'abandon dépend désormais de la distance entre le **monstre**
       et son **poste** (`> 400 px`), plus de celle à sa proie ;
       ancienne règle supprimée
5. [x] Détection limitée au territoire : fonction privée `const`
       `proieDetectee(cible)` (proie à portée de détection du monstre
       ET dans le territoire du poste), utilisée à l'identique en
       `EnGarde` et en `Retour` — un monstre qui rentre peut donc de
       nouveau être détourné par une proie, à condition qu'elle soit
       dans son territoire

**Limites connues** : un monstre qui rentre ignore une proie dès
qu'elle sort du territoire, même si elle reste proche de lui ; pas de
patrouille ni de récupération de points de vie au retour ; plusieurs
monstres gardent chacun leur poste et territoire sans se coordonner
(cf. ARCHITECTURE.md).

### Étape 26 — Mort d'un personnage et défaite de l'escouade ✅ TERMINÉE

Un personnage à 0 point de vie tombe sur place : il ne bouge plus et
devient gris. Quand les 4 sont morts, l'escouade ne reçoit plus aucun
ordre du joueur.

1. [x] `Personnage::mettreAJour` : retour immédiat si le personnage
       n'est pas vivant (ni déplacement, ni recharge d'attaque)
2. [x] `Escouade::attaquer` : la condition d'approche/arrêt ignore les
       personnages non vivants (vrai par vacuité si aucun n'est vivant,
       donc l'escouade s'arrête aussi dans ce cas)
3. [x] `Personnage::dessiner` : carré gris et sans barre de vie s'il
       n'est pas vivant (`m_couleur` conservée mais ignorée)
4. [x] `bool Escouade::estVivante() const` (vrai si au moins un
       personnage est vivant)
5. [x] Branchement dans `main.cpp` : si l'escouade n'est pas vivante,
       le clic ne pose plus de verrou, un verrou déjà posé retombe, et
       `allerVers` n'est plus appelée ; le reste de la boucle (mise à
       jour, caméra, dessin, fermeture de la fenêtre) continue de
       tourner normalement

Un personnage mort reste dans `m_personnages` (pas de retrait,
contrairement à `Monstre`) : la formation ne se resserre pas, sa place
reste vide.

**Hors de portée, toujours vrai** : retirer un mort du vecteur ;
resserrer la formation ; résurrection ou soin ; écran de défaite ;
arrêter l'ancre à la mort du dernier personnage (si elle avait un
trajet en cours, elle le termine) ; factoriser les points de vie entre
`Personnage` et `Monstre`.

### Étape 27 — Un clic engage le combat ✅ TERMINÉE

Le verrou de cible devient un état d'`Escouade` au lieu d'une variable
locale de `main.cpp` : un seul clic sur un monstre engage le combat, qui
continue seul jusqu'à ce qu'il soit rompu, sans avoir à tenir le
bouton enfoncé.

1. [x] Pas de type nommé à sortir dans son propre en-tête : l'identifiant
       de monstre est un simple `int`, déjà un paramètre de constructeur
       dans `Monstre.hpp`
2. [x] `Escouade` : `m_cibleVerrouillee` (`std::optional<int>`, vide à la
       création) et ses trois fonctions (`verrouiller`, `deverrouiller`,
       `getCibleVerrouillee`) ; tests unitaires (verrou vide au départ,
       verrouiller, remplacer, déverrouiller)
3. [x] `main.cpp` : la variable locale du verrou remplacée par les
       appels à `Escouade`, sans changer le comportement (le relâchement
       du bouton déverrouillait encore à ce stade)
4. [x] `main.cpp` : le relâchement du bouton et la perte du focus ne
       déverrouillent plus la cible ; le combat ne se rompt que dans
       trois cas (mort de la cible, clic sur le sol, clic sur un autre
       monstre) ; nouvelle variable locale `deplacementContinu`, armée
       uniquement par l'événement d'appui, pour que le déplacement
       continu de l'étape 13 reste indépendant de l'état du combat

### Étape 28 — Portées d'attaque différentes selon le personnage ✅ TERMINÉE

Deux personnages de mêlée à l'avant de la formation et deux tireurs à
l'arrière, reconnaissables à leur couleur, chacun frappant selon sa
propre portée. Étape de données : aucun comportement de déplacement ne
change.

1. [x] `Personnage` : le constructeur reçoit son `Attaque` et sa couleur
       (injection, `Personnage(sf::Vector2f centre, sf::Color couleur,
       Attaque attaque)`) au lieu de les fixer lui-même ; `Escouade`
       passe encore les mêmes valeurs qu'avant aux 4 personnages. Tests
       unitaires existants adaptés pour compiler et passer
2. [x] `Escouade` donne aux deux personnages arrière (décalage `x =
       -25`, donc à l'arrière quand l'orientation est 0 = vers `+x`) une
       `Attaque` de portée 150 px et la couleur cyan (tireurs) ; aux deux
       personnages avant (`x = +25`), portée 80 px (inchangée) et la
       couleur rouge (mêlée). Constantes nommées dans un espace de noms
       anonyme de `Escouade.cpp`

Aucune modification d'`Attaque`, `Monstre` ni `Mobile` ; pas d'énumération
ni de notion de « type » de personnage — un personnage reste défini
uniquement par ses données (couleur, attaque).

### Étape 29 — Un clic engage le combat qui continue seul — placement individuel en combat ✅ TERMINÉE

Limite de l'étape 28 : toute la formation avançait jusqu'au contact, les
tireurs finissant collés au monstre comme les mêlées. Désormais, pendant
un combat, un personnage déjà à portée de la cible cesse d'avancer et
reste sur place ; les tireurs s'arrêtent à distance pendant que les
mêlées continuent jusqu'au contact.

1. [x] `Escouade` : nouveau membre `m_cibleCombat` (`std::optional<
       sf::Vector2f>`), mis à jour à chaque appel d'`attaquer(cible,
       carte)` — seul endroit où `Escouade` apprend la position de la
       cible verrouillée. Dans `mettreAJour`, un personnage vivant à
       portée de `m_cibleCombat`, et seulement si le verrou
       (`m_cibleVerrouillee`) est posé, reçoit son propre centre comme
       point à atteindre (immobile) au lieu de sa place en formation ;
       sinon il continue de suivre `placePersonnage(i)` comme avant.
       Tests unitaires ajoutés : un personnage à portée reste figé
       pendant que les autres avancent ; sans verrou, ce même
       personnage suit quand même son emplacement (la règle ne
       s'applique qu'en combat)
2. [x] Lancer le jeu, vérifier les scénarios (approche, mort de la
       cible, retraite, changement de cible) — comportement confirmé

`Personnage`, `Attaque`, `Monstre` et `Mobile` non modifiés ; aucun
`Mobile` ni recherche de chemin individuelle par personnage, aucune
collision ajoutée — seul le chemin de l'ancre contourne les murs.

### Étape 30 — Portée mesurée de bord à bord ✅ TERMINÉE

La portée d'une attaque était comparée à la distance entre les centres
des deux entités : avec une petite portée, un personnage devait entrer
dans le monstre pour frapper, et le résultat dépendait de la taille de
la cible. Désormais, chaque entité est représentée par un cercle, et la
portée est l'écart entre les bords des deux cercles : « portée 10 »
signifie 10 px de vide entre l'attaquant et sa cible.

1. [x] Module `Cercle` (`Cercle.hpp`/`.cpp`) : structure `Cercle
       { centre, rayon }` sans comportement, et fonction libre
       `distanceBordABord(a, b)` (distance entre centres moins les deux
       rayons, jamais négative) ; ajout à `squad_core` ;
       `tests/test_Cercle.cpp` (cercles séparés, qui se touchent, qui se
       chevauchent, rayons différents) (enchaînée)
2. [x] `Personnage::getCercle()` (rayon 10) et `Monstre::getCercle()`
       (rayon 15) : rayon = demi-côté du carré dessiné ; un test chacun
       (enchaînée)
3. [x] `Attaque::estAPortee`/`frapper` reçoivent deux `Cercle` au lieu
       de deux positions ; tous les appelants adaptés : frappes des
       personnages (`Personnage::estAPortee`/`frapper(Cercle)`), riposte
       des monstres (`Monstre::frapper(Cercle)`), arrêt de l'ancre
       (`Escouade::attaquer(Cercle, carte)`), arrêt des personnages à
       portée (`m_cibleCombat` devient un `std::optional<Cercle>`), arrêt
       du monstre en poursuite (`Monstre::mettreAJour` reçoit un
       `std::optional<Cercle>`, fourni par `Escouade::personnageLePlusProche`
       qui renvoie désormais un `Cercle`) ; tests existants adaptés
       (enchaînée)
4. [x] Nouvelles valeurs de portée, bord à bord : mêlée 10 px
       (`s_porteeMelee`), tireurs 120 px (`s_porteeTireur`), monstres
       15 px (ancienne portée de 40 px, moins les deux rayons 10 + 15) ;
       tests de `Monstre` et d'`Escouade` dont les chiffres dépendaient
       des anciennes portées mis à jour

Détection et territoire des monstres, test du clic sur un monstre,
formation, choix du personnage le plus proche et lissage du chemin
restent mesurés de centre à centre ; l'apparence des entités ne change
pas (toujours des carrés).

### Étape 31 — Premier skill, déclenché au clavier ✅ TERMINÉE

Le mêlée avant-gauche possède un skill : une attaque plus forte et plus
lente (25 dégâts, 5 s de recharge, même portée que son attaque de
base), déclenchée par la touche 1 sur la cible verrouillée, avec une
barre de recharge visible sous sa barre de vie.

1. [x] `Personnage` : quatrième paramètre du constructeur,
       `std::optional<Attaque> skill` (vide = pas de skill), stocké dans
       `m_skill` ; sa recharge s'écoule dans `mettreAJour`, en combat ou
       non. `Escouade` le fournit au seul personnage 0 (mêlée
       avant-gauche), constantes `s_degatsSkill` et `s_rechargeSkill` ;
       tests existants adaptés (`std::nullopt`) (enchaînée)
2. [x] `Attaque::relancerRecharge()` (seul ajout à `Attaque`) ;
       `Personnage::declencherSkill(Cercle)` : 0 sans skill ou si mort,
       sinon délègue à `m_skill->frapper` et, si le skill part, relance
       au complet la recharge de l'attaque de base ; tests (refusé sans
       skill, mort, hors de portée, en recharge ; attaque de base
       retardée après un skill réussi) (enchaînée)
3. [x] `Escouade::declencherSkill(Cercle)` : 0 sans verrou, sinon somme
       des `declencherSkill` de tous les personnages (ceux sans skill
       renvoient 0) ; tests (refusé sans combat, 25 et non 50, hors de
       portée, recharge) (enchaînée)
4. [x] `main.cpp` : événement `KeyPressed` sur le scancode `Num1` (touche
       physique, indépendante de la disposition du clavier),
       `setKeyRepeatEnabled(false)` pour qu'une touche maintenue ne
       produise qu'un seul appui ; skill demandé avant `attaquer` dans le
       bloc du verrou, dégâts appliqués au monstre verrouillé
5. [x] `Attaque::getProgressionRecharge()` (0 juste après un coup, 1
       prête) ; barre orange de 3 px sous la barre de vie du personnage
       qui a un skill, absente pour les autres et pour un mort

Aucune modification de `Monstre`, `Mobile` ni `Cercle` ; aucune nouvelle
classe — un skill est une seconde `Attaque`.

**Hors de portée** : skills des trois autres personnages, touches 2
à 4, effets sur le monstre (renversement, zone, soin), mana, HUD,
animation ou effet visuel de frappe, mise en attente du skill.

### Étape 32 — Les monstres se bloquent entre eux ✅ TERMINÉE

Un monstre ne peut plus entrer dans un autre monstre. Technique : le
blocage (le déplacement est refusé), pas de séparation ni de poussée,
selon une liste de cercles obstacles réutilisable.

1. [x] `deplacementBloque(avant, apres, obstacles)` dans `Deplacement` :
       refuse seulement ce qui chevauche ET rapproche ; tests (enchaînée)
2. [x] `deplacerAvecGlissement` reçoit `obstacles` ; essai d'axe refusé
       par un mur ou par `deplacementBloque` ; `Mobile` et `Personnage`
       passent `{}` ; nouveaux tests (enchaînée)
3. [x] `Mobile::mettreAJour(dt, carte, obstacles)` ; `Escouade` passe
       `{}` ; test (un mobile s'arrête avant un obstacle) (enchaînée)
4. [x] `Monstre::mettreAJour(..., obstacles)` ; `main.cpp` construit pour
       chaque monstre la liste des cercles des autres monstres

**Limite connue** : un monstre bloqué ne contourne pas, il glisse si
l'angle le permet, sinon il attend (le A* ignore les entités).

**Hors de portée** : personnages bloqués par les monstres et
inversement (étapes 33 et 34), blocage entre personnages ou de l'ancre,
poussée/séparation, grille spatiale.

### Étape 33 — Les monstres bloqués par les personnages vivants ✅ TERMINÉE

Les monstres reçoivent les cercles des personnages vivants comme
obstacles, en plus de ceux des autres monstres. Même mécanisme qu'à
l'étape 32, sans le modifier.

1. [x] `Escouade::cerclesDesVivants() const` (les `getCercle()` des
       personnages vivants) ; tests (4 cercles à la création, 3 après une
       mort et le cercle du mort absent, liste vide si tous sont morts)
       (enchaînée)
2. [x] `main.cpp` : liste calculée une fois par image, après
       `escouade.mettreAJour` et avant la boucle des monstres ; chaque
       monstre en part et y ajoute les cercles des autres monstres

Seuls `Escouade` (une fonction de lecture) et `main.cpp` changent. Un
personnage mort ne bloque personne ; l'ancre n'est pas un obstacle.

**Limite connue** : sens unique — les personnages traversent encore les
monstres, un chevauchement peut donc exister ; la règle « on ne refuse
que ce qui rapproche » (étape 32) permet au monstre de s'en dégager.

**Hors de portée** : personnages bloqués par les monstres (étape 34),
blocage entre personnages ou de l'ancre, poussée, contournement par le A*,
changement de la règle de choix de cible.

### Étape 34 — Les personnages bloqués par les monstres ✅ TERMINÉE

Les personnages reçoivent les cercles des monstres comme obstacles : le
blocage entre les deux camps devient mutuel. Même mécanisme que les
étapes 32 et 33, une liste d'obstacles descend simplement jusqu'aux
personnages.

1. [x] `Personnage::mettreAJour(dt, cible, carte, obstacles)` : transmet
       la liste à `deplacerAvecGlissement` ; tests existants adaptés,
       nouveau test (un personnage s'arrête avant un obstacle)
       (enchaînée)
2. [x] `Escouade::mettreAJour(dt, carte, obstacles)` : transmet la liste
       à chaque personnage, liste vide pour l'ancre ; nouveau test
       (l'ancre dépasse l'obstacle, aucun personnage n'y entre)
       (enchaînée)
3. [x] `main.cpp` : liste des cercles de tous les monstres, calculée une
       fois par image avant `escouade.mettreAJour`

**Limite connue** : l'ancre traverse les monstres alors que les
personnages sont bloqués, donc l'ancre (et la caméra) peut s'éloigner
des personnages. Un personnage bloqué ne contourne pas (pas de A*
individuel), il glisse ou attend.

**Hors de portée** : blocage de l'ancre, blocage entre personnages,
contournement par les personnages, poussée, resserrement de la
formation.

### Étape 36 — Classe Skill, skill de soin et cible générale ✅ TERMINÉE

Un deuxième skill d'un autre type, un soin lancé par le tireur
arrière-gauche (touche 3). Au passage, tout skill hors de portée fait
marcher son lanceur vers sa cible, puis se déclenche à l'arrivée.

1. [x] `Skill.hpp/.cpp` : `TypeSkill { Degats, Soin }` et `Skill` autour
       d'une `Attaque`, avec `test_Skill.cpp` (enchaînée)
2. [x] `Personnage` possède un `std::optional<Skill>` ; skill de dégâts
       existant créé avec `TypeSkill::Degats` (enchaînée)
3. [x] `Escouade::declencherSkill(place, ...)` ; `main.cpp` relie les
       touches 1 à 4 aux places 1 à 4
4. [x] `Personnage::recevoirSoin`, `getPointsDeVie`, `getPointsDeVieMax`
       (enchaînée)
5. [x] Choix de l'allié le plus blessé dans `Escouade` (enchaînée)
6. [x] Le tireur arrière-gauche reçoit le skill de soin (30 points de vie,
       8 s, 40 px) ; touche 3, refusé hors de portée (provisoire)
7. [x] `Cible.hpp` (`CibleAllie`, `CibleMonstre`, `std::variant`) ; la
       fonction de choix renvoie un `std::optional<Cible>` (enchaînée)
8. [x] `SkillEnAttente` pour le soin : mise en attente, déplacement du
       soigneur, déclenchement à l'arrivée, annulations
9. [x] Le skill de dégâts passe par le même mécanisme : `choisirCible`
       (monstre verrouillé), attente, déplacement du mêlée, dégâts
       ajoutés à la somme d'`attaquer`, annulation par `verrouiller` et
       `deverrouiller`

**Choix** : le skill de dégâts part dans `attaquer` (qui reçoit le cercle
à jour du monstre) et le soin dans `mettreAJour` ; `main.cpp` appelle
désormais `declencherSkill` même sans verrou, avec `std::optional<Cercle>`.

**Limites connues** : un seul skill en attente à la fois ; pas de choix
manuel de la cible ; un lanceur bloqué par un monstre glisse ou attend.

**Hors de portée** : choix manuel de la cible, skills pour les personnages
2 et 4, soin de zone, résurrection, HUD, icônes, effets visuels.

### Étape 37 — Autodéfense : les personnages ripostent sans clic ✅ TERMINÉE

Jusqu'ici les personnages ne frappaient que le monstre verrouillé par un
clic, alors que les monstres frappaient sans clic. Désormais chaque
personnage vivant frappe de lui-même un monstre à sa portée ; le clic
garde son rôle (désigner la cible prioritaire et aller l'engager).

1. [x] Portée des mêlées et du skill de dégâts : 10 → 20 px (un monstre
       s'arrête à 15 px de sa proie, un mêlée attaqué ne pouvait pas
       riposter sans bouger)
2. [x] `Coup.hpp` : `MonstreVu` (identifiant + cercle) et `Coup`
       (identifiant + dégâts) (enchaînée)
3. [x] `Autodefense.hpp/.cpp` : `choisirMonstreAFrapper`, avec
       `test_Autodefense.cpp` (enchaînée)
4. [x] `Escouade::attaquer` reçoit la liste des monstres et renvoie la
       liste des coups ; `main.cpp` construit les `MonstreVu` et applique
       les coups (encore seulement avec un verrou)
5. [x] `main.cpp` demande les coups à chaque image, verrou ou non :
       l'autodéfense est active
6. [x] `CibleMonstre` reçoit l'identifiant du monstre (enchaînée)
7. [x] Le skill de dégâts sans verrou vise le monstre que son lanceur
       frapperait en autodéfense, avec la portée du skill :
       `choisirCible(type, place, monstres)`, `declencherSkill` renvoie
       une liste de `Coup` ; `choisirMonstreAFrapper` reçoit une origine
       et un test de portée au lieu d'un `Personnage`
8. [x] Mise à jour des documents

**Choix** : l'approche de l'escouade reste liée au verrou ; l'autodéfense
ne déplace personne. Sans verrou le skill part toujours dans l'image de
l'appui (cible à portée par construction), sans mise en attente, sans
créer de verrou. `Escouade` ne connaît toujours pas `Monstre`.

**Limites connues** : la règle ne regarde que la portée (pas de « qui
attaque qui ») ; aucun déplacement provoqué par l'autodéfense ; pas
d'indicateur visuel de la cible de chaque personnage ; sans verrou, le
choix entre deux monstres à portée du skill est automatique (le plus
proche).

**Hors de portée** : verrou automatique sur un attaquant, pas vers
l'attaquant, poursuite, recul des tireurs, ligne de vue, effet visuel
des coups, HUD, choix manuel de la cible d'un skill.

### Étape 38 — Places de formation toujours valides ✅ TERMINÉE

Une place de formation calculée sans regarder la carte pouvait tomber
dans un mur ou derrière un coin : le personnage poussait contre le mur
et restait coincé. `Escouade` ramène désormais une telle place vers
l'ancre.

1. [x] `Escouade::dessiner` : un point jaune à la place de formation de
       chaque personnage vivant (repère provisoire)
2. [x] `Chemin` : `replierVersOrigine` (enchaînée)
3. [x] Tests de `replierVersOrigine` (trajet libre, cible dans un mur,
       cible derrière un coin de pilier, origine = cible) (enchaînée)
4. [x] `Escouade::mettreAJour` replie chaque place ; `m_places` et
       `getPlacesDeFormation` ; tests d'`Escouade` (terrain dégagé :
       places inchangées ; ancre collée à un mur : aucune place dans le
       mur)
5. [x] Mise à jour des documents

**Choix** : repli sur le segment ancre-place par dichotomie (6
itérations) plutôt que recherche de la case libre la plus proche ; seules
les places de formation sont corrigées.

**Limites connues** : un personnage resté derrière un coin peut encore
s'y accrocher (pas de chemin individuel) ; deux places peuvent se replier
au même endroit dans un passage étroit.

**Hors de portée** : chemin A* individuel (étape 39), changement du
placement en combat ou de la marche vers un skill, écart minimal entre
places, prise en compte des monstres.

### Étape 39 — Personnages autonomes dans leur chemin ✅ TERMINÉE

Un `Personnage` visait sa cible en ligne droite : resté derrière un mur
pendant que l'escouade passait une porte ou contournait un coin, il
fonçait dans le mur. Il trouve désormais seul son chemin.

1. [x] `Personnage` possède un `Mobile` à la place de `m_centre` ;
       `Mobile::allerToutDroit` (ajout minimal accepté) pour garder la
       ligne droite sans A* (enchaînée)
2. [x] Règle hybride : ligne droite si le trajet direct est libre, sinon
       chemin A* lissé recalculé à chaque image ; tests (terrain dégagé,
       mur avec ouverture, aucun chemin)
3. [x] Recalcul limité : seulement sans chemin, chemin fini, ou cible
       éloignée de plus d'une case de la cible du dernier A* ; tests
       (enchaînée)
4. [x] Retrait des points de places de formation de l'étape 38
       (enchaînée)
5. [x] Mise à jour des documents

**Choix** : composition (un `Mobile` par personnage), pas d'héritage ;
`m_cibleChemin` dans `Personnage` plutôt qu'un accesseur de fin de chemin
dans `Mobile` ; une cible sans chemin A* est mémorisée pour ne pas
relancer un A* voué à l'échec à chaque image.

**Limites connues** : le chemin ignore les monstres (le personnage
glisse ou attend) ; pas de blocage entre personnages ; deux places
peuvent se replier au même endroit dans un passage étroit.

**Hors de portée** : A* tenant compte des entités, vitesses différentes,
ralentir l'ancre quand un personnage est en retard, déplacer des
décisions (cible, skills, autodéfense) dans `Personnage`.

### Étape 40 — Les personnages vont vers leurs agresseurs ✅ TERMINÉE

L'autodéfense ne déplaçait personne : un monstre qui frappait un tireur
laissait les mêlées plantées à leur place. `Escouade` retient maintenant
ses agresseurs et les personnages vont vers eux.

1. [x] Données et enregistrement : `Escouade::Agresseur`, `m_agresseurs`,
       `recevoirDegats(idMonstre, origine, quantite)` ; tests (enchaînée)
2. [x] Oubli : 3 s sans coup, ou absence de `monstresVus` ; cercle mis à
       jour dans `attaquer` ; tests (enchaînée)
3. [x] `choisirAgresseurAViser` (laisse de 100 px depuis la place, plus
       proche, égalité) ; tests (enchaînée)
4. [x] Branchement dans `mettreAJour` (`pointVersAgresseur`, ancre à
       l'arrêt, `m_monstreAPortee`) ; tests
5. [x] Mise à jour des documents

**Choix** : laisse mesurée depuis la place de formation ; règle générique
sans type de personnage ; l'ordre du joueur, le skill en attente et le
verrou passent avant ; `m_monstreAPortee` mémorisé dans `attaquer`.

**Limites connues** : voir `docs/combat.md` (« Agresseurs »).

**Hors de portée** : skill de provocation, déplacement de l'ancre vers
l'agresseur, verrou automatique, poursuite au-delà de la laisse, recul des
tireurs, indicateur visuel, A* tenant compte des entités.

### Étape 41 — Skill de provocation (touche 2) ✅ TERMINÉE

Le mêlée de la place 2 reçoit un skill de zone : les monstres proches
le prennent pour proie pendant quelques secondes.

1. [x] `TypeSkill::Provocation` et le skill de la place 2, sans effet ;
       tests `Skill` et `Personnage` (enchaînée)
2. [x] `Escouade::Provocation`, `m_provocations`, `provoquer`, écoulement
       et retraits ; tests (enchaînée)
3. [x] Règle unique `indiceProieDuMonstre` ; `personnageLePlusProche`
       renommée `proieDuMonstre(idMonstre, position)` ; branchée sur
       `main.cpp` et `recevoirDegats` ; tests (enchaînée)
4. [x] Déclenchement par la touche 2 dans `declencherSkill` ; tests
5. [x] Mise à jour des documents

**Choix** : état « provoqué » dans `Escouade` (`Monstre` inchangé) ;
skill de zone sans cible (pas de cas dans `Cible`) ; puissance = durée en
secondes ; `provoquer` publique pour les tests.

**Limites connues** : voir `docs/combat.md` (« Provocation »).

**Hors de portée** : indicateur visuel du monstre provoqué, autres effets
(étourdissement, ralentissement), système d'effets général, skill de la
touche 4.

### Étape 42 — Rayon de zone des skills et skill de zone ciblé (touche 4) ✅ TERMINÉE

Séparation de la portée (dans `Attaque`) et du rayon de zone (dans
`Skill`), puis premier skill en deux temps : 4 entre en mode visée, un
clic gauche lance le sort sur un cercle de dégâts.

1. [x] `Skill` : `rayonZone` (constructeur, getter) ; tests (enchaînée)
2. [x] Provocation : portée 0, zone 150 centrée sur le provocateur ;
       `Personnage::zoneDuSkill` ; tests
3. [x] `TypeSkill::Zone` et skill de la place 4 (enchaînée)
4. [x] `Escouade::skillEstPret(place)` ; tests (enchaînée)
5. [x] `Escouade::declencherSkillDeZone(place, point, monstres)` ; tests
       (enchaînée)
6. [x] `main.cpp` : mode visée (`placeEnVisee`), entrées, lancer sur
       événement d'appui
7. [x] Cercle de prévisualisation (`rayonZoneDuSkill`)
8. [x] Mise à jour des documents

**Choix** : rayon de zone dans `Skill` (0 et non lu pour les skills à cible
unique, pas de `std::optional`) ; zone = `Cercle` touché si bord à bord 0 ;
le sort part même à vide ; mode visée = état d'interface dans `main.cpp` ;
pas de booléen « attendre le relâchement » : `deplacementContinu` remis à
`false` à l'entrée en visée suffit.

**Limites connues** : voir `docs/combat.md` (« Skill de zone ciblé »).

**Hors de portée** : lecture de la portée, marche du lanceur vers le
point, `CiblePoint`, effet visuel à l'impact, HUD, dégâts aux alliés ou
dans le temps.

### Étape 43 — Portée de lancement du skill de zone, provocation toujours consommée ✅ TERMINÉE

1. [x] Provocation toujours consommée, même sans monstre dans la zone ;
       tests
2. [x] `CiblePoint` ajouté au variant `Cible`, cas traité partout où
       `Cible` est lue (enchaînée)
3. [x] `declencherSkillDeZone` lit la portée : départ immédiat ou mise en
       attente avec `CiblePoint` ; tests (enchaînée)
4. [x] Sort en attente : le lanceur marche, le sort part dans `attaquer`
       au point fixé ; annulation à la mort du lanceur ; tests (enchaînée)
5. [x] `main.cpp` : cercle de portée autour du lanceur pendant la visée
       (`cercleDePorteeDuSkill`, `getPortee`)
6. [x] Mise à jour des documents

**Choix** : `Cible` lue par `get_if` (relecture manuelle à chaque nouveau
cas) ; point fixé au clic mais zone évaluée au départ du sort ; le sort de
zone n'est pas lié au combat verrouillé (seule la mort du lanceur
l'annule) ; fonction privée `lancerSkillDeZone` partagée par le départ
immédiat et le départ après attente.

**Limites connues** : voir `docs/combat.md` (« Portée de lancement du
skill de zone »).

**Hors de portée** : ligne de vue, activation d'un monstre qui reçoit des
dégâts, plusieurs skills en attente, annulation manuelle, couleur de la
prévisualisation selon la portée, effet visuel à l'impact.

### Étape 44 — Ligne de vue pour le skill de zone ✅ TERMINÉE

1. [x] `TileMap::ligneDeVue(a, b)` (carré de 2 × 2 px balayé par
       `trajetLibre`) ; tests (enchaînée)
2. [x] `Escouade::peutLancerAuPoint` (portée ET vue) au clic et pendant
       l'attente ; refus d'un point dans un mur ; tests (enchaînée)
3. [x] L'explosion ne touche que les monstres en vue du point d'impact ;
       tests
4. [x] Mise à jour des documents

**Choix** : ligne de vue = question posée à la carte, implémentée par
`trajetLibre` pour ne pas écrire de nouveau parcours de grille ; une seule
condition partagée entre le clic et l'attente ; explosion testée centre à
centre ; la carte passée en paramètre, jamais gardée.

**Limites connues** : voir `docs/combat.md` (« Ligne de vue du skill de
zone »).

**Hors de portée** : ligne de vue pour les attaques de base, le skill de
dégâts, le soin et la provocation (étape 45), activation d'un monstre qui
reçoit des dégâts, détection des monstres, indication visuelle dans la
prévisualisation, DDA.

### Étape 45 — Fusion d'Attaque dans Skill ✅ TERMINÉE

1. [x] `Skill` porte directement portée, puissance, recharge (plus
       d'`Attaque` imbriquée) ; `Escouade` et tests adaptés (enchaînée)
2. [x] `Monstre` : son attaque est un `Skill` `Degats` (enchaînée)
3. [x] `Personnage` : `m_attaque` devient `Skill m_attaqueDeBase`
       (enchaînée)
4. [x] `Attaque.hpp`, `Attaque.cpp` et `test_Attaque.cpp` supprimés ; cas
       génériques migrés dans `test_Skill.cpp`
5. [x] Mise à jour des documents

**Choix** : refactorisation pure, aucun changement de comportement ni de
valeur ; le rôle « attaque de base » vient du membre qui range le `Skill`,
pas d'un marqueur dans le type ; composition, pas d'héritage ; `Skill`
reste ignorant de la carte ; la règle « un skill relance la recharge de
l'attaque de base » reste dans `Personnage::declencherSkill`.

**Hors de portée** : ligne de vue pour les attaques de base et les autres
skills (étape 46), activation des monstres touchés (étape 47), skills pour
les monstres, faire passer l'attaque de base par le circuit des skills.

### Étape 46 — Ligne de vue pour les attaques de base et les autres skills ✅ TERMINÉE

1. [x] `Escouade::peutAtteindre` (portée ET vue) ; `peutLancerAuPoint`
       réécrite avec elle (enchaînée)
2. [x] Autodéfense : lambda `peutAtteindre` ; tests (enchaînée)
3. [x] Combat verrouillé et agresseurs ; tests
4. [x] Skill de dégâts (avec et sans verrou) et soin, carte passée à
       `declencherSkill` et `choisirCible` ; tests (enchaînée)
5. [x] Provocation limitée aux monstres en vue ; tests
6. [x] Mise à jour des documents

**Choix** : `Skill` reste ignorant de la carte, la règle vit à un seul
endroit dans `Escouade` ; une `enum class Arme` privée choisit la portée
(attaque de base ou skill) ; vue centre à centre ; à portée sans vue =
même traitement qu'hors de portée ; la carte est un paramètre, jamais un
membre.

**Hors de portée** : ligne de vue pour la riposte et la détection des
monstres, placement pour obtenir la vue, indicateur d'une cible non
visible, vue bord à bord, activation des monstres touchés (étape 47).

### Étape 47 — Un monstre touché s'active ✅ TERMINÉE

1. [x] `Monstre::m_vientDEtreTouche`, posé par `recevoirDegats` si
       dégâts `> 0` (enchaînée)
2. [x] Transition par coup reçu dans `mettreAJour`, consommation de
       l'indicateur ; tests

**Choix** : tout dans `Monstre` ; proie habituelle, pas l'auteur du coup ;
règle du territoire pour éviter la boucle `Retour`/`Poursuite` ; indicateur
consommé à chaque appel ; une seule transition par appel.

**Hors de portée** : se jeter sur l'auteur du coup, aggro sociale, ignorer
le territoire après un coup, ligne de vue pour la détection ou la riposte,
effet visuel d'alerte.

### Étape 48 — Barre de skills à l'écran (HUD) ✅ TERMINÉE

1. [x] Police DejaVu Sans dans `assets/fonts/`, copie par CMake, chargement
       dans `main.cpp` avec gestion d'échec, vue fixe
2. [x] `EtatSkill`, `Skill::getRechargeRestante`, `Personnage::etatDuSkill`,
       `Escouade::etatDuSkill` ; tests (enchaînée)
3. [x] Fonctions pures de `BarreDeSkills` (lettre, secondes, quartier) ;
       tests (enchaînée)
4. [x] Icônes statiques (fond, contour, lettre, numéro)
5. [x] Quartier sombre pendant la recharge
6. [x] Chiffre du temps restant
7. [x] Lanceur mort (grisé) et visée active (contour jaune)
8. [x] Mise à jour des documents

**Choix** : fonctions libres sans classe `Hud` (aucun état propre) ;
`EtatSkill` comme structure sans comportement pour que `main.cpp` ignore
`Personnage` ; présentation séparée de `Skill` ; quartier calculé sur le
bord du carré ; vue par défaut pour l'interface, avec retour à `camera` en
début d'image (bug de conversion des clics trouvé en chemin).

**Hors de portée** : indication du skill en attente, infobulle, clic sur
les icônes, images d'icônes, autre texte à l'écran, redimensionnement de la
fenêtre.

### Étape 49 — Hygiène et sûreté du code (refactorisation pure) ✅ TERMINÉE

1. [x] `static_cast<float>` dans `TileMap.cpp` ; `CMakeLists.txt` à une
       source par ligne et avertissements activés (enchaînée)
2. [x] Inclusions précises de `TileMap.hpp` ; `TypeSkill.hpp` et
       `MonstreVu.hpp` ; includes inutiles retirés (enchaînée)
3. [x] `Cercle` par valeur, constantes `s_`, `Personnage::m_taille`
       supprimé, attaque de base construite à un endroit, commentaires
       périmés, règles dans CLAUDE.md (enchaînée)
4. [x] `placeValide`, `personnageDeLaPlace`, `skillEnAttenteDeDegats`,
       `annulerSkillDeDegatsEnAttente` (enchaînée)
5. [x] `trouverMonstre` et ses tests ; `trouverMonstreParId` dans
       `main.cpp`, itérateurs et pointeurs testés avant usage (enchaînée)
6. [x] `Surcharges` et `std::visit` sur `Cible` (enchaînée)
7. [x] `[[nodiscard]]` et adaptation de 39 appels de tests (enchaînée)
8. [x] `std::array<bool, s_nombrePersonnages>` pour `m_monstreAPortee` ;
       ordre d'appel `attaquer` avant `mettreAJour` documenté (enchaînée)
9. [x] Module `BarreDeVie` utilisé par `Personnage` et `Monstre`
10. [x] Mise à jour des documents

**Choix** : constantes en `constexpr` dans un namespace anonyme (style
majoritaire, `static` redondant) ; `provoquer` ignore une place invalide
(avant : comportement indéfini plus tard) ; `attaquer` garde ses `get_if`
(une seule alternative à la fois) ; `avancerSkillEnAttente` visite une copie
de la cible car une lambda peut vider le skill en attente ; barre de vie en
deux fonctions (`positionBarreDeVie`, `dessinerBarreDeVie`) pour que la
position ne soit pas calculée deux fois ; pas de type `PointsDeVie`.

**Hors de portée** : découpage de `test_Escouade.cpp` et `aides.hpp`
(étape 50), extraction de `Provocations` et `Agresseurs` (étape 51),
allègement de `main.cpp` (étape 52), `declencherSansCible` (R4),
sous-dossiers (F1).

### Étape 50 — Réorganisation des tests (refactorisation pure) ✅ TERMINÉE

1. [x] `tests/aides.hpp` (aides `inline` identiques) ; `namespace {}` pour
       les aides locales (enchaînée). 330 tests.
2. [x] `test_Escouade.cpp` (139 cas) coupé en cinq fichiers par thème :
       `deplacement` (18), `combat` (17), `skills` (61), `agresseurs` (25),
       `vue` (18) ; `CMakeLists.txt` mis à jour (enchaînée). 330 tests.
3. [x] Revue des `static_cast<void>` : 30 remplacés par une vérification
       (`REQUIRE(...empty())`, un `Coup`, `== 25`), 9 gardés (`attaquer`
       appelé pour avancer l'état, avec commentaire) (enchaînée). 330 tests.
4. [x] Test de `provoquer` à une place invalide (0 et 5) (enchaînée).
       331 tests.
5. [x] Quatre tests de rotation de la formation, l'orientation étant
       déduite de `getPlacesDeFormation()` (enchaînée). 335 tests.
6. [x] Noms de `TEST_CASE` sans accents : 151 noms modifiés (enchaînée).
       335 tests, aucun nom accentué dans `--list-tests`.
7. [x] Mise à jour des documents

**Choix** : une aide n'est partagée que si les copies sont identiques ;
`carteDegagee` devient `carteDegagee10x10` (`Mobile`, `Monstre`, `Escouade`),
`carteDegagee5x5` (`Personnage`, locale) et `carteDegageeEntouree`
(`Escouade`) ; les trois `ecoulerTemps` (signatures différentes, un fichier
chacun) et les aides propres à un fichier restent locales. Pas d'accesseur
d'orientation : elle se déduit de la première place de formation. Aucun
fichier de `src/` modifié, aucun bogue révélé.

**Hors de portée** : extraction de `Provocations` et `Agresseurs` (étape 51),
allègement de `main.cpp` (étape 52), tests de `replierVersOrigine` en direct,
`SECTION` de Catch2, sous-dossiers de `tests/`. `test_Escouade_skills.cpp`
fait 786 lignes (un peu plus que les 700 visées).

### Étape 51 — Alléger Escouade : Provocations, Agresseurs, découpage (refactorisation pure) ✅ TERMINÉE

1. [x] `.gitattributes` (`* text=auto`, `*.ttf` et `*.png` en binaire) ;
       l'index étant déjà en LF, la normalisation n'a rien changé d'autre
       (enchaînée). 335 tests.
2. [x] Aides de simulation par nombre de tics (`s_dureeTic`,
       `nombreDeTics`, `std::lround`) ; `simulerUnDixiemeDeSeconde` remplacée
       par `simuler(escouade, carte, 0.1f)` (enchaînée). 335 tests.
3. [x] Classe `Provocations` et ses 9 tests ; plus de `place - 1` dans
       `indiceProieDuMonstre` (`indiceDeLaPlace`) (enchaînée). 344 tests.
4. [x] Classe `Agresseurs` et ses 8 tests ; `Agresseur` contient un
       `MonstreVu` ; liste de `MonstreVu` construite une fois par image (D5)
       (enchaînée). 352 tests.
5. [x] `attaquer` découpée en fonctions privées nommées (enchaînée).
6. [x] `pointVoulu` et découpage de `mettreAJour` (enchaînée). 352 tests.
7. [x] Mise à jour des documents

**Mesures** : `Escouade.cpp` 674 → 697 lignes (commentaires et signatures des
nouvelles fonctions) ; `attaquer` 88 → 14 lignes ; `mettreAJour` 68 → 19.

**Choix** : l'ordre de priorité de `pointVoulu` est celui du code d'avant
(skill en attente, rester sur place à portée du verrou, agresseur, place) ;
la règle « coup à 0 = rien » reste dans `Escouade::recevoirDegats` ; les
accès des tests d'agresseurs sont adaptés (`.monstre.id`, `.monstre.cercle`),
pas ce qu'ils vérifient.

**Hors de portée** : extraction de la formation (R2c), `main.cpp` (étape 52).
