// ./src/IdSkill.hpp
#pragma once

// Identifiant STABLE d'un skill : il ne dépend ni de l'ordre des skills dans la
// liste d'un personnage ni de la touche qui les lance (la future barre de
// raccourcis le stockera).
enum class IdSkill { FrappePuissante, Provocation, Soin, Explosion };
