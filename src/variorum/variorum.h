// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef VARIORUM_H_INCLUDE
#define VARIORUM_H_INCLUDE

#include <stdio.h>

/// @brief Collect power limits and energy usage for both the package and DRAM
/// domains.
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @param [in] output Location for output (stdout, stderr, filename).
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_poll_power(FILE *output);

/// @brief Collect power limits and energy usage for both the package and DRAM
/// domains, fixed counters, TSC, APERF, and MPERF.
///
/// @supparch
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
///
/// @param [in] output Location for output (stdout, stderr, filename).
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_monitoring(FILE *output);

/*****************/
/* Cap Functions */
/*****************/
/// @brief Cap the power limits for all sockets within the node.
///
/// @supparch
/// - AMD EPYC Milan
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @param [in] socket_power_limit Desired power limit for each socket.
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_cap_each_socket_power_limit(int socket_power_limit);

/// @brief Cap the power limit of the node.
///
/// @supparch
/// - IBM Power9
/// - AMD EPYC Milan
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @param [in] node_power_limit Desired power limit for the node.
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_cap_best_effort_node_power_limit(int node_power_limit);

/// @brief Cap the power shifting ratio for the GPU (uniform on both sockets).
///
/// @supparch
/// - IBM Power9 (same ratio on both sockets)
///
/// @unsupported
/// - AMD EPYC Milan
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - NVIDIA Volta
///
/// @param [in] gpu_power_ratio Desired power ratio (percentage).
/// for the processor and GPU.
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_cap_gpu_power_ratio(int gpu_power_ratio);

/// @brief Cap the CPU frequency for all cores within a socket.
///
/// @supparch
/// - AMD EPYC Milan
/// - Intel Skylake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @param [in] cpu_freq_mhz Desired CPU frequency for each core in MHz.
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_cap_each_core_frequency_limit(int cpu_freq_mhz);

/// @brief Cap the frequency of the target processor.
///
/// @supparch
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - AMD EPYC Milan
///
/// @param [in] socketid Target socket ID.
/// @param [in] socket_freq_mhz Desired socket frequency in MHz.
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_cap_socket_frequency_limit(int socketid, int socket_freq_mhz);

/// @brief Cap the power usage identically of each GPU on the node.
///
/// @supparch
/// - NVIDIA Volta, Ampere
/// - AMD Instinct (MI-50 onwards)
/// - Intel Discrete GPU
///
/// @param [in] gpu_power_limit Desired power limit in watts for each GPU
///             on the node.
///
/// @return 0 if successful, otherwise -1
int variorum_cap_each_gpu_power_limit(int gpu_power_limit);

/*******************/
/* Print Functions */
/*******************/
/// @brief Print power limits for all known domains in long format.
///
/// @supparch
/// - AMD EPYC Milan
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_power_limit(void);

/// @brief Print power limits for all known domains in CSV format.
///
/// @supparch
/// - AMD EPYC Milan
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_power_limit(void);

/// @brief Print thermal data in long format.
///
/// @supparch
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_thermals(void);

/// @brief Print thermal data in CSV format.
///
/// @supparch
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Arctic Sound
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_thermals(void);

/// @brief Print performance counter data in long format.
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_counters(void);

/// @brief Print performance counter data in CSV format.
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_counters(void);

/// @brief Print power usage data in long format.
///
/// @supparch
/// - AMD EPYC Milan
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_power(void);

/// @brief Print power usage data in CSV format.
///
/// @supparch
/// - AMD EPYC Milan
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
/// - Intel Arctic Sound
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_power(void);

/// @brief Print current operating frequency (APERF/MPERF and PERF_STATUS) in
/// long format.
///
/// @supparch
/// - AMD EPYC Milan
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_frequency(void);

/// @brief Print current operating frequency (APERF/MPERF and PERF_STATUS) in
/// CSV format.
///
/// @supparch
/// - AMD EPYC Milan
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Arctic Sound
/// - NVIDIA Volta
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_frequency(void);

/// @brief Print if hyperthreading is enabled or disabled.
///
/// @return 0 if successful, otherwise -1
int variorum_print_hyperthreading(void);

/// @brief Print architecture topology in long format.
void variorum_print_topology(void);

/// @brief Print list of features available on a particular architecture.
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_features(void);

/// @brief Print if turbo is enabled or disabled. If enabled, then print
/// discrete frequencies in turbo range (i.e., max turbo ratio).
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_turbo(void);

/// @brief Print verbose GPU streaming multi-processor and memory utilization.
///
/// @supparch
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - NVIDIA Volta
///
/// @unsupported
/// - AMD EPYC Milan
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_gpu_utilization(void);

/// @brief Print CSV-formatted GPU streaming multi-processor and memory utilization.
///
/// @supparch
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - NVIDIA Volta
///
/// @unsupported
/// - AMD EPYC Milan
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_gpu_utilization(void);

/// @brief Print list of available frequencies from p-states, turbo, AVX, etc. ranges.
///
/// @supparch
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_available_frequencies(void);

/// @brief Print if core and socket energy is available.
///
/// @supparch
/// - AMD EPYC Milan
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_energy(void);

/// @brief Print verbose format of core and socket energy if available.
///
/// @supparch
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_print_verbose_energy(void);

/***************************/
/* Enable/Disable Features */
/***************************/
/// @brief Enable turbo feature.
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_enable_turbo(void);

/// @brief Disable turbo feature.
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
///
/// @return 0 if successful or if feature has not been implemented or is
/// not supported, otherwise -1
int variorum_disable_turbo(void);

