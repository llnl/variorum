# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

target_link_libraries(variorum ${variorum_deps})

add_definitions(-DSECOND_RUN)

set(BASIC_EXAMPLES
    variorum-cap-best-effort-node-power-limit-example
    variorum-cap-each-core-frequency-limit-example
    variorum-cap-gpu-power-limit-example
    variorum-cap-gpu-power-ratio-example
    variorum-cap-socket-frequency-limit-example
    variorum-cap-socket-power-limit-example
    variorum-disable-turbo-example
    variorum-enable-turbo-example
    variorum-get-energy-json-example
    variorum-get-frequency-json-example
    variorum-get-node-power-domain-info-json-example
    variorum-get-power-json-example
    variorum-get-thermals-json-example
    variorum-get-utilization-json-example
    variorum-get-topology-info-example
    variorum-integration-using-json-example
    variorum-monitoring-to-file-example
    variorum-poll-power-to-file-example
    variorum-poll-power-to-stdout-example
    variorum-print-available-frequencies-example
    variorum-print-counters-example
    variorum-print-energy-example
    variorum-print-features-example
    variorum-print-frequency-example
    variorum-print-gpu-utilization-example
    variorum-print-hyperthreading-example
    variorum-print-power-example
    variorum-print-power-limit-example
    variorum-print-thermals-example
    variorum-print-topology-example
    variorum-print-turbo-example
    variorum-print-verbose-counters-example
    variorum-print-verbose-energy-example
    variorum-print-verbose-frequency-example
    variorum-print-verbose-gpu-utilization-example
    variorum-print-verbose-power-example
    variorum-print-verbose-power-limit-example
    variorum-print-verbose-thermals-example
    variorum_esmi_explore
)

message(STATUS "Adding variorum examples")
foreach(EXAMPLE ${BASIC_EXAMPLES})
    message(STATUS " [*] Adding example: ${EXAMPLE}")
    add_executable(${EXAMPLE} ${EXAMPLE}.c)
    target_link_libraries(${EXAMPLE} variorum ${variorum_deps})
endforeach()

if(MPI_FOUND)
    add_subdirectory(mpi-examples)
endif()

if(OPENMP_FOUND)
    add_subdirectory(openmp-examples)
endif()

if(FORTRAN_FOUND)
    add_subdirectory(fortran-examples)
endif()

if(Python_FOUND)
    add_subdirectory(python-examples)
endif()

include_directories(${CMAKE_SOURCE_DIR}/variorum)

install(DIRECTORY using-with-make using-with-cmake
        DESTINATION examples)

# quick hack
if(VARIORUM_WITH_INTEL_GPU)
	set(CMAKE_EXE_LINKER_FLAGS "-lze_loader -lstdc++ -L${APMIDG_DIR}/lib64/ -lapmidg")
endif()
