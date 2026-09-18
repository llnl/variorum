// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <amd_apu_power_features.h>
#include <config_architecture.h>
#include <variorum_error.h>
#include <variorum_timers.h>
#include <sys/time.h>

#ifdef LIBJUSTIFY_FOUND
#include <cprintf.h>
#endif

// AMD SMI's documented unit for amdsmi_get_temp_metric() has changed across
// ROCm releases: millidegrees Celsius on older releases, whole-degree
// Celsius on newer ones (confirmed empirically: this ASIC/driver/library
// combination returns whole-degree Celsius directly, which is why the
// previous "/ 1000" conversion here silently zeroed out every real reading
// -- e.g. 46 / 1000 == 0 under integer division). Rather than hardcode one
// assumption that breaks again on the next API revision, detect it from
// the magnitude of the raw value: no real GPU/HBM operating temperature
// exceeds a few hundred degrees C, so any raw value above 1000 must be
// millidegrees.
static double amd_apu_temp_to_celsius(int64_t raw)
{
    if (raw > 1000)
    {
        return (double)raw / 1000.0;
    }
    return (double)raw;
}

// Helper: enumerate all AMD SMI processor (device) handles across every
// socket, in socket order. Mirrors the flat device-index space that
// rsmi_num_monitor_devices()/rsmi_dev_* used, so the existing
// chipid/gpus_per_socket slicing logic below still applies.
// Caller must free() the returned array. Returns NULL on failure and
// writes the flat device count to *out_count.
static amdsmi_processor_handle *amd_apu_get_all_processor_handles(
    uint32_t *out_count)
{
    amdsmi_status_t ret;
    uint32_t socket_count = 0;

    *out_count = 0;

    ret = amdsmi_get_socket_handles(&socket_count, NULL);
    if (ret != AMDSMI_STATUS_SUCCESS || socket_count == 0)
    {
        return NULL;
    }

    amdsmi_socket_handle *sockets =
        (amdsmi_socket_handle *) malloc(socket_count * sizeof(
                                            amdsmi_socket_handle));
    if (sockets == NULL)
    {
        return NULL;
    }

    ret = amdsmi_get_socket_handles(&socket_count, sockets);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        free(sockets);
        return NULL;
    }

    // First pass: total device count across all sockets.
    uint32_t total_devices = 0;
    for (uint32_t s = 0; s < socket_count; s++)
    {
        uint32_t dev_count = 0;
        amdsmi_get_processor_handles(sockets[s], &dev_count, NULL);
        total_devices += dev_count;
    }

    if (total_devices == 0)
    {
        free(sockets);
        return NULL;
    }

    amdsmi_processor_handle *devices =
        (amdsmi_processor_handle *) malloc(total_devices * sizeof(
                amdsmi_processor_handle));
    if (devices == NULL)
    {
        free(sockets);
        return NULL;
    }

    // Second pass: fill flat array in socket order.
    uint32_t idx = 0;
    for (uint32_t s = 0; s < socket_count; s++)
    {
        uint32_t dev_count = 0;
        amdsmi_get_processor_handles(sockets[s], &dev_count, NULL);
        amdsmi_get_processor_handles(sockets[s], &dev_count, &devices[idx]);
        idx += dev_count;
    }

    free(sockets);
    *out_count = total_devices;
    return devices;
}

