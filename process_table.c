#include "process_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

const gchar *column_names[NUM_COLUMNS] = {
  "Process Name",
  "Status",
  "%CPU",
  "ID",
  "Memory"
};


void populate_process_table(GtkListStore *list_store) {
    // Get the UID (User ID) of the current user
    uid_t current_uid = getuid();

    // Open the /proc directory
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Error opening /proc directory");
        exit(EXIT_FAILURE);
    }

    // Read the contents of the /proc directory
    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        // Check if the entry is a directory and represents a process
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            // Convert the process ID from string to integer
            int pid = atoi(entry->d_name);

            // Compose the path to the /proc/[pid]/comm file
            char comm_path[256];
            snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

            // Open the /proc/[pid]/comm file
            FILE *comm_file = fopen(comm_path, "r");
            if (comm_file == NULL) {
                // Skip if unable to open the comm file
                continue;
            }

            // Read the contents of the comm file to get the process name
            char process_name[256];
            if (fgets(process_name, sizeof(process_name), comm_file) != NULL) {
                // Remove the newline character if present
                size_t len = strlen(process_name);
                if (len > 0 && process_name[len - 1] == '\n') {
                    process_name[len - 1] = '\0';
                }

                // Close the comm file
                fclose(comm_file);

                // Compose the path to the /proc/[pid]/stat file
                char stat_path[256];
                snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

                // Open the /proc/[pid]/stat file
                FILE *stat_file = fopen(stat_path, "r");
                if (stat_file == NULL) {
                    // Skip if unable to open the stat file
                    continue;
                }

                // Read the contents of the stat file to get CPU time
                unsigned long utime, stime;
                if (fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %lu %lu", &utime, &stime) != 2) {
                    // Skip if unable to read CPU time
                    fclose(stat_file);
                    continue;
                }

                // Close the stat file
                fclose(stat_file);

                // Calculate %CPU
                unsigned long total_time = utime + stime;
                double seconds = sysconf(_SC_CLK_TCK); // Clock ticks per second
                double cpu_percentage = (total_time / seconds) * 100;

                // Compose the path to the /proc/[pid]/status file
                char status_path[256];
                snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

                // Open the /proc/[pid]/status file
                FILE *status_file = fopen(status_path, "r");
                if (status_file == NULL) {
                    // Skip if unable to open the status file
                    continue;
                }

                // Read the contents of the status file to get the UID and Status
                char line[256];
                uid_t process_uid = 0;
                char process_status[256] = "";
                gsize memory_usage = 0;
                while (fgets(line, sizeof(line), status_file) != NULL) {
                    // Check for the "Uid:" line
                    if (strncmp(line, "Uid:", 4) == 0) {
                        // Extract the UID from the line
                        process_uid = atoi(line + 5);
                    }

                    // Check for the "State:" line (Status)
                    if (strncmp(line, "State:", 6) == 0) {
                        // Extract the Status from the line
                        sscanf(line + 6, "%s", process_status);
                    }

                    // Check for the "VmRSS:" line (Resident Set Size)
                    if (strncmp(line, "VmRSS:", 6) == 0) {
                        // Extract the Memory Usage from the line
                        sscanf(line + 6, "%lu", &memory_usage);
                        memory_usage *= 1024; // Convert from kB to bytes
                    }
                }

                // Check if the process is owned by the current user
                if (process_uid == current_uid) {
                    // Dummy data (replace with actual process data retrieval)
                    gchar *status = g_strdup_printf("%s", process_status);
                    guint process_id = pid;

                    // Append data to the GtkListStore
                    GtkTreeIter iter;
                    gtk_list_store_append(list_store, &iter);
                    gtk_list_store_set(list_store, &iter, 
                        0, process_name,
                        1, status,
                        2, cpu_percentage,
                        3, process_id,
                        4, memory_usage,
                        -1);

                    g_free(status);
                }

                // Close the status file
                fclose(status_file);
            }
        }
    }

    // Close the /proc directory
    closedir(proc_dir);
}



