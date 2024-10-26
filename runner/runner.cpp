#include "args.h"
#include "log.h"
#include "app.h"
#include "result.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            Args::printUsage();
            return 1;
        }

        Args args(argc, argv);
        Log log(args.logFileName);
        App app(args, log);
        app.run();
        app.result.print();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        Args::printUsage();
        return 1;
    }
    return 0;
}