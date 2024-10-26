find_package(SQLite3 REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS})

add_executable(csv_to_sqlite csv_to_sqlite.cpp
        db_processor.cpp
        db_processor.h
        args.cpp
        args.h)

target_link_libraries(csv_to_sqlite PRIVATE SQLite::SQLite3)
