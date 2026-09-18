// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config_architecture.h>
#include <variorum_config.h>
#include <variorum_error.h>
#include <variorum_topology.h>

#ifdef VARIORUM_WITH_INTEL_CPU
#include <config_intel.h>
#include <msr_core.h>
#endif

#ifdef VARIORUM_WITH_INTEL_GPU
#include <config_intel_gpu.h>
#endif

#ifdef VARIORUM_WITH_IBM_CPU
#include <config_ibm.h>
#endif

#ifdef VARIORUM_WITH_NVIDIA_GPU
#include <config_nvidia.h>
#endif

#ifdef VARIORUM_WITH_INTEL_GPU
#include <config_intel_gpu.h>
#endif

#ifdef VARIORUM_WITH_ARM_CPU
#include <config_arm.h>
#endif

#ifdef VARIORUM_WITH_AMD_CPU
#include <config_amd.h>
#endif

#ifdef VARIORUM_WITH_AMD_GPU
#include <config_amd_gpu.h>
#endif

#ifdef VARIORUM_WITH_AMD_APU
#include <config_amd_apu.h>
#endif

// Current support is for CPU + GPU + APU multi-platform builds,
// but can be extended to include other accelerators in the future.
#define MAX_PLATFORMS 2

#ifdef VARIORUM_WITH_AMD_CPU
int P_MSR_CORE_IDX;
#endif

#ifdef VARIORUM_WITH_INTEL_CPU
int P_MSR_CORE_IDX;
#endif

struct platform g_platform[MAX_PLATFORMS];

int variorum_enter(const char *filename, const char *func_name, int line_num)
{
    int err = 0;
    int i;

    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("_LOG_VARIORUM_ENTER:%s:%s::%d\n", filename, func_name, line_num);
        printf("Number of registered platforms: %d\n", P_NUM_PLATFORMS);
    }

    variorum_init_func_ptrs();

    //Triggers initialization on first call.  Errors assert.
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        variorum_get_topology(NULL, NULL, NULL, i);
    }
    // Sets arch_id on all platforms
    err = variorum_detect_arch();
    if (err)
    {
        variorum_error_handler("Cannot detect architecture", err,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        return err;
    }
    // Sets function pointers on all platforms
    err = variorum_set_func_ptrs();
    if (err)
    {
        variorum_error_handler("Cannot set function pointers", err,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        return err;
    }
    return err;
}

int variorum_exit(const char *filename, const char *func_name, int line_num)
{
    int err = 0;
    int i;

    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
        printf("_LOG_VARIORUM_EXIT:%s:%s::%d\n", filename, func_name, line_num);
    }

#ifdef VARIORUM_WITH_INTEL_CPU
    err = finalize_msr();
    if (err)
    {
        return err;
    }
#endif
#ifdef VARIORUM_WITH_AMD_CPU
    esmi_exit();
#endif
#ifdef VARIORUM_WITH_NVIDIA_GPU
    shutdownNVML();
#endif
#ifdef VARIORUM_WITH_INTEL_GPU
    shutdownAPMIDG();
#endif

    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        free(g_platform[i].arch_id);
    }

    return err;
}

int variorum_detect_arch(void)
{
    int i = 0;

#ifdef VARIORUM_WITH_INTEL_CPU
    g_platform[P_INTEL_CPU_IDX].arch_id = detect_intel_arch();
    P_MSR_CORE_IDX = P_INTEL_CPU_IDX;
#endif
#ifdef VARIORUM_WITH_INTEL_GPU
    g_platform[P_INTEL_GPU_IDX].arch_id = detect_intel_gpu_arch();
#endif
#ifdef VARIORUM_WITH_IBM_CPU
    g_platform[P_IBM_CPU_IDX].arch_id = detect_ibm_arch();
#endif
#ifdef VARIORUM_WITH_NVIDIA_GPU
    g_platform[P_NVIDIA_GPU_IDX].arch_id = detect_gpu_arch();
#endif
#ifdef VARIORUM_WITH_ARM_CPU
    g_platform[P_ARM_CPU_IDX].arch_id = detect_arm_arch();
#endif
#ifdef VARIORUM_WITH_AMD_CPU
    g_platform[P_AMD_CPU_IDX].arch_id = detect_amd_arch();
    P_MSR_CORE_IDX = P_AMD_CPU_IDX;
#endif
#ifdef VARIORUM_WITH_AMD_GPU
    g_platform[P_AMD_GPU_IDX].arch_id = detect_amd_gpu_arch();
#endif
#ifdef VARIORUM_WITH_AMD_APU
    g_platform[P_AMD_APU_IDX].arch_id = detect_amd_apu_arch();
#endif

    char *val = getenv("VARIORUM_LOG");
    if (val != NULL && atoi(val) == 1)
    {
#ifdef VARIORUM_WITH_INTEL_CPU
        printf("Intel Model: 0x%lx\n", *g_platform[P_INTEL_CPU_IDX].arch_id);
#endif
#ifdef VARIORUM_WITH_IBM_CPU
        printf("IBM Model: 0x%lx\n", *g_platform[P_IBM_CPU_IDX].arch_id);
#endif
#ifdef VARIORUM_WITH_AMD_CPU
        printf("AMD Family: 0x%lx, Model: 0x%lx\n",
               (*g_platform[P_AMD_CPU_IDX].arch_id >> 8) & 0xFF,
               *g_platform[P_AMD_CPU_IDX].arch_id & 0xFF);
#endif
#ifdef VARIORUM_WITH_AMD_GPU
        printf("AMD GPU Model: MI-%ld\n", *g_platform[P_AMD_GPU_IDX].arch_id);
#endif
#ifdef VARIORUM_WITH_AMD_APU
        printf("AMD APU Model: %ld\n", *g_platform[P_AMD_APU_IDX].arch_id);
#endif
    }

    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        if (g_platform[i].arch_id == NULL)
        {
            variorum_error_handler("No architectures detected", VARIORUM_ERROR_RUNTIME,
                                   getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                                   __LINE__);
            return VARIORUM_ERROR_UNSUPPORTED_ARCH;
        }
    }

    return 0;
}

