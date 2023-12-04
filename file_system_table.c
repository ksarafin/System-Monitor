#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>

#include "file_system_table.h"

const gchar *file_system_column_names[FILE_SYSTEM_NUM_COLUMNS] = {
  "Device",
  "Directory",
  "Type",
  "Total",
  "Free",
  "Available",
  "Used"
};

void populate_file_system_table(GtkListStore *list_store) {
    FILE *file = fopen("/proc/self/mountinfo", "r");
    if (file == NULL) {
        g_warning("Failed to open /proc/self/mountinfo");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Tokenize the line using whitespace as a delimiter
        char *token;
        char *tokens[12];  // Increase the array size to accommodate all expected tokens
        int tokenCount = 0;

        token = strtok(line, " ");
        while (token != NULL && tokenCount < 12) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, " ");
        }

        if (tokenCount >= 11) {
            gchar *device_name = g_strdup(tokens[9]);

            struct statfs sb;

            statfs(tokens[4],&sb);

            if (device_name != NULL) {
                FileSystemData data = {
                    .device = device_name,
                    .directory = g_strdup(tokens[4]),
                    .type = g_strdup(tokens[8]),
                    .total_space = sb.f_blocks,
                    .free_space = sb.f_bfree,
                    .available_space = sb.f_bavail,
                    .used_space = sb.f_blocks - sb.f_bfree
                };

                // Print debug information
                //g_print("Device: %s, Directory: %s, Type: %s, Free: %lu, Available: %lu, Used: %lu\n",
                  //      data.device, data.directory, data.type, data.free_space, data.available_space, data.used_space);

                GtkTreeIter iter;
                gtk_list_store_append(list_store, &iter);
                gtk_list_store_set(list_store, &iter,
                                    0, data.device,
                                    1, data.directory,
                                    2, data.type,
                                    3, data.total_space,
                                    4, data.free_space,
                                    5, data.available_space,
                                    6, data.used_space,
                                    -1);

                // Free the memory allocated for the strings in data
                g_free(data.device);
                g_free(data.directory);
                g_free(data.type);
            }
        }
    }

    fclose(file);
}
