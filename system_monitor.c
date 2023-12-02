#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

// Define the number of columns in the table
#define NUM_COLUMNS 5
#define FILE_SYSTEM_NUM_COLUMNS 6

const gchar *column_names[NUM_COLUMNS] = {
  "Process Name",
  "Status",
  "%CPU",
  "ID",
  "Memory"
};

const gchar *file_system_column_names[FILE_SYSTEM_NUM_COLUMNS] = {
  "Device",
  "Directory",
  "Type",
  "Free",
  "Available",
  "Used"
};

typedef struct {
  gchar *device;
  gchar *directory;
  gchar *type;
  guint64 free_space;
  guint64 available_space;
  guint64 used_space;
} FileSystemData;

void populate_file_system_table(GtkListStore *list_store) {
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
}

// Callback function for the "Destroy" signal of the main window
void destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}

// Callback function for handling menu item clicks
void menu_item_clicked(GtkWidget *widget, gpointer data) {
  g_print("Menu item clicked: %s\n", (char *)data);
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

void populate_process_table(GtkListStore *list_store) {
  // You should replace this with your actual process data retrieval logic
  // For now, let's use some dummy data
  for (int i = 0; i < 10; ++i) {
    gchar *process_name = g_strdup_printf("Process %d", i);
    gchar *status = g_strdup_printf("Running");
    gdouble cpu_percentage = 10.0 * i;
    guint process_id = 1000 + i;
    gsize memory_usage = 512 * (i + 1);

    GtkTreeIter iter;
    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter, 
        0, process_name,
        1, status,
        2, cpu_percentage,
        3, process_id,
        4, memory_usage,
        -1);

    g_free(process_name);
    g_free(status);
  }
}


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

  // Create the File menu
  GtkWidget *file_menu = gtk_menu_new();

  // Create menu items for the File menu
  GtkWidget *file_menu_item1 = gtk_menu_item_new_with_label("New");
  GtkWidget *file_menu_item2 = gtk_menu_item_new_with_label("Open");
  GtkWidget *file_menu_item3 = gtk_menu_item_new_with_label("Exit");

  // Connect menu item signals
  g_signal_connect(file_menu_item1, "activate", G_CALLBACK(menu_item_clicked), "New");
  g_signal_connect(file_menu_item2, "activate", G_CALLBACK(menu_item_clicked), "Open");
  g_signal_connect(file_menu_item3, "activate", G_CALLBACK(gtk_main_quit), NULL);

  // Add menu items to the File menu
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_menu_item1);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_menu_item2);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_menu_item3);

  // Create the File menu item for the menu bar
  GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);

  // Add the File menu item to the menu bar
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);

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

  // Set the label for the "Processes" notebook page
  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(process_scrolled_window), "Processes");

  GtkWidget *resource_label = gtk_label_new("Resources");
  GtkWidget *file_system_label = gtk_label_new("File Systems");

  GtkListStore *file_system_list_store = gtk_list_store_new(FILE_SYSTEM_NUM_COLUMNS,
      G_TYPE_STRING, 
      G_TYPE_STRING, 
      G_TYPE_STRING,  
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
  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Resources"), resource_label);

  // Connect the "switch-page" signal to the callback
  //g_signal_connect(notebook, "switch-page", G_CALLBACK(on_switch_page), window);

  // Show all widgets
  gtk_widget_show_all(window);

  // Start the GTK main loop
  gtk_main();

  gtk_widget_destroy(window);

  return 0;
}
