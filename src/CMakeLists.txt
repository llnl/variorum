# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# avoid warnings for project commands w/o VERSION
cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(variorum VERSION "0.8.0" LANGUAGES C CXX)

option(USE_MSR_SAFE_BEFORE_1_5_0 "Build for msr-safe whitelist interface" OFF)

option(BUILD_SHARED_LIBS         "Build shared libraries"                 ON)
option(BUILD_TESTS               "Build tests"                            ON)

option(ENABLE_FORTRAN            "Build Fortran support"                  ON)
option(ENABLE_PYTHON             "Build Python support"                   ON)
option(ENABLE_WARNINGS           "Enable warnings"                        OFF)
option(ENABLE_MPI                "Build MPI examples"                     OFF)
option(ENABLE_OPENMP             "Build OpenMP examples"                  ON)
option(ENABLE_LIBJUSTIFY         "Enable libjustify formatting"           OFF)

option(VARIORUM_WITH_AMD_CPU     "Support AMD CPU architectures"          OFF)
option(VARIORUM_WITH_AMD_GPU     "Support AMD GPU architectures"          OFF)
option(VARIORUM_WITH_AMD_APU     "Support AMD APU architectures"          OFF)
option(VARIORUM_WITH_ARM_CPU     "Support ARM CPU architectures"          OFF)
option(VARIORUM_WITH_IBM_CPU     "Support IBM CPU architectures"          OFF)
option(VARIORUM_WITH_INTEL_CPU   "Support Intel CPU architectures"        ON)
option(VARIORUM_WITH_INTEL_GPU   "Support Intel GPU architectures"        OFF)
option(VARIORUM_WITH_NVIDIA_GPU  "Support Nvidia GPU architectures"       OFF)

option(VARIORUM_DEBUG            "Enable debug statements"                OFF)

set(HWLOC_DIR "" CACHE PATH "path to hwloc installation")
set(JANSSON_DIR "" CACHE PATH "path to jansson installation")
set(LIBJUSTIFY_DIR "" CACHE PATH "path to libjustify installation")

if(USE_MSR_SAFE_BEFORE_1_5_0)
    add_definitions(-DUSE_MSR_SAFE_BEFORE_1_5_0)
endif()

### Invoke CMake Fortran setup
if(ENABLE_FORTRAN)
    enable_language(Fortran)
endif()

### Invoke CMake Python setup
if(ENABLE_PYTHON)
    find_package(Python COMPONENTS Interpreter REQUIRED)
endif()

### Invoke CMake MPI setup
if(ENABLE_MPI)
    find_package(MPI REQUIRED)
endif()

if(ENABLE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

include(CMake/VariorumBuildType.cmake)

include(CMake/CMakeBasics.cmake)

include(CMake/SetupFortran.cmake)

include(CMake/SetupCompilerOptions.cmake)

include(CMake/Setup3rdParty.cmake)

include(CMake/SetupTests.cmake)

add_subdirectory(thirdparty_builtin)

### Enable warnings
if(ENABLE_WARNINGS)
    ENABLE_WARNINGS()
endif()

### Add our libs
add_subdirectory(variorum)

### Add our tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

### Add our examples
add_subdirectory(examples)

### Add var_monitor sampler
add_subdirectory(var_monitor)

### Add config helpers
add_subdirectory(config)

### Create CMake importable exports
install(EXPORT variorum DESTINATION lib/cmake/Variorum)
