cmake_minimum_required(VERSION 3.14)

project(LargeCoordinates)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile options for better error detection
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include FetchContent module
include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make googletest available
FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(test_large_coordinates test_large_coordinates.cpp)

# Include the current directory so the test can find LargeCoordinates.h
target_include_directories(test_large_coordinates PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link with Google Test
target_link_libraries(test_large_coordinates 
    gtest_main
    gtest
)

# Enable testing
enable_testing()

# Add the test
add_test(NAME LargeCoordinatesTest COMMAND test_large_coordinates)

# Optional: Add a custom target to run tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_large_coordinates
    COMMENT "Running unit tests"
) 