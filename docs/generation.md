# Génération procédurale

Module `GenerateurDonjon` (étape 59). Logique pure sur une grille, sans SFML.

## Technique : placement au hasard avec rejet

`placerSalles` fait `tentatives` essais (30). À chacun, il tire une taille puis
une position, et jette le rectangle s'il chevauche (`Salle::chevauche`, marge
comprise) une salle déjà gardée. La première tentative réussit toujours (rien
à chevaucher) : il y a toujours au moins une salle. Le nombre de salles n'est
pas fixé : les rejets le font varier (environ 8 à 12 sur 59 × 40).

Les tirages utilisent `std::uniform_int_distribution`, dont les bornes sont
**inclusives**. Une salle occupe les colonnes `x` à `x + largeur - 1` ; la
dernière colonne de la carte doit rester du mur, d'où
`x ∈ [1, largeurCarte - 1 - largeur]` (idem en hauteur). La rangée et la
colonne de bord sont donc toujours des murs.

## Emprise et forme creusée

Deux notions séparées dès le départ :
- l'**emprise** (`Salle`) sert au placement, au test de chevauchement et au
  centre ;
- la **forme creusée** est l'ensemble des cases qui deviennent du sol dans
  cette emprise ; elle ne vit que dans `creuserSalle` (aujourd'hui, tout le
  rectangle).

Pourquoi : le placement ne dépend jamais de la forme. Des salles rondes, en L
ou des grottes ne changeront que `creuserSalle`, tant qu'elles restent dans
leur emprise.

## Couloirs (étape 60)

**Qui relier à qui** : `choisirLiaisons`, « plus proche déjà relié ». Les
salles sont parcourues dans l'ordre de placement ; la salle 0 est la racine et
chaque salle i ≥ 1 est reliée à la plus proche parmi les salles 0 à i−1
(distance euclidienne au carré entre centres, sans racine carrée ; égalité :
le plus petit indice). C'est une version simplifiée de l'algorithme de Prim :
un arbre couvrant avec exactement n−1 liaisons, donc tout est relié par
construction (pas de boucle).

**Choix séparé du creusage** : `choisirLiaisons` est pure et sans aléatoire,
donc testable avec des salles écrites à la main. Le hasard n'intervient que
dans `genererNiveau`, qui tire le sens de chaque L (un tirage par liaison,
après le placement des salles : la même graine redonne le même donjon).

