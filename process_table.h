#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include <gtk/gtk.h>
#include <stdbool.h>


#define NUM_COLUMNS 5

typedef enum {
    ALL_PROCESSES,
    USER_PROCESSES,
    ACTIVE_PROCESSES
} ProcessListType;
extern ProcessListType process_type;

extern const gchar *column_names[NUM_COLUMNS];

void populate_process_table(GtkListStore *list_store);
GtkWidget *createProcessesPage();
void showProcessDetailsDialog(int pid);
void on_process_right_click(GtkTreeView *treeview, GdkEventButton *event, gpointer user_data);
void on_process_double_click(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

#endif
