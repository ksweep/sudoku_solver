//
//  Solver.cpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "Solver.hpp"

#include "ConstraintSolver.hpp"
#include "DepthFirstSearchSolver.hpp"

#include <iostream>

Solver::Solver(Grid& g) : _grid(g) {}

void Solver::solve() {
    if (_grid.isSolved()) {
        return;
    }

    ConstraintSolver(_grid).propagateContraints();

    if (_grid.isSolved()) {
        std::cout << "*** Solved without DFS *** " << std::endl << std::endl;
    } else {
        Grid dfsResult = DepthFirstSearchSolver(_grid).search();
        if (dfsResult.isSolved()) {
            std::cout << "*** Solved with DFS ***" << std::endl << std::endl;
            _grid = dfsResult;
        } else {
            std::cout << "*** Could NOT solve! ***" << std::endl << std::endl;
        }
    }
}

