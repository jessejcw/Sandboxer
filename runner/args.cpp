//
// Created by jesse on 10/16/24.
//

#include "args.h"
#include <iostream>

#include "args.h"
#include <iostream>

Args::Args(int argc, char* argv[]) : argc(argc), argv(argv) {
    parse();
}

void Args::printUsage() {
    std::cout << "Usage: sanbox --input <input_file> --output <output_file> --log <log_file> -- <executable> [args...]\n";
    std::cout << "  --input  <input_file>   : Input file to be sent to the executable\n";
    std::cout << "  --output <output_file>  : File to store the executable's output\n";
    std::cout << "  --log    <log_file>     : File to store error logs\n";
    std::cout << "  --                      : Separator for executable and its arguments\n";
    std::cout << "  <executable>            : The executable to run in the sandbox\n";
    std::cout << "  [args...]               : Optional arguments for the executable\n";
}

void Args::parse() {
    bool execArgsStart = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (!execArgsStart) {
            if (arg == "--input") {
                if (i + 1 < argc) {
                    inputFileName = argv[++i];
                } else {
                    throw std::runtime_error("Error: Missing input file name after --input");
                }
            } else if (arg == "--output") {
                if (i + 1 < argc) {
                    outputFileName = argv[++i];
                } else {
                    throw std::runtime_error("Error: Missing output file name after --output");
                }
            } else if (arg == "--log") {
                if (i + 1 < argc) {
                    logFileName = argv[++i];
                } else {
                    throw std::runtime_error("Error: Missing log file name after --log");
                }
            } else if (arg == "--") {
                execArgsStart = true;
            } else {
                throw std::runtime_error("Error: Unknown option " + arg);
            }
        } else {
            if (executableName.empty()) {
                executableName = arg;
            } else {
                executableArgs.push_back(arg);
            }
        }
    }

    if (inputFileName.empty() || outputFileName.empty() || logFileName.empty() || executableName.empty()) {
        throw std::runtime_error("Error: Missing required arguments");
    }
}