//
//  ConstraintSolver.cpp
//  sudoku_solver
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "ConstraintSolver.hpp"

#include "CombinationListCreator.hpp"
#include "DepthFirstSearchSolver.hpp"

#include <iostream>

ConstraintSolver::ConstraintSolver(Grid& g) : _grid(g), _editor(g) {}

#pragma mark - One possible value in cell

bool ConstraintSolver::_updateCellsWithOneCandidate() {
    bool anyCellUpdated = false;
    for (int cellIndex = 0; cellIndex < _grid.numberOfCells(); cellIndex++) {
        Cell& cell = _grid.cellAtIndex(cellIndex);
        if (cell.getValue() == -1 && cell.getCandidates().size() == 1) {
            int value = *cell.getCandidates().begin();
            _editor.setCellValueAndUpdateCandidates(cell, value, cellIndex);
            anyCellUpdated = true;
        }
    }
    return anyCellUpdated;
}

#pragma mark - Subgroup Exclusion

/**
 If a candidate c only occurs in a set of n cells N of group A and those cells are also in the same group B (A != B),
 then remove c from all cells in B that aren't in N.

 c: 1 to grid._size
 n: 2 to grid._subSize
 */

bool ConstraintSolver::_processSubgroupExclusion(const IntSet& groupIndices, const bool isRowOrColumn) {
    bool result = false;
    int gridSize = _grid.getSize();
    int subgridSize = _grid.getSubSize();
    IntToIntSetMap candidateCellLists = _grid.getCandidateCellIndexListsFromIndices(groupIndices);
    for (int currentValue = 1; currentValue <= gridSize; currentValue++) {
        for (int currentValueCount = 2; currentValueCount <= subgridSize; currentValueCount++) {
            const IntSet& currentValueIndices = candidateCellLists[currentValue];
            if (currentValueIndices.size() == currentValueCount) {
                if (isRowOrColumn) {
                    if (_grid.indicesAreInSameSubgrid(currentValueIndices)) {
                        const int cellIndex = *currentValueIndices.begin(); // just use first index since it's used to determine subgrid
                        const bool anyRemoved = _editor.removeCandidateFromSubgridOfCellIndexExcluding(currentValue, groupIndices, cellIndex);
                        result = result || anyRemoved;
                    }
                } else {
                    if (_grid.indicesAreInSameRow(currentValueIndices)) {
                        const int cellIndex = *currentValueIndices.begin(); // just use first index since it's used to determine row
                        const bool anyRemoved = _editor.removeCandidateFromRowOfCellIndexExcluding(currentValue, groupIndices, cellIndex);
                        result = result || anyRemoved;
                    } else if (_grid.indicesAreInSameColumn(currentValueIndices)) {
                        const int cellIndex = *currentValueIndices.begin(); // just use first index since it's used to determine column
                        const bool anyRemoved = _editor.removeCandidateFromColumnOfCellIndexExcluding(currentValue, groupIndices, cellIndex);
                        result = result || anyRemoved;
                    }
                }
            }
        }
    }
    return result;
}

bool ConstraintSolver::_filterCandidatesUsingSubgroupExclusion() {
    bool result = false;

    const int gridSize = _grid.getSize();
    const int subgridSize = _grid.getSubSize();

    for (int row = 0; row < gridSize; row++) {
        IntSet rowIndices = _grid.commonRowIndicesOfCellAtIndex(row * gridSize);
        const bool rowResult = _processSubgroupExclusion(rowIndices, true);
        result = result || rowResult;
    }

    for (int col = 0; col < gridSize; col++) {
        IntSet columnIndices = _grid.commonColumnIndicesOfCellAtIndex(col);
        const bool columnResult = _processSubgroupExclusion(columnIndices, true);
        result = result || columnResult;
    }

    for (int startRow = 0; startRow < gridSize; startRow += subgridSize) {
        for (int startColumn = 0; startColumn < gridSize; startColumn += subgridSize) {
            IntSet subgridIndices = _grid.commonSubgridIndicesOfCellAtIndex(startRow * gridSize + startColumn);
            const bool subgridResult = _processSubgroupExclusion(subgridIndices, false);
            result = result || subgridResult;
        }
    }

    return result;
}

