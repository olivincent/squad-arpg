// ./src/main.cpp
#include <exception>
#include <iostream>

#include "Application.hpp"

int main() {
    try {
        Application application;
        return application.executer();
    } catch (const std::exception& erreur) {
        std::cerr << erreur.what() << '\n';
        return 1;
    }
}
