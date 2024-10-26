//
// Created by jesse on 10/16/24.
//

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>

class Args {
public:
    std::string inputFileName;
    std::string outputFileName;
    std::string logFileName;
    std::string executableName;
    std::vector<std::string> executableArgs;

    Args(int argc, char* argv[]);
    static void printUsage();

private:
    int argc;
    char** argv;

    void parse();
};

#endif //ARGS_H
