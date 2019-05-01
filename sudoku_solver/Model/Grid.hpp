//
//  Grid.hpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef Grid_hpp
#define Grid_hpp

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Cell.hpp"

typedef std::unordered_set<int> IntSet;
typedef std::unordered_map<int, std::unordered_set<int>> IntToIntSetMap;
typedef std::unordered_map<int, std::string> IntToStringMap;
typedef std::vector<Cell> CellVector;

class Grid {
    int _size;
    int _subSize;
    CellVector _cells;

    void _initFromFile(const std::string filename);

    bool _rowIsValid(const int rowIndex) const;
    bool _columnIsValid(const int columnIndex) const;
    bool _subgridIsValid(const int startRow, const int startColumn) const;
    bool _allColumnsAreValid() const;
    bool _allRowsAreValid() const;
    bool _allSubgridsAreValid() const;

    bool _rowIsSolved(const int rowIndex) const;
    bool _columnIsSolved(const int columnIndex) const;
    bool _subgridIsSolved(const int startRow, const int startColumn) const;
    bool _allRowsSolved() const;
    bool _allColumnsSolved() const;
    bool _allSubgridsSolved() const;

    IntToIntSetMap _initialCandidateListMap() const;

    IntToStringMap _valuetoPrintValue() const;

    void _initializeCommonIndicesMaps();
    IntToIntSetMap _rowIndexToRowIndices;
    IntToIntSetMap _columnIndexToColumnIndices;
    IntToIntSetMap _subgridIndexToSubgridIndices;
    
public:
    Grid();
    Grid(const int s);
    Grid(const std::string filename);

    int getSize() const;
    int getSubSize() const;
    Cell& cellAtIndex(const int index);
    const Cell& cellAtIndex(const int index) const;

    bool isValid() const;
    bool isSolved() const;

    bool indicesAreInSameRow(const IntSet& indices) const;
    bool indicesAreInSameColumn(const IntSet& indices) const;
    bool indicesAreInSameSubgrid(const IntSet& indices) const;

    IntSet commonRowIndicesOfCellAtIndex(const int cellIndex) const;
    IntSet commonColumnIndicesOfCellAtIndex(const int cellIndex) const;
    IntSet commonSubgridIndicesOfCellAtIndex(const int cellIndex) const;

    IntSet rowSetOfCellIndices(const IntSet& indices) const;
    IntSet columnSetOfCellIndices(const IntSet& indices) const;

    std::string prettyPrint(const bool printSeparators) const;
    IntSet allCandidates() const;

    IntToIntSetMap getCandidateCellIndexListsFromIndices(const IntSet& indices) const;

    int getNumberOfUnansweredCellsInIndices(const IntSet& indices) const;

    int getCellIndexWithFewestCandidates() const;
    
    inline int rowOfCellIndex(const int index) const {
        return index / _size;
    }
    inline int columnOfCellIndex(const int index) const {
        return index % _size;
    }
    inline int indexAtRowAndColumn(const int row, const int column) const {
        return row * _size + column;
    }
    inline int subgridIndexAtRowAndColumn(const int row, const int column) const {
        const int subgridRow = row / _subSize;
        const int subgridColumn = column / _subSize;
        return subgridRow * _subSize + subgridColumn;
    }
    inline int numberOfCells() {
        return _size * _size;
    }
};

#endif /* Grid_hpp */
