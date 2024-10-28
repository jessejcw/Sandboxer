# Sandbox Runner

A secure process isolation and resource monitoring utility designed to run executables in a controlled environment with input/output redirection and resource usage tracking.

## Overview

The Sandbox Runner provides a secure execution environment for command-line applications, offering process isolation, resource monitoring, and detailed logging capabilities. It's particularly useful for running data processing applications that need controlled access to system resources.

## Features

- Process isolation through forking
- Input/Output redirection
- Resource usage monitoring (CPU, memory)
- Error logging with timestamps
- Pipe-based I/O handling
- Child process resource tracking
- Detailed execution statistics

## Prerequisites

- POSIX-compliant operating system (Linux/Unix)
- C++20 compiler
- CMake 3.15 or later

### Installation of Dependencies

For Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
```

For CentOS/RHEL:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake
```

For macOS:
```bash
brew install cmake
```

## Building

1. Clone the repository:
```bash
git clone <repository-url>
cd Sandboxer
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Build the project:
```bash
cmake ..
make
```

## Usage

Basic command format:
```bash
runner --input <input_file> --output <output_file> --log <log_file> -- <executable> [args...]
```

Required arguments:
- `--input`: Input file to be sent to the executable
- `--output`: File to store the executable's output
- `--log`: File to store error logs and resource usage
- `--`: Separator for executable and its arguments
- `<executable>`: The program to run in the sandbox
- `[args...]`: Optional arguments for the executable

Example:
```bash
runner --input data.csv --output result.txt --log process.log -- ./processor --verbose
```

## Resource Monitoring

The runner tracks and reports:
- User CPU time
- System CPU time
- Maximum resident set size (memory usage)
- Process execution time
- I/O statistics

Example output:
```
Execution Statistics:
User CPU Time: 0.234s
System CPU Time: 0.056s
Max RSS: 24576 KB
```

## Security Features

1. Process Isolation:
   - Separate process space
   - Controlled resource access
   - I/O redirection

2. Resource Control:
   - Memory usage monitoring
   - CPU time tracking
   - Process termination handling

## Error Handling

Errors are logged with timestamps in the specified log file:
```
[2024-10-16 09:30:00] Process started
[2024-10-16 09:30:01] Error: Unable to open input file
[2024-10-16 09:30:01] Process terminated
```

## Implementation Details

### Key Components

1. Argument Parser (`args.h`, `args.cpp`):
   - Parses command-line arguments
   - Validates required parameters
   - Handles executable arguments

2. Logger (`log.h`):
   - Timestamp-based logging
   - Error message formatting
   - File-based logging

3. Application Runner (`app.h`, `app.cpp`):
   - Process forking
   - Pipe creation
   - I/O redirection
   - Resource monitoring

4. Result Handler (`result.h`):
   - Statistics collection
   - Resource usage reporting

## Examples

1. Basic execution:
```bash
runner --input input.txt --output output.txt --log errors.log -- ./myapp --arg1 value1
```

2. Processing with resource limits:
```bash
# Set ulimit before running
ulimit -v 1000000  # Set virtual memory limit
runner --input big_data.csv --output results.txt --log process.log -- ./processor
```

3. Error handling demonstration:
```bash
runner --input nonexistent.txt --output out.txt --log errors.log -- ./app
# Check errors.log for detailed error information
```

## Troubleshooting

Common issues and solutions:

1. Permission errors:
```bash
chmod +x runner
chmod +x <executable>
```

2. Pipe errors:
   - Check system ulimit settings
   - Verify file permissions
   - Check available file descriptors

3. Resource limits:
   - Monitor process.log
   - Check system resources
   - Adjust ulimit settings

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

