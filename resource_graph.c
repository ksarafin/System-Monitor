#include "resource_graph.h"
#include "ui.h"
#include "util.h"

GraphData cpu_graph_data = {NULL, "Time", "CPU Usage (%)"}; // Set default labels for CPU graph
GraphData memory_graph_data = {NULL, "Time", "Memory Usage (%)"}; // Set default labels for Memory graph
GraphData network_graph_data = {NULL, "Time", "Network Usage (MB)"}; // Set default labels for Network graph


// Function to update total bytes read and written
/*void update_network_stats(unsigned long long *total_rx_bytes, unsigned long long *total_tx_bytes) {
    FILE *fp;
    char line[256];
    char iface[MAX_IFACE_NAME];
    unsigned long long rx_bytes, tx_bytes;

    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("Error opening /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    // Read and ignore the first two lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    // Read network statistics for each interface
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %llu %*s %*s %*s %*s %*s %*s %*s %llu",
               iface, &rx_bytes, &tx_bytes);

        // Update total bytes read and written
        *total_rx_bytes += rx_bytes;
        *total_tx_bytes += tx_bytes;
    }

    fclose(fp);
}*/

gboolean update_cpu_data(gpointer user_data) {
    // Update CPU usage
    double cpu_usage = get_cpu_usage();

    // Update the CPU graph data
    cpu_graph_data.history = g_list_prepend(cpu_graph_data.history, GINT_TO_POINTER((int)cpu_usage));

    // Trim the history if it exceeds the maximum number of data points
    if (g_list_length(cpu_graph_data.history) > MAX_HISTORY) {
        cpu_graph_data.history = g_list_delete_link(cpu_graph_data.history, g_list_last(cpu_graph_data.history));
    }
    // Refresh graphs
    gtk_widget_queue_draw(user_data);

    return G_SOURCE_CONTINUE;
}

gboolean update_memory_data(gpointer user_data) {
    // Update Memory and Swap usage
    double memory_usage = get_memory_usage();
    
    // Update the Memory graph data
    memory_graph_data.history = g_list_prepend(memory_graph_data.history, GINT_TO_POINTER((int)memory_usage));

    // Trim the history if it exceeds the maximum number of data points
    if (g_list_length(memory_graph_data.history) > MAX_HISTORY) {
        memory_graph_data.history = g_list_delete_link(memory_graph_data.history, g_list_last(memory_graph_data.history));
    }
    // Refresh graphs
    gtk_widget_queue_draw(user_data);
    return G_SOURCE_CONTINUE;
}

gboolean update_network_data(gpointer user_data) {

    // Update Network usage (dummy value, replace with actual logic)
    unsigned long long total_rx = 0, total_tx = 0;

    // Call the function to update network statistics
    //update_network_stats(&total_rx, &total_tx);
    unsigned long long  network_usage = get_network_usage();

    // Update the Network graph data
    network_graph_data.history = g_list_prepend(network_graph_data.history, GINT_TO_POINTER((int)network_usage));

    // Trim the history if it exceeds the maximum number of data points
    if (g_list_length(network_graph_data.history) > MAX_HISTORY) {
        network_graph_data.history = g_list_delete_link(network_graph_data.history, g_list_last(network_graph_data.history));
    }

    // Refresh graphs
    gtk_widget_queue_draw(user_data);

    return G_SOURCE_CONTINUE;
}

GtkWidget *create_usage_page() {
    GtkWidget *tab_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_drawing_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(cpu_drawing_area), "draw", G_CALLBACK(draw_graph), &cpu_graph_data);
    gtk_widget_set_size_request(cpu_drawing_area, 800, 300);

    GtkWidget *memory_label = gtk_label_new("Memory");
    GtkWidget *memory_drawing_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(memory_drawing_area), "draw", G_CALLBACK(draw_graph), &memory_graph_data);
    gtk_widget_set_size_request(memory_drawing_area, 200, 300);

    GtkWidget *network_label = gtk_label_new("Network");
    GtkWidget *network_drawing_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(network_drawing_area), "draw", G_CALLBACK(draw_graph), &network_graph_data);
    gtk_widget_set_size_request(network_drawing_area, 200, 300);

    gtk_box_pack_start(GTK_BOX(tab_container), cpu_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_container), cpu_drawing_area, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_container), memory_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_container), memory_drawing_area, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_container), network_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_container), network_drawing_area, FALSE, FALSE, 5);

    // Initialize the graph_data structures
    cpu_graph_data.history = NULL;
    memory_graph_data.history = NULL;
    network_graph_data.history = NULL;

    // Set up periodic update of usage data
    g_timeout_add(UPDATE_INTERVAL, update_cpu_data, cpu_drawing_area);
    g_timeout_add(UPDATE_INTERVAL, update_memory_data, memory_drawing_area);
    g_timeout_add(UPDATE_INTERVAL, update_network_data, network_drawing_area);

    return tab_container;
}
