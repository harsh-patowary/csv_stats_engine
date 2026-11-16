set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.16)
project(CsvStatsEngine VERSION 1.0 LANGUAGES CXX)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# --- 1. Pull in vector-class using FetchContent ---
include(FetchContent)
FetchContent_Declare(
    vector-class
    GIT_REPOSITORY https://github.com/harsh-patowary/vector_class.git  # Replace with your actual repo URL
    GIT_TAG        main                                              # Or a specific commit hash/tag
)
FetchContent_MakeAvailable(vector-class)

# --- 2. Pull in matrix-class using FetchContent ---
FetchContent_Declare(
    matrix-class
    GIT_REPOSITORY https://github.com/harsh-patowary/matrix_class.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(matrix-class)

# MatrixClass currently includes its bundled, older Vector.h using quotes.
# Make it include the shared VectorClass header instead.
set(MATRIX_HEADER "${matrix-class_SOURCE_DIR}/Matrix.h")
file(READ "${MATRIX_HEADER}" MATRIX_HEADER_CONTENT)
string(FIND "${MATRIX_HEADER_CONTENT}" "#include \"Vector.h\"" MATRIX_VECTOR_INCLUDE)
if(NOT MATRIX_VECTOR_INCLUDE EQUAL -1)
    string(REPLACE "#include \"Vector.h\"" "#include <Vector.h>"
        MATRIX_HEADER_CONTENT "${MATRIX_HEADER_CONTENT}")
    file(WRITE "${MATRIX_HEADER}" "${MATRIX_HEADER_CONTENT}")
else()
    string(FIND "${MATRIX_HEADER_CONTENT}" "#include <Vector.h>" MATRIX_VECTOR_INCLUDE)
endif()
if(MATRIX_VECTOR_INCLUDE EQUAL -1)
    message(FATAL_ERROR "Could not update Matrix.h to use the shared Vector.h")
endif()

# --- 2. Define the main executable ---
add_executable(csvstats 
    src/main.cpp
    src/csv_reader.cpp
)

# --- 3. Include directories for our local headers ---
target_include_directories(csvstats PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${vector-class_SOURCE_DIR}
    ${matrix-class_SOURCE_DIR}
)

# --- 4. Link the vector dependency ---
target_link_libraries(csvstats PRIVATE VectorClass)