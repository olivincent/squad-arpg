# Squad-ARPG — Monstre

`Monstre` : un ennemi qui détecte, poursuit et retourne à son poste. Son
attaque, sa portée et les obstacles sont dans `docs/combat.md` ; son
déplacement par chemin dans `docs/chemin.md`.

## Rôle et indépendance

- **Pas de classe de base commune avec `Personnage`, pas d'héritage** :
  les deux classes restent indépendantes, même si elles se déplacent
  toutes deux par un `Mobile` et portent chacune un `Skill` (attaque de base). La
  ressemblance n'implique pas de dépendance commune tant qu'aucun
  comportement partagé ne le justifie. Les points de vie et la barre de
  vie sont dupliqués volontairement.
- **`Monstre` ne connaît ni `Escouade` ni `Personnage`** : il reçoit sa
  proie (un `std::optional<EntiteVue>`, vide quand aucun personnage n'est
  vivant), ses candidats de frappe (`std::vector<EntiteVue>`) et les
  obstacles en paramètres de `mettreAJour`. C'est `main.cpp` qui fait l'intermédiaire (voir
  `docs/boucle-et-entrees.md` et `docs/combat.md`).
- **Position et taille portées par `m_mobile`** (30 × 30, 160 px/s depuis
  l'étape 69 — plus lent que l'ancre à 180 px/s et que les personnages à
  240 px/s, pour que la fuite reste possible : l'écart se creuse lentement) :
  une seule copie de ces données. `getCentre`, `contient`,
  `dessiner` et `getCercle` les lisent via `m_mobile`.
- **Possédé par `main.cpp`**, pas par `Escouade` : un
  `std::vector<Monstre>` (les monstres du `Niveau`, répartis sur la carte), sans
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
- **Armure** (étape 55) : le constructeur prend aussi l'armure (`int`, 20
  pour tous les monstres, `s_armureMonstresParDefaut` donnée par
  `Niveau::armureMonstres` via `Partie`).
  `recevoirDegats` retire `appliquerArmure(quantite, m_armure)`. Voir
  `docs/combat.md`.
- **`recevoirDegats` ne descend jamais sous zéro** : un coup qui dépasse
  le reste ne laisse pas un nombre négatif stocké.
- **Pas d'accesseur pour les points de vie** : les tests passent par
  `estVivant()` et la barre de vie est dessinée par la classe elle-même.
- **Barre de vie** : dessinée par `dessinerBarreDeVie` (module
  `BarreDeVie`, partagé avec `Personnage` depuis l'étape 49, qui reçoit `getRatio()` du `PointsDeVie` du monstre depuis l'étape 53) : fond sombre
  de la largeur du monstre, quelques pixels au-dessus de lui, recouvert
  d'un rectangle vert proportionnel aux points de vie restants.
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
  - `EnGarde` → `Poursuite` aussi si le monstre vient d'être touché (voir
    « Coup reçu ») ;
  - `Retour` → `EnGarde` une fois arrivé (`m_mobile.directionVoulue()` vide),
    et **seulement** alors : depuis l'étape 69, aucune transition de `Retour` vers
    `Poursuite` (voir « Retour au poste » plus bas).

## Détection et territoire

- **Le territoire se mesure sur le monstre, pas sur la proie** :
  `m_rayonTerritoire` (`Monstre::s_rayonTerritoire`, 600 px depuis l'étape 69,
  400 avant) est la distance maximale entre le **monstre** et son **poste**. Un
  monstre peut donc poursuivre une proie arbitrairement loin de son point de
  départ, tant que lui-même ne s'éloigne pas de plus de 600 px de son poste.
  La constante est publique pour que les tests ne recodent pas la valeur.
