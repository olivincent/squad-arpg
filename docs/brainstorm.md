# Squad-ARPG — Brainstorm

## Mode d'emploi

Ce document garde en tête les idées à reprendre plus tard. Rien ici n'est une décision, et rien ici n'est de la conception technique : pour chaque idée, seulement ce qu'elle est, ce qu'elle apporte au jeu et quelques questions à se poser le moment venu.

Quand une idée est choisie, sa conception se fait en discussion, elle entre dans `ROADMAP.md` et elle quitte ce document. Les petites pistes techniques (A\* qui tient compte des entités, effets visuels, skill en attente par personnage, etc.) sont listées dans « Autres pistes » de `ROADMAP.md`, pas ici.

## 1. Classes et synergie par couleurs

**L'idée.** S'inspirer des couleurs de *Magic: The Gathering* pour donner une identité aux classes et pousser le joueur à penser la synergie de son escouade. Chaque couleur a sa façon de jouer :

- **Blanc (sacré)** : soin direct, dégâts de radiance.
- **Noir (ombre)** : vol de vie, affaiblissements (debuffs).
- **Rouge (feu, montagne)** : dégâts de feu, armure physique.
- **Vert (nature, forêt)** : soin dans le temps, archers.

**Ce que ça apporte.** La composition du groupe devient un vrai choix : tout miser sur une couleur, ou en mélanger deux, trois ou quatre pour créer des combinaisons différentes.

**Principe à garder.** Une classe n'est pas enfermée dans une seule couleur : son arbre de skills offre des choix pour pencher d'un côté ou de l'autre. Les couleurs peuvent aussi se croiser entre personnages, par exemple une classe sacrée qui donne à un allié un buff convertissant une partie de ses dégâts en dégâts sacrés.

**Questions ouvertes.**

- Une cinquième couleur (le bleu de Magic : eau, froid, contrôle ?) ou seulement quatre ?
- Les couleurs correspondent-elles aux types de dégâts et aux résistances (section 3) ?
- Faut-il récompenser un groupe d'une seule couleur (bonus), ou laisser les combinaisons parler d'elles-mêmes ?
- Avec seulement 4 personnages, combien de mélanges restent intéressants ?

## 2. Coup double

**L'idée.** Une arme ou un passif (un voleur, par exemple) donne une chance de frapper deux fois au lieu d'une.

**Ce que ça apporte.** Un combat moins régulier, et une première raison de comparer deux armes.

**Questions ouvertes.**

- Chance fixe, ou statistique qui grandit avec l'équipement ?
- Second coup à pleins dégâts ou réduit ? Immédiat ou après un court délai visible ?
- Le second coup peut-il lui-même en déclencher un autre ?
- Seulement l'attaque de base, ou aussi les skills ?

## 3. Autres défenses

L'armure existe déjà. D'autres façons de réduire ou d'éviter un coup :

- **Esquive** : chance d'éviter tout le coup. Bon ressenti pour un voleur, mais peut sembler injuste en série.
- **Blocage au bouclier** : chance de bloquer une partie du coup. Donne une identité au tank.
- **Bouclier qui déséquilibre** : un blocage réussi allonge la prochaine recharge de l'attaquant ; la défense ralentit l'adversaire au lieu de réduire les dégâts.
- **Types de dégâts et résistances** (physique, feu, froid, poison…) : profondeur à la *Titan Quest*, mais beaucoup de contenu à équilibrer. Lien direct avec les couleurs (section 1).

**Pourquoi ça compte pour l'escouade.** Le blocage physique fait déjà du tank un mur ; la défense en ferait aussi une éponge. Les deux ensemble justifient de mettre le guerrier devant et le mage derrière.

**Questions ouvertes.** La défense vient-elle de l'équipement ou de la classe ? Les monstres ont-ils les mêmes mécanismes ?

## 4. Automatisation de l'escouade (gambits)

**L'idée.** Laisser le joueur décider quels skills partent tout seuls, à la manière des gambits de *Final Fantasy XII* : une liste de règles « condition → action ».

**Ce que ça apporte.** Une réponse à la grande question du jeu : gérer 4 personnages sans micro-gestion, sans que le jeu se joue tout seul.

**Niveaux d'ambition possibles.**

- Un interrupteur « auto » par skill.
- Une condition simple par skill (« soin si un allié est sous 50 % »).
- Une vraie liste de règles ordonnées par personnage.

