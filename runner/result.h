//
// Created by jesse on 10/16/24.
//

#ifndef RESULT_H
#define RESULT_H

class Result {
public:
    void print();

    // Profiling info
    double userCPUTime;
    double systemCPUTime;
    long maxRSS;
};

#endif // RESULT_H