- **Détection unique, dans la fonction privée `const`
  `proieDetectee(cible, carte)`** : une proie est détectée si elle existe, si
  elle est à `<=` `m_rayonDetection` (200 px) **du monstre**, à `<=`
  `m_rayonTerritoire` **du poste** ET en vue du monstre
  (`ligneDeVue` de `ChampDeVision`, en cases depuis l'étape 73 ; étape 62). La seconde condition est
  nécessaire : sans elle, un monstre au bord de son territoire détecterait une
  proie restée à la frontière et la poursuivrait pour la perdre aussitôt. Elle
  n'est appelée qu'en `EnGarde` : en `Retour`, le monstre ne détecte plus rien
  (étape 69).
- **La vue ne sert qu'à détecter** (étape 62) : en `Poursuite`, le monstre
  ne perd pas sa proie en la perdant de vue ; il la suit par le A* jusqu'à
  l'abandon habituel (plus de 600 px du poste, ou proie morte). Sinon,
  tourner un coin suffirait à semer n'importe quel monstre. Le territoire
  reste une distance pure, et un monstre touché s'active sans vue
  (`reveilleParCoup` ne la regarde pas).
- **Les distances sont mesurées à vol d'oiseau** : détection et territoire
  centre à centre ; portée d'attaque : bord à bord. La vue se décide en
  cases (`ligneDeVue`, étape 73) : le monstre et le personnage se voient
  si et seulement si le personnage voit le monstre.

## Coup reçu

- **`m_alerte`**, posé par `alerter()`, que `recevoirDegats` appelle si les
  dégâts sont `> 0` (un coup de 0 ne réveille personne). `mettreAJour`
  calcule `alerte` : alerté ET proie existante ET monstre à
  `<= m_rayonTerritoire` de son poste. Il s'ajoute à `proieDetectee` en
  `EnGarde`. En `Poursuite`, être touché ne change rien ; en `Retour`, le
  monstre est invulnérable et `alerter()` est sans effet (voir « Retour au
  poste »).
- **L'armure ne change pas le réveil** : il dépend du montant reçu `> 0`,
  pas du montant retiré. Grâce au plancher à 1 de `appliquerArmure`, un
  coup de 1 retire 1 point et réveille le monstre (testé).
- **L'indicateur est remis à faux à chaque `mettreAJour`**, qu'il ait
  servi ou non : un coup ne sert qu'une fois, pas de mémoire durable
  (touché sans proie = coup perdu).
- **Pourquoi la règle du territoire** : sans elle, un monstre hors de son
  territoire, touché, passerait en `Poursuite` puis serait renvoyé en `Retour`
  à l'image suivante (la poursuite le vérifie), en boucle, avec un A* à chaque
  coup.
- **La proie reste la proie habituelle** (provocateur, sinon personnage
  le plus proche) : le monstre ne se jette pas sur l'auteur du coup.
  `Coup`, `Escouade` et `main.cpp` n'ont pas changé.

## Retour au poste (étape 69)

- **L'exploit de la laisse** : des tireurs (portée 120 px) placés juste au-delà du
  territoire d'un monstre le touchaient quand il atteignait la frontière. Il
  abandonnait, rentrait, repassait dans son territoire, était réveillé par le coup
  suivant (étape 47), revenait… Il ne frappait jamais (portée 15 px) et ses dégâts
  s'accumulaient : on le tuait sans risque. Cause : pas d'hystérésis entre le
  seuil d'abandon et le réveil, et aucun coût à abandonner (le monstre gardait
  ses blessures).
- **Hystérésis** : en `Retour`, le monstre ne réagit à rien jusqu'à son arrivée
  au poste. Plus de transition `Retour` → `Poursuite` (ni par la détection, ni par
  `alerter()`, ni par les dégâts) ; `alerter()` n'a aucun effet en retour, donc un
  monstre en retour ne relance pas non plus d'alerte de groupe. Il n'engage aucun
  coup (`engagerAttaque` renvoie faux). Arrivé (passage en `EnGarde`), il
  redevient normal. Aucun coup engagé ne peut exister à l'entrée en `Retour` : l'état
  ne change pas pendant l'exécution.
- **Invulnérable** : `recevoirDegats` n'a aucun effet en `Retour` (ni dégâts ni
  alerte). Un monstre qui abandonne n'est donc plus une cible rentable, même pour
  un coup déjà parti.
- **Régénération** : 50 % des points de vie maximum par seconde
  (`s_regenerationRetour`, une fraction par seconde). `PointsDeVie::soigner` prend
  un `int` et un `int` à 0,83 point par image à 60 images par seconde serait tronqué
  à 0 : `m_soinAccumule` (un `float`) cumule maximum × fraction × `dt`, seule la
  partie entière est soignée et le reste est conservé. Il est remis à 0 hors
  retour et à pleine vie ; le plafond est celui de `PointsDeVie`. Un retour de
  plusieurs secondes (600 px à 160 px/s : 3,75 s) ramène toujours le monstre
  plein ; un retour très court (tout juste sorti de son territoire) ne soigne
  presque rien.
- **Un monstre en retour n'est pas une cible** : `estCiblable()` (faux en `Retour`).
  `Partie` en tire deux listes d'`EntiteVue` : les **cibles** (ciblables, pour tout
  ce qui choisit un monstre à frapper, provoquer, viser ou verrouiller) et les
  **physiques** (tous, pour les obstacles bloquants et à contourner). Voir
  `docs/boucle-et-entrees.md`.
