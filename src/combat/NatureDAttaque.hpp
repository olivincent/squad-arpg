// ./src/combat/NatureDAttaque.hpp
#pragma once

// La nature d'une attaque, une donnée de l'attaque (`docs/classes-et-combat.md`, section 7) : ce que l'esquive
// et le blocage lisent. Un coup de `Contact` se pare ou s'évite à la mêlée, un `Projectile` (flèche, boule de
// feu) contre les projectiles, une `Zone` ne s'évite ni ne se bloque : elle s'évite en se déplaçant (section 10).
// Les soins et la provocation ne sont pas des attaques : leur nature n'est jamais lue.
enum class NatureDAttaque { Contact, Projectile, Zone };
