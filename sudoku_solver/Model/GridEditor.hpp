//
//  GridEditor.hpp
//  sudoku_solver
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef GridEditor_hpp
#define GridEditor_hpp

#include <stdio.h>

#include "Grid.hpp"

class GridEditor {
    Grid& _grid;
public:
    GridEditor(Grid& g);

    bool removeCandidatesFromIndicesExcludingIndices(const IntSet& candidates, const IntSet& indices, const IntSet& excludeIndices);

    bool removeCandidateFromIndices(const int candidate, const IntSet& indices);
    bool removeCandidateFromIndicesExcludingIndices(const int candidate, const IntSet& indices, const IntSet& excludeIndices);

    bool removeCandidateFromRowOfCellIndexExcluding(const int candidate, const IntSet& excludeIndices, const int cellIndex);
    bool removeCandidateFromColumnOfCellIndexExcluding(const int candidate, const IntSet& excludeIndices, const int cellIndex);
    bool removeCandidateFromSubgridOfCellIndexExcluding(const int candidate, const IntSet& excludeIndices, const int cellIndex);

    bool removeCandidatesFromIndicesThatAreNotInCandidateSet(const IntSet& indices, const IntSet& candidatesToKeep);

    void setCellValueAndUpdateCandidates(Cell& cell, const int candidate, const int cellIndex);
};

#endif /* GridEditor_hpp */
