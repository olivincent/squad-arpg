// ./src/interface/AffichageSkills.hpp
#pragma once

#include <string>

#include "combat/IdSkill.hpp"

// Les textes de l'interface pour les skills, en UTF-8 (à passer à `sf::String::fromUtf8`).
// Le noyau ne connaît que l'identifiant ; le nom à l'écran est ici.

// « Frappe puissante », « Provocation », « Soin »…
[[nodiscard]] std::string nomDuSkill(IdSkill id);
