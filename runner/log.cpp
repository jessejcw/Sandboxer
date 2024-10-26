//
// Created by jesse on 10/16/24.
//

#include "log.h"
#include <iostream>
#include <ctime>
#include <sys/time.h>

Log::Log(const std::string& logFileName) {
    logFile.open(logFileName, std::ios::out);
    if (!logFile.is_open()) {
        std::cerr << "Error: Cannot open log file\n";
        exit(1);
    }
}

Log::~Log() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Log::LOGE(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t nowTime = tv.tv_sec;
    struct tm *nowTm = localtime(&nowTime);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", nowTm);
    // Write time-tagged message to log file
    logFile << "[" << timeStr << "." << tv.tv_usec/1000 << "] " << message;
}