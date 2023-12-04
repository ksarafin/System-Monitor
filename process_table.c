#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <signal.h>

#include "process_table.h"
#include "ui.h"

#define MAX_PATH_LEN 256
#define MAX_LINE_LEN 256

GtkListStore *processListStore;

const gchar *column_names[NUM_COLUMNS] = {
  "Process Name",
  "Status",
  "%CPU",
  "ID",
  "Memory"
};

void on_stop_process_activate(GtkMenuItem *menu_item, gpointer user_data) {
    guint process_id = GPOINTER_TO_UINT(user_data);

    if (kill(process_id, SIGSTOP) == 0) {
        g_print("Stopped Process: %u\n", process_id);
    } else {
        perror("Error stopping process");
    }
}

void on_continue_process_activate(GtkMenuItem *menu_item, gpointer user_data) {
    guint process_id = GPOINTER_TO_UINT(user_data);

    if (kill(process_id, SIGCONT) == 0) {
        g_print("Continued Process: %u\n", process_id);
    } else {
        perror("Error continuing process");
    }
}

void on_kill_process_activate(GtkMenuItem *menu_item, gpointer user_data) {
    guint process_id = GPOINTER_TO_UINT(user_data);

    if (kill(process_id, SIGKILL) == 0) {
        g_print("Killed Process: %u\n", process_id);
    } else {
        perror("Error killing process");
    }
}


void on_memory_maps_activate(GtkMenuItem *menu_item, gpointer user_data) {
    guint process_id = GPOINTER_TO_UINT(user_data);

    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%u/maps", process_id);

    /* Read the contents of the maps file */

    FILE *maps_file = fopen(maps_path, "r");
    if (!maps_file) {
        g_print("Error opening maps file.\n");
        return;
    }

    /* Create a new GTK window */

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Memory Maps");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    /* Create a scrolled window to display memory maps */

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);

    /* Create a text view to display the content of the maps file */

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    /* Read the contents of the maps file and display in the text view */

    char line[256];
    while (fgets(line, sizeof(line), maps_file) != NULL) {
        gtk_text_buffer_insert_at_cursor(buffer, line, -1);
    }

    fclose(maps_file);

    /* Add the text view to the scrolled window */

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    gtk_widget_show_all(window);
}

void on_open_files_activate(GtkMenuItem *menu_item, gpointer user_data) {
    guint process_id = GPOINTER_TO_UINT(user_data);

    char fd_path[256];
    snprintf(fd_path, sizeof(fd_path), "/proc/%u/fd/", process_id);

    /* Create a new GTK window */

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Open Files List");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    /* Create a grid to display open files with columns for name, type, and object */

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    /* Labels for column headers */

    GtkWidget *name_label = gtk_label_new("File Name");
    GtkWidget *type_label = gtk_label_new("File Type");
    GtkWidget *object_label = gtk_label_new("Object");

    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), type_label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), object_label, 2, 0, 1, 1);

    GFile *dir = g_file_new_for_path(fd_path);
    GFileEnumerator *enumerator = g_file_enumerate_children(dir, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);

    int row = 1;

    /* Iterate through the files in the directory and add them to the grid */

    GFileInfo *info;
    while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
        const char *file_name = g_file_info_get_name(info);

        char link_path[256];
        snprintf(link_path, sizeof(link_path), "%s%s", fd_path, file_name);
        char target_path[256];
        ssize_t target_length = readlink(link_path, target_path, sizeof(target_path) - 1);
        if (target_length != -1) {
            target_path[target_length] = '\0';

            GtkWidget *name_label = gtk_label_new(file_name);
            GtkWidget *type_label = gtk_label_new(g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_TYPE));
            GtkWidget *object_label = gtk_label_new(target_path);

            gtk_grid_attach(GTK_GRID(grid), name_label, 0, row, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), type_label, 1, row, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), object_label, 2, row, 1, 1);

            row++;
        }

        g_object_unref(info);
    }

    g_object_unref(enumerator);
    g_object_unref(dir);

    gtk_widget_show_all(window);
}

