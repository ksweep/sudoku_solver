//
//  ConstraintSolver.hpp
//  sudoku_solver
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef ConstraintSolver_hpp
#define ConstraintSolver_hpp

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Grid.hpp"
#include "GridEditor.hpp"

typedef std::unordered_set<int> IntSet;
typedef std::vector<int> IntVector;
typedef std::unordered_map<int, std::unordered_set<int>> IntToIntSetMap;
typedef std::unordered_map<int, bool> IntToBoolMap;
typedef std::pair<int, int> IntPair;
typedef std::vector<IntVector> IntVectorVector;
typedef std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> IntToIntToIntSetMap;
typedef std::vector<std::pair<int, int>> IntPairVector;

class ConstraintSolver {
    Grid& _grid;
    GridEditor _editor;
    void _setCandidates();
    void _setCandidatesNaive();
    bool _updateCellsWithOneCandidate();

    bool _filterCandidatesUsingSubgroupExclusion();
    bool _filterCandidatesUsingChains();
    bool _filterCandidatesUsingBoxes();
    bool _filterUsingAlternatePairs();

    bool _processSubgroupExclusion(const IntSet& indices, const bool isRowOrColumn);
    bool _processChains(const IntSet& indices, const bool isRowOrColumn);

    IntPairVector _getCellIndexPairList(const int pairValue, const bool forRow);

    void _buildPairChain(const int startIndex, const int candidateValue, IntSet& visitedIndices, IntVector& pairChain, IntToBoolMap& colorMap, IntToIntToIntSetMap& rowMap, IntToIntToIntSetMap& colMap, IntToIntToIntSetMap& subgridMap);

public:
    ConstraintSolver(Grid&);
    void solve();

    void propagateContraints();
};


#endif /* ConstraintSolver_hpp */
