cmake_minimum_required(VERSION 3.21)
project(Compiler C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
# Add the source files
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

# Set the directory for JACK files
set(JACK_FILES_DIR ${CMAKE_SOURCE_DIR}/src/jack_files)

# Set the log file name
set(LOG_FILE "${CMAKE_SOURCE_DIR}/src/logfile.log")


set(DEF_FILES_DIR ${CMAKE_SOURCE_DIR}/src/defs)

# Add the executable target
add_executable(compiler ${SOURCES})

# Include the directory containing header files
target_include_directories(compiler PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

# Set the path to the JACK files directory as a compile definition
target_compile_definitions(compiler PRIVATE JACK_FILES_DIR="${JACK_FILES_DIR}")

# Set the log file as a compile definition
target_compile_definitions(compiler PRIVATE LOG_FILE="${LOG_FILE}")

# Makes path to definitions available to the application
target_compile_definitions(compiler PRIVATE DEF_FILES_DIR=${DEF_FILES_DIR})
