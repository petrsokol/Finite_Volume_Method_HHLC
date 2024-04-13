//
// Created by petrs on 19.12.2023.
//

#ifndef GAMM_NACA_H
#define GAMM_NACA_H


#include <unordered_map>
#include "../geometry/Cell.h"
#include "../geometry/Interface.h"

class NACA {

public:
    static constexpr int wingStart = 30; //off by one, for šur -?
    static constexpr int wingLength = 200; //off by one, for šur -?

    static void updateInlet(std::unordered_map<int, Cell>& cells);

    static void updateOutlet(std::unordered_map<int, Cell>& cells);

    static void updateWalls(std::unordered_map<int, Cell>& cells, const std::unordered_map<std::pair<int, int>, Interface, pair_hash>& faces);

    static void updatePeriodicity(std::unordered_map<int, Cell>& cells);

    static void updateBounds(std::unordered_map<int, Cell>& cells, const std::unordered_map<std::pair<int, int>, Interface, pair_hash>& faces);

};


#endif //GAMM_NACA_H
