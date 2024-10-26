#include <iostream>
#include "args.h"
#include "db_processor.h"


int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            Args::printUsage();
            return 1;
        }

        Args args(argc, argv);
        DbProcessor processor(args);
        processor.process();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        Args::printUsage();
        return 1;
    }
}