- **Indice visuel** : sous F3 seulement, un monstre en retour est dessiné semi-transparent
  (alpha 128 sur 255). Le paramètre `marquerLeRetour` descend de `EcranJeu` par
  `Partie::dessiner` ; `Monstre` ne retient rien (comme `estCible`).
- **Valeurs** : territoire 600 px (400 avant), vitesse 160 px/s (120 avant) : la
  fuite était trop facile. Détection (200 px) inchangée. Deux constantes à ajuster.

## Groupe et alerte (étape 64)

- **`m_groupe` est un identifiant opaque** : il dit seulement « ces monstres
  vont ensemble ». `Monstre` ne sait pas que c'est une salle et ne connaît pas
  les autres monstres ; le champ n'est jamais nommé d'après une salle.
- **`alerter()` est la seule règle de réveil hors détection** : le réveil par
  les dégâts (étape 47) et l'alerte du groupe passent par elle, donc par la
  même condition (proie existante, monstre dans son territoire). Alerter un
  monstre déjà en poursuite ne fait rien (l'indicateur est consommé à chaque
  `mettreAJour`). Un monstre en exécution de coup garde l'alerte jusqu'à la
  fin du coup.
- **`Partie` est le médiateur** : elle note `estEnPoursuite()` de chaque
  monstre avant leur mise à jour, puis repère ceux qui viennent de passer en
  poursuite (depuis « en garde » ou « retour ») et alerte les autres membres
  de leur groupe. `Monstre` ne retient rien pour détecter la transition :
  `Partie` compare avant et après.
- **Pas de chaîne** : tous les groupes à alerter sont collectés avant le
  premier `alerter()`, donc un alerté ne relance pas d'alerte dans la même
  image. À l'image suivante, il passe en poursuite et `Partie` ré-alerte son
  groupe, mais le groupe est un ensemble fermé : rien ne passe à un autre
  groupe, et seul un membre revenu en `Retour` peut être relancé.
- **L'alerté commence à bouger à l'image suivante** : l'alerte est posée
  après la mise à jour des monstres (accepté).
- **Le retour au poste reste individuel** : chacun abandonne selon sa propre
  distance à son poste.
- **Limite de test** : « alerté hors de son territoire » est testé sur
  `Monstre` ; un monstre en retour hors territoire au début d'une image est
  trop fugace pour un test de `Partie`.

## Ordres donnés à `Mobile`

- **À l'entrée en `Retour`**, `arreter()` puis
  `allerVers(m_poste, carte, aContourner)`. L'entrée est détectée en comparant l'état
  avant et après le `switch` (`etatPrecedent != m_etat`) : redonner
  l'ordre à chaque image écraserait inutilement le chemin déjà calculé
  et refairait un A* identique à chaque tic.
- **En `Poursuite`, à chaque image** : `arreter()` si le monstre a une
  cible de frappe (voir « Proie et cible de frappe » plus bas),
  sinon `allerVers(proie.centre, carte, aContourner)`. Ici
  l'ordre est redonné à chaque image parce que la proie, un personnage,
  bouge ; le poste, lui, est fixe. Le but du monstre est de se mettre à
  portée pour frapper, pas de rejoindre sa proie.
- **Pendant le reste du `Retour`**, le chemin donné à l'entrée suffit, sauf si
  `m_mobile.estBloque()` (0,3 s sans avancer, par exemple derrière un autre
  monstre arrivé après) : `allerVers` est alors rappelé, ce qui remet le
  compteur à 0 (un recalcul toutes les 0,3 s au plus). En `EnGarde`, aucun ordre.
