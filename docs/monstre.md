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
  `dessiner` et `getCercle` les lisent via `m_mobile` (`getCercle` est une façade de
  `Mobile::getCercle` depuis l'étape 82.1).
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
  `recevoirDegats` retire `appliquerArmure(quantite, armure)`. Voir
  `docs/combat.md`.
- **`Vitalite`** (étape 82.1) : les points de vie et l'armure sont dans `m_vitalite`, le même type
  que dans `Personnage`. `recevoirDegats`, `estVivant` et `getPointsDeVie` lui délèguent. Les règles
  propres au monstre restent dans `Monstre` : l'invulnérabilité en `Retour` et l'alerte (dans
  `recevoirDegats`, avant et après la délégation), la régénération (`mettreAJour`, par
  `Vitalite::soigner`). `s_pointsDeVieMax` (100) reste une constante de `Monstre.cpp`, distincte de
  celle de `Personnage.cpp` : deux valeurs d'équilibrage égales par hasard (celle des monstres ira
  dans `ParametresMonstre` avec E1).
- **`recevoirDegats` ne descend jamais sous zéro** : un coup qui dépasse
  le reste ne laisse pas un nombre négatif stocké.
- **Pas d'accesseur pour les points de vie** : les tests passent par
  `estVivant()` et la barre de vie est dessinée par la classe elle-même.
- **Barre de vie** : dessinée par `dessinerBarreDeVie` (module
  `BarreDeVie`, partagé avec `Personnage` depuis l'étape 49, qui reçoit `getRatio()` de la `Vitalite` du monstre) : fond sombre
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
  (`s_regenerationRetour`, une fraction par seconde). `Vitalite::soigner` prend
  un `int` et un `int` à 0,83 point par image à 60 images par seconde serait tronqué
  à 0 : `m_soinAccumule` (un `float`) cumule maximum × fraction × `dt`, seule la
  partie entière est soignée et le reste est conservé. Il est remis à 0 hors
  retour et à pleine vie ; le plafond est celui de `PointsDeVie` (dans `Vitalite`). Un retour de
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

## Zone de contrôle (étape 77)

- **Une constante** : `Monstre::s_zoneDeControle` (20 px, bord à bord), la même pour tous les
  monstres ; `getZoneDeControle()` la renvoie, `getPuissanceAttaque()` donne les dégâts de son attaque
  (de simples lectures). `Monstre` ne sait rien de la règle : c'est `Partie` qui prend les photos et
  applique les coups gratuits (voir `docs/combat.md`).
- **Un monstre qui rentre à son poste n'engage personne et n'est engagé par personne** : il est exclu
  des photos des camps, comme il l'est de la liste des cibles. Il est invulnérable ; sinon un coup
  gratuit à la frontière du territoire rouvrirait l'exploit de l'étape 69.
- **Changer de proie a un prix** : un monstre qui quitte un personnage pour en
  suivre un autre (provocation, mort de la proie) donne un coup gratuit à celui qu'il quitte, s'il a une
  zone et n'engage plus personne à la fin.
- **Un monstre qui frappe gratuitement** : le coup n'arrête pas son attaque et ne touche pas sa recharge ;
  le personnage touché le compte comme agresseur (3 s).

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
- Un monstre n'évite pas de quitter la zone d'un personnage (le coup gratuit est subi, pas évité) ; pas
  de monstre qui « tient » son adversaire de lui-même (étape 77).

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Monstres** : des carrés rouges de 30 × 30 (160 px/s, 100 points de
  vie, armure 20, barre de vie). Sur la carte générée : 1 à 3 par salle sauf
  la première, au centre d'une case intérieure (une case de marge contre
  les murs), à au moins 2 cases les uns des autres et à 400 px ou plus du
  départ (rejet, 10 tentatives par monstre, abandonné sinon : une salle
  peut en avoir moins ou aucun). Sur la carte à la main (tests) : six, à
  `(660, 380)`, `(500, 580)`, `(900, 220)`,
  `(1940, 180)`, `(1500, 540)` et `(1620, 540)`. Possédés par
  `Partie` dans un `std::vector<Monstre>`. Un monstre détecte un
  personnage vivant à moins de 200 px de lui et de 600 px de son poste (sa
  position de départ) ET en vue (`ligneDeVue`, en cases) ;
  il le poursuit (A* + lissage) sans que la perte de vue y mette fin, et
  s'arrête dès qu'il peut l'atteindre : à portée d'attaque (15 px bord à
  bord, 10 dégâts, recharge 0,7–1,3 s) ET en vue (`Monstre::peutAtteindre`). Un monstre qui reçoit des dégâts (> 0) entre en poursuite de sa
  proie habituelle (provocateur, sinon le plus proche) même hors de sa
  détection, s'il est lui-même dans son territoire ; un coup reçu hors
  du territoire ne le réveille pas, et ne sert qu'une fois. Il abandonne
  quand lui-même s'éloigne de plus de 600 px de son poste, ou si la proie
  meurt, et rentre alors à son poste. **Retour au poste** (étape 69) : jusqu'à
  son arrivée, il ne réagit à rien (ni détection, ni alerte, ni dégâts, ni
  alerte de groupe), n'engage aucun coup, est **invulnérable**, **regagne la moitié de
  ses points de vie maximum par seconde** et n'est plus une cible (ni autodéfense,
  ni verrou, ni provocation, ni skill ; un clic sur lui compte comme un clic au
  sol), tout en continuant de bloquer et d'être contourné. Cela supprime
  l'exploit où des tireurs touchaient un monstre qui faisait des allers-retours
  à la frontière de son territoire. Sa proie (le personnage vivant le plus
  proche, ou le provocateur) est celle qu'il poursuit ; il frappe sa **cible de
  frappe** (étape 66) : la proie si elle est à portée ET en vue, sinon le
  personnage atteignable le plus proche (un monstre provoqué ne frappe que son
  provocateur), attaqué ou non, et s'arrête dès qu'il en a une ; un monstre touché
  s'active même sans voir son agresseur. Un monstre mort est
  retiré. **Groupes** (étape 64) : chaque monstre a un identifiant de
  groupe opaque (`Monstre::getGroupe()`), fixé à la construction : sur la
  carte générée, un groupe par salle peuplée ; sur la carte à la main, un
  groupe par monstre sauf les deux de `(1500, 540)` et `(1620, 540)`, qui
  partagent le leur. Quand un monstre passe en poursuite (depuis « en garde »),
  `Partie` appelle `Monstre::alerter()` sur les autres
  membres de son groupe : même règle que le réveil par les dégâts (proie
  existante ET monstre dans son territoire, sinon rien). Les alertés
  commencent à bouger à l'image suivante, et sont tous alertés d'un coup
  (pas de chaîne vers un autre groupe) ; le retour au poste reste
  individuel. Un monstre touché par un skill de zone de loin alerte donc
  toute sa salle.