#pragma mark - Chains

/**
 For a chain of size X:
 - get all candidates that are in 1-X cells (i.e. cell_list.size() for the value is >= 1 && <= X)

 Example from https://www.learn-sudoku.com/hidden-triplets.html:
 2: {3, 4, 6}
 3: {2, 8}
 6: {3, 4, 6}
 8: {3, 4}

 Here, X = 3. Y, the number of candidates that match the description, is 4.

 Try all (Y choose X) index combinations, seeing if the union of the X sets are the same X cells

 The index combination from the example above would be [0, 2, 3], corresponding to {2, 6, 8}. The union of their 3 sets is {3, 4, 6}.

 You can now remove all other values from cells [0, 2, 3] that aren't {2, 6, 8}.
 If these cells also fall into the same group of another type (e.g. we were testing subgrid and these 3 all all in the same row or column), you can remove {2, 6, 8} from the other cells of the second group.
 */

static IntVector _getCandidatesWithCountsUpToCount(Grid& grid, IntToIntSetMap& candidateCellLists, const int count) {
    IntVector result;
    const int gridSize = grid.getSize();
    for (int currentValue = 1; currentValue <= gridSize; currentValue++) {
        const int cellIndexListSize = (int)candidateCellLists[currentValue].size();
        if (cellIndexListSize >= 1 && cellIndexListSize <= count) {
            result.push_back(currentValue);
        }
    }
    return result;
}

bool ConstraintSolver::_processChains(const IntSet& groupIndices, const bool isRowOrColumn) {
    bool result = false;
    int maxChainSize = _grid.getNumberOfUnansweredCellsInIndices(groupIndices) - 1;
    IntToIntSetMap candidateCellLists = _grid.getCandidateCellIndexListsFromIndices(groupIndices);
    for (int currentChainSize = 1; currentChainSize <= maxChainSize; currentChainSize++) {
        IntVector candidatesThatAreInAtMostChainSizeCells = _getCandidatesWithCountsUpToCount(_grid, candidateCellLists, currentChainSize);
        if (candidatesThatAreInAtMostChainSizeCells.size() >= currentChainSize) {
            const IntVectorVector indexCombinationList = CombinationListCreator::makeCombinationList((int)candidatesThatAreInAtMostChainSizeCells.size(), currentChainSize);
            for (auto indexCombination = indexCombinationList.begin(); indexCombination != indexCombinationList.end(); ++indexCombination) {
                IntSet cellIndexSet = IntSet();
                IntSet chainValueSet = IntSet();
                for (auto currentTupleIndex = indexCombination->begin(); currentTupleIndex != indexCombination->end(); ++currentTupleIndex) {
                    const int currentCandidate = candidatesThatAreInAtMostChainSizeCells[*currentTupleIndex];
                    IntSet candidateCellList = candidateCellLists[currentCandidate];
                    chainValueSet.insert(currentCandidate);
                    cellIndexSet.insert(candidateCellList.begin(), candidateCellList.end());
                }

                if (cellIndexSet.size() == currentChainSize) {
                    // remove all other values from cells in cellIndexSet that aren't in chainValueSet
                    bool anyUpdated = _editor.removeCandidatesFromIndicesThatAreNotInCandidateSet(cellIndexSet, chainValueSet);
                    result = result || anyUpdated;
                    if (isRowOrColumn) {
                        if (_grid.indicesAreInSameSubgrid(cellIndexSet)) {
                            const int cellIndex = *cellIndexSet.begin();
                            const IntSet subgridIndices = _grid.commonSubgridIndicesOfCellAtIndex(cellIndex);
                            const bool anyRemoved = _editor.removeCandidatesFromIndicesExcludingIndices(chainValueSet, subgridIndices, cellIndexSet);
                            result = result || anyRemoved;
                        }
                    } else {
                        if (_grid.indicesAreInSameRow(cellIndexSet)) {
                            const int cellIndex = *cellIndexSet.begin();
                            const IntSet rowIndices = _grid.commonRowIndicesOfCellAtIndex(cellIndex);
                            const bool anyRemoved = _editor.removeCandidatesFromIndicesExcludingIndices(chainValueSet, rowIndices, cellIndexSet);
                            result = result || anyRemoved;
                        } else if (_grid.indicesAreInSameColumn(cellIndexSet)) {
                            const int cellIndex = *cellIndexSet.begin();
                            const IntSet columnIndices = _grid.commonColumnIndicesOfCellAtIndex(cellIndex);
                            const bool anyRemoved = _editor.removeCandidatesFromIndicesExcludingIndices(chainValueSet, columnIndices, cellIndexSet);
                            result = result || anyRemoved;
                        }
                    }
                }
            }
        }
    }
    return result;
}

