//
//  main.cpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include <iostream>
#include <chrono>

#include "Grid.hpp"
#include "Solver.hpp"

int main(int argc, const char * argv[]) {
    Grid grid = Grid("hard2.txt");

    std::cout << "INITIAL GRID" << std::endl << std::endl;
    std::cout << grid.prettyPrint(true) << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    Solver(grid).solve();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    std::cout << "FINAL GRID" << std::endl << std::endl;
    std::cout << grid.prettyPrint(true) << std::endl;

    std::cout << "Elapsed time: " << elapsed.count() << " s" <<std::endl;

    return 0;
}