- **Obstacles bloquants et à contourner** (étape 68) : `mettreAJour` reçoit
  `obstacles` (personnages vivants et autres monstres, qui le bloquent) et
  `aContourner` (les **autres monstres seulement**, construits par `Partie`).
  Un personnage sur le chemin d'un monstre n'est pas contourné : c'est quelqu'un
  à frapper (règle de l'étape 66), le monstre s'arrête dès qu'il a une cible de
  frappe. Un couloir bouché par un autre monstre reste franchissable (coût
  ajouté, pas interdiction) ; voir `docs/chemin.md`.
- **`m_mobile.mettreAJour(dt, carte, obstacles)`** est appelé à chaque
  image, quel que soit l'état ; `obstacles` est transmis tel quel.

## Riposte

- **Frapper se fait en deux temps** (étape 57, voir `docs/combat.md`) :
  `engagerAttaque(proie, candidats, carte)` (cible de frappe à portée ET en vue,
  monstre immobile, attaque prête) puis `avancerAttaque(dt, generateur)`, qui rend les dégâts
  0,3 s plus tard. `Monstre::frapper` n'existe plus.
- **`peutAtteindre(proie, carte)`** (étape 62) : règle unique, à portée
  (bord à bord) ET `ligneDeVue` (en cases, étape 73), sur le modèle de
  `Escouade::peutAtteindre` sans dépendance entre les deux classes. Un
  monstre à portée mais caché derrière un coin ne s'arrête donc pas : il
  continue vers sa proie jusqu'à la voir. Un coup déjà engagé part sans
  revérifier portée ni vue.
- **Proie et cible de frappe** (étape 66) : la proie est celle que le monstre
  poursuit (`Escouade::proieDuMonstre` : provocateur vivant, sinon vivant le
  plus proche), sans dépendre de la vue. La cible de frappe est celle qu'il
  peut frapper : `choisirCibleAFrapper` avec `peutAtteindre` comme prédicat et
  la proie comme préférée, sur les candidats de `Escouade::candidatsDeFrappe`.
  Donc la proie si elle est atteignable, sinon le candidat atteignable le plus
  proche (bord à bord ; égalité : le premier). Un monstre provoqué n'a que son
  provocateur comme candidat (provocation stricte). Le monstre choisit seul
  (règle propre au monstre) ; `Partie` lui passe proie et candidats, calculés
  sur sa position avant qu'il bouge.
- **Arrêt** : en poursuite, le monstre s'arrête dès qu'il a une cible de frappe
  (la proie ou un autre candidat) ; sinon il continue vers sa proie. Un monstre
  bloqué par un mêlée qui n'est pas sa proie le frappe donc au lieu de rester
  sans frapper. Détection, territoire, abandon, alerte de groupe et retour au
  poste restent basés sur la proie.
- **Exécution** : pendant ces 0,3 s, `mettreAJour` arrête le monstre et le
  déplace sans rien changer d'autre : ni poursuite, ni retour, ni changement
  d'état (l'état et le « coup reçu » sont traités après le coup). Un monstre
  arrivé au contact s'arrête donc un instant avant chaque coup.
- **Coup engagé** : `Monstre` retient seulement un entier (`m_idCibleEngagee`,
  la place du personnage choisi, lue par `cibleEngagee`). Le coup part toujours,
  même si la cible s'est éloignée, et touche ce personnage (`Escouade::recevoirCoup`).
  Une provocation survenue pendant l'exécution ne change pas la cible du coup
  en cours. Seule la mort du personnage visé l'annule, sans recharge
  (`Partie::faireFrapperLeMonstre` appelle `annulerAttaque`) ; un monstre ne
  s'annule jamais lui-même.
- **Indice visuel** : pendant l'exécution, le carré est éclairci de 50 %
  (`eclaircir`), le contour jaune du monstre verrouillé est inchangé.
- **La riposte dépend du statut** (étape 69, elle n'en dépendait pas avant) : un
  monstre en `Retour` n'engage aucun coup, même sur ce qui passe à sa portée.
  Elle ne dépend pas du verrou de cible.
- **`avancerAttaque(dt, generateur)`** reçoit le `Generateur` de la `Partie` par
  référence et le transmet à `Skill` : la recharge de l'attaque est tirée
  dans [0,7 s ; 1,3 s] à chaque départ de coup (moyenne 1 s, comme avant l'étape 56).
  `Monstre` ne garde pas le générateur.
- **`mettreAJour` décrémente d'abord la recharge** (`m_attaque.mettreAJour`),
  quel que soit l'état.

## Limites connues

- Un monstre touché ne se jette pas sur l'auteur du coup. L'alerte ne
  se propage qu'au sein d'un groupe : ni par proximité ni par le bruit d'un
  groupe à l'autre, pas de chef, pas d'effet visuel.
- Un monstre qui rentre ignore tout jusqu'à son poste, même une proie collée à
  lui (étape 69, voulu).
- Un monstre touché au-delà de son territoire (juste avant d'abandonner) ne
  prolonge pas sa laisse : il abandonne quand même. Piste : laisse prolongée par
  les dégâts.
- Pas de patrouille ; pas de retour au poste en groupe.
- Chaque monstre garde son propre poste et territoire, sans les partager
  ni se coordonner avec les autres.
- Le territoire ignore les murs (distance pure) ; un monstre contourne les
  autres monstres (étape 68) mais pas les personnages, et un monstre qui en
  bloque un autre dans un passage étroit est traversé au prix du coût ajouté.
- Pas de mémoire de la dernière position vue, de recherche du joueur ni de
  cône de vision.
