cmake_minimum_required(VERSION 3.10)
project(AIG_BDD_Simulator CXX)

# Ensure the compilation database is generated
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Explicitly group ALL files into a variable
set(PROJECT_SOURCES
    main.cpp
    aig.hpp  
    bdd.hpp  
    checker.hpp  
    reader.hpp  
    simulator.hpp  
    truth_table.hpp
)

# 2. Tell CMake to treat headers as source files for the build target
# (This forces static analyzers to parse them as part of this leaf node)
set_source_files_properties(
    aig.hpp bdd.hpp checker.hpp reader.hpp simulator.hpp truth_table.hpp 
    PROPERTIES HEADER_FILE_ONLY FALSE
)

# 3. Define the executable using the combined sources
add_executable(simulator ${PROJECT_SOURCES})

# 4. Explicitly include the current directory
target_include_directories(simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})