void on_process_right_click(GtkTreeView *treeview, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {

        gint x, y;
        GtkTreePath *path;
        GtkTreeViewColumn *column;
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), event->x, event->y, &path, &column, &x, &y)) {

            gint *indices = gtk_tree_path_get_indices(path);
            gint index = indices[0];

            GtkTreeModel *model;
            GtkTreeIter iter;
            model = gtk_tree_view_get_model(treeview);

            if (gtk_tree_model_get_iter(model, &iter, path)) {
                guint process_id;
                gtk_tree_model_get(model, &iter, 3, &process_id, -1);

                GtkWidget *context_menu = gtk_menu_new();

                /* Add menu items for the context menu */

                GtkWidget *kill_item = gtk_menu_item_new_with_label("Stop Process");
                GtkWidget *pause_item = gtk_menu_item_new_with_label("Continue Process");
                GtkWidget *continue_item = gtk_menu_item_new_with_label("Kill Process");
                GtkWidget *open_file_location_item = gtk_menu_item_new_with_label("Memory Maps");
                GtkWidget *properties_item = gtk_menu_item_new_with_label("Open Files");

                /* Connect each menu item to its callback function and pass the process ID as user data */

                g_signal_connect(kill_item, "activate", G_CALLBACK(on_stop_process_activate), GUINT_TO_POINTER(process_id));
                g_signal_connect(pause_item, "activate", G_CALLBACK(on_continue_process_activate), GUINT_TO_POINTER(process_id));
                g_signal_connect(continue_item, "activate", G_CALLBACK(on_kill_process_activate), GUINT_TO_POINTER(process_id));
                g_signal_connect(open_file_location_item, "activate", G_CALLBACK(on_memory_maps_activate), GUINT_TO_POINTER(process_id));
                g_signal_connect(properties_item, "activate", G_CALLBACK(on_open_files_activate), GUINT_TO_POINTER(process_id));


                /* Append menu items to the context menu */

                gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), kill_item);
                gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), pause_item);
                gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), continue_item);
                gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), open_file_location_item);
                gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), properties_item);

                gtk_widget_show_all(context_menu);

                gtk_menu_popup_at_pointer(GTK_MENU(context_menu), NULL);
            }
        }
        gtk_tree_path_free(path);
    }
}

void on_process_double_click(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {

    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(gtk_tree_view_get_model(treeview), &iter, path)) {

        guint process_id;
        gtk_tree_model_get(GTK_TREE_MODEL(user_data), &iter, 3, &process_id, -1);

        showProcessDetailsDialog(process_id);
    }
}

void read_process_name(int pid, char *name, size_t size) {
    char comm_path[MAX_PATH_LEN];
    snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

    FILE *comm_file = fopen(comm_path, "r");
    if (comm_file == NULL) {
        snprintf(name, size, "Unknown");
        return;
    }

    if (fgets(name, size, comm_file) != NULL) {
        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '\n') {
            name[len - 1] = '\0';
        }
    }

    fclose(comm_file);
}

void read_cpu_time(int pid, unsigned long *utime, unsigned long *stime) {
    char stat_path[MAX_PATH_LEN];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

    FILE *stat_file = fopen(stat_path, "r");
    if (stat_file == NULL) {
        *utime = 0;
        *stime = 0;
        return;
    }

    /* Read the contents of the stat file to get CPU time */

    fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %lu %lu", utime, stime);

    fclose(stat_file);
}

void read_uid_and_status(int pid, uid_t *uid, char *status, size_t size) {
    char status_path[MAX_PATH_LEN];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

    FILE *status_file = fopen(status_path, "r");
    if (status_file == NULL) {
        *uid = 0;
        snprintf(status, size, "Unknown");
        return;
    }

    /* Read the contents of the status file to get UID and Status */

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), status_file) != NULL) {
        if (strncmp(line, "Uid:", 4) == 0) {
            *uid = atoi(line + 5);
        }

        if (strncmp(line, "State:", 6) == 0) {
            sscanf(line + 6, "%s", status);
        }
    }

    fclose(status_file);
}

void read_memory_usage(int pid, gsize *memory_usage) {
    char status_path[MAX_PATH_LEN];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

    FILE *status_file = fopen(status_path, "r");
    if (status_file == NULL) {
        *memory_usage = 0;
        return;
    }

    /* Read the contents of the status file to get Memory Usage */

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), status_file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%lu", memory_usage);
            *memory_usage *= 1024;
        }
    }

    fclose(status_file);
}


