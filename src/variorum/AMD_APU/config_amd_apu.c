// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <config_amd_apu.h>
#include <config_architecture.h>
#include <amd_apu_power_features.h>
#include <mi300aAPU.h>
#include <variorum_error.h>

#include <amd_smi/amdsmi.h>

uint64_t *detect_amd_apu_arch(void)
{
    amdsmi_status_t ret;
    uint32_t socket_count = 0;
    uint32_t device_count = 0;
    uint64_t *model = (uint64_t *) malloc(sizeof(uint64_t));

    // Default to MI300A
    *model = AMD_MI300A;

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        // If AMDSMI fails, return MI300A as default
        return model;
    }

    ret = amdsmi_get_socket_handles(&socket_count, NULL);
    if (ret != AMDSMI_STATUS_SUCCESS || socket_count == 0)
    {
        amdsmi_shut_down();
        return model;
    }

    amdsmi_socket_handle *sockets =
        (amdsmi_socket_handle *) malloc(socket_count * sizeof(
                                            amdsmi_socket_handle));
    if (sockets == NULL)
    {
        amdsmi_shut_down();
        return model;
    }

    ret = amdsmi_get_socket_handles(&socket_count, sockets);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        free(sockets);
        amdsmi_shut_down();
        return model;
    }

    ret = amdsmi_get_processor_handles(sockets[0], &device_count, NULL);
    if (ret != AMDSMI_STATUS_SUCCESS || device_count == 0)
    {
        free(sockets);
        amdsmi_shut_down();
        return model;
    }

    amdsmi_processor_handle first_device;
    device_count = 1;
    ret = amdsmi_get_processor_handles(sockets[0], &device_count, &first_device);

    if (ret == AMDSMI_STATUS_SUCCESS)
    {
        // Check first device's board info for MI300A identification
        amdsmi_board_info_t board_info;
        ret = amdsmi_get_gpu_board_info(first_device, &board_info);

        if (ret == AMDSMI_STATUS_SUCCESS)
        {
            // Check if product name contains "MI300A" or "MI300"
            // Convert to uppercase for case-insensitive comparison
            char name_upper[256];
            int i;
            for (i = 0; board_info.product_name[i] && i < 255; i++)
            {
                name_upper[i] = (board_info.product_name[i] >= 'a' &&
                                 board_info.product_name[i] <= 'z') ?
                                board_info.product_name[i] - 32 :
                                board_info.product_name[i];
            }
            name_upper[i] = '\0';

            if (strstr(name_upper, "MI300A") != NULL ||
                strstr(name_upper, "MI300") != NULL)
            {
                *model = AMD_MI300A;
            }
        }
    }

    free(sockets);
    amdsmi_shut_down();
    return model;
}

int set_amd_apu_func_ptrs(int idx)
{
    int err = 0;

    if (*g_platform[idx].arch_id == AMD_MI300A)
    {
        /* Initialize monitoring interfaces */
        g_platform[idx].variorum_print_power = amd_apu_mi300a_get_power;
        g_platform[idx].variorum_print_thermals = amd_apu_mi300a_get_thermals;
        g_platform[idx].variorum_print_frequency = amd_apu_mi300a_get_clocks;
        g_platform[idx].variorum_print_power_limit = amd_apu_mi300a_get_power_limit;
        g_platform[idx].variorum_print_gpu_utilization =
            amd_apu_mi300a_get_gpu_utilization;

        /* Initialize JSON interfaces */
        g_platform[idx].variorum_get_thermals_json = amd_apu_mi300a_get_thermals_json;
        g_platform[idx].variorum_get_frequency_json = amd_apu_mi300a_get_clocks_json;
        g_platform[idx].variorum_get_utilization_json =
            amd_apu_mi300a_get_gpu_utilization_json;
        g_platform[idx].variorum_get_power_json = amd_apu_mi300a_get_power_json;

        /* Initialize control interfaces */
        g_platform[idx].variorum_cap_each_gpu_power_limit =
            amd_apu_mi300a_cap_each_gpu_power_limit;

        /* Initialize ENERGY interfaces - NEW FOR MI300A */
        g_platform[idx].variorum_print_energy = amd_apu_mi300a_get_energy;
        g_platform[idx].variorum_get_energy_json = amd_apu_mi300a_get_energy_json;
    }
    else
    {
        err = VARIORUM_ERROR_UNSUPPORTED_PLATFORM;
    }

    return err;
}