bool ConstraintSolver::_filterCandidatesUsingChains() {
    bool result = false;

    const int gridSize = _grid.getSize();
    const int subgridSize = _grid.getSubSize();

    for (int row = 0; row < gridSize; row++) {
        IntSet rowIndices = _grid.commonRowIndicesOfCellAtIndex(row * gridSize);
        const bool rowResult = _processChains(rowIndices, true);
        result = result || rowResult;
    }

    for (int col = 0; col < gridSize; col++) {
        IntSet columnIndices = _grid.commonColumnIndicesOfCellAtIndex(col);
        const bool columnResult = _processChains(columnIndices, true);
        result = result || columnResult;
    }

    for (int startRow = 0; startRow < gridSize; startRow += subgridSize) {
        for (int startColumn = 0; startColumn < gridSize; startColumn += subgridSize) {
            IntSet subgridIndices = _grid.commonSubgridIndicesOfCellAtIndex(startRow * gridSize + startColumn);
            const bool subgridResult = _processChains(subgridIndices, false);
            result = result || subgridResult;
        }
    }

    return result;
}

#pragma mark - Boxes

/**
 This handles "X-Wing"- and "Swordfish"-type cases covered here: http://www.sudokudragon.com/sudokustrategy.htm
 */

IntPairVector ConstraintSolver::_getCellIndexPairList(const int pairValue, const bool forRow) {
    IntPairVector cellIndexPairList;
    const int gridSize = _grid.getSize();
    for (int sizeCounter = 0; sizeCounter < gridSize; sizeCounter++) {
        IntSet indices = forRow ?  _grid.commonRowIndicesOfCellAtIndex(sizeCounter * gridSize) : _grid.commonColumnIndicesOfCellAtIndex(sizeCounter);
        IntToIntSetMap candidateCellLists = _grid.getCandidateCellIndexListsFromIndices(indices);
        IntSet cellIndexSet = candidateCellLists[pairValue];
        if (cellIndexSet.size() == 2) {
            int first = -1;
            int second = -1;
            for (auto cellIndex = cellIndexSet.begin(); cellIndex != cellIndexSet.end(); ++cellIndex) {
                if (first == -1) {
                    first = *cellIndex;
                } else {
                    second = *cellIndex;
                }
            }
            IntPair indexPair(first, second);
            cellIndexPairList.push_back(indexPair);
        }
    }
    return cellIndexPairList;
}

