cmake_minimum_required (VERSION 3.16)

# note: find_package(SQLite3 REQUIRED) already done in top-level CMakeLists
project(sqlite_orm_example)
set(CMAKE_CXX_STANDARD 17)  # 设置为 C++11 标准
include_directories(/usr/local/include)
find_package(SQLite3 REQUIRED)
find_package(SqliteOrm REQUIRED)
file(GLOB files "*.cpp")
foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    add_executable(${file_basename} ${file})
    # note: sqlite3 already linked in top-level CMakeLists
    target_link_libraries(${file_basename} PRIVATE SqliteOrm)
endforeach()
