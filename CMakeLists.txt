cmake_minimum_required(VERSION 3.14)
project(PixelCanvas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Raylib package
find_package(raylib QUIET)

if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Add executable
add_executable(${PROJECT_NAME} main.cpp)

# Link with Raylib
target_link_libraries(${PROJECT_NAME} raylib)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Copy assets if needed
# file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Windows specific options
if (WIN32)
    # Set the Windows subsystem to Windows
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
    
    # Link with winmm
    target_link_libraries(${PROJECT_NAME} winmm)
endif() 