**Tension de design.** La direction « chef d'orchestre » veut que le joueur garde un rôle : si tout est automatique, il regarde le combat. Piste : skills de base en auto, skills à forte valeur (zone, provocation) au clavier.

**Questions ouvertes.** Un ordre du joueur suspend-il les règles ? Les règles appartiennent-elles au personnage ou à sa classe ?

## 4 bis. Stratégie d'attaque par type de monstre

**L'idée.** Donner à chaque type de monstre sa manière d'attaquer : un monstre agile qui traverse les lignes pour atteindre l'arrière, un skill de type voleur pour passer à travers les entités. Une stratégie d'attaque pourrait aussi servir aux gambits du joueur (section 4).

**Trois points de branchement déjà identifiés.**

- Le choix de la proie : `Escouade::proieDuMonstre`.
- Le coût du A* : la fonction de coût de l'étape 68 (`std::function<int(sf::Vector2i)>`).
- Les entités qui bloquent le déplacement : la liste d'obstacles construite par `Partie` / `Escouade`.

**Prérequis.** L'étape 68 (A* qui contourne les obstacles mobiles). Rien n'est décidé.

## 4 ter. Zones de contrôle, la suite (étape 77)

L'étape 77 donne un prix à la fuite (attaque d'opportunité). Elle laisse des questions ouvertes, rien n'y est décidé.

- **Dégagement des tireurs** (étape 78, faite) : un tireur engagé se dégage vers sa place d'attaque quand chaque monstre qui l'engage est tenu par un autre personnage. C'est la **première règle de comportement à rendre configurable**. Depuis l'étape 82.4, le point de branchement est `choisirIntention` (`escouade/Intention`, voir `docs/formation.md`, « Intentions ») : la décision de chaque personnage y est une suite de règles par priorité, sans carte ni géométrie, et le dégagement en est la règle 2 ; `Escouade::degagementVoulu` (avec la règle pure `monstreDontSeDegager`) ne garde que le départ et la fin du dégagement.
- **Gambits pour choisir le comportement** (se greffe sur la section 4) : rester et tirer, se dégager, passer en mêlée ; ce serait une condition (« je suis engagé ») et une action au choix du joueur. Le dégagement de l'étape 78 serait l'action par défaut de « se dégager ».
- **Skills pour se faufiler** : un skill qui permet de quitter une zone sans pénalité (esquive, pas de côté, saut), ou de traverser un adversaire (voir 4 bis, monstres qui traversent les lignes).
- **Délai de réaction** : si un coup par attaquant et par image s'avère trop généreux (le coup partirait toujours à la première image où la paire se rompt), exiger un petit délai avant de frapper, ou plafonner par durée plutôt que par image.
- **Zone variable** : zone qui dépend de l'arme, d'un skill (bouclier) ou de la classe, plutôt qu'une valeur fixe de 20 px.
- **Pas de côté** : l'égalité des projections désigne la cible comme fuyard. À revoir si un pas de côté de l'attaquant qui rompt le contact donne un coup qui semble injuste.

## 5. Génération procédurale, la suite

**Carte extérieure.** Comme *Diablo 3* : une carte extérieure avec des entrées de donjons. Fixe avec des entrées tirées au hasard, terrain généré (plaines, forêts, rivières), ou assemblage de grands morceaux faits à la main.

**Autres styles de donjons.** Grottes organiques, salles rondes ou en L, assemblage de modules dessinés à la main (la qualité « faite main » de *Diablo 3*).

**Peuplement.** Nombre de monstres selon la taille de la salle, types de monstres variés, territoires, patrouilles.

## 5 bis. Barre de raccourcis, la suite (étape 75)

L'étape 75 pose trois notions séparées : les emplacements (`BarreDeRaccourcis`), les touches (table dans `Reglages`) et la disposition à l'écran (calculée par la vue). La suite se branche sur l'une d'elles sans toucher aux autres. Rien n'y est décidé.

