#pragma once
#include <memory>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <algorithm>
#include "args.h"

class DbProcessor {
public:
    DbProcessor(const Args& args);
    ~DbProcessor();

    void process();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
