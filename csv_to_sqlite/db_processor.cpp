//
// Created by jesse on 10/25/24.
//

#include "db_processor.h"
#include <sstream>
#include <random>
#include <iostream>
#include <filesystem>
#include <fstream>

class DbProcessor::Impl {
public:
    Impl(const Args& args) : args_(args), db_(nullptr), stmt_(nullptr) {
        parseFileDate();
    }

    ~Impl() {
        cleanup();
    }

    void process() {
        initializeDb();
        createTable();
        //processInput();
        processInputFile();
    }

private:
    const Args& args_;
    sqlite3* db_;
    sqlite3_stmt* stmt_;
    std::string year_, month_, day_;
    std::vector<std::string> headers_;
    int time_offset_ = 0;

    void parseFileDate() {
        if (args_.hasDate()) {
            std::string date = args_.date.value();
            if (date.length() != 8) {
                throw std::runtime_error("Date must be in YYYYMMDD format");
            }
            year_ = date.substr(0, 4);
            month_ = date.substr(4, 2);
            day_ = date.substr(6, 2);
        } else {
            throw std::runtime_error("Date parameter is required");
        }
    }

    std::string formatDate() const {
        std::stringstream date_str;
        date_str << month_ << "-" << day_ << "-" << year_.substr(2);
        return date_str.str();
    }

    std::string getTimestampFromField(const std::vector<std::string>& fields) {
        for (size_t i = 0; i < headers_.size(); ++i) {
            if (headers_[i] == "Time") {
                return fields[i];
            }
        }
        throw std::runtime_error("Time column not found in CSV");
    }

    void initializeDb() {
        int rc = sqlite3_open("trades.db", &db_);
        if (rc) {
            throw std::runtime_error("Cannot open database: " +
                std::string(sqlite3_errmsg(db_)));
        }
    }

