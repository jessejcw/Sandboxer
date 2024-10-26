//
// Created by jesse on 10/16/24.
//

#include "app.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>

App::App(const Args& args, Log& log) : args(args), log(log) {}

void App::run() {
    // Read input file
    std::ifstream inputFile(args.inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Cannot open input file\n";
        exit(1);
    }
    std::stringstream inputBuffer;
    inputBuffer << inputFile.rdbuf();
    inputFile.close();
    std::string inputData = inputBuffer.str();

    // Open output file
    std::ofstream outputFile(args.outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Cannot open output file\n";
        exit(1);
    }

    // Create pipes
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0) {
        std::cerr << "Error: Pipe creation failed\n";
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Error: Fork failed\n";
        exit(1);
    } else if (pid == 0) {
        // Child process
        // Redirect stdin
        dup2(stdin_pipe[0], STDIN_FILENO);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);

        // Redirect stdout
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);

        // Redirect stderr
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        // Prepare arguments for execvp
        std::vector<char*> execArgs;
        execArgs.push_back(const_cast<char*>(args.executableName.c_str()));
        for (auto& arg : args.executableArgs) {
            execArgs.push_back(const_cast<char*>(arg.c_str()));
        }
        execArgs.push_back(NULL);

        // Execute the executable
        if (execvp(execArgs[0], execArgs.data()) == -1) {
            std::cerr << "Error: execvp failed\n";
            exit(1);
        }
    } else {
        // Parent process
        close(stdin_pipe[0]);  // Close unused read end
        close(stdout_pipe[1]); // Close unused write end
        close(stderr_pipe[1]); // Close unused write end

        // Write input to child's stdin
        write(stdin_pipe[1], inputData.c_str(), inputData.size());
        close(stdin_pipe[1]); // Close write end after sending input

        // Read from child's stdout
        char buffer[4096];
        ssize_t nbytes;

        while ((nbytes = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
            std::string data(buffer, nbytes);
            // Replace commas with tabs
            for (char& c : data) {
                if (c == ',') c = '\t';
            }
            outputFile << data;
        }
        close(stdout_pipe[0]);
        outputFile.close();

        // Read from child's stderr
        while ((nbytes = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
            std::string data(buffer, nbytes);
            // Time-tagged error messages
            log.LOGE(data);
        }
        close(stderr_pipe[0]);

        // Wait for child process to finish and collect resource usage
        int status;
        struct rusage usage;
        if (wait4(pid, &status, 0, &usage) == -1) {
            std::cerr << "Error: wait4 failed\n";
            exit(1);
        }

        result.userCPUTime = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        result.systemCPUTime = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        result.maxRSS = usage.ru_maxrss;
    }
}