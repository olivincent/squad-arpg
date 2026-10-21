// ./src/interface/AffichageSkills.cpp
#include "interface/AffichageSkills.hpp"

// Pas de `default` : un skill ajouté sans nom fait avertir le compilateur.

std::string nomDuSkill(IdSkill id) {
    switch (id) {
        case IdSkill::FrappePuissante: return "Frappe puissante";
        case IdSkill::Provocation: return "Provocation";
        case IdSkill::Soin: return "Soin";
        case IdSkill::Explosion: return "Explosion";
        case IdSkill::FrappeSacree: return "Frappe sacrée";
        case IdSkill::ImpositionDesMains: return "Imposition des mains";
        case IdSkill::TirPercant: return "Tir perçant";
        case IdSkill::PluieDeFleches: return "Pluie de flèches";
    }
    return "";
}
