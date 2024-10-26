//
// Created by jesse on 10/16/24.
//

#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <mutex>

class Log {
public:
    Log(const std::string& logFileName);
    ~Log();
    void LOGE(const std::string& message);

private:
    std::ofstream logFile;
    std::mutex logMutex;
};

#endif // LOG_H