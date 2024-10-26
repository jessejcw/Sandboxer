//
// Created by jesse on 10/16/24.
//

#include "result.h"
#include <iostream>

void Result::print() {
    std::cout << "User CPU Time: " << userCPUTime << " sec\n";
    std::cout << "System CPU Time: " << systemCPUTime << " sec\n";
    std::cout << "Maximum Resident Set Size: " << maxRSS << " KB\n";
}