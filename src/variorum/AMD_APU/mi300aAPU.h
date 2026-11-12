// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef MI300AAPU_H_INCLUDE
#define MI300AAPU_H_INCLUDE

#include <jansson.h>
#include <sys/time.h>

// Energy monitoring functions (NEW)
int amd_apu_mi300a_get_energy(
    int verbose
);

int amd_apu_mi300a_get_energy_json(
    json_t *get_energy_obj
);

// Power monitoring
int amd_apu_mi300a_get_power(
    int verbose
);

int amd_apu_mi300a_get_power_json(
    json_t *get_power_obj
);

int amd_apu_mi300a_get_power_limit(
    int verbose
);

// Thermal monitoring
int amd_apu_mi300a_get_thermals(
    int verbose
);

int amd_apu_mi300a_get_thermals_json(
    json_t *get_thermal_obj
);

// Clock monitoring
int amd_apu_mi300a_get_clocks(
    int verbose
);

int amd_apu_mi300a_get_clocks_json(
    json_t *get_clocks_obj_json
);

// Utilization monitoring
int amd_apu_mi300a_get_gpu_utilization(
    int verbose
);

int amd_apu_mi300a_get_gpu_utilization_json(
    char **get_gpu_util_obj_str
);

// Control functions
int amd_apu_mi300a_cap_each_gpu_power_limit(
    unsigned int powerlimit
);

#endif
