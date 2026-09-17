# Squad-ARPG — Roadmap

## Méthode de travail

- On avance une pièce à la fois : d'abord la structure de données (les
  paramètres/membres), puis les fonctions une par une. Déclaration dans le
  `.hpp` avant implémentation dans le `.cpp`.
- On ne construit que ce dont on a besoin maintenant, jamais en prévision
  d'une étape future.
- Après chaque petit ajout (même une seule fonction), on compile et on
  teste avant de continuer.
- Commandes exactes (build, exécution, git) données à chaque étape.
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

Sous-étapes :

1. [x] Squelette de projet (fait à l'étape 0)
2. [x] Ouvrir une fenêtre SFML vide
3. [x] Dessiner un carré statique dedans
4. [x] Détecter un clic de souris (afficher les coordonnées en console,
       rien ne bouge encore)
5. [x] Déplacer le carré à la position du clic
6. [x] Afficher la position sous le carré (texte SFML)

### Étape 2 — Déplacement progressif ("tic" par "tic") (en cours)

Le carré ne doit plus se téléporter instantanément à la position cliquée :
il doit s'en approcher graduellement, à chaque itération de la boucle
principale, jusqu'à l'atteindre.

Sous-étapes prévues (à confirmer/détailler ensemble au fil de l'étape) :

1. [ ] À définir avec Claude : structure de données pour garder en mémoire
       la position cible (là où on a cliqué) séparément de la position
       actuelle du carré
2. [ ] À définir : logique de déplacement d'un "tic" vers la cible (pas
       fixe ou proportionnel à la distance restante — à décider)
3. [ ] Le carré s'arrête proprement une fois la cible atteinte (pas
       d'oscillation ou de dépassement)
4. [ ] Test visuel : le déplacement paraît fluide, pas saccadé

### Étape 3 — Salle avec murs (à détailler plus tard)

Sera découpée en sous-étapes une fois l'étape 2 stable et terminée.

## État actuel (à mettre à jour à chaque session)

- Dépôt Git : `squad-arpg`, poussé sur GitHub
- Bibliothèque graphique : SFML 3.0.2, intégrée à CMake
  (`find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)` +
  `target_link_libraries(main PRIVATE SFML::Graphics)`)
- Étape 1 terminée : fenêtre SFML, carré qui se téléporte au clic, position
  affichée en texte sous le carré (police système Windows, `arial.ttf`,
  chargée via chemin absolu — à revoir plus tard pour une police committée
  dans le projet)
- Étape en cours : **Étape 2** — faire bouger le carré graduellement
  ("tic" par "tic") vers la position cliquée, plutôt qu'un déplacement
  instantané
- Prochaine décision technique à prendre : comment structurer la boucle de
  déplacement progressif (position cible vs position actuelle, calcul du
  déplacement par itération)
