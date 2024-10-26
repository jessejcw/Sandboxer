#pragma once
#include <string>
#include <optional>
#include <vector>

class Args {
public:
    Args(int argc, char* argv[]);
    static void printUsage();

    // Required arguments
    std::string inputFileName;
    std::optional<std::string> type;
    std::optional<std::string> date;

    // Validation methods
    bool hasRequiredArgs() const;
    bool hasType() const { return type.has_value(); }
    bool hasDate() const { return date.has_value(); }

private:
    int argc;
    char** argv;

    void parse();
    void validateArgs() const;
    void parseArg(const std::string& arg, int& i);
};