**Forme** : `creuserCouloir` creuse deux bandes de 2 cases qui se croisent au
coude, donc le coude est plein (pas d'encoche). La largeur s'étend vers le bas
et vers la droite à partir de la ligne (ou colonne) du centre. Les centres
étant à `x + largeur / 2` avec des côtés d'au moins 6, la case supplémentaire
reste dans la salle et la bordure reste du mur.

**Ordre** : les couloirs se creusent après les salles. Un couloir qui traverse
une autre salle ou longe un mur est accepté tel quel. Il vise le centre de
l'emprise, ce qui suppose des salles rectangulaires pleines : d'autres formes
pourront demander d'ajuster ce point.

**Connexité** : garantie par construction ET vérifiée par les tests : pour
5 graines, le A* (`trouverChemin`) trouve un chemin du départ au centre de
chaque salle.

## Monstres (étape 61)

**Qui** : `placerMonstres`, fonction pure séparée (reçoit le `Generateur`, les
salles, le départ et les paramètres ; renvoie des positions en pixels), donc
testable avec des salles écrites à la main. `genererNiveau` l'appelle en
dernier, après les salles et les couloirs : ses tirages ne changent ni les
salles ni les couloirs d'une graine donnée.

**Combien et où** : aucun dans la salle 0 (le départ). Dans chaque autre
salle, un nombre voulu tiré dans [1, 3] (bornes incluses), puis pour chaque
monstre une case tirée à l'intérieur de l'emprise avec une case de marge
contre les murs ; le monstre est au centre de la case.

**Rejet** : comme pour les salles, au plus 10 tentatives par monstre. Une
position est rejetée si elle est à moins de 2 cases d'un monstre déjà placé
(Tchebychev : au moins une case libre entre deux monstres, diagonale
comprise) ou à moins de 400 px du départ. Si les 10 tentatives échouent, le
monstre est abandonné : une salle peut avoir moins de monstres que voulu,
ou aucun. Les monstres abandonnés ne sont pas retirés du nombre tiré : le
tirage du nombre voulu a toujours lieu, ce qui garde la suite des tirages
simple.

**Pourquoi 400 px** : la détection des monstres ignore les murs (200 px de
rayon de détection autour du monstre, 600 px autour de son poste depuis l'étape 69, 400 avant ; la distance minimale n'a pas été changée). Sans cette
distance minimale, un monstre d'une salle voisine détecterait l'escouade dès
le départ. La règle ne fait pas disparaître le cas : en explorant, un monstre
peut encore détecter l'escouade à travers un mur et faire le tour par le
couloir (limite connue, voir `ROADMAP.md`).

**Groupes** (étape 64) : `placerMonstres` renvoie des `MonstreDuNiveau`
(position, groupe). Le groupe vient d'un compteur qui augmente de 1 pour chaque
salle qui reçoit au moins un monstre ; tous les monstres d'une salle le
partagent. On n'utilise PAS l'indice de la salle : l'identifiant doit rester
opaque (« ces monstres vont ensemble »), et un lecteur croirait sinon que
groupe = salle ; une salle restée vide ne laisse pas de trou dans la
numérotation. Le compteur ne tire rien : les graines donnent les mêmes
positions qu'avant.

**Armure** : `s_armureMonstresParDefaut` (20, `Niveau.hpp`) est partagée par
`creerNiveau()` et `genererNiveau()`. Elle n'est pas dans `ParametresDonjon`.

## Portes (étape 70)

`placerPortes` est une fonction pure et sans aléatoire, appelée par `genererNiveau`
après le creusage des couloirs et avant `placerMonstres` : une graine redonne les
mêmes salles, couloirs et monstres qu'avant l'étape. On cherche les ouvertures par
l'extérieur de chaque salle (l'anneau de cases juste hors de l'emprise), pas en
suivant les couloirs : une porte est une suite maximale de sol de **exactement**
`largeurCouloir` cases, encadrée de murs (coins compris), avec du sol juste au-delà.
Un couloir qui traverse une salle reçoit ainsi une porte à l'entrée et une à la
sortie ; deux salles séparées d'un seul mur donnent la même porte, supprimée en
doublon. Compromis : un couloir qui longe une salle, des couloirs qui se touchent ou
toute ouverture d'une autre largeur n'ont pas de porte. La salle de départ a ses
portes fermées. Les portes sont des cases de sol du plan : ouvertes, la carte est
exactement celle d'avant (reliée, testé sur 5 graines).

## Paramètres

`ParametresDonjon` : carte 59 × 40 cases de 40 px, 30 tentatives, côtés de 6 à
12 cases, marge d'une case de mur entre deux salles, couloirs de 2 cases,
1 à 3 monstres par salle, 400 px de distance minimale au départ, 10 tentatives
par monstre. `tailleCase` y figure parce que `Niveau` la contient.

## Niveau produit

Le plan utilise les mêmes caractères que `creerNiveau()` (`#` mur, `.` sol).
Le départ de l'escouade est le centre (en pixels) de la case centrale de la
première salle placée, donc toujours sur du sol. Les portes sont celles de
`placerPortes`. Les monstres sont ceux de `placerMonstres`, avec l'armure par défaut.

## Graine unique et nouvelle carte

Une seule fonction de `main.cpp`, `nouvellePartie`, tire une graine avec
`std::random_device`, l'affiche dans la console, génère le niveau avec un
`Generateur` initialisé par elle, puis construit la `Partie` avec la même
graine : la même graine redonne la même carte et le même combat. Le
lancement, R et F5 passent tous par elle. R (seulement après une défaite) et
F5 (touche de débogage, à tout moment, y compris en visée) l'appellent par
une lambda qui réaffecte aussi le `Controleur`. Le `Niveau` n'est plus gardé
dans `main` : chaque carte est neuve, avec une nouvelle graine.

## Pas de coin diagonal (étape 73)

- **Règle** : le plan généré ne doit pas contenir de *coin diagonal*, un carré de 2 × 2 cases
  dont deux cases en diagonale sont du sol et les deux autres des murs (un damier). On ne peut
  pas y passer, mais la vision y voit à travers (`docs/vision.md`) : mieux vaut l'interdire dans
  la carte que de toucher à l'algorithme de vision.
- **`contientUnCoinDiagonal(plan)`** : fonction libre pure de `GenerateurDonjon`, sans état ; teste
  tous les 2 × 2 du plan, bords compris, dans les deux orientations (`#` mur, tout autre
  caractère sol, comme `TileMap`). Elle ne modifie rien ; `genererNiveau` ne l'appelle pas.
- **État** : le test de propriété (graines 1 à 50) passe, donc il n'y a **pas de passe de
  nettoyage** dans `genererNiveau`. Mais le motif existe : 145 graines sur 20 000 (0,7 %, première
  graine 391), une fois par graine, là où deux masses de mur se touchent par un coin. La règle est
  donc vérifiée sur un échantillon, pas garantie. Voir `docs/vision.md` pour la passe de nettoyage
  envisagée.

## Limites connues

- Des coins diagonaux (damier de 2 × 2 cases) apparaissent sur environ 0,7 % des graines (voir
  « Pas de coin diagonal ») : on y voit à travers sans pouvoir y passer.

- Pas de boucles (arbre seulement) ; portes seulement sur les ouvertures de la
  largeur du couloir ; un couloir peut traverser
  une salle ou longer un mur. Aucun problème de déplacement observé dans les
  couloirs de 2 cases.
- Nombre de monstres indépendant de la superficie de la salle ; un seul type
  de monstre ; pas de patrouille ni d'escouade de monstres.
- La détection des monstres ignore les murs : un monstre peut détecter
  l'escouade à travers un mur (observé).
- Pas de sortie du donjon ni d'objectif de niveau.
- Si la carte est trop petite pour `coteMax`, c'est une `assert`, pas une
  erreur gérée.
- `creerNiveau()` reste le terrain fixe des tests.
