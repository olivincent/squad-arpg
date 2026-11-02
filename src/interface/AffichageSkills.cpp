// ./src/interface/AffichageSkills.cpp
#include "interface/AffichageSkills.hpp"

// Pas de `default` : un skill ajouté sans nom fait avertir le compilateur.

std::string nomDuSkill(IdSkill id) {
    switch (id) {
        case IdSkill::FrappePuissante: return "Power Strike";
        case IdSkill::Provocation: return "Taunt";
        case IdSkill::Soin: return "Heal";
        case IdSkill::Explosion: return "Explosion";
        case IdSkill::FrappeSacree: return "Holy Strike";
        case IdSkill::ImpositionDesMains: return "Lay on Hands";
        case IdSkill::TirPercant: return "Piercing Shot";
        case IdSkill::PluieDeFleches: return "Rain of Arrows";
        case IdSkill::Benediction: return "Blessing";
        case IdSkill::Renouvellement: return "Renew";
        case IdSkill::Contresort: return "Counterspell";
        case IdSkill::CoupAssommant: return "Stunning Blow";
    }
    return "";
}
