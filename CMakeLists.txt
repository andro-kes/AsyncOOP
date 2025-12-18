cmake_minimum_required(VERSION 3.10)
project(AsyncOOP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable threading support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Add executable
add_executable(lab7
    src/main.cpp
    src/npc.cpp
    src/knight.cpp
    src/squirrel.cpp
    src/pegasus.cpp
    src/game.cpp
)

# Include directories
target_include_directories(lab7 PRIVATE include)

# Link threading library
target_link_libraries(lab7 PRIVATE Threads::Threads)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lab7 PRIVATE -Wall -Wextra -Wpedantic)
endif()
