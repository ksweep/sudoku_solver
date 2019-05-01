//
//  Grid.cpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "Grid.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

static const int kDefaultSize = 9;
static const int kDefaultSubSize = 3;

static bool isPerfectSquare(const int n) {
    if (n < 0) {
        return false;
    }
    const int root = round(sqrt(n));
    return n == root * root;
}

Grid::Grid() {
    _cells.resize(kDefaultSize * kDefaultSize);
    _size = kDefaultSize;
    _subSize = kDefaultSubSize;
    _initializeCommonIndicesMaps();
}

Grid::Grid(const int s) {
    if (isPerfectSquare(s)) {
        _cells.resize(s * s);
        _size = s;
        _subSize = round(sqrt(s));
    } else {
        std::cout << "invalid size: " << s << " is not perfect square" << std::endl;
        _cells.resize(kDefaultSize * kDefaultSize);
        _size = kDefaultSize;
        _subSize = kDefaultSubSize;
    }
    _initializeCommonIndicesMaps();
}

void Grid::_initFromFile(const std::string filename) {
    bool error = false;
    std::ifstream myfile(filename);
    if (myfile.is_open()) {
        std::string line;
        int lineLength = -1;
        int totalLines = 0;
        while (getline(myfile, line)) {
            if (lineLength < 0) {
                lineLength = (int)line.length();
                if (isPerfectSquare(lineLength)) {
                    _size = lineLength;
                    _subSize = round(sqrt(lineLength));
                } else {
                    error = true;
                    break;
                }
            } else if (line.length() != lineLength) {
                std::cout << "invalid input line length: " << line.length() << " but expect " << lineLength << std::endl;
                error = true;
                break;
            }
            for (char& c : line) {
                if (c >= '1' && c <= '9') {
                    int intValue = c - '0';
                    _cells.push_back(Cell(intValue));
                } else if (c >= 'a' && c <= 'g') {
                    int intValue = c - 'a' + 10;
                    _cells.push_back(Cell(intValue));
                } else {
                    _cells.push_back(Cell());
                }
            }
            totalLines += 1;
        }
        myfile.close();
        if (totalLines != lineLength) {
            std::cout << "invalid number of input lines: " << totalLines << " but expect " << lineLength << std::endl;
            error = true;
        }
    } else {
        std::cout << "unable to open file " << filename << std::endl;
        error = true;
    }
    if (error) {
        _cells.clear();
        _cells.resize(kDefaultSize * kDefaultSize);
        _size = kDefaultSize;
        _subSize = kDefaultSubSize;
    }
}

Grid::Grid(const std::string filename) {
    _initFromFile(filename);
    _initializeCommonIndicesMaps();
}

int Grid::getSize() const {
    return _size;
}

int Grid::getSubSize() const {
    return _subSize;
}

Cell& Grid::cellAtIndex(const int index) {
    return _cells[index];
}

const Cell& Grid::cellAtIndex(const int index) const {
    return _cells[index];
}

#pragma mark - Check if valid

// given a grid row, checks that there are no conflicts in the row (empty cells are allowed)
bool Grid::_rowIsValid(const int rowIndex) const {
    const int startIndex = rowIndex * _size;
    IntSet seenValues;
    for (int currentIndex = startIndex, counter = 0; counter < _size; currentIndex++, counter++) {
        const Cell& cell = _cells[currentIndex];
        if (cell.getValue() != -1) {
            if (seenValues.find(cell.getValue()) != seenValues.end()) {
                return false;
            }
            seenValues.insert(cell.getValue());
        }
    }
    return true;
}

// given a grid column, checks that there are no conflicts in the column (empty cells are allowed)
bool Grid::_columnIsValid(const int columnIndex) const {
    const int startIndex = columnIndex;
    IntSet seenValues;
    for (int currentIndex = startIndex, counter = 0; counter < _size; currentIndex += _size, counter++) {
        const Cell& cell = _cells[currentIndex];
        if (cell.getValue() != -1) {
            if (seenValues.find(cell.getValue()) != seenValues.end()) {
                return false;
            }
            seenValues.insert(cell.getValue());
        }
    }
    return true;
}

