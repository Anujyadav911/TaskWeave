cmake_minimum_required(VERSION 3.15)
project(TaskWeave VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type (Release by default for production, except for multi-config generators)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/api
    ${CMAKE_SOURCE_DIR}/utils
)

# Source files (excluding main.cpp for library)
set(LIB_SOURCES
    src/core/Task.cpp
    core/TaskLoader.cpp
    core/TaskRegistry.cpp
    src/executor/ThreadPool.cpp
    src/scheduler/PriorityScheduler.cpp
    src/scheduler/RoundRobinScheduler.cpp
    api/ApiServer.cpp
    utils/Config.cpp
    utils/Metrics.cpp
    utils/Database.cpp
)

# Header files (for IDE support)
set(HEADERS
    src/core/Task.h
    src/core/TaskState.h
    src/core/EngineState.h
    core/TaskDefinition.h
    core/TaskLoader.h
    core/TaskRegistry.h
    src/executor/ThreadPool.h
    src/scheduler/Scheduler.h
    src/scheduler/PriorityScheduler.h
    src/scheduler/RoundRobinScheduler.h
    api/ApiServer.h
    utils/Config.h
    utils/Logger.h
    utils/Metrics.h
    utils/Database.h
    third_party/json.hpp
    third_party/httplib.h
)

# Create library from sources (shared between main executable and tests)
add_library(taskweave_lib STATIC ${LIB_SOURCES} ${HEADERS})

# Link libraries to library
find_package(Threads REQUIRED)
target_link_libraries(taskweave_lib 
    PRIVATE 
    Threads::Threads
)

# Find SQLite3
find_library(SQLITE3_LIB sqlite3)
if(SQLITE3_LIB)
    target_link_libraries(taskweave_lib PRIVATE ${SQLITE3_LIB})
else()
    message(WARNING "SQLite3 library not found. Database features may not work.")
endif()

# Windows-specific configuration
if(WIN32)
    target_link_libraries(taskweave_lib PRIVATE ws2_32)
    target_compile_definitions(taskweave_lib PRIVATE
        _WIN32_WINNT=0x0A00
        WINVER=0x0A00
        NTDDI_VERSION=0x0A000000
    )
endif()

# Main executable links to library
add_executable(taskweave src/main.cpp)
target_link_libraries(taskweave PRIVATE taskweave_lib)

# Install rules
install(TARGETS taskweave
    RUNTIME DESTINATION bin
)

# Install config and web files
install(FILES 
    src/config.ini
    tasks.json
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(DIRECTORY web/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/web
    FILES_MATCHING PATTERN "*.html"
)

# Build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# ============================================================================
# Testing with GoogleTest
# ============================================================================
option(BUILD_TESTING "Build the testing tree" ON)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Disable tests in subdirectory from being run automatically
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    
    enable_testing()
    
    # Test source files
    set(TEST_SOURCES
        tests/test_task.cpp
        tests/test_scheduler.cpp
        tests/test_task_loader.cpp
    )
    
    # Create test executable
    add_executable(taskweave_tests ${TEST_SOURCES})
    
    # Link test executable with project library and GoogleTest
    target_link_libraries(taskweave_tests
        PRIVATE
        taskweave_lib
        gtest_main
        gmock
    )
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(taskweave_tests)
    
    message(STATUS "Testing enabled with GoogleTest")
endif()
