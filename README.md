# Squad-ARPG

Un ARPG en C++ (SFML 3) où l'on contrôle une **escouade** de quatre personnages comme une seule entité, sans gérer chaque personnage un par un. Inspiré d'*Aliens: Dark Descent* pour l'escouade et de *Titan Quest* pour l'action-RPG.

On choisit la classe de chaque membre, puis on descend cinq étages de donjon générés au hasard, dans le brouillard de guerre. Il faut trouver l'escalier de chaque étage et survivre jusqu'au cinquième.

> Projet personnel d'apprentissage du C++ et du génie logiciel.

<!-- Ajoute ici une capture d'écran ou un GIF : ![Aperçu](docs/apercu.gif) -->

---

## Essayer le jeu (Windows)

1. Va dans la page **[Releases](../../releases/latest)** du dépôt.
2. Télécharge `Squad-ARPG-windows.zip`.
3. Décompresse le dossier **en entier**, où tu veux.
4. Double-clique sur `main.exe`.

Aucune installation n'est nécessaire : le zip ne contient que `main.exe` et le dossier `assets/`. Garde-les ensemble, car le jeu ne démarre pas s'il ne trouve pas sa police.

> Windows SmartScreen peut afficher un avertissement, car l'exécutable n'est pas signé. Clique sur « Informations complémentaires », puis sur « Exécuter quand même ».

---

## Commandes

| Action | Commande |
|---|---|
| Déplacer l'escouade | Clic gauche sur le sol (garder le bouton enfoncé pour suivre le curseur) |
| Attaquer un monstre | Clic gauche sur le monstre (le combat continue tout seul) |
| Ouvrir une porte, ramasser un objet, prendre l'escalier | Clic gauche dessus |
| Compétences | `Q W E R` / `A S D F` / `Z X C V` : une colonne de touches par personnage (ou clic sur l'icône) |
| Compétence de zone | Sa touche, puis clic gauche pour viser |
| Annuler la visée | Clic droit ou `Échap` |
| Inventaire | `I` |
| Après une victoire ou une défaite | `R` pour revenir au choix de l'escouade |

Ces touches désignent leur **position** sur le clavier, quelle que soit la disposition. En AZERTY, la touche en haut à gauche des lettres est donc étiquetée `A`, mais le jeu l'affiche correctement.

<details>
<summary>Touches de débogage</summary>

| Touche | Effet |
|---|---|
| `F3` | Chemins, cibles visées et zones de contrôle |
| `F4` | Affiche l'ancre de l'escouade |
| `F5` | Nouvelle carte |
| `F6` | Active ou désactive le brouillard de guerre (affichage seulement) |
| `F7` | Ajoute quatre objets au hasard dans le sac |
| `F8` | Descend d'un étage |
| `F9` | Gagne un niveau |

</details>

---

## Compiler depuis les sources

Le projet se compile sous Windows avec **MSYS2 (environnement UCRT64)**.

### 1. Installer les outils

Installe [MSYS2](https://www.msys2.org/), ouvre le terminal **MSYS2 UCRT64**, puis lance :

```bash
pacman -S --needed git \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-sfml \
  mingw-w64-ucrt-x86_64-catch
```

Il faut SFML 3.0.2 ou plus récent, CMake 3.25 ou plus récent et un compilateur C++20. Catch2 sert aux tests, mais CMake le demande aussi pour compiler le jeu.

### 2. Compiler et lancer

```bash
git clone https://github.com/<ton-nom>/<ton-depot>.git
cd <ton-depot>
cmake --preset jeu
cmake --build --preset jeu
cd build/jeu && ./main.exe
```

Lance le jeu **depuis son dossier**, parce qu'il cherche `assets/` à partir du dossier courant. CMake y copie `assets/` à chaque compilation.

Pour obtenir l'exécutable autonome des Releases (lié en statique, sans console), utilise le preset `distribution` : `cmake --preset distribution && cmake --build --preset distribution`. Le résultat se trouve dans `build/distribution/`. La première configuration télécharge et compile SFML : elle demande le réseau et prend quelques minutes.

> **Compile et lance depuis le terminal MSYS2 UCRT64, pas depuis Git Bash.** Git Bash charge des DLL incompatibles. Windows affiche alors « le point d'entrée de la procédure est introuvable », ou le programme se ferme avec le code 127.

### Tests

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Le preset `debug` active les assertions et UBSan. Le preset `jeu` est la version optimisée (Release).

---

## Documentation

- [`ARCHITECTURE.md`](ARCHITECTURE.md) : les modules et leurs dépendances
- [`ROADMAP.md`](ROADMAP.md) : les étapes du projet
- [`docs/`](docs/) : les notes de conception (combat, génération, vision, etc.)

Les polices sont DejaVu Sans. Leur licence se trouve dans `assets/fonts/`.
