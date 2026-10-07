// ./src/Cercle.cpp
#include "Cercle.hpp"

float distanceBordABord(const Cercle& a, const Cercle& b) {
    const float distanceCentres = (b.centre - a.centre).length();
    const float ecart = distanceCentres - a.rayon - b.rayon;
    return (ecart < 0.0f) ? 0.0f : ecart;
}
