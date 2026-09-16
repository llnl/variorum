// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <hwloc.h>
#include <inttypes.h>
#include <jansson.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include <config_architecture.h>
#include <variorum.h>
#include <variorum_error.h>

#ifdef LIBJUSTIFY_FOUND
#include <cprintf.h>
#endif

#define MEM_FILE "/proc/meminfo"
#define CPU_FILE "/proc/stat"

uint64_t last_sum = 0;
uint64_t last_user_time = 0;
uint64_t last_sys_time = 0;
uint64_t last_idle = 0;
int state = 0;
int g_socket;
int g_core;
FILE *fp = 0;

static void print_children(hwloc_topology_t topology, hwloc_obj_t obj,
                           int depth)
{
    unsigned i;

#ifdef LIBJUSTIFY_FOUND
    if (depth == 0) //First interation
    {
        cfprintf(stdout, "%s %s %s %s\n", "Thread", "HWThread", "Core", "Socket");
    }
#endif

    if (depth == hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET))
    {
        g_socket = obj->os_index;
    }
    if (depth == hwloc_get_type_depth(topology, HWLOC_OBJ_CORE))
    {
        g_core = obj->logical_index;
    }
    if (depth == hwloc_get_type_depth(topology, HWLOC_OBJ_PU))
    {
#ifdef LIBJUSTIFY_FOUND
        cfprintf(stdout, "%d %d %d %d\n", obj->logical_index, obj->os_index, g_core,
                 g_socket);
#else
        printf("%3u %6u %8u %4u\n", obj->logical_index, obj->os_index, g_core,
               g_socket);
#endif
    }

    for (i = 0; i < obj->arity; i++)
    {
        print_children(topology, obj->children[i], depth + 1);
    }
    //exit condition
#ifdef LIBJUSTIFY_FOUND
    if ((int)obj->logical_index == hwloc_get_type_depth(topology,
            HWLOC_OBJ_NUMANODE))
    {
        cflush();
    }
#endif
}

