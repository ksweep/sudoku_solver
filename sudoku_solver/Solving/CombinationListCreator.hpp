//
//  CombinationListCreator.hpp
//
//  Copyright © 2019 Kevin Broom. All rights reserved.
//

#ifndef CombinationListCreator_hpp
#define CombinationListCreator_hpp

#include <vector>

typedef std::vector<int> IntVector;
typedef std::vector<IntVector> IntVectorVector;

class CombinationListCreator {
    static void _makeCombinationListUtility(IntVectorVector& result, IntVector& temp, const int n, const int left, const int k);

public:
    // return a list of all n choose k combinations from 0 to n - 1
    static IntVectorVector makeCombinationList(const int n, const int k);
};

#endif /* CombinationListCreator_hpp */