    void createTable() {
        const char* create_table_sql =
            "CREATE TABLE IF NOT EXISTS nodes ("
            "    guid TEXT PRIMARY KEY,"
            "    type TEXT NOT NULL,"
            "    date TEXT NOT NULL,"
            "    timestamp TEXT NOT NULL,"
            "    expiry TEXT NOT NULL,"
            "    body TEXT NOT NULL"
            ");";

        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, create_table_sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            throw std::runtime_error("SQL error: " + error);
        }
    }

    std::string generateUuid() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        ss << std::hex;

        for (int i = 0; i < 8; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 4; i++) ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 12; i++) ss << dis(gen);

        return ss.str();
    }

    std::vector<std::string> split(const std::string& line, char delimiter='\t') {
        std::vector<std::string> tokens;
        std::string token;
        std::stringstream ss(line);

        if (delimiter == '\t') {
            while (std::getline(ss, token, delimiter)) {
                if (!token.empty() && token.back() == '\r') {
                    token.pop_back();
                }
                tokens.push_back(token);
            }
        } else {
            while (std::getline(ss, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    std::string createJsonBody(const std::vector<std::string>& fields) {
        std::stringstream json;
        json << "{";
        for (size_t i = 0; i < std::min(headers_.size(), fields.size()); ++i) {
            if (i > 0) json << ",";

            // Escape the value properly
            std::string escaped_value = fields[i];
            std::string::size_type pos = 0;
            while ((pos = escaped_value.find('"', pos)) != std::string::npos) {
                escaped_value.replace(pos, 1, "\\\"");
                pos += 2;
            }

            json << "\"" << headers_[i] << "\":\"" << escaped_value << "\"";
        }
        json << "}";
        return json.str();
    }

    void beginTransaction() {
        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            throw std::runtime_error("Failed to begin transaction: " + error);
        }
    }

    void prepareStatement() {
        const char* insert_sql =
            "INSERT INTO nodes (guid, type, date, timestamp, expiry, body) "
            "VALUES (?, ?, ?, ?, ?, ?);";

        int rc = sqlite3_prepare_v2(db_, insert_sql, -1, &stmt_, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " +
                std::string(sqlite3_errmsg(db_)));
        }
    }

    char detectDelimiter(const std::string& line) {
        size_t commas = std::count(line.begin(), line.end(), ',');
        size_t tabs = std::count(line.begin(), line.end(), '\t');
        return (commas > tabs) ? ',' : '\t';
    }

    void processInput() {
        beginTransaction();
        prepareStatement();

        std::string line;
        bool isHeader = true;

        while (std::getline(std::cin, line)) {
            std::vector<std::string> fields = split(line);

            if (isHeader) {
                headers_ = fields;
                isHeader = false;
                continue;
            }

            processRow(fields);
        }

        commitTransaction();
    }

    void processInputFile() {
        // Open and read the entire file
        std::ifstream file(args_.inputFileName);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open input file: " + args_.inputFileName);
        }

        // Process the content
        beginTransaction();
        prepareStatement();

        std::string line;
        bool isHeader = true;
        char delimiter = ',';

        while (std::getline(file, line)) {
            if (line.empty()) continue;  // Skip empty lines

            if (isHeader) {
                delimiter = detectDelimiter(line);
                headers_ = split(line, delimiter);

                for (auto& header : headers_) {
                    if (!header.empty() && (unsigned char)header[0] == 0xEF) {
                        header = header.substr(3);
                    }
                }
                isHeader = false;
                continue;
            }
            delimiter = detectDelimiter(line);
            std::vector<std::string> fields = split(line, delimiter);
            processRow(fields);
        }

        commitTransaction();
        file.close();
    }

    void processRow(const std::vector<std::string>& fields) {

        std::cout << "Processing row with " << fields.size() << " fields\n";

        if (fields.size() != headers_.size()) {
            std::cerr << "Mismatch in field count. Expected " << headers_.size()
                     << ", got " << fields.size() << ". Skipping line.\n";
            for (const auto& field : fields) {
                std::cerr << field << "|";
            }
            std::cerr << "\n";
            return;
        }

        try {
            // Generate UUID
            std::string guid = generateUuid();

            // Determine type
            std::string type;
            if (args_.hasType()) {
                type = args_.type.value();
            } else {
                for (size_t i = 0; i < headers_.size(); ++i) {
                    if (headers_[i] == "Root") {
                        type = fields[i];
                        break;
                    }
                }
            }
            if (type.empty()) {
                std::cerr << "Warning: Type not found, using default\n";
                type = "DEFAULT";
            }

            // Format time
            std::string date = formatDate();

            std::string timestamp = getTimestampFromField(fields);
            if (timestamp.empty()) {
                throw std::runtime_error("Time column not found in CSV");
            }

            // Get expiry
            std::string expiry = getExpiryFromFields(fields);

            // Create JSON body
            std::string body = createJsonBody(fields);

            std::cout << "Inserting record: " << guid << ", " << type << ", "
                     << date << ", " << timestamp << ", " << expiry << "\n";

            insertRecord(guid, type, date, timestamp, expiry, body);
        } catch (const std::exception& e) {
            std::cerr << "Error processing row: " << e.what() << "\n";
        }

    }

    std::string getTypeFromFields(const std::vector<std::string>& fields) {
        auto it = std::find(headers_.begin(), headers_.end(), "Root");
        if (it != headers_.end()) {
            size_t idx = std::distance(headers_.begin(), it);
            return fields[idx];
        }
        return "";
    }

    std::string getExpiryFromFields(const std::vector<std::string>& fields) {
        auto it = std::find(headers_.begin(), headers_.end(), "Expiry");
        if (it != headers_.end()) {
            size_t idx = std::distance(headers_.begin(), it);
            return fields[idx];
        }
        return "";
    }

    std::string formatTime() {
        std::stringstream time_str;
        time_str << month_ << "-" << day_ << "-" << year_.substr(2) << " "
                 << "09:" << 30 + time_offset_ << ":00.0";
        return time_str.str();
    }

    void insertRecord(const std::string& guid, const std::string& type,
                     const std::string& date, const std::string& timestamp,
                     const std::string& expiry, const std::string& body) {

#if 0
        std::cout << "Binding values:\n"
                  << "1. GUID: " << guid << "\n"
                  << "2. Type: " << type << "\n"
                  << "3. Date: " << date << "\n"
                  << "4. Timestamp: " << timestamp << "\n"
                  << "5. Expiry: " << expiry << "\n"
                  << "6. Body length: " << body.length() << "\n";
#endif

        int rc = SQLITE_ERROR;

        // Begin a nested transaction for this record
        char* err_msg = nullptr;
        rc = sqlite3_exec(db_, "SAVEPOINT record_insert;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to create savepoint: " + std::string(err_msg));
        }

        try {
            // Your existing binding code...
            rc = sqlite3_bind_text(stmt_, 1, guid.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to bind GUID");

            rc = sqlite3_bind_text(stmt_, 2, type.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to bind type");

            rc = sqlite3_bind_text(stmt_, 3, date.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to bind date");

            rc = sqlite3_bind_text(stmt_, 4, timestamp.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to bind timestamp");

            rc = sqlite3_bind_text(stmt_, 5, expiry.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to bind expiry");

            rc = sqlite3_bind_text(stmt_, 6, body.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to bind body");

            // Execute the statement
            rc = sqlite3_step(stmt_);
            if (rc != SQLITE_DONE) {
                throw std::runtime_error("Failed to insert record: " + std::string(sqlite3_errmsg(db_)));
            }

            // Verify the insert
            sqlite3_stmt* verify_stmt;
            std::string verify_sql = "SELECT COUNT(*) FROM nodes WHERE guid = ?;";
            rc = sqlite3_prepare_v2(db_, verify_sql.c_str(), -1, &verify_stmt, nullptr);
            if (rc != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare verification statement");
            }

            sqlite3_bind_text(verify_stmt, 1, guid.c_str(), -1, SQLITE_STATIC);
            rc = sqlite3_step(verify_stmt);
            if (rc != SQLITE_ROW || sqlite3_column_int(verify_stmt, 0) != 1) {
                throw std::runtime_error("Record verification failed");
            }

            sqlite3_finalize(verify_stmt);
            sqlite3_exec(db_, "RELEASE record_insert;", nullptr, nullptr, nullptr);

        } catch (const std::exception& e) {
            // Rollback this record's insert
            sqlite3_exec(db_, "ROLLBACK TO record_insert;", nullptr, nullptr, nullptr);
            sqlite3_exec(db_, "RELEASE record_insert;", nullptr, nullptr, nullptr);
            throw;
        }

        // Reset the statement and bindings
        sqlite3_reset(stmt_);
        sqlite3_clear_bindings(stmt_);
    }

    void commitTransaction() {
        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::string error = err_msg;
            sqlite3_free(err_msg);
            throw std::runtime_error("Failed to commit transaction: " + error);
        }
    }

    void cleanup() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
            stmt_ = nullptr;
        }
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }
};

DbProcessor::DbProcessor(const Args& args) : pimpl(std::make_unique<Impl>(args)) {}
DbProcessor::~DbProcessor() = default;
void DbProcessor::process() { pimpl->process(); }
