// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <amd_apu_esmi_features.h>
#include <config_architecture.h>
#include <variorum_error.h>
#include <variorum_timers.h>
#include <sys/time.h>
#include <e_smi/e_smi.h>

#ifdef LIBJUSTIFY_FOUND
#include <cprintf.h>
#endif

// Fixed-point scale factors used by the HSMP metric table
// Q10/UQ10 = 1/2^10 for temperature, power and frequency
// UQ16 = 1/2^16 for the energy accumulators
static const double Q1 = 1.0;
static const double Q10 = 1.0 / 1024.0;
static const double UQ16 = 1.0 / 65536.0;

// Helper function to initialize ESMI once per call
static int init_esmi_if_needed(void)
{
    static int esmi_initialized = 0;

    if (!esmi_initialized)
    {
        esmi_status_t ret = esmi_init();
        if (ret != ESMI_SUCCESS)
        {
            variorum_error_handler("Could not initialize ESMI",
                                   VARIORUM_ERROR_PLATFORM_ENV,
                                   getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                                   __LINE__);
            return -1;
        }
        esmi_initialized = 1;
    }
    return 0;
}

// Helper function to get the metrics table for a socket
static int get_socket_metrics_table(uint32_t socket_id, struct hsmp_metric_table *mtbl)
{
    esmi_status_t ret = esmi_metrics_table_get(socket_id, mtbl);
    if (ret != ESMI_SUCCESS)
    {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg),
                 "esmi_metrics_table_get failed for socket %u: %s",
                 socket_id, esmi_get_err_msg(ret));
        variorum_error_handler(err_msg,
                               VARIORUM_ERROR_PLATFORM_ENV,
                               getenv("HOSTNAME"), __FILE__, __FUNCTION__,
                               __LINE__);
        return -1;
    }
    return 0;
}

void get_energy_data_esmi(int chipid, int total_sockets, int verbose, FILE *output)
{
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;
    struct hsmp_metric_table mtbl;

    gethostname(hostname, 1024);

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s %s %s %s\n",
                     "_AMD_APU_ENERGY_ESMI", "Host", "Socket",
                     "SocketEnergy_J", "CCDEnergy_J", "XCDEnergy_J",
                     "AIDEnergy_J", "HBMEnergy_J", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_ENERGY_ESMI Host Socket SocketEnergy_J CCDEnergy_J XCDEnergy_J AIDEnergy_J HBMEnergy_J Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        double socket_energy = mtbl.socket_energy_acc * UQ16;
        double ccd_energy = mtbl.ccd_energy_acc * UQ16;
        double xcd_energy = mtbl.xcd_energy_acc * UQ16;
        double aid_energy = mtbl.aid_energy_acc * UQ16;
        double hbm_energy = mtbl.hbm_energy_acc * UQ16;

        if (verbose == 1)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s: %s, %s: %d, %s: %.3f, %s: %.3f, %s: %.3f, %s: %.3f, %s: %.3f, %s: %lf sec\n",
                     "_AMD_APU_ENERGY_ESMI", hostname, "Socket", chipid,
                     "SocketEnergy_J", socket_energy,
                     "CCDEnergy_J", ccd_energy,
                     "XCDEnergy_J", xcd_energy,
                     "AIDEnergy_J", aid_energy,
                     "HBMEnergy_J", hbm_energy,
                     "Timestamp",
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output,
                    "_AMD_APU_ENERGY_ESMI Host: %s, Socket: %d, SocketEnergy: %.3f J, CCDEnergy: %.3f J, XCDEnergy: %.3f J, AIDEnergy: %.3f J, HBMEnergy: %.3f J, Timestamp: %lf sec\n",
                    hostname, chipid, socket_energy, ccd_energy, xcd_energy, aid_energy, hbm_energy,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
        else
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "_AMD_APU_ENERGY_ESMI %s %d %.3f %.3f %.3f %.3f %.3f %lf\n",
                     hostname, chipid, socket_energy, ccd_energy, xcd_energy, aid_energy, hbm_energy,
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output, "_AMD_APU_ENERGY_ESMI %s %d %.3f %.3f %.3f %.3f %.3f %lf\n",
                    hostname, chipid, socket_energy, ccd_energy, xcd_energy, aid_energy, hbm_energy,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif
}

