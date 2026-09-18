# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

# First check for user-specified ROCM_DIR (AMD SMI ships inside the ROCm tree)
if(ROCM_DIR)
    message(STATUS "Looking for AMDSMI using ROCM_DIR = ${ROCM_DIR}")

    set(AMDSMI_FOUND TRUE CACHE INTERNAL "")
    set(VARIORUM_ROCM_DIR ${ROCM_DIR} CACHE INTERNAL "")
    set(ROCM_DIR ${ROCM_DIR} CACHE PATH "" FORCE)
    set(AMDSMI_INCLUDE_DIRS ${ROCM_DIR}/include/ CACHE PATH "" FORCE)
    set(AMDSMI_LIBRARY ${ROCM_DIR}/lib/libamd_smi.so CACHE PATH "" FORCE)
    include_directories(${AMDSMI_INCLUDE_DIRS})

    message(STATUS "FOUND AMDSMI")
    message(STATUS " [*] ROCM_DIR = ${ROCM_DIR}")
    message(STATUS " [*] AMDSMI_INCLUDE_DIRS = ${AMDSMI_INCLUDE_DIRS}")
    message(STATUS " [*] AMDSMI_LIBRARY = ${AMDSMI_LIBRARY}")
# If ROCM_DIR not specified, then try to automatically find the AMD SMI header
# and library
elseif(NOT AMDSMI_FOUND)
    find_path(AMDSMI_INCLUDE_DIRS
        NAMES amd_smi/amdsmi.h
    )

    find_library(AMDSMI_LIBRARY
        NAMES libamd_smi.so
    )

    if(AMDSMI_INCLUDE_DIRS AND AMDSMI_LIBRARY)
        set(AMDSMI_FOUND TRUE CACHE INTERNAL "")
        set(VARIORUM_ROCM_DIR ${ROCM_DIR} CACHE INTERNAL "")
        set(ROCM_DIR ${ROCM_DIR} CACHE PATH "" FORCE)
        set(AMDSMI_INCLUDE_DIRS ${AMDSMI_INCLUDE_DIRS} CACHE PATH "" FORCE)
        set(AMDSMI_LIBRARY ${AMDSMI_LIBRARY} CACHE PATH "" FORCE)
        include_directories(${AMDSMI_INCLUDE_DIRS})

        message(STATUS "FOUND AMDSMI using find_library()")
        message(STATUS " [*] AMDSMI_INCLUDE_DIRS = ${AMDSMI_INCLUDE_DIRS}")
        message(STATUS " [*] AMDSMI_LIBRARY = ${AMDSMI_LIBRARY}")
    endif()
endif()

# Abort if all methods fail
if(NOT AMDSMI_FOUND)
    message(FATAL_ERROR "AMD APU support needs explicit ROCM_DIR (containing AMD SMI)")
endif()
