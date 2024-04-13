//
// Created by petrs on 01.12.2023.
//

#include <cmath>
#include "Def.h"

const std::string Def::defaultPath = R"(C:\Users\petrs\Documents\CTU\BP\Charts\Data)";
const std::string Def::defaultExtension = ".dat";

const double Def::KAPPA = 1.4;
const double Def::P0 = 1;
const double Def::P2 = 0.656;
const double Def::RHO = 1;
const double Def::ALPHA_INFINITY = 1.25 * M_PI / 180;
const double Def::EPSILON = -8;

const double Def::rhoInitial = 1;
const double Def::uInitial = 0.65;
const double Def::vInitial = 0;
const double Def::pInitial = 0.75;
const double Def::rhoEInitial = pInitial / (KAPPA - 1) + 0.5 * rhoInitial * (pow(uInitial, 2) + pow(vInitial, 2));
const Conservative Def::wInitial = Conservative(rhoInitial, rhoInitial * uInitial, rhoInitial * vInitial, rhoEInitial);

//const int Def::gl = 2;

const int Def::inner = xInner * yInner;

const double Def::yLowerBound = 0;
const double Def::yUpperBound = 1;
const double Def::xLowerBound = 0;
const double Def::xUpperBound = 3;

const int Def::xCells = xInner + 2 * gl + 1;
const int Def::yCells = yInner + 2 * gl + 1;
const int Def::cells = xCells * yCells;

const int Def::firstInner = xCells*gl + gl;

const double Def::dx = (xUpperBound - xLowerBound) / xInner;

bool Def::error = false;

int Def::innerIndex(int i) {
    return firstInner + i % xInner + (i / xInner) * xCells;
}