void get_energy_json_esmi(int chipid, int total_sockets, json_t *output)
{
    char socketID[16];
    struct hsmp_metric_table mtbl;

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    snprintf(socketID, 16, "socket_%d", chipid);

    json_object_set_new(output, "num_apus_per_node",
                        json_integer(total_sockets));

    json_t *socket_obj = json_object_get(output, socketID);
    if (socket_obj == NULL)
    {
        socket_obj = json_object();
        json_object_set_new(output, socketID, socket_obj);
    }

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        json_t *energy_obj = json_object();
        json_object_set_new(socket_obj, "energy_apu_esmi_joules", energy_obj);

        json_object_set_new(energy_obj, "socket_energy",
                            json_real(mtbl.socket_energy_acc * UQ16));
        json_object_set_new(energy_obj, "ccd_energy",
                            json_real(mtbl.ccd_energy_acc * UQ16));
        json_object_set_new(energy_obj, "xcd_energy",
                            json_real(mtbl.xcd_energy_acc * UQ16));
        json_object_set_new(energy_obj, "aid_energy",
                            json_real(mtbl.aid_energy_acc * UQ16));
        json_object_set_new(energy_obj, "hbm_energy",
                            json_real(mtbl.hbm_energy_acc * UQ16));

        // Add to node total if it exists
        double socket_energy = mtbl.socket_energy_acc * UQ16;
        if (json_object_get(output, "energy_node_joules") != NULL)
        {
            double energy_node = json_real_value(json_object_get(output, "energy_node_joules"));
            energy_node += socket_energy;
            json_object_set_new(output, "energy_node_joules", json_real(energy_node));
        }
        else
        {
            json_object_set_new(output, "energy_node_joules", json_real(socket_energy));
        }
    }
}

void get_power_data_esmi(int chipid, int total_sockets, int verbose, FILE *output)
{
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;
    struct hsmp_metric_table mtbl;

    gethostname(hostname, 1024);

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s %s\n",
                     "_AMD_APU_POWER_ESMI", "Host", "Socket",
                     "Power_W", "PowerLimit_W", "MaxPowerLimit_W", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_POWER_ESMI Host Socket Power_W PowerLimit_W MaxPowerLimit_W Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        double socket_power = mtbl.socket_power * Q10;
        double socket_power_limit = mtbl.socket_power_limit * Q10;
        double max_socket_power = mtbl.max_socket_power_limit * Q10;

        if (verbose == 1)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s: %s, %s: %d, %s: %.2f, %s: %.2f, %s: %.2f, %s: %lf sec\n",
                     "_AMD_APU_POWER_ESMI", hostname, "Socket", chipid,
                     "Power_W", socket_power,
                     "PowerLimit_W", socket_power_limit,
                     "MaxPowerLimit_W", max_socket_power,
                     "Timestamp",
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output,
                    "_AMD_APU_POWER_ESMI Host: %s, Socket: %d, Power: %.2f W, PowerLimit: %.2f W, MaxPowerLimit: %.2f W, Timestamp: %lf sec\n",
                    hostname, chipid, socket_power, socket_power_limit, max_socket_power,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
        else
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "_AMD_APU_POWER_ESMI %s %d %.2f %.2f %.2f %lf\n",
                     hostname, chipid, socket_power, socket_power_limit, max_socket_power,
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output, "_AMD_APU_POWER_ESMI %s %d %.2f %.2f %.2f %lf\n",
                    hostname, chipid, socket_power, socket_power_limit, max_socket_power,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif
}

