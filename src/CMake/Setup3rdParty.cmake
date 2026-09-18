# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

include(CMake/thirdparty/SetupHwloc.cmake)
include(CMake/thirdparty/SetupJansson.cmake)
if(VARIORUM_WITH_AMD_CPU)
include(CMake/thirdparty/Setupesmi.cmake)
endif()
if(VARIORUM_WITH_NVIDIA_GPU)
    include(CMake/thirdparty/FindNVML.cmake)
endif()
if(VARIORUM_WITH_AMD_GPU)
    include(CMake/thirdparty/FindROCM.cmake)
endif()
if(VARIORUM_WITH_AMD_APU)
    include(CMake/thirdparty/FindAMDSMI.cmake)
endif()
if(VARIORUM_WITH_INTEL_GPU)
    include(CMake/thirdparty/FindAPMIDG.cmake)
endif()

if(ENABLE_MPI)
    include(CMake/thirdparty/SetupRankstr.cmake)
endif()

if(ENABLE_LIBJUSTIFY)
    include(CMake/thirdparty/SetupLibjustify.cmake)
endif()

if(BUILD_DOCS)
    find_package(Doxygen)
    include(CMake/thirdparty/FindSphinx.cmake)
endif()

if(BUILD_TESTS)
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
    ################################
    # Enable GTest
    ################################

    # g++ v4.9.3 no longer compiles googletest.
    # g++ v6.1.0 and later are known to work.
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1)
            message(FATAL_ERROR "Minimum GNU g++ version is 6.1.0, found ${CMAKE_CXX_COMPILER_VERSION}")
        endif()
    endif()

    #
    # We always want to build gtest as a static lib, however
    # it shares our "BUILD_SHARED_LIBS" option, so we need
    # to force this value to OFF, and then restore the
    # previous setting.
    #
    set(BSL_ORIG_VALUE ${BUILD_SHARED_LIBS})

    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(thirdparty_builtin/googletest)

    set(BUILD_SHARED_LIBS ${BSL_ORIG_VALUE})

    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
endif()
