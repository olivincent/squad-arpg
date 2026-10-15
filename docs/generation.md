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

## Paramètres

`ParametresDonjon` : carte 59 × 40 cases de 40 px, 30 tentatives, côtés de 6 à
12 cases, marge d'une case de mur entre deux salles. `tailleCase` y figure
parce que `Niveau` la contient.

## Niveau produit

Le plan utilise les mêmes caractères que `creerNiveau()` (`#` mur, `.` sol).
Le départ de l'escouade est le centre (en pixels) de la case centrale de la
première salle placée, donc toujours sur du sol. Aucun monstre. Sans couloir
(étape 60), l'escouade est enfermée dans sa salle : c'est voulu.

## Graine unique

`main.cpp` tire une graine, génère le niveau avec un `Generateur` initialisé
par elle, puis construit la `Partie` avec la même graine : la même graine
redonne la même carte et le même combat. La touche R reconstruit la partie
avec le même niveau et une nouvelle graine de combat ; son comportement
définitif (nouvelle carte) viendra à l'étape 61.

## Limites connues

- Pas de couloirs, pas de monstres, escouade enfermée (étapes 60 et 61).
- Si la carte est trop petite pour `coteMax`, c'est une `assert`, pas une
  erreur gérée.
- `creerNiveau()` reste le terrain fixe des tests.
