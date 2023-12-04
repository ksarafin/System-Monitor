gcc $(pkg-config --cflags gtk+-3.0) system_monitor.c util.c ui.c system_info.c process_table.c file_system_table.c resource_graph.c $(pkg-config --libs gtk+-3.0)