void get_json_power_data_esmi(json_t *get_power_obj, int total_sockets)
{
    int chipid;
    char socketID[24];
    struct hsmp_metric_table mtbl;
    double total_apu_power = 0.0;

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    json_object_set_new(get_power_obj, "num_apus_per_node",
                        json_integer(total_sockets));

    for (chipid = 0; chipid < total_sockets; chipid++)
    {
        snprintf(socketID, 24, "socket_%d", chipid);

        json_t *socket_obj = json_object_get(get_power_obj, socketID);
        if (socket_obj == NULL)
        {
            socket_obj = json_object();
            json_object_set_new(get_power_obj, socketID, socket_obj);
        }

        if (get_socket_metrics_table(chipid, &mtbl) == 0)
        {
            json_t *apu_obj = json_object();
            json_object_set_new(socket_obj, "power_apu_esmi_watts", apu_obj);

            double socket_power = mtbl.socket_power * Q10;
            double socket_power_limit = mtbl.socket_power_limit * Q10;
            double max_socket_power = mtbl.max_socket_power_limit * Q10;

            json_object_set_new(apu_obj, "socket_power", json_real(socket_power));
            json_object_set_new(apu_obj, "socket_power_limit", json_real(socket_power_limit));
            json_object_set_new(apu_obj, "max_socket_power_limit", json_real(max_socket_power));

            total_apu_power += socket_power;
        }
    }

    // Update or create power_node_watts
    if (json_object_get(get_power_obj, "power_node_watts") != NULL)
    {
        double power_node = json_real_value(json_object_get(get_power_obj, "power_node_watts"));
        power_node += total_apu_power;
        json_object_set_new(get_power_obj, "power_node_watts", json_real(power_node));
    }
    else
    {
        json_object_set_new(get_power_obj, "power_node_watts", json_real(total_apu_power));
    }
}

void get_thermals_data_esmi(int chipid, int total_sockets, int verbose, FILE *output)
{
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;
    struct hsmp_metric_table mtbl;

    gethostname(hostname, 1024);

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s %s\n",
                     "_AMD_APU_TEMP_ESMI", "Host", "Socket",
                     "MaxSocketTemp_C", "MaxVRTemp_C", "MaxHBMTemp_C", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_TEMP_ESMI Host Socket MaxSocketTemp_C MaxVRTemp_C MaxHBMTemp_C Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        double max_socket_temp = mtbl.max_socket_temperature * Q10;
        double max_vr_temp = mtbl.max_vr_temperature * Q10;
        double max_hbm_temp = mtbl.max_hbm_temperature * Q10;

        if (verbose == 1)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s: %s, %s: %d, %s: %.2f, %s: %.2f, %s: %.2f, %s: %lf sec\n",
                     "_AMD_APU_TEMP_ESMI", hostname, "Socket", chipid,
                     "MaxSocketTemp_C", max_socket_temp,
                     "MaxVRTemp_C", max_vr_temp,
                     "MaxHBMTemp_C", max_hbm_temp,
                     "Timestamp",
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output,
                    "_AMD_APU_TEMP_ESMI Host: %s, Socket: %d, MaxSocketTemp: %.2f C, MaxVRTemp: %.2f C, MaxHBMTemp: %.2f C, Timestamp: %lf sec\n",
                    hostname, chipid, max_socket_temp, max_vr_temp, max_hbm_temp,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
        else
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "_AMD_APU_TEMP_ESMI %s %d %.2f %.2f %.2f %lf\n",
                     hostname, chipid, max_socket_temp, max_vr_temp, max_hbm_temp,
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output, "_AMD_APU_TEMP_ESMI %s %d %.2f %.2f %.2f %lf\n",
                    hostname, chipid, max_socket_temp, max_vr_temp, max_hbm_temp,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif
}

void get_thermals_json_esmi(int chipid, int total_sockets, json_t *output)
{
    char socketid[12];
    struct hsmp_metric_table mtbl;

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    snprintf(socketid, 12, "socket_%d", chipid);

    json_object_set_new(output, "num_apus_per_node",
                        json_integer(total_sockets));

    json_t *socket_obj = json_object_get(output, socketid);
    if (socket_obj == NULL)
    {
        socket_obj = json_object();
        json_object_set_new(output, socketid, socket_obj);
    }

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        json_t *temp_obj = json_object();
        json_object_set_new(socket_obj, "APU_ESMI", temp_obj);

        json_object_set_new(temp_obj, "max_socket_temp_celsius",
                            json_real(mtbl.max_socket_temperature * Q10));
        json_object_set_new(temp_obj, "max_vr_temp_celsius",
                            json_real(mtbl.max_vr_temperature * Q10));
        json_object_set_new(temp_obj, "max_hbm_temp_celsius",
                            json_real(mtbl.max_hbm_temperature * Q10));
    }
}

