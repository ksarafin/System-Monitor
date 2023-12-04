#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <sys/vfs.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>

#include "system_info.h"
#include "file_system_table.h"
#include "process_table.h"
#include "util.h"
#include "ui.h"
#include "resource_graph.h"

#define MAX_IFACE_NAME 16
GtkWidget *main_window = NULL;

typedef struct {
    unsigned long long user, nice, system, idle;
} CPUStats;

gboolean show_all_processes = FALSE;

GtkWidget *view_all_processes_item;
GtkWidget *view_user_processes_item;
GtkWidget *view_active_processes_item;
extern GtkListStore *processListStore;

GtkWidget *createBasicInfoPage();

ProcessListType process_type = ALL_PROCESSES;

void on_view_option_selected(GtkCheckMenuItem *menu_item, gpointer user_data) {
    if (menu_item == GTK_CHECK_MENU_ITEM(view_all_processes_item)) {
        process_type = ALL_PROCESSES;
    } else if (menu_item == GTK_CHECK_MENU_ITEM(view_user_processes_item)) {
        process_type = USER_PROCESSES;
    } else if (menu_item == GTK_CHECK_MENU_ITEM(view_active_processes_item)) {
        process_type = ACTIVE_PROCESSES;
    }

    populate_process_table(GTK_LIST_STORE(user_data));
}

void destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}

gboolean refresh_process_list(gpointer user_data) {
    GtkListStore *list_store = GTK_LIST_STORE(user_data);

    /* Repopulate the process table with the updated filter */

    populate_process_table(list_store);
    return TRUE;
}

int main(int argc, char *argv[]) {

  gtk_init(&argc, &argv);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "System Monitor");
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 1100);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  main_window = window;

  /* Create a vertical box to organize widgets */

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  /* Create a menu bar */

  GtkWidget *menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

  view_all_processes_item = gtk_radio_menu_item_new_with_label(NULL, "View All Processes");
  view_user_processes_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(view_all_processes_item), "View User Processes");
  view_active_processes_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(view_all_processes_item), "View Active Processes");

  GtkWidget *view_menu = gtk_menu_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_all_processes_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_user_processes_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_active_processes_item);

  GtkWidget *view_menu_item = gtk_menu_item_new_with_label("View");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_item), view_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view_menu_item);

  /* Create a notebook for tabs */

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
      G_TYPE_STRING);

  /* Create the process table view */

  GtkWidget *process_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(process_list_store));

  /* Add columns to the process table */

  for (int i = 0; i < NUM_COLUMNS; ++i) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
        column_names[i], renderer, "text", i, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(process_treeview), column);
  }

  /* Populate the process table with data */

  populate_process_table(process_list_store);

  /* Create a scrolled window for the process table */

  GtkWidget *process_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(process_scrolled_window),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(process_scrolled_window), 200);
  gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(process_scrolled_window), 400);

  gtk_container_add(GTK_CONTAINER(process_scrolled_window), process_treeview);

  gint process_notebook_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), process_scrolled_window, process_label);

  g_signal_connect(process_treeview, "button-press-event", G_CALLBACK(on_process_right_click), process_list_store);
  g_signal_connect(process_treeview, "row-activated", G_CALLBACK(on_process_double_click), process_list_store);


  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(process_scrolled_window), "Processes");
  GtkWidget *usage_page = create_usage_page();
  GtkWidget *resource_label = gtk_label_new("Resources");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_usage_page(), resource_label);

  GtkWidget *file_system_label = gtk_label_new("File Systems");

  GtkListStore *file_system_list_store = gtk_list_store_new(FILE_SYSTEM_NUM_COLUMNS,
      G_TYPE_STRING,
      G_TYPE_STRING,
      G_TYPE_STRING,
      G_TYPE_UINT64,
      G_TYPE_UINT64,
      G_TYPE_UINT64,
      G_TYPE_UINT64);

  /* Create the file systems table view */

  GtkWidget *file_system_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(file_system_list_store));

  /* Add columns to the file systems table */

  for (int i = 0; i < FILE_SYSTEM_NUM_COLUMNS; ++i) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
        file_system_column_names[i], renderer, "text", i, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_system_treeview), column);
  }

  /* Populate the file systems table with data */

  populate_file_system_table(file_system_list_store);

  /* Create a scrolled window for the file systems table */

  GtkWidget *file_system_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_system_scrolled_window),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(file_system_scrolled_window), 200);
  gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(file_system_scrolled_window), 400);

  gtk_container_add(GTK_CONTAINER(file_system_scrolled_window), file_system_treeview);

  gint file_system_notebook_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), file_system_scrolled_window, file_system_label);

  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(file_system_scrolled_window), "File Systems");

  g_signal_connect(view_all_processes_item, "toggled", G_CALLBACK(on_view_option_selected), process_list_store);

  g_signal_connect(view_user_processes_item, "toggled", G_CALLBACK(on_view_option_selected), process_list_store);

  gtk_widget_show_all(window);

  g_timeout_add_seconds(15, refresh_process_list, process_list_store);

  gtk_main();

  gtk_widget_destroy(window);

  return 0;
}