/*void populate_process_table(GtkListStore *list_store) {
    // Get the UID (User ID) of the current user
    uid_t current_uid = getuid();

    // Open the /proc directory
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Error opening /proc directory");
        exit(EXIT_FAILURE);
    }

    // Read the contents of the /proc directory
    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        // Check if the entry is a directory and represents a process
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            // Convert the process ID from string to integer
            int pid = atoi(entry->d_name);

            // Compose the path to the /proc/[pid]/comm file
            char comm_path[256];
            snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

            // Open the /proc/[pid]/comm file
            FILE *comm_file = fopen(comm_path, "r");
            if (comm_file == NULL) {
                // Skip if unable to open the comm file
                continue;
            }

            // Read the contents of the comm file to get the process name
            char process_name[256];
            if (fgets(process_name, sizeof(process_name), comm_file) != NULL) {
                // Remove the newline character if present
                size_t len = strlen(process_name);
                if (len > 0 && process_name[len - 1] == '\n') {
                    process_name[len - 1] = '\0';
                }

                // Close the comm file
                fclose(comm_file);

                // Compose the path to the /proc/[pid]/stat file
                char stat_path[256];
                snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

                // Open the /proc/[pid]/stat file
                FILE *stat_file = fopen(stat_path, "r");
                if (stat_file == NULL) {
                    // Skip if unable to open the stat file
                    continue;
                }

                // Read the contents of the stat file to get CPU time
                unsigned long utime, stime;
                if (fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %lu %lu", &utime, &stime) != 2) {
                    // Skip if unable to read CPU time
                    fclose(stat_file);
                    continue;
                }

                // Close the stat file
                fclose(stat_file);

                // Calculate %CPU
                unsigned long total_time = utime + stime;
                double seconds = sysconf(_SC_CLK_TCK); // Clock ticks per second
                double cpu_percentage = (total_time / seconds) * 100;

                // Compose the path to the /proc/[pid]/status file
                char status_path[256];
                snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

                // Open the /proc/[pid]/status file
                FILE *status_file = fopen(status_path, "r");
                if (status_file == NULL) {
                    // Skip if unable to open the status file
                    continue;
                }

                // Read the contents of the status file to get the UID and Status
                char line[256];
                uid_t process_uid = 0;
                char process_status[256] = "";
                gsize memory_usage = 0;
                while (fgets(line, sizeof(line), status_file) != NULL) {
                    // Check for the "Uid:" line
                    if (strncmp(line, "Uid:", 4) == 0) {
                        // Extract the UID from the line
                        process_uid = atoi(line + 5);
                    }

                    // Check for the "State:" line (Status)
                    if (strncmp(line, "State:", 6) == 0) {
                        // Extract the Status from the line
                        sscanf(line + 6, "%s", process_status);
                    }

                    // Check for the "VmRSS:" line (Resident Set Size)
                    if (strncmp(line, "VmRSS:", 6) == 0) {
                        // Extract the Memory Usage from the line
                        sscanf(line + 6, "%lu", &memory_usage);
                        memory_usage *= 1024; // Convert from kB to bytes
                    }
                }

                // Check if the process is owned by the current user or show_all_processes is true
                if (g_show_all_processes || process_uid == current_uid) {
                    // Dummy data (replace with actual process data retrieval)
                    gchar *status = g_strdup_printf("%s", process_status);
                    guint process_id = pid;

                    // Append data to the GtkListStore
                    GtkTreeIter iter;
                    gtk_list_store_append(list_store, &iter);
                    gtk_list_store_set(list_store, &iter, 
                        0, process_name,
                        1, status,
                        2, cpu_percentage,
                        3, process_id,
                        4, memory_usage,
                        -1);

                    g_free(status);
                }

                // Close the status file
                fclose(status_file);
            }
        }
    }

    // Close the /proc directory
    closedir(proc_dir);
}*/
