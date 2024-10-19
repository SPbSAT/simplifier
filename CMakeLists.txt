# ===================================== GENERAL ===================================== #

cmake_minimum_required(VERSION 3.22.3)
project(csat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Compilation database is needed for static linter.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(
    CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=.*;
    -checks=-*,modernize-*,-modernize-use-trailing-return-type,-modernize-avoid-c-arrays,-modernize-type-traits;
    -warnings-as-errors=*;)

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
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -pedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX)
else()

endif()

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
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DBUFF_IS_IFF")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -ftree-vectorize")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -fgcse-sm -fgcse-las -fdevirtualize-at-ltrans")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fipa-pta -floop-parallelize-all -ftree-vectorize -fstdarg-opt")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmodulo-sched -fmodulo-sched-allow-regmoves -fgraphite-identity")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ftree-loop-im -fivopts -fsplit-ivs-in-unroller -fprefetch-loop-arrays ")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

else()

endif()

# *********************************************************************************** #

# =================================== PREPROCESSOR =================================== #

add_executable(simplify app/simplify.cpp)
target_link_libraries(simplify argparse)

# *********************************************************************************** #

# =================================== CLANG-TIDY =================================== #

#option(CMake_RUN_CLANG_TIDY "Run clang-tidy along the compiler." OFF)
#if(CMake_RUN_CLANG_TIDY)
#    if(CMake_SOURCE_DIR STREQUAL CMake_BINARY_DIR)
#        message(FATAL_ERROR "CMake_RUN_CLANG_TIDY requires an out-of-source build!")
#    endif()
#
#    find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
#    if(NOT CLANG_TIDY_COMMAND)
#        message(WARNING "CMake_RUN_CLANG_TIDY is ON but clang-tidy is not found!")
#        set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE)
#    else()
#        set(CLANG_TIDY_CHECKS "-*,modernize-*")
#        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/src/*'")
#    endif()
#
#    # Create a preprocessor definition that depends on .clang-tidy content so
#    # the compile command will change when .clang-tidy changes.  This ensures
#    # that a subsequent build re-runs clang-tidy on all sources even if they
#    # do not otherwise need to be recompiled.  Nothing actually uses this
#    # definition.  We add it to targets on which we run clang-tidy just to
#    # get the build dependency on the .clang-tidy file.
#    file(SHA1 ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy clang_tidy_sha1)
#    set(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
#    unset(clang_tidy_sha1)
#endif()
#configure_file(.clang-tidy .clang-tidy COPYONLY)

# *********************************************************************************** #
