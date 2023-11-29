#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

// Function to get available disk space
unsigned long getAvailableDiskSpace() {
    FILE *fp = popen("df / --output=avail | tail -n 1", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    unsigned long availableSpace;
    fscanf(fp, "%lu", &availableSpace);

    // Close the pipe
    pclose(fp);

    return availableSpace;
}

// Function to get basic system information
void getSystemInfo(char* infoBuffer, int bufferSize) {
    struct utsname systemInfo;
    uname(&systemInfo);

    struct sysinfo info;
    sysinfo(&info);

    unsigned long availableSpace = getAvailableDiskSpace();

    snprintf(infoBuffer, bufferSize,
             "<b>OS Release:</b> %s\n"
             "<b>Kernel Version:</b> %s\n"
             "<b>Memory:</b> %lu MB\n"
             "<b>Processor:</b> %s\n"
             "<b>Available Disk Space:</b> %lu MB",
             systemInfo.release,
             systemInfo.version,
             (info.totalram / 1024) / 1024,
             systemInfo.machine,
             availableSpace / 1024);
}

// Function to get CPU information
void getCPUInfo(char* infoBuffer, int bufferSize) {
    FILE *fp = popen("lscpu", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    // Read the output of the command
    fgets(infoBuffer, bufferSize, fp);

    // Close the pipe
    pclose(fp);
}

// Callback function for the "Destroy" signal of the main window
void onDestroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "System Monitor");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 900);

    // Set up signal handler for destroying the window
    g_signal_connect(window, "destroy", G_CALLBACK(onDestroy), NULL);

    // Set up frame

    GtkWidget *frame = gtk_frame_new("");
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

    // Create a notebook to hold different tabs
    /*GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);*/

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(window), paned);

    // Create a notebook to hold different tabs
    GtkWidget *notebook = gtk_notebook_new();
    gtk_paned_pack1(GTK_PANED(paned), notebook, TRUE, TRUE);

    // Create and add the first tab (Basic System Information)
    GtkWidget *tab1Label = gtk_label_new("System");
    GtkWidget *tab1Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(notebook), tab1Box);

    // Add header for system information
    GtkWidget *systemInfoLabel = gtk_label_new("System Information");
    gtk_box_pack_start(GTK_BOX(tab1Box), systemInfoLabel, FALSE, FALSE, 5);

    char basicInfoBuffer[1024];
    getSystemInfo(basicInfoBuffer, sizeof(basicInfoBuffer));
    //GtkWidget *basicInfoLabel = gtk_label_new(basicInfoBuffer);
    GtkWidget *basicInfoLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(basicInfoLabel), basicInfoBuffer);
    gtk_box_pack_start(GTK_BOX(tab1Box), basicInfoLabel, FALSE, FALSE, 0);

    // Create and add the second tab (CPU Information)
    GtkWidget *tab2Label = gtk_label_new("CPU Info");
    GtkWidget *tab2Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(notebook), tab2Box);

    // Add header for CPU information
    GtkWidget *cpuInfoLabel = gtk_label_new("CPU Information");
    gtk_box_pack_start(GTK_BOX(tab2Box), cpuInfoLabel, FALSE, FALSE, 5);

    char cpuInfoBuffer[1024];
    getCPUInfo(cpuInfoBuffer, sizeof(cpuInfoBuffer));
    GtkWidget *cpuInfoLabelContent = gtk_label_new(cpuInfoBuffer);
    gtk_box_pack_start(GTK_BOX(tab2Box), cpuInfoLabelContent, FALSE, FALSE, 0);

    // Set up process menu bar

    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *fileMenu = gtk_menu_new();

    GtkWidget *fileMi = gtk_menu_item_new_with_label("File");
    GtkWidget *quitMi = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
    gtk_box_pack_start(GTK_WINDOW(window), menubar, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(window);

    // Run the GTK main loop
    gtk_main();

    return 0;
}
