#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include <gtk/gtk.h>
#include <stdbool.h>


#define NUM_COLUMNS 5

extern const gchar *column_names[NUM_COLUMNS];

void populate_process_table(GtkListStore *list_store);

#endif // PROCESS_TABLE_H
