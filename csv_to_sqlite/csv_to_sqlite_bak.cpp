//
// Created by jesse on 10/24/24.
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sqlite3.h>

// Function to split a string by a delimiter
std::vector<std::string> split(const std::string& line, char delimiter=',') {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(line);

    while (std::getline(ss, token, delimiter)) {
        tokens.emplace_back(token);
    }

    return tokens;
}

int main(int argc, char* argv[]) {
    // Initialize SQLite
    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);
    if (rc) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return 1;
    }

    std::string line;
    std::vector<std::string> headers;
    bool isHeader = true;
    int recordCount = 0;

    // Read CSV data from stdin
    while (std::getline(std::cin, line)) {
        if (isHeader) {
            headers = split(line);
            isHeader = false;

            // Construct CREATE TABLE statement
            std::stringstream createStmt;
            createStmt << "CREATE TABLE IF NOT EXISTS records (id INTEGER PRIMARY KEY AUTOINCREMENT";
            for (const auto& header : headers) {
                // Sanitize header names (remove spaces and special characters)
                std::string sanitized = header;
                for (auto& ch : sanitized) {
                    if (!isalnum(ch)) ch = '_';
                }
                createStmt << ", " << sanitized << " TEXT";
            }
            createStmt << ");";

            rc = sqlite3_exec(db, createStmt.str().c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "SQL error during table creation: " << sqlite3_errmsg(db) << "\n";
                sqlite3_close(db);
                return 1;
            }

            // Begin transaction
            rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
            if (rc != SQLITE_OK) {
                std::cerr << "SQL error during BEGIN TRANSACTION: " << sqlite3_errmsg(db) << "\n";
                sqlite3_close(db);
                return 1;
            }

            // Prepare INSERT statement
            std::stringstream insertStmt;
            insertStmt << "INSERT INTO records (";
            for (size_t i = 0; i < headers.size(); ++i) {
                std::string sanitized = headers[i];
                for (auto& ch : sanitized) {
                    if (!isalnum(ch)) ch = '_';
                }
                insertStmt << sanitized;
                if (i < headers.size() - 1) insertStmt << ", ";
            }
            insertStmt << ") VALUES (";
            for (size_t i = 0; i < headers.size(); ++i) {
                insertStmt << "?";
                if (i < headers.size() - 1) insertStmt << ", ";
            }
            insertStmt << ");";

            // Prepare the statement
            sqlite3_stmt* stmt;
            rc = sqlite3_prepare_v2(db, insertStmt.str().c_str(), -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                std::cerr << "Failed to prepare INSERT statement: " << sqlite3_errmsg(db) << "\n";
                sqlite3_close(db);
                return 1;
            }

            // Process each subsequent line as a record
            while (std::getline(std::cin, line)) {
                std::vector<std::string> fields = split(line);
                if (fields.size() != headers.size()) {
                    std::cerr << "Mismatch in number of fields. Expected " << headers.size()
                              << ", got " << fields.size() << ". Skipping line.\n";
                    continue;
                }

                // Bind values
                for (size_t i = 0; i < fields.size(); ++i) {
                    rc = sqlite3_bind_text(stmt, static_cast<int>(i + 1), fields[i].c_str(), -1, SQLITE_TRANSIENT);
                    if (rc != SQLITE_OK) {
                        std::cerr << "Failed to bind value: " << sqlite3_errmsg(db) << "\n";
                        sqlite3_finalize(stmt);
                        sqlite3_close(db);
                        return 1;
                    }
                }

                // Execute the statement
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE) {
                    std::cerr << "Failed to execute INSERT statement: " << sqlite3_errmsg(db) << "\n";
                } else {
                    recordCount++;
                }

                // Reset the statement for the next use
                sqlite3_reset(stmt);
            }

            // Finalize the statement
            sqlite3_finalize(stmt);

            // Commit transaction
            rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
            if (rc != SQLITE_OK) {
                std::cerr << "SQL error during COMMIT: " << sqlite3_errmsg(db) << "\n";
                sqlite3_close(db);
                return 1;
            }

        } else {
            // This block won't be reached as all data is processed in the header section
            // Kept for completeness
        }
    }

    // Close the database connection
    sqlite3_close(db);

    // Output the number of records inserted
    std::cout << "Successfully inserted " << recordCount << " records into the database.\n";

    return 0;
}
