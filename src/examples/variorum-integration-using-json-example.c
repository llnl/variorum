// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <variorum.h>
#include <variorum_topology.h>

#ifdef SECOND_RUN
static inline double do_work(int input)
{
    int i;
    double result = (double)input;

    for (i = 0; i < 100000; i++)
    {
        result += i * result;
    }

    return result;
}
#endif

void parse_json_power_obj(char *s, int num_sockets)
{

    const char *hostname = NULL;
    json_t *node_obj = NULL;
    json_t *power_obj = json_loads(s, JSON_DECODE_ANY, NULL);
    void *iter = json_object_iter(power_obj);

    /* This is tailored to the nested structure that we have created (see docs). */
    /* Just for the first level, we use the iterator to obtain the hostname,
     * as this is encoded as a key to reduce verbosity. */

    while (iter)
    {
        hostname = json_object_iter_key(iter);
        node_obj = json_object_iter_value(iter);
        if (node_obj == NULL)
        {
            printf("JSON object not found");
            exit(0);
        }
        /* The following should return NULL after the first call per our object. */
        iter = json_object_iter_next(power_obj, iter);
    }

    uint64_t timestamp;
    int num_gpus_per_socket = -1;

    //  Extract node-levels value from node object
    timestamp = json_integer_value(json_object_get(node_obj, "timestamp"));

    printf("Hostname: %s\n", hostname);
    printf("Timestamp: %lu\n", timestamp);

    // If we're on a GPU-only build, we don't have power_node_watts.
    if (json_object_get(node_obj, "power_node_watts") != NULL)
    {
        double power_node;
        power_node = json_real_value(json_object_get(node_obj, "power_node_watts"));
        printf("Node Power: %0.2lf Watts\n", power_node);
    }

    // If we're on a CPU-only build, we don't have num_gpus_per_socket
    if (json_object_get(node_obj, "num_gpus_per_socket") != NULL)
    {
        num_gpus_per_socket = json_integer_value(json_object_get(node_obj,
                              "num_gpus_per_socket"));
        printf("Number of GPUs per socket: %d\n", num_gpus_per_socket);
    }

    //  Extract socket and GPU-level data from the node object

    int i;
    char socketID[20];

    for (i = 0; i < num_sockets; ++i)
    {
        snprintf(socketID, 20, "socket_%d", i);
        json_t *socket_obj = json_object_get(node_obj, socketID);
        if (socket_obj == NULL)
        {
            printf("Socket object not found!\n");
            exit(0);
        }

        //If we're on a GPU-only build, we don't have power_cpu_watts
        if (json_object_get(socket_obj, "power_cpu_watts") != NULL)
        {
            double power_cpu;
            power_cpu = json_real_value(json_object_get(socket_obj, "power_cpu_watts"));
            printf("Socket %d, CPU Power: %0.2lf Watts\n", i, power_cpu);
        }

        //If we're on a GPU-only build, we don't have power_mem_watts
        if (json_object_get(socket_obj, "power_mem_watts") != NULL)
        {
            double power_mem;
            power_mem = json_real_value(json_object_get(socket_obj, "power_mem_watts"));
            printf("Socket %d, Mem Power: %0.2lf Watts\n", i, power_mem);
        }

        if (num_gpus_per_socket > 0)
        {
            json_t *gpu_obj = json_object_get(socket_obj, "power_gpu_watts");
            json_t *apu_obj = json_object_get(socket_obj, "power_apu_watts");
            // If not APU but there are GPUs
	    if (gpu_obj != NULL)
	    {
                const char *key;
                json_t *value;

                json_object_foreach(gpu_obj, key, value)
                {
                    printf("Socket %d, %s Power: %0.2lf Watts\n", i, key, json_real_value(value));
                }

	    }
	    // If APU
	    else if (apu_obj != NULL)
	    {
                const char *key;
                json_t *value;

                json_object_foreach(apu_obj, key, value)
                {
                    printf("APU %d, %s Power: %0.2lf Watts\n", i, key, json_real_value(value));
                }

	    }
	    else
	    {
                printf("Neither GPU nor APU object not found! \n");
                exit(0);
	    }

        }

    }

    // Clean up memory.
    json_decref(power_obj);
}


int main(void)
{
    int ret;
    int num_sockets = 0;
    char *s = NULL;
#ifdef SECOND_RUN
    int i;
    int size = 1E4;
    volatile double x = 0.0;
#endif

    /* Determine number of sockets */
    num_sockets = variorum_get_num_sockets();
    if (num_sockets <= 0)
    {
        printf("HWLOC returned an invalid number of sockets. Exiting.\n");
        exit(-1);
    }

    ret = variorum_get_power_json(&s);
    if (ret != 0)
    {
        printf("First run: JSON get node power failed!\n");
        free(s);
        exit(-1);
    }

    /* Print the entire JSON object and then the parsed JSON object */
    printf("\n*****JSON object received from first run is: \n");
    puts(s);
    parse_json_power_obj(s, num_sockets);

    /* Deallocate the string */
    free(s);

#ifdef SECOND_RUN
    for (i = 0; i < size; i++)
    {
        x += do_work(i);
    }
    printf("Final result: %f\n", x);

    ret = variorum_get_power_json(&s);
    if (ret != 0)
    {
        printf("Second run: JSON get node power failed!\n");
        free(s);
        exit(-1);
    }

    printf("\n*****JSON object received from second run is: \n");
    puts(s);
    parse_json_power_obj(s, num_sockets);
#endif

    /* Deallocate the string */
    free(s);

    return ret;
}
