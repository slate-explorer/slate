cmake_minimum_required(VERSION 3.20)

project(Slate
    VERSION 0.1.0
    DESCRIPTION "A compact, lightweight cross-platform file explorer"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Automatically find all C++ source files
file(GLOB_RECURSE SLATE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Build executable
add_executable(Slate
    ${SLATE_SOURCES}
)

# Include directories
target_include_directories(Slate
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
) 

target_compile_options(Slate PRIVATE /utf-8)