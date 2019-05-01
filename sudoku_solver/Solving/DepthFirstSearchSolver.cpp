//
//  DepthFirstSearchSolver.cpp
//  sudoku_solver
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "DepthFirstSearchSolver.hpp"

#include "ConstraintSolver.hpp"

#include <iostream>

DepthFirstSearchSolver::DepthFirstSearchSolver(Grid& g) : _grid(g) {}

void DepthFirstSearchSolver::_pushChildrenOfState(const Grid& state, GridStack& gridStack) {
    const int gridSize = state.getSize();

    const int nextCellIndex = state.getCellIndexWithFewestCandidates();
    const Cell& nextCell = state.cellAtIndex(nextCellIndex);

    const IntSet& candidates = nextCell.getCandidates();
    for (auto candidate = candidates.begin(); candidate != candidates.end(); ++ candidate) {
        Grid child = state;
        child.cellAtIndex(nextCellIndex).setValue(*candidate, gridSize);
        ConstraintSolver(child).propagateContraints();
        gridStack.push(child);
        if (child.isSolved()) {
            break;
        }
    }
}

Grid DepthFirstSearchSolver::search() {
    Grid result = _grid;

    GridStack gridStack;
    gridStack.push(_grid);

    while (!gridStack.empty()) {
        Grid currentGrid = gridStack.top();
        gridStack.pop();
        if (currentGrid.isSolved()) {
            result = currentGrid;
            break;
        }
        if (currentGrid.isValid()) {
            _pushChildrenOfState(currentGrid, gridStack);
        }
    }

    return result;
}