bool ConstraintSolver::_filterCandidatesUsingBoxes() {
    bool result = false;

    const int gridSize = _grid.getSize();
    const int subgridSize = _grid.getSubSize();

    for (int pairValue = 1; pairValue <= gridSize; pairValue++) {
        // get all pairs in each row
        IntPairVector cellIndexPairList = _getCellIndexPairList(pairValue, true);

        int cellIndexPairListSize = (int)cellIndexPairList.size();
        if (cellIndexPairListSize >= 2) {
            for (int groupSize = 2; groupSize <= subgridSize; groupSize++) {
                IntVectorVector indexCombinationList = CombinationListCreator::makeCombinationList(cellIndexPairListSize, groupSize);
                for (auto indexCombination = indexCombinationList.begin(); indexCombination != indexCombinationList.end(); ++indexCombination) {
                    IntVector& idxCombo = *indexCombination;

                    // get all the cell indices
                    IntSet cellIndexSet = IntSet();
                    for (auto index = idxCombo.begin(); index != idxCombo.end(); ++index) {
                        IntPair cellIndexPair = cellIndexPairList[*index];
                        cellIndexSet.insert(cellIndexPair.first);
                        cellIndexSet.insert(cellIndexPair.second);
                    }

                    // see if the cells only take up groupSize columns
                    IntSet columnSet = _grid.columnSetOfCellIndices(cellIndexSet);

                    // if so, remove that value from the columns
                    if (columnSet.size() == groupSize) {
                        for (auto columnIndex = columnSet.begin(); columnIndex != columnSet.end(); ++columnIndex) {
                            IntSet columnIndices = _grid.commonColumnIndicesOfCellAtIndex(*columnIndex);
                            const bool anyUpdated = _editor.removeCandidateFromIndicesExcludingIndices(pairValue, columnIndices, cellIndexSet);
                            result = result || anyUpdated;
                        }
                    }
                }
            }
        }
    }

    // copied and pasted for columns (slight modifications):
    for (int pairValue = 1; pairValue <= gridSize; pairValue++) {
        // get all pairs in each row
        IntPairVector cellIndexPairList = _getCellIndexPairList(pairValue, false);

        int cellIndexPairListSize = (int)cellIndexPairList.size();
        if (cellIndexPairListSize >= 2) {
            for (int groupSize = 2; groupSize <= cellIndexPairListSize; groupSize++) {
                IntVectorVector indexCombinationList = CombinationListCreator::makeCombinationList(cellIndexPairListSize, groupSize);
                for (auto indexCombination = indexCombinationList.begin(); indexCombination != indexCombinationList.end(); ++indexCombination) {
                    IntVector& idxCombo = *indexCombination;

                    // get all the cell indices
                    IntSet cellIndexSet = IntSet();
                    for (auto index = idxCombo.begin(); index != idxCombo.end(); ++index) {
                        IntPair cellIndexPair = cellIndexPairList[*index];
                        cellIndexSet.insert(cellIndexPair.first);
                        cellIndexSet.insert(cellIndexPair.second);
                    }

                    // see if the cells only take up groupSize columns
                    IntSet rowSet = _grid.rowSetOfCellIndices(cellIndexSet);

                    // if so, remove that value from the columns
                    if (rowSet.size() == groupSize) {
                        for (auto rowIndex = rowSet.begin(); rowIndex != rowSet.end(); ++rowIndex) {
                            IntSet rowIndices = _grid.commonRowIndicesOfCellAtIndex(*rowIndex * gridSize);
                            const bool anyUpdated = _editor.removeCandidateFromIndicesExcludingIndices(pairValue, rowIndices, cellIndexSet);
                            result = result || anyUpdated;
                        }
                    }
                }
            }
        }
    }

    return result;
}

#pragma mark - Alternate Pairs

/**
 This handles Alternate Pair Deduction covered here: http://www.sudokudragon.com/advancedstrategy.htm
 */