void populate_process_table(GtkListStore *list_store) {

    if (!GTK_IS_LIST_STORE(list_store)) {
    g_printerr("Error: Invalid GtkListStore\n");
    return;
}
    gtk_list_store_clear(list_store);

    uid_t current_uid = getuid();

    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Error opening /proc directory");
        exit(EXIT_FAILURE);
    }

    /* Read the contents of the /proc directory */

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            int pid = atoi(entry->d_name);

            char comm_path[256];
            snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

            FILE *comm_file = fopen(comm_path, "r");
            if (comm_file == NULL) {
                continue;
            }

            /* Read the contents of the comm file to get the process name */

            char process_name[256];
            if (fgets(process_name, sizeof(process_name), comm_file) != NULL) {
                size_t len = strlen(process_name);
                if (len > 0 && process_name[len - 1] == '\n') {
                    process_name[len - 1] = '\0';
                }

                fclose(comm_file);

                char stat_path[256];
                snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

                FILE *stat_file = fopen(stat_path, "r");
                if (stat_file == NULL) {
                    continue;
                }

                /* Read the contents of the stat file to get CPU time */

                unsigned long utime, stime;
                if (fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %lu %lu", &utime, &stime) != 2) {
                    fclose(stat_file);
                    continue;
                }

                fclose(stat_file);

                unsigned long total_time = utime + stime;
                double seconds = sysconf(_SC_CLK_TCK); // Clock ticks per second
                double cpu_percentage = (total_time / seconds) * 100;

                char status_path[256];
                snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

                FILE *status_file = fopen(status_path, "r");
                if (status_file == NULL) {
                    continue;
                }

                /* Read the contents of the status file to get the UID and Status */

                char line[256];
                uid_t process_uid = 0;
                char process_status[256] = "";
                double memory_usage = 0;
                unsigned long long rss_memory = 0;
                while (fgets(line, sizeof(line), status_file) != NULL) {
                    if (strncmp(line, "Uid:", 4) == 0) {
                        process_uid = atoi(line + 5);
                    }

                    if (strncmp(line, "State:", 6) == 0) {
                        sscanf(line + 6, "%s", process_status);
                    }

                    if (strncmp(line, "VmRSS:", 6) == 0) {
                        sscanf(line, "%*s %llu kB", &rss_memory);
                        memory_usage = ((double)rss_memory)/1024;
                    }
                }

                bool can_add = false;
                if (process_type == ALL_PROCESSES ||
                    (process_type == USER_PROCESSES && process_uid == current_uid)) {
                        can_add = true;


                } else if (process_type == ACTIVE_PROCESSES) {
                    if (strcmp(process_status, "R") == 0) {
                        can_add = true;
                    }
                }

                if(can_add) {
                    gchar *status = g_strdup_printf("%s", process_status);
                    gchar *mem_string_mb = g_strdup_printf("%.2f MB\n", memory_usage);
                    guint process_id = pid;

                    GtkTreeIter iter;
                    gtk_list_store_append(list_store, &iter);
                    gtk_list_store_set(list_store, &iter,
                        0, process_name,
                        1, status,
                        2, cpu_percentage,
                        3, process_id,
                        4, mem_string_mb,
                        -1);

                    g_free(status);
                }

                fclose(status_file);
            }
        }
    }

    closedir(proc_dir);
}

void showProcessDetailsDialog(int pid) {
    char stat_path[256];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

    FILE *stat_file = fopen(stat_path, "r");
    if (stat_file == NULL) {
        perror("Error opening stat file");
        return;
    }

    /* Read the contents of the stat file to get process information */

    int process_pid;
    char process_name[256];
    char state;
    int ppid, pgrp, session, tty_nr;
    unsigned long long utime, stime, vsize, rss;
    unsigned long starttime;
    int matches = fscanf(stat_file, "%d (%255[^)]) %c %d %d %d %d %*d %*d %*d %*d %*d %llu %llu %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %llu",
                        &process_pid, process_name, &state, &ppid, &pgrp, &session, &tty_nr, &utime, &stime, &vsize, &rss, &starttime);


    fclose(stat_file);

    if (matches != 12) {
        perror("Error reading stat file");
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Process Details", NULL, 0, "OK", GTK_RESPONSE_OK, NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 200);

    /* Create labels to display process information */
    GtkWidget *nameLabel = gtk_label_new(g_strdup_printf("Name: %s", process_name));
    GtkWidget *pidLabel = gtk_label_new(g_strdup_printf("PID: %d", process_pid));
    GtkWidget *stateLabel = gtk_label_new(g_strdup_printf("State: %c", state));
    GtkWidget *memoryLabel = gtk_label_new(g_strdup_printf("Memory: %llu KB", rss));
    GtkWidget *vMemoryLabel = gtk_label_new(g_strdup_printf("Virtual Memory: %llu KB", vsize));
    GtkWidget *cpuTimeLabel = gtk_label_new(g_strdup_printf("CPU Time: %llu:%llu", utime, stime));
    GtkWidget *startTimeLabel = gtk_label_new(g_strdup_printf("Start Time: %lu", starttime));

    /* Create a vertical box to organize labels */

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);

    /* Add labels to the vertical box */
    gtk_box_pack_start(GTK_BOX(vbox), nameLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), pidLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), stateLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), memoryLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), vMemoryLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cpuTimeLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), startTimeLabel, FALSE, FALSE, 0);

    GtkWidget *closeButton = gtk_button_new_with_label("Close");
    g_signal_connect_swapped(closeButton, "clicked", G_CALLBACK(gtk_widget_destroy), dialog);

    GtkWidget *contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(contentArea), closeButton);

    gtk_widget_show_all(dialog);
}

