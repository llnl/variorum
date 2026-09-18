// Copyright 2019-2023 Lawrence Livermore National Security, LLC and other
// Variorum Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

//
// Simple exploratory example for the AMD ESMI metrics table API
// This demonstrates the new variorum_get_amd_esmi_metrics_json() API
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <variorum.h>

void print_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", prog);
    fprintf(stderr, "Explore AMD MI300A metrics using ESMI metrics table\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -n <count>    Number of samples (default: 5)\n");
    fprintf(stderr, "  -d <delay>    Delay between samples in seconds (default: 1)\n");
    fprintf(stderr, "  -h            Show this help\n");
}

int main(int argc, char **argv)
{
    int num_samples = 5;
    int delay_sec = 1;
    int opt;
    char *metrics_json = NULL;

    while ((opt = getopt(argc, argv, "n:d:h")) != -1)
    {
        switch (opt)
        {
        case 'n':
            num_samples = atoi(optarg);
            break;
        case 'd':
            delay_sec = atoi(optarg);
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    // Initialize variorum
    int ret = variorum_init();
    if (ret != 0)
    {
        fprintf(stderr, "Error: Failed to initialize variorum\n");
        return 1;
    }

    printf("=== AMD ESMI Metrics Exploration ===\n");
    printf("Samples: %d, Delay: %d sec\n\n", num_samples, delay_sec);

    for (int i = 0; i < num_samples; i++)
    {
        printf("--- Sample %d/%d ---\n", i + 1, num_samples);

        // Call the new exploratory API
        ret = variorum_get_amd_esmi_metrics_json(&metrics_json);
        if (ret != 0)
        {
            fprintf(stderr, "Error: Failed to get ESMI metrics (code: %d)\n", ret);
            return 1;
        }

        // Print the JSON output
        if (metrics_json != NULL)
        {
            printf("%s\n", metrics_json);
            free(metrics_json);
            metrics_json = NULL;
        }

        if (i < num_samples - 1)
        {
            sleep(delay_sec);
        }
    }

    printf("\n=== Exploration Complete ===\n");

    return 0;
}
