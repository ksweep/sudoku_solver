//
//  Cell.hpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef Cell_hpp
#define Cell_hpp

#include <unordered_set>

typedef std::unordered_set<int> IntSet;

class Cell {
    int _value;
    IntSet _candidates;
public:
    Cell();
    Cell(const int);

    int getValue() const;
    void setValue(const int v, const int gridSize);

    const IntSet& getCandidates() const;
    void setCandidates(const IntSet candidates);

    int eraseCandidate(const int candidate);
};

#endif /* Cell_hpp */
