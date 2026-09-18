# Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
# Variorum Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: MIT

set(variorum_amd_apu_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/mi300aAPU.h
  ${CMAKE_CURRENT_SOURCE_DIR}/amd_apu_power_features.h
  ${CMAKE_CURRENT_SOURCE_DIR}/amd_apu_esmi_features.h
  CACHE INTERNAL "")

set(variorum_amd_apu_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/mi300aAPU.c
  ${CMAKE_CURRENT_SOURCE_DIR}/amd_apu_power_features.c
  ${CMAKE_CURRENT_SOURCE_DIR}/amd_apu_esmi_features.c
  CACHE INTERNAL "")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${variorum_includes})

add_library(variorum_amd_apu OBJECT
            ${variorum_amd_apu_sources}
            ${variorum_amd_apu_headers})

### Shared libraries need PIC
set_property(TARGET variorum_amd_apu PROPERTY POSITION_INDEPENDENT_CODE 1)
