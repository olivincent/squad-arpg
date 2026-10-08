# Squad-ARPG — Monstre

`Monstre` : un ennemi qui détecte, poursuit et retourne à son poste. Son
attaque, sa portée et les obstacles sont dans `docs/combat.md` ; son
déplacement par chemin dans `docs/chemin.md`.

## Rôle et indépendance

- **Pas de classe de base commune avec `Personnage`, pas d'héritage** :
  les deux classes restent indépendantes, même si elles se déplacent
  toutes deux par un `Mobile` et portent chacune une `Attaque`. La
  ressemblance n'implique pas de dépendance commune tant qu'aucun
  comportement partagé ne le justifie. Les points de vie et la barre de
  vie sont dupliqués volontairement.
- **`Monstre` ne connaît ni `Escouade` ni `Personnage`** : il reçoit sa
  proie (un `std::optional<Cercle>`, vide quand aucun personnage n'est
  vivant) et les obstacles en paramètres de `mettreAJour`, et frappe un
  simple `Cercle`. C'est `main.cpp` qui fait l'intermédiaire (voir
  `docs/boucle-et-entrees.md` et `docs/combat.md`).
- **Position et taille portées par `m_mobile`** (30 × 30, 120 px/s — plus
  lent que les personnages à 240 px/s, pour qu'un groupe reste
  rattrapable) : une seule copie de ces données. `getCentre`, `contient`,
  `dessiner` et `getCercle` les lisent via `m_mobile`.
- **Possédé par `main.cpp`**, pas par `Escouade` : un
  `std::vector<Monstre>` (six monstres répartis sur la carte), sans
  classe dédiée (`Horde`, `Monstres`…) pour le porter. Un monstre mort
  est retiré du vecteur (`std::erase_if`, à chaque image) : l'objet
  cesse d'exister.

## Identifiant, vie et dessin

- **Identifiant** : le constructeur prend un centre et un identifiant ;
  `explicit` n'a donc plus de sens. C'est `main.cpp` qui attribue les
  identifiants (variable locale `prochainId`), pas un compteur `static`
  dans `Monstre`. L'identifiant sert à désigner la cible verrouillée,
  jamais un indice ni un pointeur.
- **`contient`** réutilise `sf::FloatRect::contains` plutôt que de
  réimplémenter sa convention (bords gauche/haut inclus, droit/bas
  exclus).
- **`recevoirDegats` ne descend jamais sous zéro** : un coup qui dépasse
  le reste ne laisse pas un nombre négatif stocké.
- **Pas d'accesseur pour les points de vie** : les tests passent par
  `estVivant()` et la barre de vie est dessinée par la classe elle-même.
- **Barre de vie** : un rectangle de fond sombre (largeur du monstre,
  quelques pixels au-dessus de lui), recouvert d'un rectangle vert
  proportionnel aux points de vie restants.
- **Couleur** : toujours rouge, donc une valeur fixe dans `dessiner`, pas
  un membre.
- **Contour jaune sur la cible verrouillée** : `dessiner` reçoit un
  paramètre `bool estCible`, calculé par `main.cpp` par comparaison
  directe de `escouade.getCibleVerrouillee()` avec `getId()` (un
  `std::optional<int>` comparé à un `int` gère déjà le cas vide).
  `Monstre` ne stocke pas le fait d'être visé : le verrou d'`Escouade`
  reste l'unique source de vérité. Contour de 3 px vers l'intérieur
  (`setOutlineThickness(-3.0f)`), pour ne pas toucher la barre de vie ni
  changer l'empreinte du monstre.

## États et transitions

- **`enum class Etat { EnGarde, Poursuite, Retour }`, privé à
  `Monstre.hpp`**, plutôt que des booléens : un type à trois valeurs rend
  impossible l'état incohérent « en poursuite et en retour à la fois »
  que deux `bool` indépendants auraient permis. `Etat` n'existe que pour
  `Monstre`.
- **« À portée, en train de frapper » n'est pas un statut** : c'est
  recalculé à chaque image à partir de la distance
  (`m_attaque.estAPortee`). Un quatrième statut aurait dupliqué une
  information déjà disponible, au risque de la désynchroniser.