void get_clocks_data_esmi(int chipid, int total_sockets, int verbose, FILE *output)
{
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;
    struct hsmp_metric_table mtbl;

    gethostname(hostname, 1024);

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s %s %s %s %s\n",
                     "_AMD_APU_CLOCKS_ESMI", "Host", "Socket",
                     "CCLKLimit_MHz", "GFXCLKLimit_MHz", "FCLK_MHz", "UCLK_MHz", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_CLOCKS_ESMI Host Socket CCLKLimit_MHz GFXCLKLimit_MHz FCLK_MHz UCLK_MHz Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        double cclk_limit = mtbl.cclk_frequency_limit * Q10;
        double gfxclk_limit = mtbl.gfxclk_frequency_limit * Q10;
        double fclk_freq = mtbl.fclk_frequency * Q10;
        double uclk_freq = mtbl.uclk_frequency * Q10;

        if (verbose == 1)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s: %s, %s: %d, %s: %.2f, %s: %.2f, %s: %.2f, %s: %.2f, %s: %lf sec\n",
                     "_AMD_APU_CLOCKS_ESMI", hostname, "Socket", chipid,
                     "CCLKLimit_MHz", cclk_limit,
                     "GFXCLKLimit_MHz", gfxclk_limit,
                     "FCLK_MHz", fclk_freq,
                     "UCLK_MHz", uclk_freq,
                     "Timestamp",
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output,
                    "_AMD_APU_CLOCKS_ESMI Host: %s, Socket: %d, CCLKLimit: %.2f MHz, GFXCLKLimit: %.2f MHz, FCLK: %.2f MHz, UCLK: %.2f MHz, Timestamp: %lf sec\n",
                    hostname, chipid, cclk_limit, gfxclk_limit, fclk_freq, uclk_freq,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
        else
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "_AMD_APU_CLOCKS_ESMI %s %d %.2f %.2f %.2f %.2f %lf\n",
                     hostname, chipid, cclk_limit, gfxclk_limit, fclk_freq, uclk_freq,
                     (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#else
            fprintf(output, "_AMD_APU_CLOCKS_ESMI %s %d %.2f %.2f %.2f %.2f %lf\n",
                    hostname, chipid, cclk_limit, gfxclk_limit, fclk_freq, uclk_freq,
                    (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0);
#endif
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif
}

void get_clocks_json_esmi(int chipid, int total_sockets, json_t *output)
{
    char socketid[12];
    struct hsmp_metric_table mtbl;

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    snprintf(socketid, 12, "socket_%d", chipid);

    json_object_set_new(output, "num_apus_per_node",
                        json_integer(total_sockets));

    json_t *socket_obj = json_object_get(output, socketid);
    if (socket_obj == NULL)
    {
        socket_obj = json_object();
        json_object_set_new(output, socketid, socket_obj);
    }

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        json_t *clock_obj = json_object();
        json_object_set_new(socket_obj, "clocks_apu_esmi_mhz", clock_obj);

        json_object_set_new(clock_obj, "cclk_frequency_limit",
                            json_real(mtbl.cclk_frequency_limit * Q10));
        json_object_set_new(clock_obj, "gfxclk_frequency_limit",
                            json_real(mtbl.gfxclk_frequency_limit * Q10));
        json_object_set_new(clock_obj, "fclk_frequency",
                            json_real(mtbl.fclk_frequency * Q10));
        json_object_set_new(clock_obj, "uclk_frequency",
                            json_real(mtbl.uclk_frequency * Q10));

        // Add per-XCC GFXCLK frequencies (assuming up to 8 XCCs)
        json_t *gfxclk_xcc_obj = json_object();
        json_object_set_new(clock_obj, "gfxclk_xcc_frequencies", gfxclk_xcc_obj);

        for (int i = 0; i < 8; i++)
        {
            char xcc_name[16];
            snprintf(xcc_name, sizeof(xcc_name), "xcc_%d", i);
            json_object_set_new(gfxclk_xcc_obj, xcc_name,
                                json_real(mtbl.gfxclk_frequency[i] * Q10));
        }
    }
}

void get_all_metrics_data_esmi(int chipid, int total_sockets, int verbose, FILE *output)
{
    char hostname[1024];
    static int init = 0;
    static struct timeval start;
    struct timeval now;
    struct hsmp_metric_table mtbl;

    gethostname(hostname, 1024);

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    if (!init)
    {
        init = 1;
        gettimeofday(&start, NULL);
        if (verbose == 0)
        {
#ifdef LIBJUSTIFY_FOUND
            cfprintf(output, "%s %s %s %s\n",
                     "_AMD_APU_ALL_METRICS_ESMI", "Host", "Socket", "Timestamp_sec");
#else
            fprintf(output,
                    "_AMD_APU_ALL_METRICS_ESMI Host Socket Timestamp_sec\n");
#endif
        }
    }

    gettimeofday(&now, NULL);

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0;

        if (verbose == 1)
        {
            fprintf(output, "\n=== AMD APU ESMI Metrics for Socket %d (Host: %s, Time: %.3f sec) ===\n",
                    chipid, hostname, elapsed);
            fprintf(output, "Timestamp:            %.0f ns\n", mtbl.timestamp * Q1);
            fprintf(output, "Accumulation Counter: %.0f\n", mtbl.accumulation_counter * Q1);
            fprintf(output, "\nPower:\n");
            fprintf(output, "  Socket Power:       %.2f W\n", mtbl.socket_power * Q10);
            fprintf(output, "  Power Limit:        %.2f W\n", mtbl.socket_power_limit * Q10);
            fprintf(output, "  Max Power Limit:    %.2f W\n", mtbl.max_socket_power_limit * Q10);
            fprintf(output, "\nTemperature:\n");
            fprintf(output, "  Max Socket Temp:    %.2f C\n", mtbl.max_socket_temperature * Q10);
            fprintf(output, "  Max VR Temp:        %.2f C\n", mtbl.max_vr_temperature * Q10);
            fprintf(output, "  Max HBM Temp:       %.2f C\n", mtbl.max_hbm_temperature * Q10);
            fprintf(output, "\nEnergy:\n");
            fprintf(output, "  Socket Energy:      %.3f J\n", mtbl.socket_energy_acc * UQ16);
            fprintf(output, "  CCD Energy:         %.3f J\n", mtbl.ccd_energy_acc * UQ16);
            fprintf(output, "  XCD Energy:         %.3f J\n", mtbl.xcd_energy_acc * UQ16);
            fprintf(output, "  AID Energy:         %.3f J\n", mtbl.aid_energy_acc * UQ16);
            fprintf(output, "  HBM Energy:         %.3f J\n", mtbl.hbm_energy_acc * UQ16);
            fprintf(output, "\nFrequencies:\n");
            fprintf(output, "  CCLK Limit:         %.2f MHz\n", mtbl.cclk_frequency_limit * Q10);
            fprintf(output, "  GFXCLK Limit:       %.2f MHz\n", mtbl.gfxclk_frequency_limit * Q10);
            fprintf(output, "  FCLK:               %.2f MHz\n", mtbl.fclk_frequency * Q10);
            fprintf(output, "  UCLK:               %.2f MHz\n", mtbl.uclk_frequency * Q10);
            fprintf(output, "\nPer-XCC GFXCLK:\n");
            for (int i = 0; i < 8; i++)
            {
                fprintf(output, "  XCC %d:              %.2f MHz\n", i, mtbl.gfxclk_frequency[i] * Q10);
            }
            fprintf(output, "\nActivity:\n");
            fprintf(output, "  Socket C0 Residency: %.2f %%\n", mtbl.socket_c0_residency * Q10);
            fprintf(output, "  Socket GFX Busy:     %.2f %%\n", mtbl.socket_gfx_busy * Q10);
            fprintf(output, "  DRAM BW Utilization: %.2f %%\n", mtbl.dram_bandwidth_utilization * Q10);
            fprintf(output, "\n");
        }
        else
        {
            fprintf(output, "_AMD_APU_ALL_METRICS_ESMI %s %d %.3f\n",
                    hostname, chipid, elapsed);
        }
    }

#ifdef LIBJUSTIFY_FOUND
    cflush();
#endif
}

