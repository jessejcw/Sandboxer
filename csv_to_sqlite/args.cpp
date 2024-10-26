#include "args.h"
#include <iostream>
#include <stdexcept>

Args::Args(int argc, char* argv[]) : argc(argc), argv(argv) {
    parse();
    validateArgs();
}

void Args::printUsage() {
    std::cout << "Usage: csv_to_sqlite --input <input_file> --type <type> --date <date>\n"
              << "\nRequired Arguments:\n"
              << "  --input <input_file>  : Input file to process\n"
              << "  --type  <type>        : Specify the type for processing\n"
              << "  --date  <date>        : Specify the date (format: YYYYMMDD)\n"
              << "\nExample:\n"
              << "  csv_to_sqlite --input trades.csv --type TSLA --date 20241016\n";
}

bool Args::hasRequiredArgs() const {
    return !inputFileName.empty() && type.has_value() && date.has_value();
}

void Args::validateArgs() const {
    std::vector<std::string> missingArgs;

    if (inputFileName.empty()) missingArgs.push_back("--input");
    if (!type.has_value()) missingArgs.push_back("--type");
    if (!date.has_value()) missingArgs.push_back("--date");

    if (!missingArgs.empty()) {
        std::string error = "Missing required arguments:";
        for (const auto& arg : missingArgs) {
            error += " " + arg;
        }
        throw std::runtime_error(error);
    }
}

void Args::parseArg(const std::string& arg, int& i) {
    if (arg == "--input") {
        if (i + 1 < argc) {
            inputFileName = argv[++i];
        } else {
            throw std::runtime_error("Error: Missing input file name after --input");
        }
    } else if (arg == "--type") {
        if (i + 1 < argc) {
            type = argv[++i];
        } else {
            throw std::runtime_error("Error: Missing value after --type");
        }
    } else if (arg == "--date") {
        if (i + 1 < argc) {
            std::string dateVal = argv[++i];
            if (dateVal.length() != 8) {
                throw std::runtime_error("Error: Date must be in YYYYMMDD format");
            }
            date = dateVal;
        } else {
            throw std::runtime_error("Error: Missing value after --date");
        }
    } else {
        throw std::runtime_error("Error: Unknown option: " + arg);
    }
}

void Args::parse() {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        try {
            parseArg(arg, i);
        } catch (const std::runtime_error& e) {
            throw;
        }
    }
}