- **`m_poste`** est le centre donné au constructeur, jamais modifié :
  l'ancre fixe à laquelle le monstre revient.
- **Transitions, une seule par appel de `mettreAJour`**, dans un
  `switch` sur `m_etat` :
  - `EnGarde` → `Poursuite` si `proieDetectee` ;
  - `Poursuite` → `Retour` si plus de proie, ou si le monstre lui-même
    est à `>` `m_rayonTerritoire` de son poste ;
  - `Retour` → `Poursuite` si `proieDetectee` (vérifié avant l'arrivée,
    pour qu'une proie qui redevient pertinente au moment même où le
    monstre atteint son poste reprenne le dessus) ;
  - `Retour` → `EnGarde` sinon, une fois arrivé
    (`m_mobile.directionVoulue()` vide).

## Détection et territoire

- **Le territoire se mesure sur le monstre, pas sur la proie** :
  `m_rayonTerritoire` (400 px) est la distance maximale entre le
  **monstre** et son **poste**. Un monstre peut donc poursuivre une proie
  arbitrairement loin de son point de départ, tant que lui-même ne
  s'éloigne pas de plus de 400 px de son poste.
- **Détection unique, dans la fonction privée `const`
  `proieDetectee(cible)`** : une proie est détectée si elle existe, si
  elle est à `<=` `m_rayonDetection` (200 px) **du monstre** ET à `<=`
  `m_rayonTerritoire` **du poste**. La seconde condition est nécessaire :
  sans elle, un monstre qui vient de rentrer au bord de son territoire
  redétecterait aussitôt une proie restée à la frontière et tremblerait
  entre `Poursuite` et `Retour` au lieu de rentrer franchement. La même
  fonction est appelée en `EnGarde` et en `Retour`, pour que les deux
  appliquent exactement la même règle.
- **Les distances sont mesurées à vol d'oiseau** et ignorent les murs :
  un monstre peut détecter et poursuivre une proie à travers un mur,
  sans ligne de vue. Détection et territoire : centre à centre
  (`proieDetectee` ne lit que `cible->centre`) ; portée d'attaque : bord
  à bord.

## Ordres donnés à `Mobile`

- **À l'entrée en `Retour` seulement**, `arreter()` puis
  `allerVers(m_poste, carte)`. L'entrée est détectée en comparant l'état
  avant et après le `switch` (`etatPrecedent != m_etat`) : redonner
  l'ordre à chaque image écraserait inutilement le chemin déjà calculé
  et refairait un A* identique à chaque tic.
- **En `Poursuite`, à chaque image** : `arreter()` si la proie est à
  portée de `m_attaque`, sinon `allerVers(proie.centre, carte)`. Ici
  l'ordre est redonné à chaque image parce que la proie, un personnage,
  bouge ; le poste, lui, est fixe. Le but du monstre est de se mettre à
  portée pour frapper, pas de rejoindre sa proie.
- **En `EnGarde` et pendant le reste du `Retour`**, aucun ordre : le
  chemin donné à l'entrée suffit à `Mobile`.
- **`m_mobile.mettreAJour(dt, carte, obstacles)`** est appelé à chaque
  image, quel que soit l'état ; `obstacles` est transmis tel quel.

## Riposte

- **`frapper(cible)` délègue entièrement à `m_attaque.frapper`**, avec
  `getCercle()` comme origine, sans règle supplémentaire (contrairement à
  `Personnage::frapper`, qui vérifie d'abord `estVivant()`).
- **La riposte ne dépend pas du statut** : un monstre en retour frappe
  aussi ce qui passe à sa portée. Elle ne dépend pas non plus du verrou
  de cible.
- **`mettreAJour` décrémente d'abord la recharge** (`m_attaque.mettreAJour`),
  quel que soit l'état.

## Limites connues

- Un monstre qui rentre ignore une proie dès qu'elle sort du territoire,
  même si elle reste proche de lui (le filtre `proieDetectee` l'exige par
  construction).
- Pas de patrouille ni de récupération de points de vie pendant le
  retour.
- Chaque monstre garde son propre poste et territoire, sans les partager
  ni se coordonner avec les autres.
- Détection et poursuite ignorent les murs (pas de ligne de vue) ; un
  monstre bloqué ne contourne pas (le A* ignore les entités).
