//
// Created by jesse on 10/16/24.
//

#include "args.h"
#include <iostream>

#include "args.h"
#include <iostream>
#include <sstream>

Args::Args(int argc, char* argv[]) : argc(argc), argv(argv) {
    parse();
}

void Args::printUsage() {
    std::cout << "Usage: sanbox --input <input_file> --output <output_file> --log <log_file> -- <executable> [args...]\n";
    std::cout << "  --input  <file>:<type>:<date>   : Input file to be sent to the executable\n";
    std::cout << "  --output <output_file>          : File to store the executable's output\n";
    std::cout << "  --log    <log_file>             : File to store error logs\n";
    std::cout << "  --                              : Separator for executable and its arguments\n";
    std::cout << "  <executable>                    : The executable to run in the sandbox\n";
    std::cout << "  [args...]                       : Optional arguments for the executable\n";
}

void Args::parseInputFormat(const std::string& input) {
    std::stringstream ss(input);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(ss, token, ':')) {
        parts.push_back(token);
    }

    if (parts.size() != 3) {
        throw std::runtime_error("Error: Input format must be 'file:type:date'");
    }

    inputFileName = parts[0];

    executableArgs.push_back("--input");
    executableArgs.push_back(parts[0]);
    executableArgs.push_back("--type");
    executableArgs.push_back(parts[1]);
    executableArgs.push_back("--date");
    executableArgs.push_back(parts[2]);
}

// In runner's args.cpp
void Args::parse() {
    bool execArgsStart = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (!execArgsStart) {
            if (arg == "--input") {
                if (i + 1 < argc) {
                    parseInputFormat(argv[++i]);
                } else {
                    throw std::runtime_error("Error: Missing value after --input");
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
