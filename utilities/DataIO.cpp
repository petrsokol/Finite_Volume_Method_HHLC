//
// Created by petrs on 19.04.2024.
//

#include <ctime>
#include <fstream>
#include <chrono>
#include "DataIO.h"
#include "../fluid_dynamics/Def.h"
#include "../structures/Primitive.h"
#include "../fluid_dynamics/NACA.h"

std::string DataIO::getDate() {
    // Get the current time
    std::time_t currentTime = std::time(nullptr);

    // Convert the current time to a local time
    std::tm* localTime = std::localtime(&currentTime);

    // Extract the date components
    int year = localTime->tm_year % 100; // tm_year counts years since 1900
    int month = localTime->tm_mon + 1;    // tm_mon counts months from 0
    int day = localTime->tm_mday;
    std::string leadZeroMonth = (month < 10) ? "0" : "";
    std::string leadZeroDay = (day < 10) ? "0" : "";
    return std::to_string(year) + "_" + leadZeroMonth + std::to_string(month) + "_" + leadZeroDay + std::to_string(day);
}


std::string DataIO::getTime() {
    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert current time to tm struct for easy manipulation
    std::tm *timeInfo = std::localtime(&currentTime);

    // Extract hours and minutes
    int hours = timeInfo->tm_hour;
    int minutes = timeInfo->tm_min;

    // Create a string stream to build the formatted time string
    std::ostringstream formattedTime;
    formattedTime << std::setfill('0') << std::setw(2) << hours << "h" << std::setw(2) << minutes << "m";

    // Return the formatted time string
    return formattedTime.str();
}

void DataIO::exportToCSV(const std::unordered_map<int, Cell> &cells, const std::string& dir, const std::string& name, int reps) {
    std::ofstream stream(dir + "\\" + name + "_" + getTime() + "_" + std::to_string(reps) + ".csv");
    stream << "\"X\", \"Y\", \"Z\", \"MACH_NUMBER\", \"PRESSURE\"\n";

    for (int i = 0; i < Def::inner; ++i) {
        int k = Def::innerIndex(i);
        Primitive pv = Primitive::computePV(cells.at(k).w);
        double mach = pv.U / pv.c;
        stream << cells.at(k).tx << ", " << cells.at(k).ty << ", 1, " << mach << ", " << pv.p << "\n";
    } stream.close();
}


void DataIO::exportToDAT(const std::unordered_map<int, Cell> &cells, const std::string &dir, const std::string &name, int reps) {
    std::ofstream stream(dir + "\\" + name + "_" + getTime() + "_" + std::to_string(reps) + ".dat");

    int upperBound = Def::isNaca ? NACA::wingLength : Def::xInner;
    int offset = Def::isNaca ? Def::firstInner + NACA::wingStart : Def::firstInner;

    for (int i = 0; i < upperBound; ++i) {
        int k = offset + i;
        Primitive pv = Primitive::computePV(cells.at(k).w);
        double mach = pv.U / pv.c;
        stream << cells.at(k).tx << " " << cells.at(k).ty << " " << mach << " " << pv.p << "\n";
    }

    stream.close();
}

void DataIO::exportPointsToCSV(const std::unordered_map<int, Cell> &cells, std::vector<Point> &points,
                               const std::string &dir, const std::string &name, int reps,
                               const std::string &time) {
    DataIO::updatePointValues(cells, points);
    std::ofstream stream(dir + "\\" + name + "_" + time + "_" + std::to_string(reps) + ".csv");

    stream << "\"X\", \"Y\", \"Z\", \"MACH_NUMBER\", \"PRESSURE\"\n";
    // values to cell vertices
    int innerVertices = (Def::xInner + 1) * (Def::yInner + 1);
    for (int i = 0; i < innerVertices; ++i) {
        int k = Def::innerPointIndex(i);
        stream << points[k].x << ", " << points[k].y << ", 1";

        for (int j = 0; j < points[k].values.size(); ++j) {
            stream << ", " << points[k].values[j];
        }

        stream << std::endl;
    }
    stream.close();
}

void
DataIO::exportPointsToDat(const std::unordered_map<int, Cell> &cells, std::vector<Point> &points,
                          const std::string &dir,
                          const std::string &name, const std::string &time, int reps) {
    DataIO::updatePointValues(cells, points);
    std::ofstream stream(dir + "\\" + name + "_" + time + "_" + std::to_string(reps) + ".dat");

    int upperBound = Def::isNaca ? NACA::wingLength + 1 : Def::xInner + 1; // +1 -> vertices in row = cells in row + 1
    int offset = Def::isNaca ? Def::firstInner + NACA::wingStart : Def::firstInner;

    for (int i = 0; i < upperBound; ++i) {
        int k = offset + i;
        stream << points[k].x << " " << points[k].y << " 1";

        for (int j = 0; j < points[k].values.size(); ++j) {
            stream << " " << points[k].values[j];
        }

        stream << std::endl;
    }

    stream.close();
}

void DataIO::exportVectorToDat(const std::vector<double> vector, const std::string &dir, const std::string &name,
                               const std::string &time) {
    std::ofstream stream(dir + "\\" + name + "_" + time + "_" + ".dat");

    int vectorSize = vector.size();
    for (int i = 0; i < vectorSize; ++i) {
        stream << i << " " << vector[i] << std::endl;
    } stream.close();
}

void DataIO::updatePointValues(const std::unordered_map<int, Cell> &cells, std::vector<Point> &points) {
    // loop over all inner Cells
    for (int i = 0; i < Def::inner; ++i) {
        int k = Def::innerIndex(i);
        Primitive pv = Primitive::computePV(cells.at(k).w);
        double mach = pv.U / pv.c;

        // bottom left corner
        points.at(k).values[0] += mach;
        points.at(k).values[1] += pv.p;
        points.at(k).contributors++;

        // bottom right corner
        points.at(k + 1).values[0] += mach;
        points.at(k + 1).values[1] += pv.p;
        points.at(k + 1).contributors++;

        // top left corner
        points.at(k + Def::xCells).values[0] += mach;
        points.at(k + Def::xCells).values[1] += pv.p;
        points.at(k + Def::xCells).contributors++;

        // top right corner
        points.at(k + Def::xCells + 1).values[0] += mach;
        points.at(k + Def::xCells + 1).values[1] += pv.p;
        points.at(k + Def::xCells + 1).contributors++;
    }

    // averaging values based on number of contributors
    int pointCount = points.size();
    for (int i = 0; i < pointCount; ++i) {
        if (points[i].contributors == 0) {
            continue;
        } else {
            // prepared for any number of values
            for (int j = 0; j < points[i].values.size(); ++j) {
                points[i].values[j] /= points[i].contributors;
            }
        }
    }
}

