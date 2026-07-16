cmake_minimum_required(VERSION 3.14)
project(MetroRunning VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MetroRunning MetroRunningA.cpp)

# Windows console app (not GUI)
if(WIN32)
    set_target_properties(MetroRunning PROPERTIES
        WIN32_EXECUTABLE OFF
    )
endif()