### Limites connues

- Le territoire des monstres reste une distance pure (il ignore les
  murs) ; une fois en poursuite, un monstre ne perd pas sa proie en la
  perdant de vue (il la suit par le A* jusqu'à l'abandon habituel) ; la
  ligne de vue se décide en cases (étape 73), d'un centre à l'autre.
- Un monstre qui rentre ignore tout jusqu'à son poste, même une proie collée à
  lui (voulu, étape 69) ; un monstre touché juste avant d'abandonner ne prolonge
  pas sa laisse ; la régénération ne soigne presque rien si le retour est très
  court ; pas de patrouille ; pas de coordination entre monstres.

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `Monstre` (`monstres/`) — un ennemi

Un carré rouge de 30 × 30 qui détecte un personnage (à moins de 200 px, dans son territoire et en vue), le poursuit sans dépendre de la vue, frappe
à portée et retourne à son poste (160 px/s ; territoire de 600 px). Au retour, il ne réagit à rien
jusqu'à son arrivée, il est invulnérable, il n'est plus une cible et il regagne ses points de vie
(`docs/monstre.md`). Possédé par `Partie`. Ne connaît ni
`Escouade` ni `Personnage` : il reçoit sa proie, ses candidats de frappe (des
`EntiteVue`) et ses obstacles en paramètres. La proie est celle qu'il poursuit ;
la cible de frappe est celle qui est à portée ET en vue.

Données membres : `m_mobile`, `m_vitalite` (`Vitalite` : points de vie et armure), `m_id`,
`m_attaque`, `m_rayonDetection`, `m_rayonTerritoire`, `m_etat` (`Etat`,
`enum class` privé : `EnGarde`, `Poursuite`, `Retour`), `m_poste`,
`m_groupe` (`int`, opaque), `m_alerte` (`bool`, posé par `alerter`,
consommé par `mettreAJour`), `m_soinAccumule` (`float`, points de vie rendus pas encore entiers pendant le retour, 0 ailleurs), `m_idCibleEngagee` (`std::optional<int>`, place du personnage visé par le coup en exécution) ; constantes publiques `static constexpr float s_rayonTerritoire` (600 px) et `s_zoneDeControle` (20 px, bord à bord).

