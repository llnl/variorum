// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

#include <mi300aAPU.h>
#include <config_architecture.h>
#include <variorum_error.h>
#include <amd_apu_power_features.h>

int amd_apu_mi300a_get_energy(int verbose)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_energy_data(iter, nsockets, verbose, stdout);
    }
    return 0;
}

int amd_apu_mi300a_get_energy_json(json_t *get_energy_obj)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);

    for (iter = 0; iter < nsockets; iter++)
    {
        get_energy_json(iter, nsockets, get_energy_obj);
    }

    return 0;
}

int amd_apu_mi300a_get_power(int verbose)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_power_data(iter, nsockets, verbose, stdout);
    }
    return 0;
}

int amd_apu_mi300a_get_power_json(json_t *get_power_obj)
{
    char *val = getenv("VARIORUM_LOG");
    unsigned nsockets;

    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    get_json_power_data(get_power_obj, nsockets);

    return 0;
}

int amd_apu_mi300a_get_thermals(int verbose)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_thermals_data(iter, nsockets, verbose, stdout);
    }
    return 0;
}

int amd_apu_mi300a_get_thermals_json(json_t *get_thermal_obj)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);

    for (iter = 0; iter < nsockets; iter++)
    {
        get_thermals_json(iter, nsockets, get_thermal_obj);
    }

    return 0;
}

int amd_apu_mi300a_get_clocks(int verbose)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_clocks_data(iter, nsockets, verbose, stdout);
    }
    return 0;
}

int amd_apu_mi300a_get_clocks_json(json_t *get_clocks_obj_json)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);

    for (iter = 0; iter < nsockets; iter++)
    {
        get_clocks_json(iter, nsockets, get_clocks_obj_json);
    }

    return 0;
}

int amd_apu_mi300a_get_power_limit(int verbose)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_power_limit_data(iter, nsockets, verbose, stdout);
    }
    return 0;
}

int amd_apu_mi300a_get_gpu_utilization(int verbose)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_gpu_utilization_data(iter, nsockets, verbose, stdout);
    }
    return 0;
}

int amd_apu_mi300a_get_gpu_utilization_json(char **get_gpu_util_obj_str)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    json_t *get_util_obj = json_object();
    unsigned iter = 0;
    unsigned nsockets;

#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif

    for (iter = 0; iter < nsockets; iter++)
    {
        get_gpu_utilization_data_json(iter, nsockets, get_util_obj);
    }

    *get_gpu_util_obj_str = json_dumps(get_util_obj, JSON_INDENT(4));
    json_decref(get_util_obj);
    return 0;
}

int amd_apu_mi300a_cap_each_gpu_power_limit(unsigned int powerlimit)
{
    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("Running %s\n", __FUNCTION__);
    }

    unsigned iter = 0;
    unsigned nsockets;
#ifdef VARIORUM_WITH_AMD_APU
    variorum_get_topology(&nsockets, NULL, NULL, P_AMD_APU_IDX);
#endif
    for (iter = 0; iter < nsockets; iter++)
    {
        cap_each_gpu_power_limit(iter, nsockets, powerlimit);
    }
    return 0;
}