/****************/
/* JSON Support */
/****************/
/// @brief Populate a string in JSON format with total node power, socket-level,
/// memory-level and GPU-level power.
///
/// @supparch
/// - AMD EPYC Milan
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
/// - NVIDIA Volta
/// - AMD Radeon Instinct GPUs (MI50 onwards)
///
/// @param [out] get_power_obj_str String (passed by reference) that contains the
/// node-level power information.
///
/// @return 0 if successful, otherwise -1. Note that feature not implemented
/// returns a -1 for the JSON APIs so that users don't have to explicitly
/// check for NULL strings.
int variorum_get_power_json(char **get_power_obj_str);

/// @brief Populate a string in JSON format with total CPU node utilization, user
/// CPU utilization, kernel CPU utilization, total node memory utilization, and GPU
/// utilization.
///
/// Format:
/// {
///     "hostname": {
///         "CPU": {
///             "total_util%": total_CPU_utilization,
///             "user_util%": user_utilization,
///             "system_util%": system_utilization,
///         },
///         "GPU": {
///             Socket_n : {
///                 GPUnm_util% : GPU_utilization
///         },
///         "memory_util%": total_memory_utilization,
///         "timestamp" : timestamp
/// }
/// where n is the socket number and m is the GPU id.
///
/// @supparch
/// - AMD Radeon Instinct GPUs (MI50 onwards)
/// - NVIDIA Volta
///
/// @unsupported
/// - AMD EPYC Milan
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Ice Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @param [out] get_util_obj_str String (passed by reference) that contains node-level
/// utilization information.
///
/// @returns 0 if successful, otherwise -1. Note that feature not implemented
/// returns a -1 for the JSON APIs so that users don't have to explicitly
/// check for NULL strings.
int variorum_get_utilization_json(char **get_util_obj_str);

/// @brief Populate a string in JSON format with measurable and controllable
/// power domains, along with the ranges.
///
/// @supparch
/// - AMD EPYC Milan
/// - ARM Juno r2
/// - Ampere Neoverse N1
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
/// - Intel Sapphire Rapids
///
/// @param [out] get_domain_obj_str String (passed by reference) that contains the
/// node-level domain information.
///
/// @return 0 if successful, otherwise -1. Note that feature not implemented
/// returns a -1 for the JSON APIs so that users don't have to explicitly
/// check for NULL strings.
int variorum_get_node_power_domain_info_json(char **get_domain_obj_str);

/// @brief Populate a string in nested JSON format for temperature readouts.
///
/// Format:
/// hostname {
///     Socket_n {
///         CPU {
///             Core {
///                 Sensor Name : Temp in C
///             },
///             Mem {
///                 Sensor Name : Temp in C
///             }
///         }
///         GPU {
///             Device : Temp in C
///         }
///     }
/// }
/// where n is the socket number
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - IBM Power9
/// - AMD Instinct
/// - NVIDIA Volta
///
/// @param [out] get_thermal_obj_str String (passed by reference) that contains
/// node-level thermal information.
///
/// @returns 0 if successful, otherwise -1. Note that feature not implemented
/// returns a -1 for the JSON APIs so that users don't have to explicitly
/// check for NULL strings.
int variorum_get_thermals_json(char **get_thermal_obj_str);

/// @brief Populate a string in JSON format with node level frequency information
///
/// @supparch
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - IBM Power9
/// - AMD Instinct
/// - NVIDIA Volta
///
/// @param [out] get_frequency_obj_str String (passed by reference) that contains the
/// node-level frequency information.
///
/// @return 0 if successful, otherwise -1. Note that feature not implemented
/// returns a -1 for the JSON APIs so that users don't have to explicitly
/// check for NULL strings.
int variorum_get_frequency_json(char **get_frequency_obj_str);

/// @brief Populate a string in JSON format with node level energy information
///
/// @supparch
/// - IBM Power9
/// - Intel Sandy Bridge
/// - Intel Ivy Bridge
/// - Intel Haswell
/// - Intel Broadwell
/// - Intel Skylake
/// - Intel Kaby Lake
/// - Intel Cascade Lake
/// - Intel Cooper Lake
///
/// @param [out] get_energy_obj_str String (passed by reference) containing
/// the node-level energy information.
///
/// @return 0 if successful, otherwise -1. Note that feature not implemented
/// returns a -1 for the JSON APIs so that users don't have to explicitly
/// check for NULL strings.
int variorum_get_energy_json(char **get_energy_obj_str);

/// @brief Returns Variorum version as a constant string.
///
/// @supparch
/// - All architectures
///
/// @return Returns a constant string containing the
/// current value of VARIORUM_VERSION.
#define QuoteIdent(ident) #ident
#define QuoteMacro(macro) QuoteIdent(macro)
char *variorum_get_current_version(void);

/***********/
/* Testing */
/***********/
/// @brief Test for memory leaks.
int variorum_tester(void);

/*********************************/
/* Exploratory AMD ESMI API      */
/*********************************/
/// @brief Explore AMD metrics using the ESMI metrics table API.
///
/// This is an exploratory API for playing with AMD MI300A metrics
/// using the esmi_metrics_table_get() interface. It returns a JSON
/// string with all available metrics from the ESMI metrics table.
///
/// @supparch
/// - AMD MI300A APU
///
/// @param [out] metrics_json_str String (passed by reference) that contains
/// all metrics from the ESMI metrics table in JSON format.
///
/// @return 0 if successful, otherwise -1.
int variorum_get_amd_esmi_metrics_json(char **metrics_json_str);

#endif
