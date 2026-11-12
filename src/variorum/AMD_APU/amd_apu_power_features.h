// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef AMD_APU_POWER_FEATURES_H_INCLUDE
#define AMD_APU_POWER_FEATURES_H_INCLUDE

#include <stdio.h>
#include <jansson.h>
#include <amd_smi/amdsmi.h>

// Energy monitoring functions
void get_energy_data(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_energy_json(
    int chipid,
    int total_sockets,
    json_t *output
);

// Power monitoring functions
void get_power_data(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_json_power_data(
    json_t *output,
    int nsockets
);

// Thermal monitoring functions
void get_thermals_data(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_thermals_json(
    int chipid,
    int total_sockets,
    json_t *output
);

// Clock monitoring functions
void get_clocks_data(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_clocks_json(
    int chipid,
    int total_sockets,
    json_t *output
);

// Power limit functions
void get_power_limit_data(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void cap_each_gpu_power_limit(
    int chipid,
    int total_sockets,
    unsigned int powerlimit
);

// GPU utilization functions
void get_gpu_utilization_data(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_gpu_utilization_data_json(
    int chipid,
    int total_sockets,
    json_t *output
);

#endif
