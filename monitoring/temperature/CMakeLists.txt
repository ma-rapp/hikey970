cmake_minimum_required (VERSION 2.6)
project (temperature)
add_executable(temperature temperature.cpp ../utils/console.cpp)

SET(GCC_COMPILE_FLAGS "-static -std=c++11 -Wall -Werror -O3")
SET(GCC_LINK_FLAGS    "")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

include_directories(../utils)
