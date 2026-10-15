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

## Paramètres

`ParametresDonjon` : carte 59 × 40 cases de 40 px, 30 tentatives, côtés de 6 à
12 cases, marge d'une case de mur entre deux salles, couloirs de 2 cases.
`tailleCase` y figure parce que `Niveau` la contient.

## Niveau produit

Le plan utilise les mêmes caractères que `creerNiveau()` (`#` mur, `.` sol).
Le départ de l'escouade est le centre (en pixels) de la case centrale de la
première salle placée, donc toujours sur du sol. Aucun monstre.

## Graine unique

`main.cpp` tire une graine, génère le niveau avec un `Generateur` initialisé
par elle, puis construit la `Partie` avec la même graine : la même graine
redonne la même carte et le même combat. La touche R reconstruit la partie
avec le même niveau et une nouvelle graine de combat ; son comportement
définitif (nouvelle carte) viendra à l'étape 61.

## Limites connues

- Pas de monstres (étape 61).
- Pas de boucles (arbre seulement), pas de portes ; un couloir peut traverser
  une salle ou longer un mur.
- Les problèmes de déplacement dans les couloirs de 2 cases sont à noter
  après les essais visuels (voir `ROADMAP.md`) ; non corrigés à l'étape 60.
- Si la carte est trop petite pour `coteMax`, c'est une `assert`, pas une
  erreur gérée.
- `creerNiveau()` reste le terrain fixe des tests.