void get_all_metrics_json_esmi(int chipid, int total_sockets, json_t *output)
{
    char socketid[12];
    struct hsmp_metric_table mtbl;

    if (init_esmi_if_needed() != 0)
    {
        return;
    }

    snprintf(socketid, 12, "socket_%d", chipid);

    json_object_set_new(output, "num_apus_per_node",
                        json_integer(total_sockets));

    json_t *socket_obj = json_object_get(output, socketid);
    if (socket_obj == NULL)
    {
        socket_obj = json_object();
        json_object_set_new(output, socketid, socket_obj);
    }

    if (get_socket_metrics_table(chipid, &mtbl) == 0)
    {
        json_t *esmi_obj = json_object();
        json_object_set_new(socket_obj, "esmi_metrics_table", esmi_obj);

        // Timestamp and counters
        json_object_set_new(esmi_obj, "timestamp_ns", json_real(mtbl.timestamp * Q1));
        json_object_set_new(esmi_obj, "accumulation_counter", json_real(mtbl.accumulation_counter * Q1));

        // Power
        json_t *power_obj = json_object();
        json_object_set_new(esmi_obj, "power_watts", power_obj);
        json_object_set_new(power_obj, "socket_power", json_real(mtbl.socket_power * Q10));
        json_object_set_new(power_obj, "socket_power_limit", json_real(mtbl.socket_power_limit * Q10));
        json_object_set_new(power_obj, "max_socket_power_limit", json_real(mtbl.max_socket_power_limit * Q10));

        // Temperature
        json_t *temp_obj = json_object();
        json_object_set_new(esmi_obj, "temperature_celsius", temp_obj);
        json_object_set_new(temp_obj, "max_socket_temperature", json_real(mtbl.max_socket_temperature * Q10));
        json_object_set_new(temp_obj, "max_vr_temperature", json_real(mtbl.max_vr_temperature * Q10));
        json_object_set_new(temp_obj, "max_hbm_temperature", json_real(mtbl.max_hbm_temperature * Q10));

        // Energy
        json_t *energy_obj = json_object();
        json_object_set_new(esmi_obj, "energy_joules", energy_obj);
        json_object_set_new(energy_obj, "socket_energy", json_real(mtbl.socket_energy_acc * UQ16));
        json_object_set_new(energy_obj, "ccd_energy", json_real(mtbl.ccd_energy_acc * UQ16));
        json_object_set_new(energy_obj, "xcd_energy", json_real(mtbl.xcd_energy_acc * UQ16));
        json_object_set_new(energy_obj, "aid_energy", json_real(mtbl.aid_energy_acc * UQ16));
        json_object_set_new(energy_obj, "hbm_energy", json_real(mtbl.hbm_energy_acc * UQ16));

        // Frequencies
        json_t *freq_obj = json_object();
        json_object_set_new(esmi_obj, "frequencies_mhz", freq_obj);
        json_object_set_new(freq_obj, "cclk_frequency_limit", json_real(mtbl.cclk_frequency_limit * Q10));
        json_object_set_new(freq_obj, "gfxclk_frequency_limit", json_real(mtbl.gfxclk_frequency_limit * Q10));
        json_object_set_new(freq_obj, "fclk_frequency", json_real(mtbl.fclk_frequency * Q10));
        json_object_set_new(freq_obj, "uclk_frequency", json_real(mtbl.uclk_frequency * Q10));

        // Per-XCC GFXCLK
        json_t *gfxclk_xcc_obj = json_object();
        json_object_set_new(freq_obj, "gfxclk_xcc", gfxclk_xcc_obj);
        for (int i = 0; i < 8; i++)
        {
            char xcc_name[16];
            snprintf(xcc_name, sizeof(xcc_name), "xcc_%d", i);
            json_object_set_new(gfxclk_xcc_obj, xcc_name, json_real(mtbl.gfxclk_frequency[i] * Q10));
        }

        // Activity
        json_t *activity_obj = json_object();
        json_object_set_new(esmi_obj, "activity_percent", activity_obj);
        json_object_set_new(activity_obj, "socket_c0_residency", json_real(mtbl.socket_c0_residency * Q10));
        json_object_set_new(activity_obj, "socket_gfx_busy", json_real(mtbl.socket_gfx_busy * Q10));
        json_object_set_new(activity_obj, "dram_bandwidth_utilization", json_real(mtbl.dram_bandwidth_utilization * Q10));
    }
}
