# Squad-ARPG — Roadmap

## Méthode de travail

- On avance une pièce à la fois : d'abord la structure de données (les
  paramètres/membres), puis les fonctions une par une. Déclaration dans le
  `.hpp` avant implémentation dans le `.cpp`.
- Claude ne code pas à la place d'Olivier — syntaxe, noms pertinents,
  indices, jamais la solution finale prête à copier-coller.
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

### Étape 1 — Carré qui bouge au clic (en cours)

Sous-étapes prévues :

1. [x] Squelette de projet (fait à l'étape 0)
2. [ ] Ouvrir une fenêtre SFML vide
3. [ ] Dessiner un carré statique dedans
4. [ ] Détecter un clic de souris (afficher les coordonnées en console,
       rien ne bouge encore)
5. [ ] Déplacer le carré à la position du clic
6. [ ] Afficher la position sous le carré (texte SFML)

### Étape 2 — Salle avec murs (à détailler plus tard)

Sera découpée en sous-étapes une fois l'étape 1 stable et terminée.

## État actuel (à mettre à jour à chaque session)

- Dépôt Git : `squad-arpg`, poussé sur GitHub
- Bibliothèque graphique : SFML (confirmé, pas encore intégrée à CMake)
- Étape en cours : **Étape 1, sous-étape 2** (ouvrir une fenêtre SFML vide)
- Prochaine décision technique à prendre : intégration de SFML dans
  `CMakeLists.txt` (méthode d'installation/liaison encore à définir avec
  MSYS2 UCRT64)