// given a grid subgrid, checks that there are no conflicts in the subgrid (empty cells are allowed)
bool Grid::_subgridIsValid(const int startRow, const int startColumn) const {
    IntSet seenValues;
    for (int row = startRow, rowCounter = 0; rowCounter < _subSize; row++, rowCounter++) {
        for (int col = startColumn, colCounter = 0; colCounter < _subSize; col++, colCounter++) {
            int currentIndex = indexAtRowAndColumn(row, col);
            const Cell& cell = _cells[currentIndex];
            if (cell.getValue() != -1) {
                if (seenValues.find(cell.getValue()) != seenValues.end()) {
                    return false;
                }
                seenValues.insert(cell.getValue());
            }
        }
    }
    return true;
}

bool Grid::_allRowsAreValid() const {
    for (int row = 0; row < _size; row++) {
        if (!_rowIsValid(row)) {
            return false;
        }
    }
    return true;
}

bool Grid::_allColumnsAreValid() const {
    for (int col = 0; col < _size; col++) {
        if (!_columnIsValid(col)) {
            return false;
        }
    }
    return true;
}

bool Grid::_allSubgridsAreValid() const {
    for (int row = 0; row < _size; row += _subSize) {
        for (int col = 0; col < _size; col += _subSize) {
            if (!_subgridIsValid(row, col)) {
                return false;
            }
        }
    }
    return true;
}

bool Grid::isValid() const {
    return _allRowsAreValid() && _allColumnsAreValid() && _allSubgridsAreValid();
}

#pragma mark - Check if solved

IntSet Grid::allCandidates() const {
    static IntSet result;
    static bool initialized = false;
    if (!initialized) {
        for (int i = 1; i <= _size; i++) {
            result.insert(i);
        }
        initialized = true;
    }
    return result;
}

// given a grid row, checks that each cell has a unique value
bool Grid::_rowIsSolved(const int rowIndex) const {
    const int startIndex = rowIndex * _size;
    IntSet remainingValues = allCandidates();
    for (int currentIndex = startIndex, counter = 0; counter < _size; currentIndex++, counter++) {
        const Cell& cell = _cells[currentIndex];
        if (cell.getValue() == -1) {
            return false;
        }
        remainingValues.erase(cell.getValue());
    }
    return remainingValues.size() == 0;
}

// given a grid column, checks that each cell has a unique value
bool Grid::_columnIsSolved(const int columnIndex) const {
    const int startIndex = columnIndex;
    IntSet remainingValues = allCandidates();
    for (int currentIndex = startIndex, counter = 0; counter < _size; currentIndex += _size, counter++) {
        const Cell& cell = _cells[currentIndex];
        if (cell.getValue() == -1) {
            return false;
        }
        remainingValues.erase(cell.getValue());
    }
    return remainingValues.size() == 0;
}

// given a grid subgrid, checks that each cell has a unique value
bool Grid::_subgridIsSolved(const int startRow, const int startColumn) const {
    IntSet remainingValues = allCandidates();
    for (int row = startRow, rowCounter = 0; rowCounter < _subSize; row++, rowCounter++) {
        for (int col = startColumn, colCounter = 0; colCounter < _subSize; col++, colCounter++) {
            int currentIndex = indexAtRowAndColumn(row, col);
            const Cell& cell = _cells[currentIndex];
            if (cell.getValue() == -1) {
                return false;
            }
            remainingValues.erase(cell.getValue());
        }
    }
    return remainingValues.size() == 0;
}

bool Grid::_allRowsSolved() const {
    for (int row = 0; row < _size; row++) {
        if (!_rowIsSolved(row)) {
            return false;
        }
    }
    return true;
}

bool Grid::_allColumnsSolved() const {
    for (int col = 0; col < _size; col++) {
        if (!_columnIsSolved(col)) {
            return false;
        }
    }
    return true;
}

bool Grid::_allSubgridsSolved() const {
    for (int row = 0; row < _size; row += _subSize) {
        for (int col = 0; col < _size; col += _subSize) {
            if (!_subgridIsSolved(row, col)) {
                return false;
            }
        }
    }
    return true;
}

bool Grid::isSolved() const {
    return _allRowsSolved() && _allColumnsSolved() && _allSubgridsSolved();
}

#pragma mark - Test if indices are in same group

bool Grid::indicesAreInSameRow(const IntSet& indices) const {
    int rowIndex = -1;
    for (auto index = indices.begin(); index != indices.end(); ++index) {
        const int currentRowIndex = rowOfCellIndex(*index);
        if (rowIndex == -1) {
            rowIndex = currentRowIndex;
        } else if (rowIndex != currentRowIndex) {
            return false;
        }
    }
    return true;
}

