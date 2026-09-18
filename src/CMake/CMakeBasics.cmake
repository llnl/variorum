# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

###########################
# Standard CMake Options #
###########################


## Fail if someone tries to config an in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "In-source builds are not supported. Please remove "
                       "CMakeCache.txt from the 'src' dir and configure an "
                       "out-of-source build in another directory.")
endif()

### Always use position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMake build tool name: ${CMAKE_BUILD_TOOL}")

### Enable warnings
macro(ENABLE_WARNINGS)
    if(FORTRAN_FOUND AND ("${CMAKE_Fortran_COMPILER}" MATCHES "ifort$" OR "${CMAKE_Fortran_COMPILER}" MATCHES "ifx$"))
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -warn")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endmacro()

if(ENABLE_WARNINGS)
    message(STATUS "Building with warnings (ENABLE_WARNINGS == ON)")
else()
    message(STATUS "Building without warnings (ENABLE_WARNINGS == OFF)")
endif()

#######################
# Msr-Safe Dependency #
#######################
if(USE_MSR_SAFE_BEFORE_1_5_0)
    message(STATUS "Building for msr-safe<1.5.0 msr_whitelist interface (USE_MSR_SAFE_BEFORE_1_5_0 == ON)")
else()
    message(STATUS "Building for msr-safe==1.5.0 msr_allowlist interface (USE_MSR_SAFE_BEFORE_1_5_0 == OFF)")
endif()

##########################
# Platform Architectures #
##########################
if(VARIORUM_WITH_INTEL_CPU)
    message(STATUS "Building support for Intel CPU architectures (VARIORUM_WITH_INTEL_CPU == ON)")
else()
    message(STATUS "Building without support for Intel CPU architectures (VARIORUM_WITH_INTEL_CPU == OFF)")
endif()
if(VARIORUM_WITH_INTEL_GPU)
    message(STATUS "Building support for Intel GPU architectures (VARIORUM_WITH_INTEL_GPU == ON)")
else()
    message(STATUS "Building without support for Intel GPU architectures (VARIORUM_WITH_INTEL_GPU == OFF)")
endif()
if(VARIORUM_WITH_IBM_CPU)
    message(STATUS "Building support for IBM CPU architectures (VARIORUM_WITH_IBM_CPU == ON)")
else()
    message(STATUS "Building without support for IBM CPU architectures (VARIORUM_WITH_IBM_CPU == OFF)")
endif()
if(VARIORUM_WITH_AMD_CPU)
    message(STATUS "Building support for AMD CPU architectures (VARIORUM_WITH_AMD_CPU == ON)")
else()
    message(STATUS "Building without support for AMD CPU architectures (VARIORUM_WITH_AMD_CPU == OFF)")
endif()
if(VARIORUM_WITH_NVIDIA_GPU)
    message(STATUS "Building support for Nvidia CPU architectures (VARIORUM_WITH_NVIDIA_GPU == ON)")
else()
    message(STATUS "Building without support for Nvidia CPU architectures (VARIORUM_WITH_NVIDIA_GPU == OFF)")
endif()
if(VARIORUM_WITH_ARM_CPU)
    message(STATUS "Building support for ARM CPU architectures (VARIORUM_WITH_ARM_CPU == ON)")
else()
    message(STATUS "Building without support for ARM CPU architectures (VARIORUM_WITH_ARM_CPU == OFF)")
endif()
if(VARIORUM_WITH_AMD_GPU)
    message(STATUS "Building support for AMD GPU architectures (VARIORUM_WITH_AMD_GPU == ON)")
else()
    message(STATUS "Building without support for AMD GPU architectures (VARIORUM_WITH_AMD_GPU == OFF)")
endif()
if(VARIORUM_WITH_AMD_APU)
    message(STATUS "Building support for AMD APU architectures (VARIORUM_WITH_AMD_APU == ON)")
else()
    message(STATUS "Building without support for AMD APU architectures (VARIORUM_WITH_AMD_APU == OFF)")
endif()

#############
# DEBUGGING #
#############
if (VARIORUM_DEBUG)
    message(STATUS "Building with debug statements (VARIORUM_DEBUG == ON)")
else()
    message(STATUS "Building without debug statements (VARIORUM_DEBUG == OFF)")
endif()

#################
# Documentation #
#################
if(BUILD_DOCS)
    message(STATUS "Building documentation (BUILD_DOCS == ON)")
else()
    message(STATUS "Building without documentation (BUILD_DOCS == OFF)")
endif()

#########################
# Shared vs Static Libs #
#########################
if(BUILD_SHARED_LIBS)
    message(STATUS "Building variorum as a shared library (BUILD_SHARED_LIBS == ON)")
else()
    message(STATUS "Building variorum as a static library (BUILD_SHARED_LIBS == OFF)")
endif()

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
