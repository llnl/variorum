# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

set(VARIORUM_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")

if(FORTRAN_FOUND)
    set(VARIORUM_FORTRAN_COMPILER ${CMAKE_Fortran_COMPILER})
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/variorum_config.h.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/variorum_config.h")

set(variorum_headers
  config_architecture.h
  variorum.h
  variorum_timers.h
  variorum_error.h
  variorum_topology.h
)

set(variorum_sources
  config_architecture.c
  variorum.c
  variorum_timers.c
  variorum_error.c
  variorum_topology.c
)

set(variorum_deps ""
    CACHE INTERNAL ""
)

set(variorum_includes
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HWLOC_INCLUDE_DIRS}
    ${ESMI_INCLUDE_DIRS}
)

if(VARIORUM_WITH_INTEL_CPU)
    add_subdirectory(msr)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/msr)
    list(APPEND variorum_headers Intel/config_intel.h ${variorum_intel_headers} ${variorum_msr_headers})
    list(APPEND variorum_sources Intel/config_intel.c ${variorum_msr_sources})
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_intel>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/Intel)
    add_subdirectory(Intel)
endif()

if(VARIORUM_WITH_INTEL_GPU)
    list(APPEND variorum_headers Intel_GPU/config_intel_gpu.h ${variorum_apmidg_headers} ${APMIDG_INCLUDE_DIRS}/libapmidg.h)
    list(APPEND variorum_sources Intel_GPU/config_intel_gpu.c)
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_intel_gpu>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/Intel_GPU ${APMIDG_INCLUDE_DIRS})
    add_subdirectory(Intel_GPU)
endif()

if(VARIORUM_WITH_IBM_CPU)
    list(APPEND variorum_headers IBM/config_ibm.h ${variorum_ibm_headers})
    list(APPEND variorum_sources IBM/config_ibm.c)
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_ibm>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/IBM)
    add_subdirectory(IBM)
endif()

if(VARIORUM_WITH_NVIDIA_GPU)
    list(APPEND variorum_headers Nvidia_GPU/config_nvidia.h ${variorum_nvidia_headers} ${NVML_INCLUDE_DIRS}/nvml.h)
    list(APPEND variorum_sources Nvidia_GPU/config_nvidia.c)
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_nvidia>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/Nvidia_GPU ${NVML_INCLUDE_DIRS})
    add_subdirectory(Nvidia_GPU)
endif()

if(VARIORUM_WITH_ARM_CPU)
    list(APPEND variorum_headers ARM/config_arm.h ${variorum_arm_headers})
    list(APPEND variorum_sources ARM/config_arm.c)
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_arm>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/ARM)
    add_subdirectory(ARM)
endif()

if(VARIORUM_WITH_AMD_CPU)
    add_subdirectory(msr)
    list(APPEND variorum_headers AMD/config_amd.h ${variorum_amd_headers} ${variorum_msr_headers})
    list(APPEND variorum_sources AMD/config_amd.c ${variorum_msr_sources})
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_amd>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/AMD)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/msr)
    add_subdirectory(AMD)
endif()


if(VARIORUM_WITH_AMD_GPU)
    list(APPEND variorum_headers AMD_GPU/config_amd_gpu.h ${variorum_amd_gpu_headers} ${ROCM_INCLUDE_DIRS}/rocm_smi/rocm_smi.h)
    list(APPEND variorum_sources AMD_GPU/config_amd_gpu.c)
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_amd_gpu>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/AMD_GPU ${ROCM_INCLUDE_DIRS})
    add_subdirectory(AMD_GPU)
endif()

if(VARIORUM_WITH_AMD_APU)
    list(APPEND variorum_headers AMD_APU/config_amd_apu.h ${variorum_amd_apu_headers} ${AMDSMI_INCLUDE_DIRS}/amd_smi/amdsmi.h)
    list(APPEND variorum_sources AMD_APU/config_amd_apu.c)
    list(APPEND variorum_deps $<TARGET_OBJECTS:variorum_amd_apu>)
    list(APPEND variorum_includes ${CMAKE_CURRENT_SOURCE_DIR}/AMD_APU ${AMDSMI_INCLUDE_DIRS})
    add_subdirectory(AMD_APU)
endif()

if(FORTRAN_FOUND)
    add_library(variorum_fortran OBJECT ../wrappers/fortran/variorum.f90)
    list(APPEND variorum_sources $<TARGET_OBJECTS:variorum_fortran>)

    #####################################################
    # Setup install to copy the fortran modules
    #####################################################
    install(FILES
            ${CMAKE_Fortran_MODULE_DIRECTORY}/variorum.mod
            DESTINATION include)
endif()

if(BUILD_SHARED_LIBS)
    add_library(variorum SHARED
                ${variorum_sources}
                ${variorum_headers}
                ${variorum_deps})
else()
    add_library(variorum STATIC
                ${variorum_sources}
                ${variorum_headers}
                ${variorum_deps})
endif()


target_link_libraries(variorum PUBLIC ${HWLOC_LIBRARY})
target_link_libraries(variorum PUBLIC ${JANSSON_LIBRARY})
target_link_libraries(variorum PUBLIC m)
if(LIBJUSTIFY_FOUND)
    target_link_libraries(variorum PUBLIC ${LIBJUSTIFY_LIBRARY})
endif()

if(VARIORUM_WITH_INTEL_GPU)
target_link_libraries(variorum PUBLIC ${APMIDG_LIBRARY})
endif()
if(VARIORUM_WITH_NVIDIA_GPU)
target_link_libraries(variorum PUBLIC ${NVML_LIBRARY})
endif()
if(VARIORUM_WITH_AMD_CPU)
target_link_libraries(variorum PUBLIC ${ESMI_LIBRARY})
endif()
if(VARIORUM_WITH_AMD_GPU)
target_link_libraries(variorum PUBLIC ${ROCM_LIBRARY})
endif()
if(VARIORUM_WITH_AMD_APU)
target_link_libraries(variorum PUBLIC ${AMDSMI_LIBRARY})
endif()

install(TARGETS variorum
        EXPORT  variorum
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib)

set(variorum_install_headers
    variorum.h
    variorum_topology.h
)

install(FILES ${variorum_install_headers}
        DESTINATION include)

include_directories(${variorum_includes})
