// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef AMD_APU_ESMI_FEATURES_H_INCLUDE
#define AMD_APU_ESMI_FEATURES_H_INCLUDE

#include <stdio.h>
#include <jansson.h>

// ESMI-based monitoring functions using esmi_metrics_table_get() interface
// These are test/development APIs to compare with the amdsmi_*() interface

// Energy monitoring via ESMI metrics table
void get_energy_data_esmi(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_energy_json_esmi(
    int chipid,
    int total_sockets,
    json_t *output
);

// Power monitoring via ESMI metrics table
void get_power_data_esmi(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_json_power_data_esmi(
    json_t *output,
    int nsockets
);

// Thermal monitoring via ESMI metrics table
void get_thermals_data_esmi(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_thermals_json_esmi(
    int chipid,
    int total_sockets,
    json_t *output
);

// Clock monitoring via ESMI metrics table
void get_clocks_data_esmi(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_clocks_json_esmi(
    int chipid,
    int total_sockets,
    json_t *output
);

// Comprehensive metrics dump via ESMI metrics table
void get_all_metrics_data_esmi(
    int chipid,
    int total_sockets,
    int verbose,
    FILE *output
);

void get_all_metrics_json_esmi(
    int chipid,
    int total_sockets,
    json_t *output
);

#endif