- **Écran « Configurer les touches »** dans les options, avec un préréglage classique (1 à 0, - et =, barre en ligne droite) ; sauvegarde des touches dans `reglages.ini` (les touches ne sont pas encore lues ni écrites). Le préréglage classique demandera une disposition en ligne (un enum de disposition, pas écrit à l'étape 75).
- **Deuxième barre, seulement cliquable**, pour les auras et les buffs : une autre instance de `BarreDeRaccourcis`, une valeur de plus dans `Barre` (`ClicInterface`). Skills à bascule associés.
- **Interface pour placer ses skills** (glisser-déposer) ; skills appris en montant de niveau.
- **Survol et infobulles** sur la barre ; clic droit sur une icône.
- **Doublons dans la barre** : aujourd'hui une même adresse peut être placée sur deux emplacements (par exemple le même soin en Q et en A). Décision reportée : les autoriser (utile si on place la même compétence sur deux touches), ou les interdire (placer un skill déjà placé le déplace). À trancher avec l'interface de placement.
- **Plus de 4 skills par personnage** : la disposition par défaut ignore les suivants ; à reprendre avec la montée de niveau et l'interface de placement.

## 5 ter. File d'événements de combat, la suite (étape 79)

L'étape 79 pose une file d'`EvenementDeCombat` produite par `Partie` (dégâts et soins réellement appliqués) et lue par `TextesFlottants`. Elle accueille d'autres types d'événements et d'autres lecteurs sans réécriture. Rien n'y est décidé, sauf l'étape 80 (dans `ROADMAP.md`).

- **Étape 83 (prévue)** : le nom du skill lancé au-dessus du lanceur et « Provoqué » au-dessus des monstres provoqués. Un type d'événement de plus et un style de plus.
- **« Immunisé »** au-dessus d'un monstre qui rentre à son poste quand on le frappe : aujourd'hui un montant de 0 ne produit rien. Il faudrait un événement distinct (ce n'est pas un montant).
- **Critiques, « raté », résistances** : si un jour le combat les connaît, un texte et une couleur de plus.
- **Sons** : un son par type d'événement (coup, soin, coup gratuit), lu dans la même file.
- **Journal de combat** : une liste défilante des événements, utile pour comprendre ce qui se passe quand l'escouade agit seule (gambits).
- **Statistiques de fin de partie** : dégâts infligés et subis par personnage, soins rendus, coups gratuits ; une lecture de la file cumulée.
- **Plafond des dégâts par les points de vie restants** : le coup qui tue affiche aujourd'hui son plein montant. À revoir si le nombre doit refléter la vie réellement retirée.
- **Cumul** : additionner les textes de la même cible qui se suivent de près plutôt que de les empiler.

## 5 quater. Inventaire et équipement, la suite (étape 83)

Le modèle existe (`docs/objets.md`) ; ces idées ont été écartées de l'étape 83.

- **Ceinture et cape** : deux emplacements de plus ; il suffit d'ajouter des valeurs à `Emplacement` et à `CategorieObjet`, et une règle à `peutAller`.
- **Grille d'inventaire à la Diablo** : des objets qui occupent plusieurs cases. Écartée : le sac à cases fixes suffit au glisser-déposer simple (étape 85), et la grille demande une forme par objet.
- **Clic droit pour équiper** : équiper un objet du sac vers son emplacement évident, sans glisser. À reconsidérer après l'étape 85 ; avec deux anneaux et trois personnages, « l'emplacement évident » n'existe pas toujours.
- **Restrictions d'équipement par classe** : un mage sans cuirasse de mailles. Cela donnerait un sens aux classes, mais fixerait aussi le rôle des futurs arbres de skills ; à décider avec eux.
- **Affixes aléatoires et rareté** : l'`Objet` ne porte aujourd'hui que son identifiant. Des affixes demanderaient d'y ajouter des données, et de ne plus supposer que deux objets du même modèle sont égaux.
- **Armes et deux mains** : classe `Arme` (intervalle de recharge de l'attaque de base), catégorie d'arme dans `peutAller`, règle d'occupation de la main gauche pour une arme à deux mains.
- **Sauvegarde de l'équipement** entre deux parties, **butin au sol** et tables de butin des monstres.

## 6. Idées en vrac

- **État de l'escouade** (en combat, en mouvement, au repos) : utile pour la mana et les gambits.
- **Mana** qui se régénère hors combat.
- **Caméra** qui dézoome pour garder tout le groupe à l'écran.
- **Brouillard de guerre** : pousse à garder le groupe uni.
- **Passage à Unreal Engine** pour la 3D : décision lointaine, raison de plus pour garder la logique de jeu séparée du rendu.
