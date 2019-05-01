//
//  GridEditor.cpp
//  sudoku_solver
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "GridEditor.hpp"

GridEditor::GridEditor(Grid& g) : _grid(g) {}

#pragma mark - Remove method
bool GridEditor::removeCandidatesFromIndicesExcludingIndices(const IntSet& candidates, const IntSet& indices, const IntSet& excludeIndices) {
    bool anyErased = false;
    for (auto index = indices.begin(); index != indices.end(); ++index) {
        if (excludeIndices.find(*index) == excludeIndices.end()) {
            Cell& cell = _grid.cellAtIndex(*index);
            for (auto candidate = candidates.begin(); candidate != candidates.end(); ++candidate) {
                const int numberErased = cell.eraseCandidate(*candidate);
                if (numberErased > 0) {
                    anyErased = true;
                }
            }
        }
    }
    return anyErased;
}

#pragma mark - Remove convenience

bool GridEditor::removeCandidateFromIndices(const int candidate, const IntSet& indices) {
    return removeCandidateFromIndicesExcludingIndices(candidate, indices, IntSet());
}

bool GridEditor::removeCandidateFromIndicesExcludingIndices(const int candidate, const IntSet& indices, const IntSet& excludeIndices) {
    return removeCandidatesFromIndicesExcludingIndices(IntSet({candidate}), indices, excludeIndices);
}

#pragma mark - Remove using cell index

bool GridEditor::removeCandidateFromRowOfCellIndexExcluding(const int candidate, const IntSet& excludeIndices, const int cellIndex) {
    IntSet rowIndices = _grid.commonRowIndicesOfCellAtIndex(cellIndex);
    return removeCandidateFromIndicesExcludingIndices(candidate, rowIndices, excludeIndices);
}

bool GridEditor::removeCandidateFromColumnOfCellIndexExcluding(const int candidate, const IntSet& excludeIndices, const int cellIndex) {
    IntSet columnIndices = _grid.commonColumnIndicesOfCellAtIndex(cellIndex);
    return removeCandidateFromIndicesExcludingIndices(candidate, columnIndices, excludeIndices);
}

bool GridEditor::removeCandidateFromSubgridOfCellIndexExcluding(const int candidate, const IntSet& excludeIndices, const int cellIndex) {
    IntSet subgridIndices = _grid.commonSubgridIndicesOfCellAtIndex(cellIndex);
    return removeCandidateFromIndicesExcludingIndices(candidate, subgridIndices, excludeIndices);
}

#pragma mark -

bool GridEditor::removeCandidatesFromIndicesThatAreNotInCandidateSet(const IntSet& indices, const IntSet& candidatesToKeep) {
    bool anyErased = false;
    const int gridSize = _grid.getSize();
    for (auto index = indices.begin(); index != indices.end(); ++index) {
        for (int candidate = 1; candidate <= gridSize; candidate++) {
            if (candidatesToKeep.find(candidate) == candidatesToKeep.end()) {
                Cell& cell = _grid.cellAtIndex(*index);
                int numberErased = cell.eraseCandidate(candidate);
                if (numberErased > 0) {
                    anyErased = true;
                }
            }
        }
    }
    return anyErased;
}

#pragma mark - Set value and update candidates

void GridEditor::setCellValueAndUpdateCandidates(Cell& cell, const int candidate, const int cellIndex) {
    cell.setValue(candidate, _grid.getSize());

    const Cell& currentCell = _grid.cellAtIndex(cellIndex);
    const int cellValue = currentCell.getValue();
    const IntSet excludeIndices = IntSet({cellIndex});

    removeCandidateFromRowOfCellIndexExcluding(cellValue, excludeIndices, cellIndex);
    removeCandidateFromColumnOfCellIndexExcluding(cellValue, excludeIndices, cellIndex);
    removeCandidateFromSubgridOfCellIndexExcluding(cellValue, excludeIndices, cellIndex);
}
