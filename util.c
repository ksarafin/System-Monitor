#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <signal.h>

double get_cpu_usage() {
    FILE *stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        perror("Error opening /proc/stat");
        return -1.0; // Return an error value
    }

    // Read the first line from /proc/stat
    char line[256];
    if (fgets(line, sizeof(line), stat_file) == NULL) {
        perror("Error reading /proc/stat");
        fclose(stat_file);
        return -1.0; // Return an error value
    }

    fclose(stat_file);

    // Extract CPU usage information from the line
    char cpu_name[5];
    unsigned long long user, nice, system, idle;
    sscanf(line, "%s %llu %llu %llu %llu", cpu_name, &user, &nice, &system, &idle);

    // Calculate total and used CPU time
    unsigned long long total_cpu_time = user + nice + system + idle;
    unsigned long long used_cpu_time = total_cpu_time - idle;

    // Calculate CPU usage percentage
    double cpu_usage = ((double)used_cpu_time / total_cpu_time) * 100.0;

    return cpu_usage;
}

double get_process_cpu_usage(int pid) {
    char filename[256];
    FILE *stat_file;

    // Construct the filename for the process's stat file
    snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);

    // Open the process's stat file for reading
    stat_file = fopen(filename, "r");
    if (stat_file == NULL) {
        perror("Error opening process stat file");
        return -1.0; // Return an error value
    }

    // Read the contents of the process's stat file
    char line[256];
    if (fgets(line, sizeof(line), stat_file) == NULL) {
        perror("Error reading process stat file");
        fclose(stat_file);
        return -1.0; // Return an error value
    }

    fclose(stat_file);

    // Extract the necessary information from the line
    int process_pid;
    char process_name[256];
    char state;
    unsigned long utime, stime, cutime, cstime;

    sscanf(line, "%d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %lu %lu",
           &process_pid, process_name, &state, &utime, &stime, &cutime, &cstime);

    // Calculate total process time
    unsigned long total_process_time = utime + stime + cutime + cstime;

    // Calculate process CPU usage percentage
    double process_cpu_usage = ((double)total_process_time) / sysconf(_SC_CLK_TCK);

    return process_cpu_usage;
}



double get_memory_usage() {
    FILE *meminfo_file = fopen("/proc/meminfo", "r");
    if (meminfo_file == NULL) {
        perror("Error opening /proc/meminfo");
        return -1.0; // Return an error value
    }

    unsigned long long total_memory = 0, free_memory = 0, available_memory = 0;

    // Search for the specific strings and read their values
    char line[256];
    while (fgets(line, sizeof(line), meminfo_file) != NULL) {
        if (strstr(line, "MemTotal") != NULL) {
            sscanf(line, "%*s %llu kB", &total_memory);
        } else if (strstr(line, "MemFree") != NULL) {
            sscanf(line, "%*s %llu kB", &free_memory);
        } else if (strstr(line, "MemAvailable") != NULL) {
            sscanf(line, "%*s %llu kB", &available_memory);
        }
    }

    fclose(meminfo_file);

    // Ensure the values are positive before calculating
    if (total_memory <= 0 || free_memory <= 0 || available_memory <= 0) {
        perror("Invalid memory values");
        return -1.0;
    }

    // Calculate memory usage percentage
    double memory_usage = ((double)(total_memory - available_memory) / total_memory) * 100.0;

    return memory_usage;
}

double get_memory_usage_MB(int process_id) {
    char proc_status_path[256];
    FILE *status_file;

    // Create the path to the process-specific status file
    snprintf(proc_status_path, sizeof(proc_status_path), "/proc/%d/status", process_id);

    // Open the process-specific status file
    status_file = fopen(proc_status_path, "r");
    if (status_file == NULL) {
        perror("Error opening /proc/[process_id]/status");
        return -1.0; // Return an error value
    }

    unsigned long long rss_memory = 0;

    // Search for the specific string and read its value
    char line[256];
    while (fgets(line, sizeof(line), status_file) != NULL) {
        if (strstr(line, "VmRSS") != NULL) {
            sscanf(line, "%*s %llu kB", &rss_memory);
            break; // No need to continue reading after finding the VmRSS field
        }
    }

    fclose(status_file);

    // Ensure the value is positive before converting to MB
    if (rss_memory <= 0) {
        perror("Invalid RSS memory value");
        return -1.0;
    }

    // Convert memory usage to megabytes
    double memory_usage_MB = (double)rss_memory / 1024.0;

    return memory_usage_MB;
}

unsigned long long prev_received_bytes = 0;
unsigned long long prev_transmitted_bytes = 0;

double get_network_usage() {
    FILE *netstat_file = fopen("/proc/net/dev", "r");

    if (netstat_file == NULL) {
        perror("Error opening /proc/net/dev");
        return -1.0; // Return an error value
    }

    char line[256];
    unsigned long long total_received_bytes = 0, total_transmitted_bytes = 0;

    // Read and discard the first two lines as they contain headers
    fgets(line, sizeof(line), netstat_file);
    fgets(line, sizeof(line), netstat_file);

    // Iterate through the file
    while (fgets(line, sizeof(line), netstat_file) != NULL) {
        unsigned long long received_bytes = 0, transmitted_bytes = 0;
        sscanf(line, "%*s %llu %*u %*u %*u %*u %*u %*u %*u %*u %llu", &received_bytes, &transmitted_bytes);

        // Sum up the values for all interfaces
        total_received_bytes += received_bytes;
        total_transmitted_bytes += transmitted_bytes;
    }

    fclose(netstat_file);

    // Calculate network usage in megabytes
    double network_usage = 0;
    if (prev_received_bytes != 0) {
        network_usage = ((double)(total_received_bytes + total_transmitted_bytes - prev_received_bytes - prev_transmitted_bytes) / (1024 * 1024));
    }

    // Update the global variables for the next call
    prev_received_bytes = total_received_bytes;
    prev_transmitted_bytes = total_transmitted_bytes;

    return network_usage;
}