void get_energy_data(int chipid, int total_sockets, int verbose, FILE *output)
{
    amdsmi_status_t ret;
    uint32_t num_devices = 0;
    amdsmi_processor_handle *devices = NULL;
    int gpus_per_socket;
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;

    gethostname(hostname, 1024);

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not initialize AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        exit(-1);
    }

    devices = amd_apu_get_all_processor_handles(&num_devices);
    if (devices == NULL || num_devices == 0)
    {
        variorum_error_handler("Could not get number of GPU devices",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        amdsmi_shut_down();
        return;
    }

    gpus_per_socket = num_devices / total_sockets;

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s\n",
                     "_AMD_APU_ENERGY", "Host", "Socket", "DeviceID",
                     "Energy_uJ", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_ENERGY Host Socket DeviceID Energy_uJ Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    for (int i = chipid * gpus_per_socket;
         i < (chipid + 1) * gpus_per_socket; i++)
    {
        uint64_t energy_counter = 0;
        float counter_resolution = 0.0;
        uint64_t energy_timestamp = 0;

        // Get cumulative energy counter via AMD SMI
        // energy_counter: accumulated energy in microjoules
        // counter_resolution: resolution in microjoules
        // energy_timestamp: timestamp in nanoseconds
        ret = amdsmi_get_energy_count(devices[i], &energy_counter,
                                      &counter_resolution, &energy_timestamp);

        // Handle case where energy is not supported on some devices
        if ((ret != AMDSMI_STATUS_SUCCESS) && (ret != AMDSMI_STATUS_NOT_SUPPORTED))
        {
            variorum_error_handler("AMDSMI energy API was not successful",
                                   VARIORUM_ERROR_PLATFORM_ENV,
                                   getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                                   __LINE__);
        }

        // Only print if we got valid data
        if (ret == AMDSMI_STATUS_SUCCESS)
        {
            if (verbose == 1)
            {
#ifdef LIBJUSTIFY_FOUND
                cfprintf(output, "%s: %s, %s: %d, %s: %d, %s: %lu, %s: %lf sec\n",
                         "_AMD_APU_ENERGY", hostname, "Socket", chipid,
                         "DeviceID", i, "Energy_uJ", energy_counter, "Timestamp",
                         (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
                fprintf(output,
                        "_AMD_APU_ENERGY Host: %s, Socket: %d, DeviceID: %d,"
                        " Energy: %lu uJ, Timestamp: %lf sec\n",
                        hostname, chipid, i, energy_counter,
                        (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
            }
            else
            {
#ifdef LIBJUSTIFY_FOUND
                cfprintf(output, "_AMD_APU_ENERGY %s %d %d %lu %lf\n",
                         hostname, chipid, i, energy_counter,
                         (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
                fprintf(output, "_AMD_APU_ENERGY %s %d %d %lu %lf\n",
                        hostname, chipid, i, energy_counter,
                        (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
            }
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif

    free(devices);

    ret = amdsmi_shut_down();
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not shutdown AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
    }
}

void get_energy_json(int chipid, int total_sockets, json_t *output)
{
    amdsmi_status_t ret;
    uint32_t num_devices = 0;
    amdsmi_processor_handle *devices = NULL;
    int gpus_per_socket;
    double total_apu_energy = 0.0;
    char socketID[16];
    char deviceID[32];

    snprintf(socketID, 16, "socket_%d", chipid);

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not initialize AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        return;
    }

    devices = amd_apu_get_all_processor_handles(&num_devices);
    if (devices == NULL || num_devices == 0)
    {
        variorum_error_handler("Could not get number of GPU devices",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        amdsmi_shut_down();
        return;
    }

    gpus_per_socket = num_devices / total_sockets;

    // num_gpus_per_socket: XCDs/GCDs per APU socket, from AMD SMI's
    // processor-handle count -- this varies by compute partition mode
    // (e.g. SPX/TPX/CPX), not a fixed constant. 6 on Tuolumne's current
    // (SPX) partitioning of MI300A's 6 XCDs; a differently-partitioned
    // node will correctly report a different value here.
    // num_apus_per_node: physical APU sockets in the node (4 on Tuolumne).
    json_object_set_new(output, "num_gpus_per_socket",
                        json_integer(gpus_per_socket));
    json_object_set_new(output, "num_apus_per_node",
                        json_integer(total_sockets));

    json_t *socket_obj = json_object_get(output, socketID);
    if (socket_obj == NULL)
    {
        socket_obj = json_object();
        json_object_set_new(output, socketID, socket_obj);
    }

    // Primary field, matching the established GPU energy JSON convention
    // from Nvidia_GPU (energy_gpu_joules / GPU_%d): a single real-valued
    // Joules reading per device, so external tools consuming energy JSON
    // across platforms see the same shape regardless of vendor.
    json_t *gpu_obj = json_object();
    json_object_set_new(socket_obj, "energy_apu_joules", gpu_obj);

    // Empirically confirmed on a CPX-partitioned node (same test as the
    // power path): only one device handle per socket returns
    // AMDSMI_STATUS_SUCCESS for amdsmi_get_energy_count(); the rest return
    // AMDSMI_STATUS_NOT_SUPPORTED. Add the first successful reading per
    // socket, not a specific index.
    int socket_energy_counted = 0;

    for (int i = chipid * gpus_per_socket;
         i < (chipid + 1) * gpus_per_socket; i++)
    {
        uint64_t energy_counter = 0;
        float counter_resolution = 0.0;
        uint64_t energy_timestamp = 0;

        ret = amdsmi_get_energy_count(devices[i], &energy_counter,
                                      &counter_resolution, &energy_timestamp);

        // Only add to JSON if we got valid data
        if (ret == AMDSMI_STATUS_SUCCESS)
        {
            // energy_counter is a raw hardware tick count, not Joules;
            // counter_resolution is microjoules per tick (AMD SMI reports
            // it rather than us hardcoding it, since it can vary by
            // ASIC/firmware). Convert ticks -> uJ -> J here so the JSON
            // API only ever exposes the final Joules value.
            double energy_joules = (double)energy_counter *
                                   (double)counter_resolution / 1e6;

            snprintf(deviceID, 32, "APU_%d", i);
            json_object_set_new(gpu_obj, deviceID, json_real(energy_joules));

            if (!socket_energy_counted)
            {
                total_apu_energy += energy_joules;
                socket_energy_counted = 1;
            }
        }
    }

    // Mirrors this file's own power_node_watts rollup in get_json_power_data:
    // always ensure energy_node_joules exists, adding to it if a CPU-side
    // object already set one.
    if (json_object_get(output, "energy_node_joules") != NULL)
    {
        double energy_node;
        energy_node = json_real_value(json_object_get(output,
                                      "energy_node_joules"));
        energy_node += total_apu_energy;
        json_object_set_new(output, "energy_node_joules",
                            json_real(energy_node));
    }
    else
    {
        json_object_set_new(output, "energy_node_joules",
                            json_real(total_apu_energy));
    }

    free(devices);

    ret = amdsmi_shut_down();
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not shutdown AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
    }
}

// Power monitoring
void get_power_data(int chipid, int total_sockets, int verbose, FILE *output)
{
    amdsmi_status_t ret;
    uint32_t num_devices = 0;
    amdsmi_processor_handle *devices = NULL;
    int gpus_per_socket;
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;

    gethostname(hostname, 1024);

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not initialize AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        exit(-1);
    }

    devices = amd_apu_get_all_processor_handles(&num_devices);
    if (devices == NULL || num_devices == 0)
    {
        variorum_error_handler("Could not get number of APU devices",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        amdsmi_shut_down();
        return;
    }

    gpus_per_socket = num_devices / total_sockets;

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s\n",
                     "_AMD_APU_POWER_USAGE", "Host", "Socket", "DeviceID",
                     "Power", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_POWER_USAGE Host Socket DeviceID Power Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    for (int i = chipid * gpus_per_socket;
         i < (chipid + 1) * gpus_per_socket; i++)
    {
        amdsmi_power_info_t power_info;
        double pwr_val_flt = -1.0;

        // amdsmi_get_power_info() reports current_socket_power directly in
        // Watts (unlike rsmi_dev_power_get(), which returned microwatts),
        // so no unit conversion is needed here.
        ret = amdsmi_get_power_info(devices[i], &power_info);

        // On newer APUs, device power may only be reported at certain device IDs
        // Handle gracefully if not supported on some devices
        if ((ret != AMDSMI_STATUS_SUCCESS) && (ret != AMDSMI_STATUS_NOT_SUPPORTED))
        {
            variorum_error_handler("AMDSMI power API was not successful",
                                   VARIORUM_ERROR_PLATFORM_ENV,
                                   getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                                   __LINE__);
        }

        if (ret == AMDSMI_STATUS_SUCCESS)
        {
            pwr_val_flt = (double)power_info.current_socket_power;

            if (verbose == 1)
            {
#ifdef LIBJUSTIFY_FOUND
                cfprintf(output, "%s: %s, %s: %d, %s: %d, %s: %0.2lf, %s: %lf sec\n",
                         "_AMD_APU_POWER_USAGE", hostname, "Socket", chipid,
                         "DeviceID", i, "Power", pwr_val_flt, "Timestamp",
                         (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
                fprintf(output,
                        "_AMD_APU_POWER_USAGE Host: %s, Socket: %d, DeviceID: %d,"
                        " Power: %0.2lf W, Timestamp: %lf sec\n",
                        hostname, chipid, i, pwr_val_flt,
                        (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
            }
            else
            {
#ifdef LIBJUSTIFY_FOUND
                cfprintf(output, "_AMD_APU_POWER_USAGE %s %d %d %0.2lf %lf\n",
                         hostname, chipid, i, pwr_val_flt,
                         (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
                fprintf(output, "_AMD_APU_POWER_USAGE %s %d %d %0.2lf %lf\n",
                        hostname, chipid, i, pwr_val_flt,
                        (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
            }
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif

    free(devices);

    ret = amdsmi_shut_down();
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not shutdown AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
    }
}

void get_json_power_data(json_t *get_power_obj, int total_sockets)
{
    int chipid;
    uint32_t num_devices = 0;
    amdsmi_processor_handle *devices = NULL;
    int gpus_per_socket;
    double pwr_val_flt = 0.0;
    double total_apu_power = 0.0;
    int d;

    amdsmi_status_t ret;

    static size_t devIDlen = 24;
    char devID[devIDlen];
    char socketID[24];

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not initialize AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        exit(-1);
    }

    devices = amd_apu_get_all_processor_handles(&num_devices);
    if (devices == NULL || num_devices == 0)
    {
        variorum_error_handler("Could not get number of APU devices",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        amdsmi_shut_down();
        exit(-1);
    }

    gpus_per_socket = num_devices / total_sockets;

    // num_gpus_per_socket: XCDs/GCDs per APU socket, from AMD SMI's
    // processor-handle count -- this varies by compute partition mode
    // (e.g. SPX/TPX/CPX), not a fixed constant. 6 on Tuolumne's current
    // (SPX) partitioning of MI300A's 6 XCDs; a differently-partitioned
    // node will correctly report a different value here.
    // num_apus_per_node: physical APU sockets in the node (4 on Tuolumne).
    json_object_set_new(get_power_obj, "num_gpus_per_socket",
                        json_integer(gpus_per_socket));
    json_object_set_new(get_power_obj, "num_apus_per_node",
                        json_integer(total_sockets));

    for (chipid = 0; chipid < total_sockets; chipid++)
    {
        snprintf(socketID, devIDlen, "socket_%d", chipid);

        json_t *socket_obj = json_object_get(get_power_obj, socketID);
        if (socket_obj == NULL)
        {
            socket_obj = json_object();
            json_object_set_new(get_power_obj, socketID, socket_obj);
        }

        json_t *apu_obj = json_object();
        json_object_set_new(socket_obj, "power_apu_watts", apu_obj);

        // Empirically confirmed on a CPX-partitioned node: only one device
        // handle per socket returns AMDSMI_STATUS_SUCCESS for power; the
        // other XCD handles return AMDSMI_STATUS_NOT_SUPPORTED (status 2)
        // rather than a duplicated value. So the correct rollup is "add the
        // first successful reading in this socket", not "add the reading
        // from a specific index" -- which device succeeds isn't guaranteed
        // to be positionally first, so gate on success, not index.
        int socket_power_counted = 0;

        // Iterate over all APU device handles for this socket
        for (d = chipid * gpus_per_socket;
             d < (chipid + 1) * gpus_per_socket; ++d)
        {
            amdsmi_power_info_t power_info;

            ret = amdsmi_get_power_info(devices[d], &power_info);

            if (ret == AMDSMI_STATUS_SUCCESS)
            {
                // Already in Watts; no conversion needed (unlike RSMI's microwatts)
                pwr_val_flt = (double)power_info.current_socket_power;
                snprintf(devID, devIDlen, "APU_%d", d);
                json_object_set_new(apu_obj, devID, json_real(pwr_val_flt));

                if (!socket_power_counted)
                {
                    total_apu_power += pwr_val_flt;
                    socket_power_counted = 1;
                }
            }
        }
    }

    // If we have an existing CPU object with power_node_watts, update its value
    if (json_object_get(get_power_obj, "power_node_watts") != NULL)
    {
        double power_node;
        power_node = json_real_value(json_object_get(get_power_obj,
                                     "power_node_watts"));
        power_node += total_apu_power;
        json_object_set_new(get_power_obj, "power_node_watts",
                            json_real(power_node));
    }
    else
    {
        json_object_set_new(get_power_obj, "power_node_watts",
                            json_real(total_apu_power));
    }

    free(devices);

    ret = amdsmi_shut_down();
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not shutdown AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
    }
}

void get_thermals_data(int chipid, int total_sockets, int verbose, FILE *output)
{
    amdsmi_status_t ret;
    uint32_t num_devices = 0;
    amdsmi_processor_handle *devices = NULL;
    int gpus_per_socket;
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;

    // Sensors to sample per device: Edge, Junction (aka Hotspot), and VRAM
    // (AMD SMI's closest equivalent to RSMI_TEMP_TYPE_MEMORY -- see note
    // below on the AMDSMI_TEMPERATURE_TYPE_VRAM mapping).
    static const amdsmi_temperature_type_t sensor_types[] =
    {
        AMDSMI_TEMPERATURE_TYPE_EDGE,
        AMDSMI_TEMPERATURE_TYPE_JUNCTION,
        AMDSMI_TEMPERATURE_TYPE_VRAM
    };
    static const char *sensor_labels[] = { "Edge", "Junction", "HBM" };
    static const int num_sensors = 3;

    gethostname(hostname, 1024);

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not initialize AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        exit(-1);
    }

    devices = amd_apu_get_all_processor_handles(&num_devices);
    if (devices == NULL || num_devices == 0)
    {
        variorum_error_handler("Could not get number of APU devices",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        amdsmi_shut_down();
        return;
    }

    gpus_per_socket = num_devices / total_sockets;

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s %s\n",
                     "_AMD_APU_TEMPERATURE", "Host", "Socket", "DeviceID",
                     "Sensor", "Temperature", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_TEMPERATURE Host Socket DeviceID Sensor Temperature Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    int i;
    for (i = chipid * gpus_per_socket; i < (chipid + 1) * gpus_per_socket; i++)
    {
        for (int s = 0; s < num_sensors; s++)
        {
            int64_t temp_val = -1;
            double temp_val_flt = -1.0;

            // Get temperature for this sensor (edge/junction/VRAM die temperature)
            ret = amdsmi_get_temp_metric(devices[i], sensor_types[s],
                                         AMDSMI_TEMP_CURRENT, &temp_val);

            // Not every sensor is present on every device/partition -- treat
            // AMDSMI_STATUS_NOT_SUPPORTED as expected and skip it, same as
            // the energy/power paths already do elsewhere in this file.
            if ((ret != AMDSMI_STATUS_SUCCESS) && (ret != AMDSMI_STATUS_NOT_SUPPORTED))
            {
                variorum_error_handler("AMDSMI temp API was not successful",
                                       VARIORUM_ERROR_PLATFORM_ENV,
                                       getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                                       __LINE__);
            }

            if (ret != AMDSMI_STATUS_SUCCESS)
            {
                continue;
            }

            temp_val_flt = amd_apu_temp_to_celsius(temp_val);

            if (verbose == 1)
            {
#ifdef LIBJUSTIFY_FOUND
                cfprintf(output,
                         "%s: %s, %s: %d, %s: %d, %s: %s, %s: %0.2lf, %s: %lf sec\n",
                         "_AMD_APU_TEMPERATURE", hostname,
                         "Socket", chipid,
                         "DeviceID", i,
                         "Sensor", sensor_labels[s],
                         "Temperature", temp_val_flt,
                         "Timestamp", (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) /
                         1000000.0);
#else
                fprintf(output,
                        "_AMD_APU_TEMPERATURE Host: %s, Socket: %d, DeviceID: %d,"
                        " Sensor: %s, Temperature: %0.2lf C, Timestamp: %lf sec\n",
                        hostname, chipid, i, sensor_labels[s], temp_val_flt,
                        (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
            }
            else
            {
#ifdef LIBJUSTIFY_FOUND
                cfprintf(output, "_AMD_APU_TEMPERATURE %s %d %d %s %0.2lf %lf\n",
                         hostname, chipid, i, sensor_labels[s], temp_val_flt,
                         (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
                fprintf(output, "_AMD_APU_TEMPERATURE %s %d %d %s %0.2lf %lf\n",
                        hostname, chipid, i, sensor_labels[s], temp_val_flt,
                        (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
            }
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif

    free(devices);

    ret = amdsmi_shut_down();
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not shutdown AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
    }
}

void get_thermals_json(int chipid, int total_sockets, json_t *output)
{
    amdsmi_status_t ret;
    uint32_t num_devices = 0;
    amdsmi_processor_handle *devices = NULL;
    int gpus_per_socket;
    char hostname[1024];

    // Sensors to sample per device: Edge, Junction (aka Hotspot), and VRAM
    // (AMD SMI's closest equivalent to RSMI_TEMP_TYPE_MEMORY).
    static const amdsmi_temperature_type_t sensor_types[] =
    {
        AMDSMI_TEMPERATURE_TYPE_EDGE,
        AMDSMI_TEMPERATURE_TYPE_JUNCTION,
        AMDSMI_TEMPERATURE_TYPE_VRAM
    };
    static const char *sensor_labels[] = { "edge", "junction", "hbm" };
    static const int num_sensors = 3;

    gethostname(hostname, 1024);

    ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not initialize AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        exit(-1);
    }

    devices = amd_apu_get_all_processor_handles(&num_devices);
    if (devices == NULL || num_devices == 0)
    {
        variorum_error_handler("Could not get number of APU devices",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        amdsmi_shut_down();
        return;
    }

    gpus_per_socket = num_devices / total_sockets;

    // num_gpus_per_socket: XCDs/GCDs per APU socket, from AMD SMI's
    // processor-handle count -- this varies by compute partition mode
    // (e.g. SPX/TPX/CPX), not a fixed constant. 6 on Tuolumne's current
    // (SPX) partitioning of MI300A's 6 XCDs; a differently-partitioned
    // node will correctly report a different value here.
    // num_apus_per_node: physical APU sockets in the node (4 on Tuolumne).
    json_object_set_new(output, "num_gpus_per_socket",
                        json_integer(gpus_per_socket));
    json_object_set_new(output, "num_apus_per_node",
                        json_integer(total_sockets));

    char socketid[12];
    snprintf(socketid, 12, "socket_%d", chipid);

    // Check if socket object is in node object
    json_t *socket_obj = json_object_get(output, socketid);
    if (socket_obj == NULL)
    {
        socket_obj = json_object();
        json_object_set_new(output, socketid, socket_obj);
    }

    // APU object for temperatures
    json_t *apu_obj = json_object();
    json_object_set_new(socket_obj, "APU", apu_obj);

    int i;
    for (i = chipid * gpus_per_socket; i < (chipid + 1) * gpus_per_socket; i++)
    {
        for (int s = 0; s < num_sensors; s++)
        {
            int64_t temp_val = -1;
            double temp_val_flt = -1.0;

            ret = amdsmi_get_temp_metric(devices[i], sensor_types[s],
                                         AMDSMI_TEMP_CURRENT, &temp_val);

            // Not every sensor is present on every device/partition -- treat
            // AMDSMI_STATUS_NOT_SUPPORTED as expected and skip it.
            if ((ret != AMDSMI_STATUS_SUCCESS) && (ret != AMDSMI_STATUS_NOT_SUPPORTED))
            {
                variorum_error_handler("AMDSMI temp API was not successful",
                                       VARIORUM_ERROR_PLATFORM_ENV,
                                       getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                                       __LINE__);
            }

            if (ret != AMDSMI_STATUS_SUCCESS)
            {
                continue;
            }

            temp_val_flt = amd_apu_temp_to_celsius(temp_val);

            // APU temperature entry, one per sensor
            char apuid[48];
            snprintf(apuid, 48, "temp_celsius_apu_%d_%s", i, sensor_labels[s]);
            json_object_set_new(apu_obj, apuid, json_real(temp_val_flt));
        }
    }

    free(devices);

    ret = amdsmi_shut_down();
    if (ret != AMDSMI_STATUS_SUCCESS)
    {
        variorum_error_handler("Could not shutdown AMDSMI",
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
    }
}

void get_clocks_data(int chipid, int total_sockets, int verbose, FILE *output)
{
    // Placeholder - implementation would mirror AMD_GPU clock monitoring
}

void get_clocks_json(int chipid, int total_sockets, json_t *output)
{
    // Placeholder - implementation would mirror AMD_GPU clock monitoring
}

void get_power_limit_data(int chipid, int total_sockets, int verbose,
                          FILE *output)
{
    // Placeholder - implementation would mirror AMD_GPU power cap monitoring
}

void cap_each_gpu_power_limit(int chipid, int total_sockets,
                              unsigned int powerlimit)
{
    // Placeholder - implementation would mirror AMD_GPU power cap setting
}

void get_gpu_utilization_data(int chipid, int total_sockets, int verbose,
                              FILE *output)
{
    // Placeholder - implementation would mirror AMD_GPU utilization monitoring
}

void get_gpu_utilization_data_json(int chipid, int total_sockets,
                                   json_t *output)
{
    // Placeholder - implementation would mirror AMD_GPU utilization monitoring
}
