//
// Created by jesse on 10/16/24.
//

#ifndef APP_H
#define APP_H

#include "args.h"
#include "log.h"
#include "result.h"

class App {
public:
    App(const Args& args, Log& log);
    void run();

    Result result;

private:
    const Args& args;
    Log& log;
};

#endif // APP_H