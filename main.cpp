#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "structures/Conservative.h"
#include "fluid_dynamics/Def.h"
#include "utilities/Lines.h"
#include "geometry/Point.h"
#include "geometry/Interface.h"
#include "geometry/Cell.h"
#include "fluid_dynamics/Scheme.h"
#include "fluid_dynamics/Bound.h"
#include "structures/Primitive.h"
#include "fluid_dynamics/NACA.h"

int main() {
    std::cout << "Dobry DEN!!!!" << std::endl;

    //GAMM CHANNEL GEOMETRY
    /*
    std::vector<double> x_vector(Def::xCells);
    std::vector<double> bot(Def::xCells, Def::yLowerBound);
    std::vector<double> top(Def::xCells, Def::yUpperBound);

    for (int i = 0; i < Def::xCells; ++i) { x_vector[i] = (i - Def::gl) * Def::dx;}

    Lines::arc(x_vector, bot, 1, 2, 0.1);

    std::vector<Point> points = Point::createPoints(bot, top);
     */

    //NACA 0012 GEOMETRY
    /*
    std::vector<Point> points{};
    std::ifstream input("mesh.dat");
    for (int j = 0; j < Def::NACA_Y_INNER; ++j) {
        for (int i = 0; i < Def::NACA_X_INNER; ++i) {
            double x;
            double y;
            input >> x;
            input >> y;
            points.emplace_back(x, y, j * Def::NACA_X_INNER + i);
        }
    }

    std::vector<Point> under{};
    for (int j = 0; j < Def::gl; ++j) {
        for (int i = 0; i < Def::NACA_X_INNER; ++i) {
            double dy = points[i + Def::NACA_X_INNER].y - points[i].y;
            double dx = points[i + Def::NACA_X_INNER].x - points[i].x;
            double x = points[i].x - dx * (Def::gl - j);
            double y = points[i].y - dy * (Def::gl - j);
            under.emplace_back(x, y, i);
        }
    }
    for (const auto &item: under) item.toString();

    std::vector<Point> over{};
    for (int j = 0; j < Def::gl; ++j) {
        for (int i = 0; i < Def::NACA_X_INNER; ++i) {
            double dy = points[(Def::NACA_Y_INNER - 1) * Def::NACA_X_INNER + i].y - points[(Def::NACA_Y_INNER - 2) * Def::NACA_X_INNER + i].y;
            double dx = points[(Def::NACA_Y_INNER - 1) * Def::NACA_X_INNER + i].x - points[(Def::NACA_Y_INNER - 2) * Def::NACA_X_INNER + i].x;
            double x = points[(Def::NACA_Y_INNER - 1) * Def::NACA_X_INNER + i].x + dx * (j + 1);
            double y = points[(Def::NACA_Y_INNER - 1) * Def::NACA_X_INNER + i].y + dy * (j + 1);
            over.emplace_back(x, y, i);
        }
    }
    for (const auto &item: over) item.toString();

    std::ofstream naca_points("under_mid_over.dat");
    for (const auto & point : under) {
        naca_points << point.x << " " << point.y << std::endl;
    }
    for (const auto &point: points) {
        naca_points << point.x << " " << point.y << std::endl;
    }
    for (const auto &point : over) {
        naca_points << point.x << " " << point.y << std::endl;
    }
    naca_points.close();
     */

    /*
    //LR
    std::vector<Point> lr{};
    for (int j = 0; j < Def::NACA_Y; ++j) {
        //left
        for (int i = 0; i < Def::gl; ++i) {
            int k = Def::NACA_X_INNER * j;
            double dx = points[k + 1].x - points[k].x;
            double dy = points[k + 1].y - points[k].y;
            double x = points[k].x - (Def::gl - i) * dx;
            double y = points[k].y - (Def::gl - i) * dy;
            lr.emplace_back(x, y, -1);
        }
        //right
        for (int i = 0; i < Def::gl; ++i) {
            int k = Def::NACA_X_INNER * (j + 1) - 1;
            std::cout << k << std::endl;
            double dx = points[k - 1].x - points[k].x;
            double dy = points[k - 1].y - points[k].y;
            double x = points[k].x - (i + 1) * dx;
            double y = points[k].y - (i + 1) * dy;
            lr.emplace_back(x, y, -1);
        }
    }

    std::vector<Point> allpoints{};
    for (int j = 0; j < Def::NACA_Y; ++j) {
        for (int i = 0; i < Def::gl; ++i) {
            int k = j * 2 * Def::gl + i;
            allpoints.push_back(lr[k]);
        }
        for (int i = 0; i < Def::NACA_X_INNER; ++i) {
            int k = j * Def::NACA_X_INNER + i;
            allpoints.push_back(points[k]);
        }
        for (int i = 0; i < Def::gl; ++i) {
            int k = j * 2 * Def::gl + Def::gl + i;
            allpoints.push_back(lr[k]);
        }
    }

    for (int i = 0; i < 600; ++i) {
        allpoints[i].toString();
    }

    std::ofstream out("ghostMesh.dat");
    for (const auto &point: allpoints) {
        out << point.x << " " << point.y << "\n";
    } out.close();
     */

    std::vector<Point> points{};
    std::ifstream input("ghostMesh.dat");
    for (int j = 0; j < Def::NACA_Y; ++j) {
        for (int i = 0; i < Def::NACA_X; ++i) {
            double x;
            double y;
            input >> x;
            input >> y;
            points.emplace_back(x, y, j * Def::NACA_X + i);
        }
    }

    std::unordered_map<std::pair<int, int>, Interface, pair_hash> faces = Interface::createInnerFaces(points);
    std::unordered_map<int, Cell> cells = Cell::createCells(points);


    double t = 0;
    int reps = 0;
    double rezi = 1;
    std::string NAME = "240219_subsonic";
    while (rezi > Def::EPSILON && !Def::error && reps < 30000) {
        reps++;
        /* GLOBAL TIME STEP
        double dt = Scheme::computeDT(cells, 0.5);
        std::unordered_map<int, double> deltaT = Scheme::LocalTimeStep(cells, 0.5);
        t += dt;
         */
        Scheme::updateCellDT(cells, 0.5);

        NACA::updateBounds(cells, faces);
//        Scheme::computeHLLC(cells, faces, dt); // GLOBAL TIME STEP
        Scheme::computeHLLC_localTimeStep(cells, faces);

        rezi = Scheme::computeRezi_localTimeStep(cells);
        Scheme::updateCells(cells);

        if (reps % 100 == 0) {
            std::cout << "reps: " << reps << ", rezi: " << rezi << /*", dt: " << dt <<*/ "\n";
        }

        if (reps % 1000 == 0) {
            std::ofstream outputFile(Def::defaultPath + "\\" + NAME + std::to_string(reps) + ".csv");
            outputFile << "\"X\", \"Y\", \"Z\", \"MACH_NUMBER\"\n";
            for (int i = 0; i < Def::inner; ++i) {
                int k = Def::innerIndex(i);
                Primitive pv = Primitive::computePV(cells.at(k).w);
                double mach = pv.U / pv.c;
                outputFile << cells.at(k).tx << ", " << cells.at(k).ty << ", 1, " << mach << "\n";
            } outputFile.close();
            std::cout << "timestep " << reps << " recorded successfully" << std::endl;

            std::ofstream ofstreamMach(Def::defaultPath + "\\" + NAME + "_mach_" + std::to_string(reps) + ".dat");
            std::ofstream ofstreamPressure(Def::defaultPath + "\\" + NAME + "_pressure_" + std::to_string(reps) + ".dat");
            for (int j = 0; j < NACA::wingLength; ++j) {
                int k = Def::xCells * Def::gl + Def::gl + NACA::wingStart + j;
                Primitive primitive = Primitive::computePV(cells.at(k).w);
                double localMachNumber = primitive.U / primitive.c;
                double localPressure = primitive.p;
                ofstreamMach << cells.at(k).tx << " " << localMachNumber << std::endl;
                ofstreamPressure << cells.at(k).tx << " " << localPressure << std::endl;
            }
            ofstreamMach.close();
            ofstreamPressure.close();
            std::cout << "timestep " << reps << " recorded successfully" << std::endl;
        }
    }

    std::ofstream outputStreamWing(Def::defaultPath + "\\" + NAME + std::to_string(reps) + ".dat");
//    outputStreamWing << "\"X\", \"Y\", \"Z\", \"MACH_NUMBER\"\n";
    for (int j = 0; j < NACA::wingLength; ++j) {
        int k = Def::xCells * Def::gl + Def::gl + NACA::wingStart + j;
        Primitive primitive = Primitive::computePV(cells.at(k).w);
        double localMachNumber = primitive.U / primitive.c;
        outputStreamWing << cells.at(k).tx << " " << localMachNumber << std::endl;
    }
    outputStreamWing.close();
    std::cout << "timestep " << reps << " recorded successfully" << std::endl;



    // Write data
//    std::ofstream outputFile(Def::defaultPath + "\\" + NAME + ".dat");
//    for (int i = 0; i < Def::inner; ++i) {
//        int k = Def::innerIndex(i);
//        Primitive pv = Primitive::computePV(cells.at(k).w);
//        double mach = pv.U / pv.c;
//        outputFile << cells.at(k).tx << " " << cells.at(k).ty << " " << cells.at(k).w.r1 << "\n";
//        outputFile << cells.at(k).tx << " " << cells.at(k).ty << " " << mach << "\n";
//    } outputFile.close();

//    std::ofstream outputVec(Def::defaultPath + "\\" + "mach_along_bot_8000_150x50.dat");
//    for (int i = 0; i < Def::xInner; ++i) {
//        int k = Def::firstInner + i;
//        Primitive pv = Primitive::computePV(cells.at(k).w);
//        double mach = pv.U / pv.c;
//        outputVec << cells.at(k).tx << " " << mach << "\n";
//    } outputVec.close();

    if (Def::error) {std::cout << "error at rep " << reps << std::endl;}

    std::cout << "nashledanou" << std::endl;
    return 0;
}

//todo snadný přepínání mezi režimama
//todo lepší práce s daty - program vytvoří vlastní složku, bude linkovat s pythonem apod