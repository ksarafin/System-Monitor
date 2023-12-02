#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

// Function to get available disk space
unsigned long get_available_disk_space() {
    FILE *fp = popen("df / --output=avail | tail -n 1", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    unsigned long available_space;
    fscanf(fp, "%lu", &available_space);

    // Close the pipe
    pclose(fp);

    return available_space;
}

// Function to get basic system information
void get_system_info(char* info_buffer, int buffer_size) {
    struct utsname system_info;
    uname(&system_info);

    struct sysinfo info;
    sysinfo(&info);

    unsigned long available_space = get_available_disk_space();

    snprintf(info_buffer, buffer_size,
             "<b>System Information</b>\n\n"
             "<b>OS Release:</b> %s\n"
             "<b>Kernel Version:</b> %s\n"
             "<b>Memory:</b> %lu MB\n"
             "<b>Processor:</b> %s\n"
             "<b>Available Disk Space:</b> %lu MB",
             system_info.release,
             system_info.version,
             (info.totalram / 1024) / 1024,
             system_info.machine,
             available_space / 1024);
}

// Function to get CPU information
void get_cpu_info(char* info_buffer, int buffer_size) {
    FILE *fp = popen("lscpu", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    // Read the output of the command
    fgets(info_buffer, buffer_size, fp);

    // Close the pipe
    pclose(fp);
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
    GtkWidget *resource_label = gtk_label_new("Resources");
    GtkWidget *file_system_label = gtk_label_new("File Systems");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Processes"), process_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Resources"), resource_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("File Systems"), file_system_label);

    // Connect the "switch-page" signal to the callback
    //g_signal_connect(notebook, "switch-page", G_CALLBACK(on_switch_page), window);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    gtk_widget_destroy(window);

    return 0;
}