bool Grid::indicesAreInSameColumn(const IntSet& indices) const {
    int columnIndex = -1;
    for (auto index = indices.begin(); index != indices.end(); ++index) {
        const int currentColumnIndex = columnOfCellIndex(*index);
        if (columnIndex == -1) {
            columnIndex = currentColumnIndex;
        } else if (columnIndex != currentColumnIndex) {
            return false;
        }
    }
    return true;
}

bool Grid::indicesAreInSameSubgrid(const IntSet& indices) const {
    int subgridRowIndex = -1;
    int subgridColumnIndex = -1;
    for (auto index = indices.begin(); index != indices.end(); ++index) {
        const int currentRowIndex = rowOfCellIndex(*index);
        const int currentColumnIndex = columnOfCellIndex(*index);
        const int currentSubgridRow = currentRowIndex / _subSize;
        const int currentSubgridColumn = currentColumnIndex / _subSize;
        if (subgridRowIndex == -1) {
            subgridRowIndex = currentSubgridRow;
            subgridColumnIndex = currentSubgridColumn;
        } else if (subgridRowIndex != currentSubgridRow || subgridColumnIndex != currentSubgridColumn) {
            return false;
        }
    }
    return true;
}

#pragma mark - Common group index getters

static IntSet buildCommonRowIndicesOfCellAtIndex(const int cellIndex, const int gridSize) {
    const int rowIndex = cellIndex / gridSize;
    const int startIndex = rowIndex * gridSize;
    IntSet indices;
    for (int currentIndex = startIndex, counter = 0; counter < gridSize; currentIndex++, counter++) {
        indices.insert(currentIndex);
    }
    return indices;
}

static IntSet buildCommonColumnIndicesOfCellAtIndex(const int cellIndex, const int gridSize) {
    const int columnIndex = cellIndex % gridSize;
    IntSet indices;
    for (int currentIndex = columnIndex, counter = 0; counter < gridSize; currentIndex += gridSize, counter++) {
        indices.insert(currentIndex);
    }
    return indices;
}

static IntSet buildCommonSubgridIndicesOfCellAtIndex(const int cellIndex, const int gridSize, const int subgridSize) {
    const int cellRowIndex = cellIndex / gridSize;
    const int cellColumnIndex = cellIndex % gridSize;
    const int startRow = (cellRowIndex / subgridSize) * subgridSize;
    const int startColumn = (cellColumnIndex / subgridSize) * subgridSize;
    IntSet indices;
    for (int row = startRow, rowCounter = 0; rowCounter < subgridSize; row++, rowCounter++) {
        for (int col = startColumn, colCounter = 0; colCounter < subgridSize; col++, colCounter++) {
            int currentIndex = row * gridSize + col;
            indices.insert(currentIndex);
        }
    }
    return indices;
}

void Grid::_initializeCommonIndicesMaps() {
    for (int row = 0; row < _size; row++) {
        _rowIndexToRowIndices[row] = buildCommonRowIndicesOfCellAtIndex(row * _size, _size);
    }
    for (int col = 0; col < _size; col++) {
        _columnIndexToColumnIndices[col] = buildCommonColumnIndicesOfCellAtIndex(col, _size);
    }
    for (int row = 0; row < _size; row += _subSize) {
        for (int col = 0; col < _size; col += _subSize) {
            const int subgridRow = row / _subSize;
            const int subgridColumn = col / _subSize;
            const int index = subgridRow * _subSize + subgridColumn;
            const int cellIndex = indexAtRowAndColumn(row, col);
            _subgridIndexToSubgridIndices[index] = buildCommonSubgridIndicesOfCellAtIndex(cellIndex, _size, _subSize);
        }
    }
}

IntSet Grid::commonRowIndicesOfCellAtIndex(const int cellIndex) const {
    return _rowIndexToRowIndices.at(rowOfCellIndex(cellIndex));
}

IntSet Grid::commonColumnIndicesOfCellAtIndex(const int cellIndex) const {
    return _columnIndexToColumnIndices.at(columnOfCellIndex(cellIndex));
}

IntSet Grid::commonSubgridIndicesOfCellAtIndex(const int cellIndex) const {
    const int cellRowIndex = rowOfCellIndex(cellIndex);
    const int cellColumnIndex = columnOfCellIndex(cellIndex);
    const int subgridIndex = subgridIndexAtRowAndColumn(cellRowIndex, cellColumnIndex);
    return _subgridIndexToSubgridIndices.at(subgridIndex);
}