void ConstraintSolver::_buildPairChain(const int startIndex, const int candidateValue, IntSet& visitedIndices, IntVector& pairChain, IntToBoolMap& colorMap, IntToIntToIntSetMap& rowMap, IntToIntToIntSetMap& colMap, IntToIntToIntSetMap& subgridMap) {
    // for indexing into maps
    const int rowIndex = _grid.rowOfCellIndex(startIndex);
    const int colIndex = _grid.columnOfCellIndex(startIndex);
    const int subgridIndex = _grid.subgridIndexAtRowAndColumn(rowIndex, colIndex);

    // check for pair in current row
    const IntSet rowCandidateCellList = rowMap[rowIndex][candidateValue];
    if (rowCandidateCellList.size() == 2 && rowCandidateCellList.find(startIndex) != rowCandidateCellList.end()) {
        IntVector cellIndexPair = IntVector(rowCandidateCellList.begin(), rowCandidateCellList.end());
        const int otherCellIndex = cellIndexPair[0] == startIndex ? cellIndexPair[1] : cellIndexPair[0];
        if (visitedIndices.find(otherCellIndex) == visitedIndices.end()) {
            visitedIndices.insert(otherCellIndex);
            pairChain.push_back(otherCellIndex);
            const bool parentColor = colorMap[startIndex];
            colorMap[otherCellIndex] = !parentColor;
            _buildPairChain(otherCellIndex, candidateValue, visitedIndices, pairChain, colorMap, rowMap, colMap, subgridMap);
        }
    }

    // check for pair in current column
    const IntSet columnCandidateCellList = colMap[colIndex][candidateValue];
    if (columnCandidateCellList.size() == 2 && columnCandidateCellList.find(startIndex) != columnCandidateCellList.end()) {
        IntVector cellIndexPair = IntVector(columnCandidateCellList.begin(), columnCandidateCellList.end());
        const int otherCellIndex = cellIndexPair[0] == startIndex ? cellIndexPair[1] : cellIndexPair[0];
        if (visitedIndices.find(otherCellIndex) == visitedIndices.end()) {
            visitedIndices.insert(otherCellIndex);
            pairChain.push_back(otherCellIndex);
            const bool parentColor = colorMap[startIndex];
            colorMap[otherCellIndex] = !parentColor;
            _buildPairChain(otherCellIndex, candidateValue, visitedIndices, pairChain, colorMap, rowMap, colMap, subgridMap);
        }
    }

    // check for pair in current subgrid
    const IntSet subgridCandidateCellList = subgridMap[subgridIndex][candidateValue];
    if (subgridCandidateCellList.size() == 2 && subgridCandidateCellList.find(startIndex) != subgridCandidateCellList.end()) {
        IntVector cellIndexPair = IntVector(subgridCandidateCellList.begin(), subgridCandidateCellList.end());
        const int otherCellIndex = cellIndexPair[0] == startIndex ? cellIndexPair[1] : cellIndexPair[0];
        if (visitedIndices.find(otherCellIndex) == visitedIndices.end()) {
            visitedIndices.insert(otherCellIndex);
            pairChain.push_back(otherCellIndex);
            const bool parentColor = colorMap[startIndex];
            colorMap[otherCellIndex] = !parentColor;
            _buildPairChain(otherCellIndex, candidateValue, visitedIndices, pairChain, colorMap, rowMap, colMap, subgridMap);
        }
    }
}

// row index -> candidate -> cell list (IntSet)
static IntToIntToIntSetMap _rowMapForCandidate(Grid& grid) {
    std::unordered_map<int, IntToIntSetMap> result;
    const int gridSize = grid.getSize();
    for (int row = 0; row < gridSize; row++) {
        IntSet indices = grid.commonRowIndicesOfCellAtIndex(row * gridSize);
        result[row] = grid.getCandidateCellIndexListsFromIndices(indices);
    }
    return result;
}

// column index -> candidate -> cell list (IntSet)
static IntToIntToIntSetMap _columnMapForCandidate(Grid& grid) {
    std::unordered_map<int, IntToIntSetMap> result;
    const int gridSize = grid.getSize();
    for (int col = 0; col < gridSize; col++) {
        IntSet indices = grid.commonColumnIndicesOfCellAtIndex(col);
        result[col] = grid.getCandidateCellIndexListsFromIndices(indices);
    }
    return result;
}

// subgrid index -> candidate -> cell list (IntSet)
static IntToIntToIntSetMap _subgridMapForCandidate(Grid& grid) {
    std::unordered_map<int, IntToIntSetMap> result;
    const int gridSize = grid.getSize();
    const int subgridSize = grid.getSubSize();
    for (int row = 0; row < gridSize; row += subgridSize) {
        for (int col = 0; col < gridSize; col += subgridSize) {
            const int subgridIndex = grid.subgridIndexAtRowAndColumn(row, col);
            IntSet indices = grid.commonSubgridIndicesOfCellAtIndex(row * gridSize + col);
            result[subgridIndex] = grid.getCandidateCellIndexListsFromIndices(indices);
        }
    }
    return result;
}

