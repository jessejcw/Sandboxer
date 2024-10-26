# CSV to SQLite Converter

A command-line utility that converts CSV trade data into a SQLite database with an immutable record design pattern.

## Overview

This utility processes trade data from CSV files and stores them in a SQLite database. Each record is stored with a unique GUID and includes trade-specific information such as timestamp, type, expiry date, and additional trade details in a JSON format.

## Features

- CSV parsing with support for both comma and tab delimiters
- Automatic delimiter detection
- Immutable record storage with GUID
- JSON serialization of trade data
- Transaction support for data integrity
- Support for large datasets through efficient batch processing
- Robust error handling and validation

## Prerequisites

- C++20 or later
- SQLite3 development libraries
- CMake 3.15 or later

### Installation of Dependencies

For Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsqlite3-dev
```

For CentOS/RHEL:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake sqlite-devel
```

For macOS:
```bash
brew install cmake sqlite3
```

## Building

1. Clone the repository:
```bash
git clone <repository-url>
cd csv-to-sqlite
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
csv_to_sqlite --type <type> --date <YYYYMMDD> --input <input_file>
```

Required arguments:
- `--type`: Specify the type for processing (e.g., TSLA)
- `--date`: Specify the date in YYYYMMDD format (e.g., 20241016)
- `--input`: Input CSV file path

Example:
```bash
csv_to_sqlite --type TSLA --date 20241016 --input trades.csv
```

## Input File Format

The CSV file should contain headers and can use either comma or tab as delimiter. Example format:
```
Time,Root,Expiry,Type,Strike,Qty,Price,Notional,Bid,Ask,Side,Volatility,Change,Delta,Open Interest,Exchange,Condition,Execution,Description,Hedge Price
09:30:00.0040000,TSLA,21-Feb-25,P,140,1,2.40,240,0.00,0.00,Other,59.1,0.0024,-0.0619,1068,PHLX,AutoExecution,,,221.46
```

## Database Schema

The SQLite database creates a table with the following schema:

```sql
CREATE TABLE nodes (
    guid TEXT PRIMARY KEY,
    type TEXT NOT NULL,
    date TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    expiry TEXT NOT NULL,
    body TEXT NOT NULL
);
```

Where:
- `guid`: Unique identifier for each record
- `type`: Trade type (e.g., TSLA)
- `date`: Processing date
- `timestamp`: Trade timestamp
- `expiry`: Option expiry date
- `body`: JSON string containing all trade data

## Error Handling

The program includes comprehensive error handling for:
- File I/O errors
- Malformed CSV data
- Database operations
- Invalid command-line arguments

Errors are logged to stderr with descriptive messages.

## Performance

The utility uses SQLite transactions for optimal insertion performance. Large files are processed in batches to maintain memory efficiency.

## Limitations

- Records are immutable; no update operations are supported
- All data is stored in a single SQLite database file
- The program assumes a specific CSV format with required columns

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
