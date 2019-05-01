//
//  Cell.cpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "Cell.hpp"

Cell::Cell () {
    _value = -1;
    _candidates = IntSet();
}

Cell::Cell (const int v) {
    _value = v;
    _candidates = IntSet();
}

int Cell::getValue() const {
    return _value;
}

void Cell::setValue(const int v, const int gridSize) {
    if (v >= 1 && v <= gridSize) {
        _value = v;
        _candidates.clear();
    }
}

const IntSet& Cell::getCandidates() const {
    return _candidates;
}

void Cell::setCandidates(const IntSet c) {
    _candidates = c;
}

int Cell::eraseCandidate(const int candidate) {
    return (int)_candidates.erase(candidate);
}
