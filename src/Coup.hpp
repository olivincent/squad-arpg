// ./src/Coup.hpp
#pragma once
#include "Cercle.hpp"

// Un monstre tel que l'escouade le voit : son identifiant et sa forme.
struct MonstreVu {
    int id;
    Cercle cercle;
};

// Des dégâts à infliger au monstre portant cet identifiant.
struct Coup {
    int idMonstre;
    int degats;
};