#pragma mark - Row and column indices from cell index set

IntSet Grid::rowSetOfCellIndices(const IntSet& indices) const {
    IntSet result = IntSet();
    for (auto cellIndex = indices.begin(); cellIndex != indices.end(); ++cellIndex) {
        const int currentRow = rowOfCellIndex(*cellIndex);
        result.insert(currentRow);
    }
    return result;
}

IntSet Grid::columnSetOfCellIndices(const IntSet& indices) const {
    IntSet result = IntSet();
    for (auto cellIndex = indices.begin(); cellIndex != indices.end(); ++cellIndex) {
        const int currentColumn = columnOfCellIndex(*cellIndex);
        result.insert(currentColumn);
    }
    return result;
}

#pragma mark -

IntToIntSetMap Grid::_initialCandidateListMap() const {
    static IntToIntSetMap result;
    static bool initialized = false;
    if (!initialized) {
        int gridSize = _size;
        for (int i = 1; i <= gridSize; i++) {
            result[i] = IntSet();
        }
        initialized = true;
    }
    return result;
}

IntToIntSetMap Grid::getCandidateCellIndexListsFromIndices(const IntSet& indices) const {
    IntToIntSetMap result = _initialCandidateListMap();
    for (auto currentIndex = indices.begin(); currentIndex != indices.end(); ++currentIndex) {
        const Cell& currentCell = _cells[*currentIndex];
        const IntSet& candidates = currentCell.getCandidates();
        for (auto candidate = candidates.begin(); candidate != candidates.end(); ++candidate) {
            IntSet& currentIndices = result[*candidate];
            currentIndices.insert(*currentIndex);
        }
    }
    return result;
}

#pragma mark - Printing

IntToStringMap Grid::_valuetoPrintValue() const {
    static IntToStringMap result;
    static bool initialized = false;
    if (!initialized) {
        // currently enough for 5x5 grids
        result[1] = "1";
        result[2] = "2";
        result[3] = "3";
        result[4] = "4";
        result[5] = "5";
        result[6] = "6";
        result[7] = "7";
        result[8] = "8";
        result[9] = "9";
        result[10] = "A";
        result[11] = "B";
        result[12] = "C";
        result[13] = "D";
        result[14] = "E";
        result[15] = "F";
        result[16] = "G";
        result[17] = "H";
        result[18] = "I";
        result[19] = "J";
        result[20] = "K";
        result[21] = "L";
        result[22] = "M";
        result[23] = "N";
        result[24] = "O";
        result[25] = "P";
        initialized = true;
    }
    return result;
}

std::string Grid::prettyPrint(const bool printSeparators) const {
    IntToStringMap map = _valuetoPrintValue();
    std::string result = "";
    for (int row = 0; row < _size; row++) {
        for (int col = 0; col < _size; col++) {
            if (printSeparators && col > 0 && col % _subSize == 0) {
                result += "|";
            }
            const Cell& cell = _cells[indexAtRowAndColumn(row, col)];
            if (cell.getValue() == -1) {
                result += ".";
            } else {
                result += map[cell.getValue()];
            }
        }
        result += "\n";
        if (printSeparators && row > 0 && row < _size - 1 && row % _subSize == _subSize - 1) {
            for (int i = 0; i < _size + _subSize - 1; i++) {
                if (i % (_subSize + 1) - _subSize == 0) {
                    result += "+";
                } else {
                    result += "-";
                }
            }
            result += "\n";
        }
    }
    return result;
}

#pragma mark -

int Grid::getNumberOfUnansweredCellsInIndices(const IntSet& indices) const {
    int result = 0;
    for (auto currentIndex = indices.begin(); currentIndex != indices.end(); ++currentIndex) {
        const Cell& currentCell = _cells[*currentIndex];
        if (currentCell.getValue() == -1) {
            result += 1;
        }
    }
    return result;
}

int Grid::getCellIndexWithFewestCandidates() const {
    const int cellCount = _size * _size;
    int minSize = _size;
    int result = -1;
    for (int i = 0; i < cellCount; i++) {
        const Cell& cell = _cells[i];
        if (cell.getValue() == -1) {
            const int candidatesSize = (int)cell.getCandidates().size();
            if (candidatesSize < minSize) {
                minSize = candidatesSize;
                result = i;
            }
        }
    }
    return result;
}