void variorum_get_topology(unsigned *nsockets, unsigned *ncores,
                           unsigned *nthreads, int idx)
{
    int rc;

    static int init_variorum_get_topology = 0;

    gethostname(g_platform[idx].hostname, 1024);

    if (!init_variorum_get_topology)
    {
        // Commenting per Stephanie's code
        // Patki guesses this should be left uncommented

        // init_variorum_get_topology = 1;

        rc = variorum_init_topology();

        if (rc != 0)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc topology initialization error. "
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);

        }

        g_platform[idx].num_sockets = variorum_get_num_sockets();
        //-1 if Several levels exist with OBJ_SOCKET
        if (g_platform[idx].num_sockets == -1)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports that HWLOC_OBJ_SOCKETs exist "
                    "at multiple levels of the topology DAG.  "
                    "Variorum doesn't handle this case.  "
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);
        }
        // 0 if No levels exist with OBJ_SOCKET
        if (g_platform[idx].num_sockets == 0)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports no HWLOC_OBJ_SOCKETs exist.  "
                    "Variorum doesn't handle this case.  "
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);
        }

        g_platform[idx].total_cores = variorum_get_num_cores();
        if (g_platform[idx].total_cores == -1)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports HWLOC_OJB_COREs exist "
                    "at multiple levels of the topology DAG.  "
                    "Variorum doesn't handle this case.  "
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);
        }
        if (g_platform[idx].total_cores == 0)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports no HWLOC_OBJ_COREs exist."
                    "Variorum doesn't handle this case."
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);
        }

        g_platform[idx].total_threads = variorum_get_num_threads();
        if (g_platform[idx].total_threads == -1)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports that HWLOC_OBJ_PUs exist "
                    "at multiple levels of the topology DAG.  "
                    "Variorum doesn't handle this case."
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);
        }
        if (g_platform[idx].total_threads == 0)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports no HWLOC_OBJ_COREs exist.  "
                    "Variorum doesn't handle this case.  "
                    "Exiting.", __FILE__, __LINE__);
            exit(-1);
        }

        g_platform[idx].num_cores_per_socket = g_platform[idx].total_cores /
                                               g_platform[idx].num_sockets;
        if (g_platform[idx].total_cores % g_platform[idx].num_sockets != 0)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports the number of cores (%d) mod "
                    "the number of sockets (%d) is not zero.  "
                    "Something is amiss.  Exiting.",
                    __FILE__, __LINE__,
                    g_platform[idx].total_cores,
                    g_platform[idx].num_sockets);
            exit(-1);
        }

        g_platform[idx].num_threads_per_core = g_platform[idx].total_threads /
                                               g_platform[idx].total_cores;
        if (g_platform[idx].total_threads % g_platform[idx].total_cores != 0)
        {
            fprintf(stderr, "%s:%d "
                    "hwloc reports the number of threads (%d) mod "
                    "the number of cores (%d) is not zero.  "
                    "Something is amiss.  Exiting.",
                    __FILE__, __LINE__,
                    g_platform[idx].total_threads,
                    g_platform[idx].total_cores);
            exit(-1);
        }

    }

    if (nsockets != NULL)
    {
        *nsockets = g_platform[idx].num_sockets;
    }

    if (ncores != NULL)
    {
        *ncores = g_platform[idx].total_cores;
    }

    if (nthreads != NULL)
    {
        *nthreads = g_platform[idx].total_threads;
    }
}

