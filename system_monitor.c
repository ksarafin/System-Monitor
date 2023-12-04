#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <sys/vfs.h>

#include "process_table.h"

// Define the number of columns in the table
#define FILE_SYSTEM_NUM_COLUMNS 7

typedef struct {
  gchar *device;
  gchar *directory;
  gchar *type;
  guint64 total_space;
  guint64 free_space;
  guint64 available_space;
  guint64 used_space;
} FileSystemData;

const gchar *file_system_column_names[FILE_SYSTEM_NUM_COLUMNS] = {
  "Device",
  "Directory",
  "Type",
  "Total",
  "Free",
  "Available",
  "Used"
};

/*void populate_file_system_table(GtkListStore *list_store) {
  // Replace this with your actual file system data retrieval logic
  // For now, let's use some dummy data
  for (int i = 0; i < 5; ++i) {
    FileSystemData data = {
      .device = g_strdup_printf("/dev/sd%d", i),
      .directory = g_strdup_printf("/mnt/fs%d", i),
      .type = g_strdup("ext4"),
      .free_space = 1024 * i,
      .available_space = 512 * i,
      .used_space = 512 * i
    };

    GtkTreeIter iter;
    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter,
        0, data.device,
        1, data.directory,
        2, data.type,
        3, data.free_space,
        4, data.available_space,
        5, data.used_space,
        -1);

    g_free(data.device);
    g_free(data.directory);
    g_free(data.type);
  }
}*/


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
            gchar *device_name = g_strdup(tokens[4]);

            struct statfs sb;

            statfs(tokens[4],&sb);

            if (device_name != NULL) {
                FileSystemData data = {
                    .device = device_name,
                    .directory = g_strdup(tokens[3]),
                    .type = strdup("IDK"),
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

void on_process_right_click(GtkTreeView *treeview, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
        // Get the path at the click position
        gint x, y;
        GtkTreePath *path;
        GtkTreeViewColumn *column;
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), event->x, event->y, &path, &column, &x, &y)) {
            // Convert the path to an integer index
            gint *indices = gtk_tree_path_get_indices(path);
            gint index = indices[0];

            // Create the context menu
            GtkWidget *context_menu = gtk_menu_new();

            // Add menu items for the context menu
            GtkWidget *kill_item = gtk_menu_item_new_with_label("Stop Process");
            GtkWidget *pause_item = gtk_menu_item_new_with_label("Continue Process");
            GtkWidget *continue_item = gtk_menu_item_new_with_label("Kill Process");
            GtkWidget *open_file_location_item = gtk_menu_item_new_with_label("Memory Maps");
            GtkWidget *properties_item = gtk_menu_item_new_with_label("Open Files");

            // Append menu items to the context menu
            gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), kill_item);
            gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), pause_item);
            gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), continue_item);
            gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), open_file_location_item);
            gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), properties_item);

            // Show the context menu
            gtk_widget_show_all(context_menu);

            // Popup the context menu at the click position
            gtk_menu_popup_at_pointer(GTK_MENU(context_menu), NULL);
        }
        gtk_tree_path_free(path);
    }
}

// Callback function for the "Destroy" signal of the main window
void destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}


// Callback function for handling notebook page switch
/*void on_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
  GtkWidget *window = GTK_WIDGET(user_data);

  // Get the size requisition of the current page
  GtkRequisition requisition;
  gtk_widget_get_preferred_size(page, NULL, &requisition);

  // Set the window size based on the size requisition
  gtk_window_resize(GTK_WINDOW(window), requisition.width, requisition.height);
}*/

int main(int argc, char *argv[]) {

  gtk_init(&argc, &argv);

  // Create the main window
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "System Monitor");
  gtk_window_set_default_size(GTK_WINDOW(window), 700, 900);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // Create a vertical box to organize widgets
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  // Create a menu bar
  GtkWidget *menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

  GtkWidget *view_menu = gtk_menu_new();
  GtkWidget *view_all_processes_item = gtk_radio_menu_item_new_with_label(NULL, "View All Processes");
  GtkWidget *view_user_processes_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(view_all_processes_item), "View User Processes");

  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_all_processes_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_user_processes_item);

  GtkWidget *view_menu_item = gtk_menu_item_new_with_label("View");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_item), view_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view_menu_item);


  // Create a notebook for tabs
  GtkWidget *notebook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

  GtkWidget *system_label = gtk_label_new("System");

  char basicinfo_buffer[1024];
  get_system_info(basicinfo_buffer, sizeof(basicinfo_buffer));
  GtkWidget *basicInfoLabel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(basicInfoLabel), basicinfo_buffer);

  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), basicInfoLabel, system_label);

  GtkWidget *process_label = gtk_label_new("Processes");

  GtkListStore *process_list_store = gtk_list_store_new(NUM_COLUMNS,
      G_TYPE_STRING,
      G_TYPE_STRING,
      G_TYPE_DOUBLE,
      G_TYPE_UINT,
      G_TYPE_UINT64);

  // Create the process table view
  GtkWidget *process_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(process_list_store));

  // Add columns to the process table
  for (int i = 0; i < NUM_COLUMNS; ++i) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
        column_names[i], renderer, "text", i, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_treeview), column);
  }

  // Populate the process table with data (replace this with your actual data)
  populate_process_table(process_list_store);

  // Create a scrolled window for the process table
  GtkWidget *process_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(process_scrolled_window),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(process_scrolled_window), 200);
  gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(process_scrolled_window), 400);

  // Add the process table to the scrolled window
  gtk_container_add(GTK_CONTAINER(process_scrolled_window), process_treeview);

  gint process_notebook_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), process_scrolled_window, process_label);

  g_signal_connect(process_treeview, "button-press-event", G_CALLBACK(on_process_right_click), NULL);

  // Set the label for the "Processes" notebook page
  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(process_scrolled_window), "Processes");

  GtkWidget *resource_label = gtk_label_new("Resources");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Resources"), resource_label);

  GtkWidget *file_system_label = gtk_label_new("File Systems");

  GtkListStore *file_system_list_store = gtk_list_store_new(FILE_SYSTEM_NUM_COLUMNS,
      G_TYPE_STRING,
      G_TYPE_STRING,
      G_TYPE_STRING,
      G_TYPE_UINT64,
      G_TYPE_UINT64,
      G_TYPE_UINT64,
      G_TYPE_UINT64);

  // Create the file systems table view
  GtkWidget *file_system_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(file_system_list_store));

  // Add columns to the file systems table
  for (int i = 0; i < FILE_SYSTEM_NUM_COLUMNS; ++i) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
        file_system_column_names[i], renderer, "text", i, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_system_treeview), column);
  }

  // Populate the file systems table with data (replace this with your actual data)
  populate_file_system_table(file_system_list_store);

  // Create a scrolled window for the file systems table
  GtkWidget *file_system_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_system_scrolled_window),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(file_system_scrolled_window), 200);
  gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(file_system_scrolled_window), 400);

  // Add the file systems table to the scrolled window
  gtk_container_add(GTK_CONTAINER(file_system_scrolled_window), file_system_treeview);

  // Append the file systems table to the notebook
  gint file_system_notebook_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), file_system_scrolled_window, file_system_label);

  // Set the label for the "File Systems" notebook page
  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(file_system_scrolled_window), "File Systems");

  // Connect the "switch-page" signal to the callback
  //g_signal_connect(notebook, "switch-page", G_CALLBACK(on_switch_page), window);

  // Show all widgets
  gtk_widget_show_all(window);

  // Start the GTK main loop
  gtk_main();

  gtk_widget_destroy(window);

  return 0;
}
