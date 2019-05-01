//
//  CombinationListCreator.cpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#include "CombinationListCreator.hpp"

void CombinationListCreator::_makeCombinationListUtility(IntVectorVector& result, IntVector& temp, const int n, const int left, const int k) {
    if (k == 0) {
        result.push_back(temp);
        return;
    }

    for (int i = left; i <= n; i++) {
        temp.push_back(i - 1); // subtract 1 for 0-based
        _makeCombinationListUtility(result, temp, n, i + 1, k - 1);
        temp.pop_back();
    }
}

IntVectorVector CombinationListCreator::makeCombinationList(const int n, const int k) {
    IntVectorVector result;
    IntVector temp;
    _makeCombinationListUtility(result, temp, n, 1, k);
    return result;
}