| Fonction | Rôle |
|---|---|
| `Monstre(sf::Vector2f centre, int id, int armure, int groupe)` | Crée le monstre à ce centre, avec cet identifiant, cette armure (20, donnée par `Niveau`), ce groupe (donné par `Niveau`) et 100 points de vie ; son poste est ce centre |
| `int getGroupe() const` | Identifiant de groupe ; `[[nodiscard]]` |
| `float getZoneDeControle() const` / `int getPuissanceAttaque() const` | Zone de contrôle (`s_zoneDeControle`) et dégâts de son attaque ; de simples lectures, elles ne touchent ni la recharge ni le coup engagé ; `[[nodiscard]]` |
| `void alerter()` | Demande d'entrer en poursuite de sa proie habituelle, même hors de la détection, si le monstre est dans son territoire ; prise en compte au prochain `mettreAJour`, sans effet en poursuite ni en retour. `recevoirDegats` (dégâts `> 0`) l'appelle |
| `bool estEnPoursuite() const` | Statut « attaque » (`Etat::Poursuite`), lu par `Partie` ; `[[nodiscard]]` |
| `bool estCiblable() const` | Faux en `Retour` (le monstre n'est la cible de personne, mais reste un obstacle) ; vrai sinon ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget& rendu, bool estCible, bool marquerLeRetour) const` | Dessine le monstre (éclairci de 50 % pendant l'exécution d'un coup ; semi-transparent, alpha 128, s'il est en retour et que `marquerLeRetour`, c'est-à-dire F3) et sa barre de vie ; ajoute un contour jaune si `estCible` |
| `void dessinerChemin(sf::RenderTarget& rendu) const` | Débogage (F3) : trait rouge fin du centre du monstre à travers son chemin restant |
| `void mettreAJour(float dt, std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte, const std::vector<Cercle>& obstacles, const std::vector<Cercle>& aContourner)` | Décrémente la recharge. Pendant l'exécution d'un coup : arrête le monstre et le déplace (immobile), sans toucher à l'état. Sinon met à jour l'état (détection de la proie soumise à la ligne de vue, `proieDetectee`, seulement en `EnGarde` ; la poursuite ne dépend pas de la vue ; `Retour` ne se termine qu'à l'arrivée au poste, sans retour possible vers `Poursuite`), regagne des points de vie en `Retour` (50 % du maximum par seconde, par l'accumulateur `m_soinAccumule`) et donne ses ordres à `m_mobile` (poursuite de la proie, arrêt dès qu'une cible de frappe existe, retour au poste — le chemin du retour est aussi recalculé quand `estBloque()`), puis le déplace ; `obstacles` le bloquent, `aContourner` (les autres monstres, pas les personnages) est ce que ses chemins évitent |
| `bool peutAtteindre(Cercle cible, const TileMap& carte) const` | La cible est-elle à portée d'attaque (bord à bord) ET en vue (`ligneDeVue`, en cases) ? Même règle que `Escouade::peutAtteindre` ; `[[nodiscard]]` |
| `bool engagerAttaque(std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte)` | Engage un coup (0,3 s d'exécution) sur la cible de frappe : faux s'il n'y en a pas, si le monstre est en retour, s'il est en mouvement ou si l'attaque n'est pas prête. La place choisie est retenue jusqu'au départ. `Partie` le tente à chaque image (l'échec est normal) ; `[[nodiscard]]` |
| `std::optional<int> cibleEngagee() const` | Place du personnage visé par le coup en exécution (pas forcément la proie) ; vide hors exécution |
| `std::optional<EntiteVue> cibleDeFrappe(...) const` (privée) | `choisirCibleAFrapper` avec `peutAtteindre` comme prédicat et l'id de la proie comme préféré : la proie si atteignable, sinon le candidat atteignable le plus proche (bord à bord ; égalité : le premier) |
| `int avancerAttaque(float dt, Generateur& generateur)` | Fait avancer l'exécution ; rend les dégâts au départ du coup, 0 sinon ; `[[nodiscard]]` |
| `void annulerAttaque()` | Annule le coup en exécution, sans recharge consommée (appelée par `Partie` quand la proie engagée meurt) |
| `sf::Vector2f getCentre() const` | Centre du monstre |
| `Cercle getCercle() const` | Cercle centré sur le monstre, de rayon 15 (façade de `Mobile::getCercle`) |
| `int getId() const` | Identifiant du monstre |
| `bool contient(sf::Vector2f point) const` | Le point (pixels du monde) est-il dans le carré du monstre ? |
| `int recevoirDegats(int quantite)` | Sans aucun effet en `Retour` (invulnérable, pas d'alerte) ni sur un monstre mort : renvoie 0. Sinon délègue à `Vitalite::recevoirDegats` (dégâts après armure, sans descendre sous zéro) ; si `quantite > 0`, mémorise que le monstre vient d'être touché ; renvoie les dégâts après armure |
| `bool estVivant() const` | Points de vie `> 0` |
| `int getPointsDeVie() const` | Points de vie actuels |