bool ConstraintSolver::_filterUsingAlternatePairs() {
    bool result = false;

    IntToIntToIntSetMap rowToCandidateToCellList = _rowMapForCandidate(_grid);
    IntToIntToIntSetMap colToCandidateToCellList = _columnMapForCandidate(_grid);
    IntToIntToIntSetMap subgridToCandidateToCellList = _subgridMapForCandidate(_grid);

    const int gridSize = _grid.getSize();
    for (int candidateValue = 1; candidateValue <= gridSize; candidateValue++) {
        IntSet visitedIndices = IntSet();
        for (int cellIndex = 0; cellIndex < _grid.numberOfCells(); cellIndex++) {
            visitedIndices.insert(cellIndex);
            IntVector pairChain;
            IntToBoolMap colorMap;
            pairChain.push_back(cellIndex);
            colorMap[cellIndex] = true; // doesn't matter what start is
            _buildPairChain(cellIndex, candidateValue, visitedIndices, pairChain, colorMap, rowToCandidateToCellList, colToCandidateToCellList, subgridToCandidateToCellList);

            const int pairChainSize = (int)pairChain.size();
            if (pairChainSize > 2) {
                IntVectorVector indexCombinationList = CombinationListCreator::makeCombinationList(pairChainSize, 2);
                for (auto indexCombination = indexCombinationList.begin(); indexCombination != indexCombinationList.end(); ++indexCombination) {
                    IntVector& idxCombo = *indexCombination;
                    const int firstIndex = pairChain[idxCombo[0]];
                    const int secondIndex = pairChain[idxCombo[1]];
                    const IntSet pairSet = IntSet({firstIndex, secondIndex});
                    if (colorMap[firstIndex] != colorMap[secondIndex] && !_grid.indicesAreInSameRow(pairSet) && !_grid.indicesAreInSameColumn(pairSet)) {
                        const int rowFirst = _grid.rowOfCellIndex(firstIndex);
                        const int colFirst = _grid.columnOfCellIndex(firstIndex);
                        const int rowSecond = _grid.rowOfCellIndex(secondIndex);
                        const int colSecond = _grid.columnOfCellIndex(secondIndex);
                        const int firstIntersectionIndex = _grid.indexAtRowAndColumn(rowFirst, colSecond);
                        const int secondIntersectionIndex = _grid.indexAtRowAndColumn(rowSecond, colFirst);
                        const IntSet pairIntersectionSet = IntSet({firstIntersectionIndex, secondIntersectionIndex});
                        const bool candidateRemoved = _editor.removeCandidateFromIndices(candidateValue, pairIntersectionSet);
                        result = result || candidateRemoved;
                    }
                }
            }
        }
    }
    return result;
}

#pragma mark - Set possible values

static void _eraseCandidateIfAppropriate(Grid& grid, IntSet& indices, IntSet& candidates, const int cellIndex) {
    for (auto index = indices.begin(); index != indices.end(); ++index) {
        if (*index != cellIndex) {
            Cell& otherCell = grid.cellAtIndex(*index);
            if (otherCell.getValue() != -1) {
                candidates.erase(otherCell.getValue());
            }
        }
    }
}

void ConstraintSolver::_setCandidatesNaive() {
    for (int cellIndex = 0; cellIndex < _grid.numberOfCells(); cellIndex++) {
        Cell& cell = _grid.cellAtIndex(cellIndex);
        if (cell.getValue() == -1) {
            IntSet candidates = _grid.allCandidates();
            IntSet rowIndices = _grid.commonRowIndicesOfCellAtIndex(cellIndex);
            _eraseCandidateIfAppropriate(_grid, rowIndices, candidates, cellIndex);
            IntSet columnIndices = _grid.commonColumnIndicesOfCellAtIndex(cellIndex);
            _eraseCandidateIfAppropriate(_grid, columnIndices, candidates, cellIndex);
            IntSet subgridIndices = _grid.commonSubgridIndicesOfCellAtIndex(cellIndex);
            _eraseCandidateIfAppropriate(_grid, subgridIndices, candidates, cellIndex);
            cell.setCandidates(candidates);
        }
    }
}

void ConstraintSolver::_setCandidates() {
    _setCandidatesNaive();
    while (true) {
        const bool subgroupResult = _filterCandidatesUsingSubgroupExclusion();
        const bool chainResult = _filterCandidatesUsingChains();
        const bool boxResult = _filterCandidatesUsingBoxes();
        const bool alternatePairResult = _filterUsingAlternatePairs();
        if (!subgroupResult && !chainResult && !boxResult && !alternatePairResult) {
            break;
        }
    }
}

#pragma mark - Solve loop

void ConstraintSolver::propagateContraints() {
    _setCandidates();

    while (_updateCellsWithOneCandidate()) {
        if (_grid.isSolved()) {
            break;
        }
    }
}