int variorum_tester(void)
{
    int err = 0;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_poll_power(FILE *output)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_poll_power == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_poll_power(output);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_monitoring(FILE *output)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_monitoring == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_monitoring(output);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_power_limit(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_power_limit == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_power_limit(0);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_power_limit(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_power_limit == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_power_limit(1);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

void variorum_print_topology(void)
{
    hwloc_topology_t topo;
    int i;

    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        hwloc_topology_init(&topo);
        hwloc_topology_load(topo);

        variorum_get_topology(NULL, NULL, NULL, i);

#ifdef LIBJUSTIFY_FOUND
        cfprintf(stdout, "=================\n");
        cfprintf(stdout, "Platform Topology\n");
        cfprintf(stdout, "=================\n");
        cfprintf(stdout, "  %-s: %-s\n", "Hostname", g_platform[i].hostname);
        cfprintf(stdout, "  %-s: %-d\n", "Num Sockets", g_platform[i].num_sockets);
        cfprintf(stdout, "  %-s: %-d\n", "Num Cores per Socket",
                 g_platform[i].num_cores_per_socket);
        cfprintf(stdout, "  %-s: %-d\n", "Num Threads per Core",
                 g_platform[i].num_threads_per_core);
        if (g_platform[i].num_threads_per_core == 1)
        {
            cfprintf(stdout, "  %-s: %-s\n", "  Hyperthreading", "No");
        }
        else
        {
            cfprintf(stdout, "  %-s: %-s\n", "  Hyperthreading", "Yes");
        }

        cfprintf(stdout, "\n");
        cfprintf(stdout, "  %-s: %-d\n", "Total Num of Cores",
                 g_platform[i].total_cores);
        cfprintf(stdout, "  %-s: %-d\n", "Total Num of Threads",
                 g_platform[i].total_threads);
        cfprintf(stdout, "\n");
        cfprintf(stdout, "Layout:\n");
        cfprintf(stdout, "-------\n");
        cflush();
#else
        fprintf(stdout, "=================\n");
        fprintf(stdout, "Platform Topology\n");
        fprintf(stdout, "=================\n");
        fprintf(stdout, "  Hostname            : %s\n", g_platform[i].hostname);
        fprintf(stdout, "  Num Sockets         : %d\n", g_platform[i].num_sockets);
        fprintf(stdout, "  Num Cores per Socket: %d\n",
                g_platform[i].num_cores_per_socket);
        fprintf(stdout, "  Num Threads per Core: %d\n",
                g_platform[i].num_threads_per_core);
        if (g_platform[i].num_threads_per_core == 1)
        {
            fprintf(stdout, "    Hyperthreading    : No\n");
        }
        else
        {
            fprintf(stdout, "    Hyperthreading    : Yes\n");
        }
        fprintf(stdout, "\n");
        fprintf(stdout, "  Total Num of Cores  : %d\n", g_platform[i].total_cores);
        fprintf(stdout, "  Total Num of Threads: %d\n", g_platform[i].total_threads);
        fprintf(stdout, "\n");
        fprintf(stdout, "Layout:\n");
        fprintf(stdout, "-------\n");
        fprintf(stdout, "Thread HWThread Core Socket\n");
#endif

        print_children(topo, hwloc_get_root_obj(topo), 0);
        hwloc_topology_destroy(topo);
    }

    return;
}

// The best effort node power limit is a node-level API, and cannot be implemented
// at a per-platform level. This can only be set by what we define as the
// 'primary' platform, e.g. IBM Power9 CPU or Intel and AMD CPUs, and internally,
// the implementation can determine how much power to allocate to CPU, GPUs and
// memory where applicable. Current implementation is basic, and in the future,
// we will update this API to do something reasonable at the node-level for
// integration with tools such as Flux or SLURM.

int variorum_cap_best_effort_node_power_limit(int node_power_limit)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    // Obtain the index corresponding to the primary platform.
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
#ifdef VARIORUM_WITH_INTEL_CPU
        i = P_INTEL_CPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_IBM_CPU
        i = P_IBM_CPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_AMD_CPU
        i = P_AMD_CPU_IDX;
        break;
#endif
    }

    if (g_platform[i].variorum_cap_best_effort_node_power_limit == NULL)
    {
        variorum_error_handler("Feature not yet implemented or is not supported",
                               VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                               getenv("HOSTNAME"), __FILE__,
                               __FUNCTION__, __LINE__);
        return 0;
    }
    err = g_platform[i].variorum_cap_best_effort_node_power_limit(
              node_power_limit);
    if (err)
    {
        return -1;
    }

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_cap_gpu_power_ratio(int gpu_power_ratio)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_cap_gpu_power_ratio == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_cap_gpu_power_ratio(gpu_power_ratio);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_cap_each_socket_power_limit(int socket_power_limit)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_cap_each_socket_power_limit == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_cap_each_socket_power_limit(socket_power_limit);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_cap_each_core_frequency_limit(int core_freq_mhz)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_cap_each_core_frequency_limit == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_cap_each_core_frequency_limit(core_freq_mhz);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_cap_socket_frequency_limit(int socketid, int socket_freq_mhz)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_cap_socket_frequency_limit == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_cap_socket_frequency_limit(socketid,
                socket_freq_mhz);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_cap_each_gpu_power_limit(int gpu_power_limit)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_cap_each_gpu_power_limit == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_cap_each_gpu_power_limit(gpu_power_limit);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_features(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_features == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_features();
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_thermals(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_thermals == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_thermals(0);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_thermals(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_thermals == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_thermals(1);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_counters(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_counters == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_counters(0);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_counters(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_counters == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_counters(1);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_frequency(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_frequency == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_frequency(0);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_frequency(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_frequency == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_frequency(1);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_power(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_power == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_power(0);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_power(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_power == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_power(1);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_hyperthreading(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        int hyperthreading = (g_platform[i].num_threads_per_core == 1) ? 0 : 1;
#ifdef LIBJUSTIFY_FOUND
        if (hyperthreading == 1)
        {
            cfprintf(stdout, "  %-s %s\n", "Hyperthreading:", "Enabled");
            cfprintf(stdout, "  %-s %-d\n", "Num Thread Per Core: ",
                     g_platform[i].num_threads_per_core);
        }
        else
        {
            cfprintf(stdout, "  %-s %s\n", "Hyperthreading:", "Disabled");
        }
#else
        if (hyperthreading == 1)
        {
            fprintf(stdout, "  Hyperthreading:       Enabled\n");
            fprintf(stdout, "  Num Thread Per Core:  %d\n",
                    g_platform[i].num_threads_per_core);
        }
        else
        {
            fprintf(stdout, "  Hyperthreading:       Disabled\n");
        }
#endif

    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);

#ifdef LIBJUSTIFY_FOUND
    cflush(); //TODO: Create a silent version on err that still frees.
#endif

    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_turbo(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_turbo == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_turbo();
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_gpu_utilization(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_gpu_utilization == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_gpu_utilization(0);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_gpu_utilization(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_gpu_utilization == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_print_gpu_utilization(1);
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_enable_turbo(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_enable_turbo == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_enable_turbo();
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_disable_turbo(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_disable_turbo == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_disable_turbo();
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_get_power_json(char **get_power_obj_str)
{
    int err = 0;
    int i;
    uint64_t ts;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    char hostname[1024];
    gethostname(hostname, 1024);

    struct timeval tv;

    json_t *get_power_obj = json_object();
    json_t *node_obj = json_object();
    json_object_set_new(get_power_obj, hostname, node_obj);

    gettimeofday(&tv, NULL);
    ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    json_object_set_new(node_obj, "timestamp", json_integer(ts));

    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_get_power_json == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_get_power_json(node_obj);
        if (err)
        {
            // For the JSON functions, we return a -1 here, so users don't need
            // to explicitly check for NULL strings.
            return -1;
        }
    }

    *get_power_obj_str = json_dumps(get_power_obj, JSON_INDENT(4));
    json_decref(get_power_obj);

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_get_utilization_json(char **get_util_obj_str)
{
    int err = 0;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    char hostname[1024];
    struct timeval tv;
    uint64_t ts;
    gethostname(hostname, 1024);
    gettimeofday(&tv, NULL);
    ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    char str[100];
    const char d[2] = " ";
    char *token, *s, *p;
    int i = 0;
    uint64_t sum = 0;
    uint64_t idle = 0;
    uint64_t user_time = 0;
    uint64_t nice_time = 0;
    uint64_t sum_user_time = 0;
    uint64_t iowait = 0;
    uint64_t sum_idle = 0;
    double cpu_util = 0.0;
    double user_util = 0.0;
    double sys_util = 0.0;
    double mem_util = 0.0;
    int rc, j;
    char lbuf[256];
    char metric_name[256];
    uint64_t metric_value;
    uint64_t mem_total = 0;
    uint64_t mem_free = 0;
    uint64_t sys_time = 0;
    int strcp;
    int idx = -1;

    json_t *get_util_obj = NULL;
    json_t *get_cpu_util_obj = NULL;
    json_t *get_timestamp_obj = NULL;
    json_t *cpu_util_obj = NULL;

    // Look for a GPU build and get an ID.
    for (idx = 0; idx < P_NUM_PLATFORMS; idx++)
    {
#ifdef VARIORUM_WITH_INTEL_GPU
        idx = P_INTEL_GPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_NVIDIA_GPU
        idx = P_NVIDIA_GPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_AMD_GPU
        idx = P_AMD_GPU_IDX;
        break;
#endif
    }

    // If we have a GPU build, obtain the GPU object first.
#if defined(VARIORUM_WITH_NVIDIA_GPU) || defined(VARIORUM_WITH_AMD_GPU) || defined(VARIORUM_WITH_INTEL_GPU)
    int ret;
    char *gpu_util_str = NULL;
    // get gpu utilization
    ret = g_platform[idx].variorum_get_utilization_json(&gpu_util_str);
    if (ret != 0)
    {
        printf("JSON get gpu utilization failed. Exiting.\n");
        free(gpu_util_str);
        return -1;
    }

    /* Load the existing GPU string as a JSON object using Jansson */
    get_util_obj = json_loads(gpu_util_str, JSON_DECODE_ANY, NULL);
    get_cpu_util_obj = json_object_get(get_util_obj, hostname);
    get_timestamp_obj = json_object_get(get_cpu_util_obj, "timestamp");
    cpu_util_obj = json_object_get(get_cpu_util_obj, "CPU");
#endif

    //CPU-only build will have this object as NULL.
    if (get_util_obj == NULL)
    {
        get_util_obj = json_object();
        get_cpu_util_obj = json_object();
        json_object_set_new(get_util_obj, hostname, get_cpu_util_obj);
    }

    if (get_timestamp_obj == NULL)
    {
        json_object_set_new(get_cpu_util_obj, "timestamp", json_integer(ts));
    }

    if (cpu_util_obj == NULL)
    {
        cpu_util_obj = json_object();
        json_object_set_new(get_cpu_util_obj, "CPU", cpu_util_obj);
    }

    // read /proc/stat file
    fp = fopen(CPU_FILE, "r");
    if (fp == NULL)
    {
        return -1;
    }
    // read the first line (cpu)
    if (fgets(str, 100, fp) == NULL)
    {
        return -1;
    }
    token = strtok(str, d);
    sum = 0;
    // get required values to compute cpu utilizations
    while (token != NULL)
    {
        token = strtok(NULL, d);
        if (token != NULL)
        {
            sum += strtol(token, &p, 10);
            if (i == 3)
            {
                idle = strtol(token, &p, 10);
            }
            if (i == 0)
            {
                user_time = strtol(token, &p, 10);
            }
            if (i == 1)
            {
                nice_time = strtol(token, &p, 10);
            }
            if (i == 2)
            {
                sys_time = strtol(token, &p, 10);
            }
            if (i == 4)
            {
                iowait = strtol(token, &p, 10);
            }
            sum_idle = idle + iowait;
            sum_user_time = user_time + nice_time;
            i++;
        }
    }

    fclose(fp);
    // make the utilization metrics 0 at the first sample
    if (state)
    {
        user_util = ((sum_user_time - last_user_time) / (double)(sum - last_sum)) * 100;
        sys_util = ((sys_time - last_sys_time) / (double)(sum - last_sum)) * 100;
        cpu_util = (1 - ((sum_idle - last_idle) / (double)(sum - last_sum))) * 100;
    }
    else
    {
        user_util = 0.0;
        sys_util = 0.0;
        cpu_util = 0.0;
    }

    last_user_time = sum_user_time;
    last_sum = sum;
    last_sys_time = sys_time;
    last_idle = sum_idle;

    json_object_set_new(cpu_util_obj, "total_util%", json_real(cpu_util));
    json_object_set_new(cpu_util_obj, "user_util%", json_real(user_util));
    json_object_set_new(cpu_util_obj, "system_util%", json_real(sys_util));
    fp = fopen(MEM_FILE, "r");
    if (fp == NULL)
    {
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    do
    {
        s = fgets(lbuf, sizeof(lbuf), fp);
        if (!s)
        {
            break;
        }

        rc = sscanf(lbuf, "%s%" PRIu64, metric_name, &metric_value);
        if (rc < 2)
        {
            break;
        }

        /* Strip the colon from metric name if present */
        j = strlen(metric_name);
        if (i && metric_name[j - 1] == ':')
        {
            metric_name[j - 1] = '\0';
        }
        strcp = strcmp(metric_name, "MemTotal");
        if (strcp == 0)
        {
            mem_total = metric_value;
        }
        strcp = strcmp(metric_name, "MemFree");
        if (strcp == 0)
        {
            mem_free = metric_value;
        }
    }
    while (s);

    if (state)
    {
        mem_util = (1 - (double)(mem_free) / (mem_total)) * 100;
    }
    else
    {
        mem_util = 0.0;
    }

    fclose(fp);
    json_object_set_new(get_cpu_util_obj, "memory_util%", json_real(mem_util));
    *get_util_obj_str = json_dumps(get_util_obj, JSON_INDENT(4));
    json_decref(get_util_obj);
    state = 1;

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

// The variorum_get_node_power_domain_info_json is a node-level API, and cannot
// be implemented at a per-component (eg CPU, GPU) level. This can only be available
// on what we define as the 'primary' platform, e.g. IBM Power9 CPU or Intel and AMD CPUs,
// and internally, the implementation can update domain information of CPU, GPUs and
// memory where applicable. Current implementation is basic, and in the future,
// we will update this API to do obtain and return the GPU domain info using a new
// JSON API for GPU power domain info.

int variorum_get_node_power_domain_info_json(char **get_domain_obj_str)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    // Obtain the index corresponding to the primary platform.
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
#ifdef VARIORUM_WITH_INTEL_CPU
        i = P_INTEL_CPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_IBM_CPU
        i = P_IBM_CPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_AMD_CPU
        i = P_AMD_CPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_ARM_CPU
        i = P_ARM_CPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_AMD_APU
        i = P_AMD_APU_IDX;
        break;
#endif
    }

    if (g_platform[i].variorum_get_node_power_domain_info_json == NULL)
    {
        variorum_error_handler("Feature not yet implemented or is not supported",
                               VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                               getenv("HOSTNAME"), __FILE__,
                               __FUNCTION__, __LINE__);
        // For the JSON functions, we return a -1 here, so users don't need
        // to explicitly check for NULL strings.
        return -1;
    }
    err = g_platform[i].variorum_get_node_power_domain_info_json(
              get_domain_obj_str);
    if (err)
    {
        return -1;
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_get_thermals_json(char **get_thermal_obj_str)
{
    int err = 0;
    int i;
    uint64_t ts;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    char hostname[1024];
    gethostname(hostname, 1024);

    struct timeval tv;

    json_t *get_thermal_obj = json_object();
    json_t *node_obj = json_object();
    json_object_set_new(get_thermal_obj, hostname, node_obj);

    gettimeofday(&tv, NULL);
    ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    json_object_set_new(node_obj, "timestamp", json_integer(ts));

    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_get_thermals_json == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_get_thermals_json(node_obj);
        if (err)
        {
            return -1;
        }
    }

    *get_thermal_obj_str = json_dumps(get_thermal_obj, JSON_INDENT(4));
    json_decref(get_thermal_obj);

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_get_frequency_json(char **get_frequency_obj_str)
{
    int err = 0;
    int i;
    char hostname[1024];
    uint64_t ts;
    struct timeval tv;
    gethostname(hostname, 1024);
    gettimeofday(&tv, NULL);

    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    json_t *get_frequency_obj = json_object();
    json_t *node_obj = json_object();
    json_object_set_new(get_frequency_obj, hostname, node_obj);

    ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    json_object_set_new(node_obj, "timestamp", json_integer(ts));

    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_get_frequency_json == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                   getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            continue;
        }
        err = g_platform[i].variorum_get_frequency_json(node_obj);
        if (err)
        {
            printf("Error with variorum get frequency json platform %d\n", i);
        }
    }

    *get_frequency_obj_str = json_dumps(get_frequency_obj, JSON_INDENT(4));
    json_decref(get_frequency_obj);

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

/*
int variorum_get_gpu_power_json(char **get_power_obj_str)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    // Obtain the index corresponding to the primary platform.
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
#ifdef VARIORUM_WITH_NVIDIA_GPU
        i = P_NVIDIA_GPU_IDX;
        break;
#endif
#ifdef VARIORUM_WITH_AMD_GPU
        i = P_AMD_GPU_IDX;
        break;
#endif
    }

    if (g_platform[i].variorum_get_gpu_power_json == NULL)
    {
        variorum_error_handler("Feature not yet implemented or is not supported",
                               VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                               getenv("HOSTNAME"), __FILE__,
                               __FUNCTION__, __LINE__);
        // For the JSON functions, we return a -1 here, so users don't need
        // to explicitly check for NULL strings.
        return -1;
    }

    err = g_platform[i].variorum_get_gpu_power_json(get_power_obj_str);
    if (err)
    {
        return -1;
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}
*/

int variorum_print_available_frequencies(void)
{
    int err = 0;
    int i;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].variorum_print_available_frequencies == NULL)
        {
            variorum_error_handler("Feature not yet implemented or is not supported",
                                   VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED, getenv("HOSTNAME"), __FILE__,
                                   __FUNCTION__, __LINE__);
            return 0;
        }
        err = g_platform[i].variorum_print_available_frequencies();
        if (err)
        {
            return -1;
        }
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_energy(void)
{
    int err = 0;
    int i;
    int has_cpu = 0;
    int has_gpu = 0;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    // If we have a GPU-only build, we should exit with a helpful message.
    // If we have a CPU-only or CPU+GPU multi-platform build, we should print
    // the node-level energy.
    // First check if we have a CPU platform, then check for a GPU platform

#if defined(VARIORUM_WITH_INTEL_CPU) || defined(VARIORUM_WITH_AMD_CPU) || defined(VARIORUM_WITH_IBM_CPU) || defined(VARIORUM_WITH_AMD_APU)
    has_cpu = 1;
#endif
#if defined(VARIORUM_WITH_NVIDIA_GPU) || defined(VARIORUM_WITH_AMD_GPU) || defined(VARIORUM_WITH_INTEL_GPU)
    has_gpu = 1;
#endif

    // CPU-only or multi-platform build
    if ((has_cpu && has_gpu) || (has_cpu))
    {
        for (i = 0; i < P_NUM_PLATFORMS; i++)
        {
            if (g_platform[i].variorum_print_energy == NULL)
            {
                variorum_error_handler("Feature not yet implemented or is not supported",
                                       VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED, getenv("HOSTNAME"), __FILE__,
                                       __FUNCTION__, __LINE__);
                return 0;
            }
            err = g_platform[i].variorum_print_energy(0);
            if (err)
            {
                return -1;
            }
        }
    }
    else
    {
        // We have a GPU-only build, currently doesn't support get_energy
        variorum_error_handler("Feature not yet implemented or is not supported",
                               VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED, getenv("HOSTNAME"), __FILE__,
                               __FUNCTION__, __LINE__);
        return 0;
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);

    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_print_verbose_energy(void)
{
    int err = 0;
    int i;
    int has_cpu = 0;
    int has_gpu = 0;
    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    // If we have a GPU-only build, we should exit with a helpful message.
    // If we have a CPU-only or CPU+GPU multi-platform build, we should print
    // the node-level energy.
    // First check if we have a CPU platform, then check for a GPU platform

#if defined(VARIORUM_WITH_INTEL_CPU) || defined(VARIORUM_WITH_AMD_CPU) || defined(VARIORUM_WITH_IBM_CPU) || defined(VARIORUM_WITH_AMD_APU)
    has_cpu = 1;
#endif
#if defined(VARIORUM_WITH_NVIDIA_GPU) || defined(VARIORUM_WITH_AMD_GPU) || defined(VARIORUM_WITH_INTEL_GPU)
    has_gpu = 1;
#endif

    // CPU-only or multi-platform build
    if ((has_cpu && has_gpu) || (has_cpu))
    {
        for (i = 0; i < P_NUM_PLATFORMS; i++)
        {
            if (g_platform[i].variorum_print_energy == NULL)
            {
                variorum_error_handler("Feature not yet implemented or is not supported",
                                       VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED, getenv("HOSTNAME"), __FILE__,
                                       __FUNCTION__, __LINE__);
                return 0;
            }
            err = g_platform[i].variorum_print_energy(1);
            if (err)
            {
                return -1;
            }
        }
    }
    else
    {
        // We have a GPU-only build, currently doesn't support get_energy
        variorum_error_handler("Feature not yet implemented or is not supported",
                               VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED, getenv("HOSTNAME"), __FILE__,
                               __FUNCTION__, __LINE__);
        return 0;
    }
    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

int variorum_get_energy_json(char **get_energy_obj_str)
{
    int err = 0;
    int i;
    int has_cpu = 0;
    int has_gpu = 0;
    char hostname[1024];
    uint64_t ts;
    struct timeval tv;
    gethostname(hostname, 1024);
    gettimeofday(&tv, NULL);

    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    json_t *get_energy_obj = json_object();
    json_t *node_obj = json_object();
    json_object_set_new(get_energy_obj, hostname, node_obj);

    ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    json_object_set_new(node_obj, "timestamp", json_integer(ts));

    // If we have a GPU-only build, we should exit with a helpful message.
    // If we have a CPU-only or CPU+GPU multi-platform build, we should print
    // the node-level energy.
    // First check if we have a CPU platform, then check for a GPU platform

#if defined(VARIORUM_WITH_INTEL_CPU) || defined(VARIORUM_WITH_AMD_CPU) || defined(VARIORUM_WITH_IBM_CPU) || defined(VARIORUM_WITH_AMD_APU)
    has_cpu = 1;
#endif
#if defined(VARIORUM_WITH_NVIDIA_GPU) || defined(VARIORUM_WITH_AMD_GPU) || defined(VARIORUM_WITH_INTEL_GPU)
    has_gpu = 1;
#endif

    // CPU-only or multi-platform build
    if ((has_cpu && has_gpu) || (has_cpu))
    {
        for (i = 0; i < P_NUM_PLATFORMS; i++)
        {
            if (g_platform[i].variorum_get_energy_json == NULL)
            {
                variorum_error_handler("Feature not yet implemented or is not supported",
                                       VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                                       getenv("HOSTNAME"), __FILE__,
                                       __FUNCTION__, __LINE__);
                return 0;
            }
            err = g_platform[i].variorum_get_energy_json(node_obj);
            if (err)
            {
                printf("Error with variorum get frequency json platform %d\n", i);
            }
            *get_energy_obj_str = json_dumps(get_energy_obj, JSON_INDENT(4));
        }
    }
    else
    {
        // We have a GPU-only build, currently doesn't support get_energy
        variorum_error_handler("Feature not yet implemented or is not supported",
                               VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED, getenv("HOSTNAME"), __FILE__,
                               __FUNCTION__, __LINE__);
        *get_energy_obj_str = json_dumps(get_energy_obj, JSON_INDENT(4));
        return 0;
    }

    json_decref(get_energy_obj);

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return err;
}

char *variorum_get_current_version()
{
    return QuoteMacro(VARIORUM_VERSION);
}

// Exploratory API for AMD ESMI metrics table interface
// This provides a simple way to get all metrics in one call for experimentation
int variorum_get_amd_esmi_metrics_json(char **metrics_json_str)
{
#ifdef VARIORUM_WITH_AMD_APU
    int err = 0;
    char hostname[1024];
    uint64_t ts;
    struct timeval tv;

    err = variorum_enter(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }

    gethostname(hostname, 1024);
    gettimeofday(&tv, NULL);
    ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;

    json_t *metrics_obj = json_object();
    json_t *node_obj = json_object();
    json_object_set_new(metrics_obj, hostname, node_obj);
    json_object_set_new(node_obj, "timestamp", json_integer(ts));

    // Get the number of sockets
    int *nsockets = NULL;
    variorum_get_topology(&nsockets, NULL, NULL);
    int total_sockets = (nsockets != NULL) ? *nsockets : 1;

    // Use the ESMI metrics table interface to get all metrics
    // This calls the exploratory API from amd_apu_esmi_features
    extern void get_all_metrics_json_esmi(int chipid, int total_sockets, json_t *output);
    get_all_metrics_json_esmi(0, total_sockets, node_obj);

    *metrics_json_str = json_dumps(metrics_obj, JSON_INDENT(4));
    json_decref(metrics_obj);

    err = variorum_exit(__FILE__, __FUNCTION__, __LINE__);
    if (err)
    {
        return -1;
    }
    return 0;
#else
    variorum_error_handler("Feature only available for AMD MI300A APU",
                           VARIORUM_ERROR_FEATURE_NOT_IMPLEMENTED,
                           getenv("HOSTNAME"), __FILE__,
                           __FUNCTION__, __LINE__);
    return -1;
#endif
}
