// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef CONFIG_AMD_APU_H_INCLUDE
#define CONFIG_AMD_APU_H_INCLUDE

#include <stdint.h>

uint64_t *detect_amd_apu_arch(void);

int set_amd_apu_func_ptrs(int idx);

#endif
