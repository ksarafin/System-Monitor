#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int g_num_samples = 0; 
int g_delay_seconds = 0;
long int g_last_total_time = 0;
long int g_last_idle_time = 0;

void calculate_memory_swap_usage(double *memory_usage, double *swap_usage) {
    // Open /proc/meminfo for reading
    FILE *meminfo_file = fopen("/proc/meminfo", "r");
    if (meminfo_file == NULL) {
        perror("Error opening /proc/meminfo");
        exit(1);
    }

    // Variables to store memory and swap information
    long total_memory, free_memory, total_swap, free_swap;
    char line[100];

    // Read lines from /proc/meminfo and extract relevant information
    while (fgets(line, sizeof(line), meminfo_file)) {
        if (sscanf(line, "MemTotal: %ld kB", &total_memory) == 1) {
            // Found total memory information
        } else if (sscanf(line, "MemFree: %ld kB", &free_memory) == 1) {
            // Found free memory information
        } else if (sscanf(line, "SwapTotal: %ld kB", &total_swap) == 1) {
            // Found total swap information
        } else if (sscanf(line, "SwapFree: %ld kB", &free_swap) == 1) {
            // Found free swap information
        }
    }
    // Close the file
    fclose(meminfo_file);

    // Calculate memory and swap usage percentages
    *memory_usage = ((total_memory - free_memory) * 100.0) / total_memory;
    *swap_usage = ((total_swap - free_swap) * 100.0) / total_swap;
}

void calculate_network_usage() {
    // Open /proc/net/dev for reading
    FILE *netdev_file = fopen("/proc/net/dev", "r");
    if (netdev_file == NULL) {
        perror("Error opening /proc/net/dev");
        exit(1);
    }

    // Variables to store network information
    char line[256];
    long received_bytes, transmitted_bytes;

    // Skip the first two lines of headers in /proc/net/dev
    fgets(line, sizeof(line), netdev_file);
    fgets(line, sizeof(line), netdev_file);

    // Read lines from /proc/net/dev and extract relevant information
    if (fgets(line, sizeof(line), netdev_file) != NULL) {
        // Assuming there is at least one network interface
        sscanf(line, "%*s %ld %*d %*d %*d %*d %*d %*d %*d %*ld %*ld %*ld %*ld %*ld %*ld %*ld %*ld %ld",
               &received_bytes, &transmitted_bytes);
    } else {
        fprintf(stderr, "Error reading /proc/net/dev");
        exit(1);
    }
    // close the file
    fclose(netdev_file);

    // Print or store the network usage
    printf("Received Bytes: %ld, Transmitted Bytes: %ld\n", received_bytes, transmitted_bytes);
}

double calculate_cpu_usage() { 
    // variables to store information from /proc/stat
    char stat_line[100];
    const char delimiter[2] = " ";
    char *token = NULL;
    long int total_time = 0;
    long int idle_time = 0;
    double idle_percentage = 0.0;

    // open /proc/stat for reading
    FILE *stat_file = fopen("/proc/stat", "r");
    int field_index = 0;

    // read a line from /proc/stat
    fgets(stat_line, sizeof(stat_line), stat_file);
    fclose(stat_file);

    // tokenize the line using space as the delimiter
    token = strtok(stat_line, delimiter);
    total_time = 0;

    // process each token
    while (token != NULL) {
        token = strtok(NULL, delimiter);

        if (token != NULL) {
            total_time += atoi(token);
        }

        // check if the token corresponds to idle time
        if (field_index == 3) {
            idle_time = atoi(token);
        }

        field_index++;
    }

    // calculate idle percentage
    idle_percentage = 100 - (idle_time - g_last_idle_time) * 100.0 / (total_time - g_last_total_time);

    // update last values for the next iteration
    g_last_idle_time = idle_time;
    g_last_total_time = total_time;

    // return the calculated CPU usage percentage
    return 100 - idle_percentage;
}


int main(int argc, char *argv[]) {
    // check if the correct number of command-line arguments is provided
    if (argc != 3) {
        printf("Usage: %s <number of samples /proc/stat should be polled followed by delay in seconds>\n", argv[0]);
    } else {
        // parse command-line arguments
        g_num_samples = atoi(argv[1]);
        g_delay_seconds = atoi(argv[2]);

        // loop for the specified number of samples
        while (g_num_samples > 0) {
            // open /proc/stat for reading
            double cpu_usage = calculate_cpu_usage();

            // Calculate memory and swap usage
            double memory_usage = 0.0;
            double swap_usage = 0.0;
            calculate_memory_swap_usage(&memory_usage, &swap_usage);
            calculate_network_usage();

            // print the result
            //printf("Busy for: %.2lf%% of the time.\n", idle_percentage);

            // decrement the number of samples and sleep for the specified delay
            g_num_samples--;
            sleep(g_delay_seconds);
        }
    }
    return 0;
}