void variorum_init_func_ptrs()
{
    int i = 0;
    for (i = 0; i < P_NUM_PLATFORMS; i++)
    {
        g_platform[i].variorum_print_power_limit = NULL;
        g_platform[i].variorum_cap_socket_frequency_limit = NULL;
        g_platform[i].variorum_cap_best_effort_node_power_limit = NULL;
        g_platform[i].variorum_cap_gpu_power_ratio = NULL;
        g_platform[i].variorum_cap_each_socket_power_limit = NULL;
        g_platform[i].variorum_cap_each_core_frequency_limit = NULL;
        g_platform[i].variorum_print_available_frequencies = NULL;
        g_platform[i].variorum_cap_each_gpu_power_limit = NULL;
        g_platform[i].variorum_print_features = NULL;
        g_platform[i].variorum_print_thermals = NULL;
        g_platform[i].variorum_print_counters = NULL;
        g_platform[i].variorum_print_frequency = NULL;
        g_platform[i].variorum_print_power = NULL;
        g_platform[i].variorum_enable_turbo = NULL;
        g_platform[i].variorum_disable_turbo = NULL;
        g_platform[i].variorum_print_turbo = NULL;
        g_platform[i].variorum_poll_power = NULL;
        g_platform[i].variorum_print_gpu_utilization = NULL;
        g_platform[i].variorum_get_utilization_json = NULL;
        g_platform[i].variorum_monitoring = NULL;
        g_platform[i].variorum_get_power_json = NULL;
        g_platform[i].variorum_get_node_power_domain_info_json = NULL;
        g_platform[i].variorum_print_energy = NULL;
        g_platform[i].variorum_get_thermals_json = NULL;
        g_platform[i].variorum_get_frequency_json = NULL;
        g_platform[i].variorum_get_energy_json = NULL;
    }
}

int variorum_set_func_ptrs()
{
    int err = 0;

#ifdef VARIORUM_WITH_INTEL_CPU
    err = set_intel_func_ptrs(P_INTEL_CPU_IDX);
    if (err)
    {
        return err;
    }
    err = init_msr();
#endif
#ifdef VARIORUM_WITH_INTEL_GPU
    err = set_intel_gpu_func_ptrs(P_INTEL_GPU_IDX);
#endif
#ifdef VARIORUM_WITH_IBM_CPU
    err = set_ibm_func_ptrs(P_IBM_CPU_IDX);
#endif
#ifdef VARIORUM_WITH_NVIDIA_GPU
    err = set_nvidia_func_ptrs(P_NVIDIA_GPU_IDX);
#endif
#ifdef VARIORUM_WITH_ARM_CPU
    err = set_arm_func_ptrs(P_ARM_CPU_IDX);
#endif
#ifdef VARIORUM_WITH_AMD_CPU
    err = set_amd_func_ptrs(P_AMD_CPU_IDX);
#endif
#ifdef VARIORUM_WITH_AMD_GPU
    err = set_amd_gpu_func_ptrs(P_AMD_GPU_IDX);
#endif
#ifdef VARIORUM_WITH_AMD_APU
    err = set_amd_apu_func_ptrs(P_AMD_APU_IDX);
#endif
    return err;
}

////setfixedcounters = fixed_ctr0,
////featureC = ?? /* does not have max non turbo ratio */
//}
//};

#if 0 /* To implement later */
///* Sandy Bridge: 06_2D, 06_2A */
//{
//    dump_power_limits = therm_status
//                        //setfixedcounters = fixed_ctr0,
//                        //setpowerlimit = perf_status
//                        //featureC = ?? /* does not have max non turbo ratio */
//}
///* Haswell, Haswell-E: 06_3C, 06_45, 06_46, 06_3F */
//{
//    getfixedcounters = max_non_turbo_ratio;
//},
///* Broadwell: 06_3D, 06_47, 06_4F, 06_56 */
//{
//    getfixedcounters = max_non_turbo_ratio;
//}
///* Skylake: 06_4E, 06_5E */
//{
//}
///* Kaby Lake: 06_8E, 06_9E */
//{
//}
///* Xeon Phi: 06_57 */
//{
//}
#endif

#if 0 /* Do not want to make 95% of users know how to use this */
//struct *platform make_arch(enum arch_e arch)
//{
//    struct platform *platform_ptr;
//    if (arch == SANDY_BRIDGE)
//    {
//        platform_ptr = &allplatforms[0];
//    }
//    else if (arch == IVY_BRIDGE)
//    {
//        platform_ptr = &allplatforms[1];
//    }
//    else if (arch == HASWELL)
//    {
//        platform_ptr = &allplatforms[2];
//    }
//    else if (arch == BROADWELL)
//    {
//        platform_ptr = &allplatforms[3];
//    }
//
//    return platform_ptr;
//};
//
///// Front-facing APIs
//struct platform *platform_ptr = make_arch(HASWELL);
#endif
