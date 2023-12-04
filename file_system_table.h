// file_system_table.h
#ifndef FILE_SYSTEM_TABLE_H
#define FILE_SYSTEM_TABLE_H

#include <gtk/gtk.h>

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

extern const gchar *file_system_column_names[FILE_SYSTEM_NUM_COLUMNS];

void populate_file_system_table(GtkListStore *list_store);

#endif // FILE_SYSTEM_TABLE_H
