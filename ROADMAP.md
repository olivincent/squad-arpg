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

### Étape 2 — Déplacement progressif ("tic" par "tic") ✅ TERMINÉE

1. [x] Position cible (`cible`) stockée séparément de la position du carré
2. [x] Déplacement d'un pas fixe par tic vers la cible (vecteur direction
       normalisé × vitesse)
3. [x] Arrêt exact sur la cible : si la distance restante ≤ pas, on pose
       le carré sur la cible (pas d'oscillation)
4. [x] Vitesse indépendante de la cadence : delta time via `sf::Clock`,
       vitesse en pixels par seconde

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
- Étape 2 terminée : le carré se déplace à vitesse constante (180 px/s)
  vers le point cliqué et s'arrête exactement dessus
- Le texte de position se téléporte encore au point cliqué (ne suit pas
  le carré)
- Étape en cours : **Étape 3** — salle avec murs, à découper en sous-étapes
