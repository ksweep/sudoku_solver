//
//  Solver.hpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef Solver_hpp
#define Solver_hpp

#include "Grid.hpp"

class Solver {
    Grid& _grid;
public:
    Solver(Grid&);
    void solve();
};

#endif /* Solver_hpp */
