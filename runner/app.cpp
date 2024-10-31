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
    // Open output file
    std::ofstream outputFile(args.outputFileName);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Cannot open output file\n";
        exit(1);
    }

    // Create pipes for stdout and stderr
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0) {
        std::cerr << "Error: Pipe creation failed\n";
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Error: Fork failed\n";
        exit(1);
    } else if (pid == 0) {
        // Child process
        // Redirect stdout
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);

        // Redirect stderr
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        // Close unnecessary file descriptors
        // Close stdin if you want to prevent child from reading from it
        // Alternatively, you can redirect stdin to /dev/null
        int devnull = open("/dev/null", O_RDONLY);
        dup2(devnull, STDIN_FILENO);
        close(devnull);

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
        close(stdout_pipe[1]); // Close unused write end
        close(stderr_pipe[1]); // Close unused write end

        // Read from child's stdout and stderr concurrently
        char buffer[4096];
        bool stdout_eof = false;
        bool stderr_eof = false;

        fd_set readfds;
        int maxfd = std::max(stdout_pipe[0], stderr_pipe[0]) + 1;

        while (!stdout_eof || !stderr_eof) {
            FD_ZERO(&readfds);
            if (!stdout_eof)
                FD_SET(stdout_pipe[0], &readfds);
            if (!stderr_eof)
                FD_SET(stderr_pipe[0], &readfds);

            int ret = select(maxfd, &readfds, NULL, NULL, NULL);
            if (ret == -1) {
                perror("select");
                exit(1);
            }

            if (FD_ISSET(stdout_pipe[0], &readfds)) {
                ssize_t nbytes = read(stdout_pipe[0], buffer, sizeof(buffer));
                if (nbytes > 0) {
                    std::string data(buffer, nbytes);
                    // Replace commas with tabs
                    for (char& c : data) {
                        if (c == ',') c = '\t';
                    }
                    outputFile << data;
                } else if (nbytes == 0) {
                    stdout_eof = true;
                } else {
                    perror("read stdout");
                    exit(1);
                }
            }

            if (FD_ISSET(stderr_pipe[0], &readfds)) {
                ssize_t nbytes = read(stderr_pipe[0], buffer, sizeof(buffer));
                if (nbytes > 0) {
                    std::string data(buffer, nbytes);
                    // Time-tagged error messages
                    log.LOGE(data);
                } else if (nbytes == 0) {
                    stderr_eof = true;
                } else {
                    perror("read stderr");
                    exit(1);
                }
            }
        }
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        outputFile.close();

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