//
//  DepthFirstSearchSolver.hpp
//  sudoku_solver
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef DepthFirstSearchSolver_hpp
#define DepthFirstSearchSolver_hpp

#include <list>
#include <stack>

#include "Grid.hpp"

typedef std::stack<Grid> GridStack;
typedef std::unordered_set<int> IntSet;

class DepthFirstSearchSolver {
    Grid& _grid;
    void _pushChildrenOfState(const Grid& state, GridStack& gridStack);

public:
    DepthFirstSearchSolver(Grid&);
    Grid search();
};


#endif /* DepthFirstSearchSolver_hpp */
