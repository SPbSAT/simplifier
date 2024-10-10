# ===================================== GENERAL ===================================== #

cmake_minimum_required(VERSION 3.22.3)
project(csat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(.)
include_directories(third_party/argparse/include/argparse)
include_directories(third_party/googletest/googletest/include)

add_subdirectory(third_party/argparse)
add_subdirectory(third_party/googletest)

add_subdirectory(tests)

# Resolve build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
endif()

# Echo build mode.
MESSAGE(STATUS "Build mode is ${CMAKE_BUILD_TYPE}")

# *********************************************************************************** #


# ================================== COMPILER FLAGS ================================= #

# Enable warnings on compile stage.
if (MSVC)
    add_compile_options(/W4 /WX)
else ()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "-g")

# DEBUG options:
# 1. -O0 disables optimization for profiling purposes.
# 2. BUFF_IS_IFF -- makes parser automatically treat BUFF gates as IFF gates.
# 3. ENABLE_DEBUG_LOGGING -- enables logging on level "Debug".
# 4. ENABLE_PROFILING -- enables profiling.
#
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DBUFF_IS_IFF -DENABLE_DEBUG_LOGGING -DENABLE_PROFILING ")

# RELEASE options:
# 1. -O3 stands for aggressive compiler optimization.
# 2. BUFF_IS_IFF -- makes parser automatically treat BUFF gates as IFF gates.
# 3. Flag `-ffast-math` is enabled since all important variables are integer variations,
#    and floating point calculations are used only in supportive heuristic mechanisms.
#
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DBUFF_IS_IFF -ffast-math -fgcse-sm -fgcse-las -fdevirtualize-at-ltrans")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fipa-pta -floop-parallelize-all -ftree-vectorize -fstdarg-opt")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmodulo-sched -fmodulo-sched-allow-regmoves -fgraphite-identity")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ftree-loop-im -fivopts -fsplit-ivs-in-unroller -fprefetch-loop-arrays ")

# *********************************************************************************** #

# =================================== PREPROCESSOR =================================== #

add_executable(simplify main.cpp)
target_link_libraries(simplify argparse)

# *********************************************************************************** #
